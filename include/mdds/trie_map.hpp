/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2015-2020 Kohei Yoshida
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************/

#ifndef INCLUDED_MDDS_TRIE_MAP_HPP
#define INCLUDED_MDDS_TRIE_MAP_HPP

#include "trie_map_itr.hpp"

#include <vector>
#include <string>
#include <deque>
#include <map>
#include <memory>
#include <limits>

namespace mdds {

namespace trie {

namespace detail {

struct copy_to_pack
{
};

struct move_to_pack
{
};

template<typename TrieT, typename PackedT>
struct dump_packed_buffer;

} // namespace detail

/** Serializer for numeric data types. */
template<typename T>
struct numeric_value_serializer
{
    static constexpr bool variable_size = false;

    static constexpr size_t value_size = sizeof(T);

    static void write(std::ostream& os, const T& v);

    static void read(std::istream& is, size_t n, T& v);
};

/** Serializer for variable-size data types. */
template<typename T>
struct variable_value_serializer
{
    static constexpr bool variable_size = true;

    static void write(std::ostream& os, const T& v);

    static void read(std::istream& is, size_t n, T& v);
};

/**
 * Serializer for standard sequence container whose value type is of
 * numeric value type.
 */
template<typename T>
struct numeric_sequence_value_serializer
{
    using element_serializer = numeric_value_serializer<typename T::value_type>;

    static constexpr bool variable_size = true;

    static void write(std::ostream& os, const T& v);

    static void read(std::istream& is, size_t n, T& v);
};

/**
 * Default value serializer for mdds::packed_trie_map's load_state and
 * save_state methods.  The primary template is used for numeric value
 * types, and template specializations exist for std::string, as
 * well as sequence containers, such as std::vector, whose elements are of
 * numeric types.
 */
template<typename T, typename U = void>
struct value_serializer : numeric_value_serializer<T>
{
};

template<typename T>
struct value_serializer<T, typename std::enable_if<mdds::detail::has_value_type<T>::value>::type>
    : numeric_sequence_value_serializer<T>
{
};

template<>
struct value_serializer<std::string> : variable_value_serializer<std::string>
{
};

struct default_traits
{
    /**
     * Unit value type of a buffer used in packed_trie_map to store its content.
     * It must be an unsigned integral type.
     *
     * @note Maximum number of key-value pairs that can be stored in the
     *       packed_trie_map variant is the maximum value that can be expressed
     *       by this type minus one.  For instance, if the size of this type is
     *       8-bits, only up to 255 key-value pairs can be stored.
     */
    using pack_value_type = uintptr_t;
};

/**
 * Specifies the type of human-readable output to dump.
 */
enum class dump_structure_type
{
    /**
     * Dump the in-memory buffer that stores the trie content in a linear
     * fashion.
     */
    packed_buffer,

    /**
     * Dump the traversal result of the trie in depth-first order.
     */
    trie_traversal,
};

} // namespace trie

template<typename KeyT, typename ValueT, typename TraitsT>
class packed_trie_map;

/**
 * Trie map provides storage for multiple key-value pairs where keys are
 * either strings, or otherwise consist of arrays of characters.  The keys
 * are stored in an ordered tree structure known as trie, or sometimes
 * referred to as prefix tree.
 */
template<typename KeyT, typename ValueT, typename TraitsT = trie::default_traits>
class trie_map
{
    friend class packed_trie_map<KeyT, ValueT, TraitsT>;
    friend class trie::detail::iterator_base<trie_map, true>;
    friend class trie::detail::iterator_base<trie_map, false>;
    friend class trie::detail::const_iterator<trie_map>;
    friend class trie::detail::iterator<trie_map>;
    friend class trie::detail::search_results<trie_map>;
    friend trie::detail::get_node_stack_type<trie_map, std::true_type>;
    friend trie::detail::get_node_stack_type<trie_map, std::false_type>;

public:
    using traits_type = TraitsT;
    typedef packed_trie_map<KeyT, ValueT, TraitsT> packed_type;
    typedef KeyT key_type;
    typedef typename key_type::value_type key_unit_type;
    typedef ValueT value_type;
    typedef size_t size_type;

