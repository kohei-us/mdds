/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2015-2016 Kohei Yoshida
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

namespace mdds {

namespace trie {

/**
 * String trait that uses std::string as the key type.  This trait can be
 * used with mdds::trie_map or mdds::packed_trie_map.
 */
struct std_string_trait
{
    /** type used to store a key value. */
    typedef std::string key_type;

    /**
     * type used to build an intermediate key value, from which a final key
     * value is to be created.  It is expected to be an array structure whose
     * content is contiguous in memory.  Its elements must be of
     * key_unit_type.
     */
    typedef std::string key_buffer_type;

    /**
     * type that represents a single character inside a key or a key buffer
     * object.  A key object is expected to store a series of elements of this
     * type.
     */
    typedef char key_unit_type;

    /**
     * Function called to create and initialize a buffer object from a given
     * initial key value.
     *
     * @param str pointer to the first character of the key value.
     * @param length length of the key value.
     *
     * @return buffer object containing the specified key value.
     */
    static key_buffer_type to_key_buffer(const key_unit_type* str, size_t length)
    {
        return key_buffer_type(str, length);
    }

    /**
     * Function called to create and initialize a buffer object from a given
     * initial key value.
     *
     * @param key key value
     *
     * @return buffer object containing the specified key value.
     */
    static key_buffer_type to_key_buffer(const key_type& key)
    {
        return key_buffer_type(key);
    }

    static const key_unit_type* buffer_data(const key_buffer_type& buf)
    {
        return buf.data();
    }

    static size_t buffer_size(const key_buffer_type& buf)
    {
        return buf.size();
    }

    /**
     * Function called to append a single character to the end of a key
     * buffer.
     *
     * @param buffer buffer object to append character to.
     * @param c character to append to the buffer.
     */
    static void push_back(key_buffer_type& buffer, key_unit_type c)
    {
        buffer.push_back(c);
    }

    /**
     * Function called to remove a single character from the tail of an
     * existing key buffer.
     *
     * @param buffer buffer object to remove character from.
     */
    static void pop_back(key_buffer_type& buffer)
    {
        buffer.pop_back();
    }

    /**
     * Function called to create a final string object from an existing
     * buffer.
     *
     * @param buf buffer object to create a final string object from.
     *
     * @return string object whose content is created from the buffer object.
     */
    static key_type to_key(const key_buffer_type& buf)
    {
        return buf;
    }
};

}

template<typename _KeyTrait, typename _ValueT>
class packed_trie_map;

/**
 * Trie map provides storage for multiple key-value pairs where keys are
 * either strings, or otherwise consist of arrays of characters.  The keys
 * are stored in an ordered tree structure known as trie, or sometimes
 * referred to as prefix tree.
 */
template<typename _KeyTrait, typename _ValueT>
class trie_map
{
    friend class packed_trie_map<_KeyTrait, _ValueT>;
    friend class trie::detail::iterator_base<trie_map>;
    friend class trie::detail::search_results<trie_map>;

public:
    typedef packed_trie_map<_KeyTrait, _ValueT> packed_type;
    typedef _KeyTrait key_trait_type;
    typedef typename key_trait_type::key_type key_type;
    typedef typename key_trait_type::key_buffer_type key_buffer_type;
    typedef typename key_trait_type::key_unit_type   key_unit_type;
    typedef _ValueT value_type;
    typedef size_t size_type;
    typedef std::pair<key_type, value_type> key_value_type;

    typedef trie::detail::iterator_base<trie_map> const_iterator;
    typedef trie::detail::search_results<trie_map> search_results;

private:

    struct trie_node
    {
        typedef std::map<key_unit_type, trie_node> children_type;

        children_type children;
        value_type value;
        bool has_value;

        trie_node() : value(value_type()), has_value(false) {}
    };

    struct stack_item
    {
        typedef typename trie_node::children_type::const_iterator child_pos_type;
        const trie_node* node;
        child_pos_type child_pos;

        stack_item(const trie_node* _node, const child_pos_type& _child_pos) :
            node(_node), child_pos(_child_pos) {}

        bool operator== (const stack_item& r) const
        {
            return node == r.node && child_pos == r.child_pos;
        }

        bool operator!= (const stack_item& r) const
        {
            return !operator== (r);
        }
    };

    typedef std::vector<stack_item> node_stack_type;

public:

    /**
     * Default constructor.
     */
    trie_map();

    const_iterator begin() const;

    const_iterator end() const;

    /**
     * Insert a new key-value pair.
     *
     * @param key key value.
     * @param value value to associate with the key.
     */
    void insert(const key_type& key, const value_type& value);

    /**
     * Insert a new key-value pair.
     *
     * @param key pointer to the first character of a character array that
     *            stores key value.
     * @param len length of the character array storing the key.
     * @param value value to associate with the key.
     */
    void insert(const key_unit_type* key, size_type len, const value_type& value);

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
    size_type size() const;