    using const_iterator = trie::detail::const_iterator<trie_map>;
    using iterator = trie::detail::iterator<trie_map>;
    typedef trie::detail::search_results<trie_map> search_results;

private:
    struct trie_node
    {
        typedef std::map<key_unit_type, trie_node> children_type;

        children_type children;
        value_type value;
        bool has_value;

        trie_node();
        trie_node(const trie_node& other);
        trie_node(trie_node&& other);

        void swap(trie_node& other);
    };

    template<bool IsConst>
    struct stack_item
    {
        using _is_const = std::bool_constant<IsConst>;

        using child_pos_type =
            typename mdds::detail::get_iterator_type<typename trie_node::children_type, _is_const>::type;

        using trie_node_type = typename mdds::detail::const_or_not<trie_node, _is_const>::type;

        trie_node_type* node;
        child_pos_type child_pos;

        stack_item(trie_node_type* _node, const child_pos_type& _child_pos) : node(_node), child_pos(_child_pos)
        {}

        bool operator==(const stack_item& r) const
        {
            return node == r.node && child_pos == r.child_pos;
        }

        bool operator!=(const stack_item& r) const
        {
            return !operator==(r);
        }
    };

    using const_node_stack_type = std::vector<stack_item<true>>;
    using node_stack_type = std::vector<stack_item<false>>;

public:
    /**
     * Represents an individual node of a trie.
     */
    class const_node_type
    {
        friend class trie_map;

        const trie_node* m_ref_node = nullptr;

        const_node_type(const trie_node* ref_node);

    public:
        const_node_type();
        const_node_type(const const_node_type& other);

        const_node_type& operator=(const const_node_type& other);

        /**
         * Query whether or not the node references an existing node in a tree.
         *
         * @return True if the node references an existing node in a tree,
         *         or false if the node does not reference any node in any tree.
         */
        bool valid() const;

        /**
         * Query whether or not the node has at least one child node.
         *
         * @return True if the node has at least one child node, or false if the
         *         node has no child nodes at all.
         */
        bool has_child() const;

        /**
         * Query whether or not the node has a value associated with it.
         *
         * @return True if the node has a value, otherwise false.
         */
        bool has_value() const;

        /**
         * Access the value associated with the node.
         *
         * @return Reference to the value associated with the node.
         *
         * @warning The caller must ensure that the node has a value via
         *          has_value() before calling this method to access it.
         *          Calling this method on a node without a value is undefined.
         */
        const value_type& value() const;

        /**
         * Move to a child node by a unit key.
         *
         * @param c A unit key associated with a child node relative to the
         *          current node.
         *
         * @return A valid node if a child node exists for the unit key passed
         *         to this method, otherwise an invalid node is returned.
         */
        const_node_type child(key_unit_type c) const;
    };

    /**
     * Default constructor.
     */
    trie_map();

    trie_map(const trie_map& other);

    trie_map(trie_map&& other);

    const_iterator begin() const;

    iterator begin();

    const_iterator end() const;

    iterator end();

    trie_map& operator=(trie_map other);

    void swap(trie_map& other);

    /**
     * Obtain a root node of the trie to traverse it node-by-node.
     *
     * @return Root node of the trie.
     */
    const_node_type root_node() const;

    /**
     * Insert a new key-value pair.
     *
     * @param key key value.
     * @param value value to associate with the key.
     */
    void insert(const key_type& key, value_type value);

    /**
     * Insert a new key-value pair.
     *
     * @param key pointer to the first character of a character array that
     *            stores key value.
     * @param len length of the character array storing the key.
     * @param value value to associate with the key.
     */
    void insert(const key_unit_type* key, size_type len, value_type value);

    /**
     * Erase a key and the value associated with it.
     *
     * @param key pointer to the first character of a character array that
     *            stores key value.
     * @param len length of the character array storing the key.
     *
     * @return true if a key is erased, false otherwise.
     */
    bool erase(const key_unit_type* key, size_type len);

    /**
     * Find a value associated with a specified key.
     *
     * @param key key to match.
     *
     * @return immutable iterator that references a value associated with the
     *         key, or the end position in case the key is not found.
     */
    const_iterator find(const key_type& key) const;

    /**
     * Find a value associated with a specified key.
     *
     * @param input pointer to an array whose value represents the key to
     *              match.
     * @param len length of the matching key value.
     *
     * @return immutable iterator that references a value associated with the
     *         key, or the end position in case the key is not found.
     */
    const_iterator find(const key_unit_type* input, size_type len) const;

    /**
     * Find a value associated with a specified key.
     *
     * @param key key to match.
     *
     * @return mutable iterator that references a value associated with the
     *         key, or the end position in case the key is not found.
     */
    iterator find(const key_type& key);

    /**
     * Find a value associated with a specified key.
     *
     * @param input pointer to an array whose value represents the key to
     *              match.
     * @param len length of the matching key value.
     *
     * @return mutable iterator that references a value associated with the
     *         key, or the end position in case the key is not found.
     */
    iterator find(const key_unit_type* input, size_type len);

    /**
     * Retrieve all key-value pairs whose keys start with specified prefix.
     * You can also retrieve all key-value pairs by passing a null prefix and
     * a length of zero.
     *
     * @param prefix prefix to match.
     *
     * @return results object that contains all matching key-value pairs. The
     *         results are sorted by the key in ascending order.
     */
    search_results prefix_search(const key_type& prefix) const;

    /**
     * Retrieve all key-value pairs whose keys start with specified prefix.
     * You can also retrieve all key-value pairs by passing a null prefix and
     * a length of zero.
     *
     * @param prefix pointer to an array whose value represents the prefix to
     *               match.
     * @param len length of the prefix value to match.
     *
     * @return results object that contains all matching key-value pairs. The
     *         results are sorted by the key in ascending order.
     */
    search_results prefix_search(const key_unit_type* prefix, size_type len) const;

    /**
     * Return the number of entries in the map.
     *
     * @return the number of entries in the map.
     */
    size_type size() const;

    bool empty() const noexcept;

    /**
     * Empty the container.
     */
    void clear();

    /**
     * Create a compressed and immutable version of the container which
     * provides better search performance and requires much less memory
     * footprint.
     *
     * @note Calling this method will move all stored values into the packed
     *       variant.  You should make a copy of the instance first if you need
     *       to preserve the original.
     *
     * @return an instance of mdds::packed_trie_map with the same content,
     *         with all the values stored in the original instance moved into
     *         the returned instance.
     *
     * @throws mdds::size_error When the number of entries exceeds maximum
     *                allowed number of key-value pairs.
     *                See trie::default_traits::pack_value_type for more
     *                detailed explanation.
     */
    packed_type pack();

private:
    void insert_into_tree(trie_node& node, const key_unit_type* key, const key_unit_type* key_end, value_type value);

    const trie_node* find_prefix_node(
        const trie_node& node, const key_unit_type* prefix, const key_unit_type* prefix_end) const;

    template<bool IsConst>
    void find_prefix_node_with_stack(
        std::vector<stack_item<IsConst>>& node_stack, mdds::detail::const_t<trie_node, IsConst>& node,
        const key_unit_type* prefix, const key_unit_type* prefix_end) const;

    template<bool IsConst>
    key_type build_key_buffer_from_node_stack(const std::vector<stack_item<IsConst>>& node_stack) const;

    void count_values(size_type& n, const trie_node& node) const;

private:
    trie_node m_root;
};

/**
 * An immutable trie container that packs its content into a contiguous
 * array to achieve both space efficiency and lookup performance.  The user
 * of this data structure must provide a pre-constructed list of key-value
 * entries that are sorted by the key in ascending order, or construct from
 * an instance of mdds::trie_map.
 *
 * Note that, since this container is immutable, the content of the
 * container cannot be modified once constructed.
 */
template<typename KeyT, typename ValueT, typename TraitsT = trie::default_traits>
class packed_trie_map
{
    using pack_value_type = typename TraitsT::pack_value_type;
    using packed_type = std::vector<pack_value_type>;

    friend class trie::detail::packed_iterator_base<packed_trie_map>;
    friend class trie::detail::packed_search_results<packed_trie_map>;
    friend class trie_map<KeyT, ValueT, TraitsT>;
    friend struct trie::detail::dump_packed_buffer<packed_trie_map, packed_type>;

public:
    using traits_type = TraitsT;
    typedef KeyT key_type;
    typedef typename key_type::value_type key_unit_type;
    typedef ValueT value_type;
    typedef size_t size_type;
    typedef trie::detail::packed_iterator_base<packed_trie_map> const_iterator;
    typedef trie::detail::packed_search_results<packed_trie_map> search_results;