    /**
     * Empty the container.
     */
    void clear();

    /**
     * Create a compressed and immutable version of the container which
     * provides better search performance and requires much less memory
     * footprint.
     *
     * @return an instance of mdds::packed_trie_map with the same content.
     */
    packed_type pack() const;

private:
    void insert_into_tree(
        trie_node& node, const key_unit_type* key, const key_unit_type* key_end, const value_type& value);

    const trie_node* find_prefix_node(
        const trie_node& node, const key_unit_type* prefix, const key_unit_type* prefix_end) const;

    void find_prefix_node_with_stack(
        node_stack_type& node_stack,
        const trie_node& node, const key_unit_type* prefix, const key_unit_type* prefix_end) const;

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
template<typename _KeyTrait, typename _ValueT>
class packed_trie_map
{
    friend class trie::detail::packed_iterator_base<packed_trie_map>;
    friend class trie::detail::packed_search_results<packed_trie_map>;

public:
    typedef _KeyTrait key_trait_type;
    typedef typename key_trait_type::key_type key_type;
    typedef typename key_trait_type::key_buffer_type key_buffer_type;
    typedef typename key_trait_type::key_unit_type   key_unit_type;
    typedef _ValueT value_type;
    typedef size_t size_type;
    typedef std::pair<key_type, value_type> key_value_type;
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

        entry(const key_unit_type* _key, size_type _keylen, value_type _value) :
            key(_key), keylen(_keylen), value(_value) {}
    };

private:
    struct trie_node
    {
        key_unit_type key;
        const value_type* value;

        std::deque<trie_node*> children;

        trie_node(key_unit_type _key) : key(_key), value(nullptr) {}
    };

    struct stack_item
    {
        const uintptr_t* node_pos;
        const uintptr_t* child_pos;
        const uintptr_t* child_end;

        stack_item(const uintptr_t* _node_pos, const uintptr_t* _child_pos, const uintptr_t* _child_end) :
            node_pos(_node_pos), child_pos(_child_pos), child_end(_child_end) {}

        bool operator== (const stack_item& other) const
        {
            return node_pos == other.node_pos && child_pos == other.child_pos;
        }

        bool operator!= (const stack_item& other) const
        {
            return !operator==(other);
        }

        bool has_value() const
        {
            const value_type* pv = reinterpret_cast<const value_type*>(*node_pos);
            return pv;
        }

        const value_type* get_value() const
        {
            return reinterpret_cast<const value_type*>(*node_pos);
        }
    };

    typedef std::vector<stack_item> node_stack_type;

    typedef std::deque<trie_node> node_pool_type;
    typedef std::vector<uintptr_t> packed_type;
    typedef std::deque<value_type> value_store_type;
    typedef std::vector<std::tuple<size_t, key_unit_type>> child_offsets_type;

public:

    /**
     * Not implemented.
     */
    packed_trie_map() = delete;

    /**
     * Constructor that initializes the content from a static list of
     * key-value entries.  The caller <em>must</em> ensure that the key-value
     * entries are sorted in ascending order, else the behavior is undefined.
     *
     * @param entries pointer to the array of key-value entries.
     * @param entry_size size of the key-value entry array.
     * @param null_value null value to return when the find method fails to
     *                   find a matching entry.
     */
    packed_trie_map(const entry* entries, size_type entry_size);

    /**
     * Constructor to allow construction of an instance from the content of a
     * mdds::trie_map instance.
     *
     * @param other mdds::trie_map instance to build content from.
     */
    packed_trie_map(const trie_map<key_trait_type, value_type>& other);

    const_iterator begin() const;

    const_iterator end() const;

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
    size_type size() const;

private:
    node_stack_type get_root_stack() const;

    void traverse_range(
        trie_node& root, node_pool_type& node_pool, const entry* start, const entry* end,
        size_type pos);

    size_type compact_node(const trie_node& node);
    size_type compact_node(const typename trie_map<_KeyTrait, _ValueT>::trie_node& node);

    void push_child_offsets(size_type offset, const child_offsets_type& child_offsets);

    void compact(const trie_node& root);
    void compact(const typename trie_map<_KeyTrait, _ValueT>::trie_node& root);

    const uintptr_t* find_prefix_node(
        const uintptr_t* p, const key_unit_type* prefix, const key_unit_type* prefix_end) const;

    void find_prefix_node_with_stack(
        node_stack_type& node_stack,
        const uintptr_t* p, const key_unit_type* prefix, const key_unit_type* prefix_end) const;

#ifdef MDDS_TRIE_MAP_DEBUG
    void dump_node(key_buffer_type& buffer, const trie_node& node) const;
    void dump_trie(const trie_node& root) const;
    void dump_packed_trie(const std::vector<uintptr_t>& packed) const;
#endif

private:
    size_type m_entry_size;

    value_store_type m_value_store;
    packed_type m_packed;
};

}

#include "trie_map_def.inl"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