    /**
     * Single key-value entry.  Caller must provide at compile time a static
     * array of these entries.
     */
    struct entry
    {
        const key_unit_type* key;
        size_type keylen;
        value_type value;

        entry(const key_unit_type* _key, size_type _keylen, value_type _value)
            : key(_key), keylen(_keylen), value(_value)
        {}
    };

private:
    using value_store_type = std::deque<value_type>;

    /** Special value representing a value-not-set. */
    static constexpr auto null_value = std::numeric_limits<pack_value_type>::max();
    /** Maximum allowed index for value. */
    static constexpr auto max_value_pos = null_value - 1u;

    struct trie_node
    {
        key_unit_type key;
        const value_type* value;

        std::deque<trie_node*> children;

        trie_node(key_unit_type _key) : key(_key), value(nullptr)
        {}
    };

    struct stack_item
    {
        const value_store_type* value_store = nullptr;

        const pack_value_type* node_pos = nullptr;
        const pack_value_type* child_pos = nullptr;
        const pack_value_type* child_end = nullptr;

        stack_item(
            const value_store_type* _value_store, const pack_value_type* _node_pos, const pack_value_type* _child_pos,
            const pack_value_type* _child_end)
            : value_store(_value_store), node_pos(_node_pos), child_pos(_child_pos), child_end(_child_end)
        {}

        bool operator==(const stack_item& other) const
        {
            return value_store == other.value_store && node_pos == other.node_pos && child_pos == other.child_pos &&
                   child_end == other.child_end;
        }

        bool operator!=(const stack_item& other) const
        {
            return !operator==(other);
        }

        bool has_value() const
        {
            return *node_pos != null_value;
        }

        pack_value_type get_value_pos() const
        {
            return *node_pos;
        }
    };

    typedef std::vector<stack_item> node_stack_type;
    typedef std::deque<trie_node> node_pool_type;
    typedef std::vector<std::tuple<size_t, key_unit_type>> child_offsets_type;

    packed_trie_map(trie::detail::move_to_pack, trie_map<KeyT, ValueT, TraitsT>& from);

public:
    /**
     * Represents an individual node of a trie.
     */
    class const_node_type
    {
        friend class packed_trie_map;

        const packed_type* m_packed = nullptr;
        const value_store_type* m_value_store = nullptr;
        const pack_value_type* m_pos = nullptr;

        const_node_type(const packed_type* packed, const value_store_type* value_store, const pack_value_type* p);

    public:
        const_node_type() = default;
        const_node_type(const const_node_type& other) = default;

        const_node_type& operator=(const const_node_type& other);

        /**
         * Query whether or not the node references an existing node in a tree.
         *
         * @return True if the node references an existing node in a tree,
         *         or false if the node does not reference any node in any tree.
         */
        bool valid() const;

        /**
         * Query whether or not the node has at least one child node.
         *
         * @return True if the node has at least one child node, or false if the
         *         node has no child nodes at all.
         */
        bool has_child() const;

        /**
         * Query whether or not the node has a value associated with it.
         *
         * @return True if the node has a value, otherwise false.
         */
        bool has_value() const;

        /**
         * Access the value associated with the node.
         *
         * @return Reference to the value associated with the node.
         *
         * @warning The caller must ensure that the node has a value via
         *          has_value() before calling this method to access it.
         *          Calling this method on a node without a value is undefined.
         */
        const value_type& value() const;

        /**
         * Move to a child node by a unit key.
         *
         * @param c A unit key associated with a child node relative to the
         *          current node.
         *
         * @return A valid node if a child node exists for the unit key passed
         *         to this method, otherwise an invalid node is returned.
         */
        const_node_type child(key_unit_type c) const;
    };

    packed_trie_map();

    /**
     * Constructor that initializes the content from a static list of
     * key-value entries.  The caller <em>must</em> ensure that the key-value
     * entries are sorted in ascending order, else the behavior is undefined.
     *
     * @param entries pointer to the array of key-value entries.
     * @param entry_size size of the key-value entry array.
     *
     * @throws mdds::size_error When the number of entries exceeds
     *                maximum allowed number of key-value pairs. See
     *                trie::default_traits::pack_value_type for more detailed
     *                explanation.
     */
    packed_trie_map(const entry* entries, size_type entry_size);

    /**
     * Constructor to allow construction of an instance from the content of a
     * mdds::trie_map instance.
     *
     * @param other mdds::trie_map instance to build content from.
     *
     * @throws mdds::size_error When the number of entries exceeds
     *                maximum allowed number of key-value pairs. See
     *                trie::default_traits::pack_value_type for more detailed
     *                explanation.
     */
    packed_trie_map(const trie_map<KeyT, ValueT, TraitsT>& other);

    packed_trie_map(const packed_trie_map& other);

    packed_trie_map(packed_trie_map&& other);

    packed_trie_map& operator=(packed_trie_map other);

    bool operator==(const packed_trie_map& other) const;

    bool operator!=(const packed_trie_map& other) const;

    const_iterator begin() const;

    const_iterator end() const;

    const_iterator cbegin() const;

    const_iterator cend() const;

    /**
     * Find a value associated with a specified key.
     *
     * @param key key to match.
     *
     * @return iterator that references a value associated with the key, or
     *         the end position in case the key is not found.
     */
    const_iterator find(const key_type& key) const;

    /**
     * Find a value associated with a specified key.
     *
     * @param input pointer to an array whose value represents the key to
     *              match.
     * @param len length of the matching key value.
     *
     * @return iterator that references a value associated with the key, or
     *         the end position in case the key is not found.
     */
    const_iterator find(const key_unit_type* input, size_type len) const;

    /**
     * Retrieve all key-value pairs whose keys start with specified prefix.
     * You can also retrieve all key-value pairs by passing a null prefix and
     * a length of zero.
     *
     * @param prefix prefix to match.
     *
     * @return results object containing all matching key-value pairs.
     */
    search_results prefix_search(const key_type& prefix) const;

    /**
     * Retrieve all key-value pairs whose keys start with specified prefix.
     * You can also retrieve all key-value pairs by passing a null prefix and
     * a length of zero.
     *
     * @param prefix pointer to an array whose value represents the prefix to
     *               match.
     * @param len length of the prefix value to match.
     *
     * @return results object that contains all matching key-value pairs. The
     *         results are sorted by the key in ascending order.
     */
    search_results prefix_search(const key_unit_type* prefix, size_type len) const;

    /**
     * Return the number of entries in the map.
     *
     * @return the number of entries in the map.
     */
    size_type size() const noexcept;

    bool empty() const noexcept;

    void swap(packed_trie_map& other);

    /**
     * Obtain a root node of the trie to traverse it node-by-node.
     *
     * @return Root node of the trie.
     */
    const_node_type root_node() const;

    /**
     * Save the state of the instance of this class to an external buffer.
     *
     * @param os output stream to write the state to.
     */
    template<typename FuncT = trie::value_serializer<value_type>>
    void save_state(std::ostream& os) const;

    /**
     * Restore the state of the instance of this class from an external
     * buffer.
     *
     * @param is input stream to load the state from.
     */
    template<typename FuncT = trie::value_serializer<value_type>>
    void load_state(std::istream& is);

    /**
     * Dump the structure of the trie content in a specified human-readable
     * textural format.
     *
     * @param type Output format type.
     */
    std::string dump_structure(trie::dump_structure_type type) const;

private:
    void dump_trie_traversal(std::ostream& os) const;

    node_stack_type get_root_stack() const;

    void traverse_range(
        trie_node& root, node_pool_type& node_pool, const entry* start, const entry* end, size_type pos);

    size_type compact_node(const trie_node& node);

    template<typename ModeT, typename NodeT>
    size_type compact_node(ModeT, NodeT& node);

    void check_value_size_or_throw() const;

    size_type push_value_to_store(trie::detail::copy_to_pack, const value_type& value);
    size_type push_value_to_store(trie::detail::move_to_pack, value_type& value);

    void push_child_offsets(size_type offset, const child_offsets_type& child_offsets);

    void init_pack();
    void compact(const trie_node& root);

    template<typename ModeT, typename NodeT>
    void compact(ModeT, NodeT& root);

    template<typename _Handler>
    void traverse_tree(_Handler hdl) const;

private:
    value_store_type m_value_store;
    packed_type m_packed;
};

} // namespace mdds

#include "trie_map_def.inl"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
