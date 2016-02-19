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
    /** type used to store a final string content. */
    typedef std::string string_type;

    /**
     * type used to build an intermediate string value, from which a final
     * string object is to be created.
     */
    typedef std::string buffer_type;

    /**
     * type that represents a single character inside a string or a buffer
     * object.  A string object is expected to store a series of characters of
     * this type.
     */
    typedef char char_type;

    /**
     * Function called to create and initialize a buffer object from a given
     * initial string value.
     *
     * @param str pointer to the first character of string value.
     * @param length length of the string value.
     *
     * @return buffer object containing the specified string value.
     */
    static buffer_type init_buffer(const char_type* str, size_t length)
    {
        return buffer_type(str, length);
    }

    /**
     * Function called to append a single character to the end of a string
     * buffer.
     *
     * @param buffer buffer object to append character to.
     * @param c character to append to the buffer.
     */
    static void push_back(buffer_type& buffer, char_type c)
    {
        buffer.push_back(c);
    }

    /**
     * Function called to remove a single character from the tail of an
     * existing string buffer.
     *
     * @param buffer buffer object to remove character from.
     */
    static void pop_back(buffer_type& buffer)
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
    static string_type to_string(const buffer_type& buf)
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
    friend class trie::iterator_base<trie_map>;
public:
    typedef packed_trie_map<_KeyTrait, _ValueT> packed_type;
    typedef _KeyTrait key_trait_type;
    typedef typename key_trait_type::string_type string_type;
    typedef typename key_trait_type::buffer_type buffer_type;
    typedef typename key_trait_type::char_type   char_type;
    typedef _ValueT value_type;
    typedef size_t size_type;
    typedef std::pair<string_type, value_type> key_value_type;

    typedef trie::iterator_base<trie_map> const_iterator;

private:

    struct trie_node
    {
        typedef std::map<char_type, trie_node> children_type;

        children_type children;
        value_type value;
        bool has_value;

        trie_node() : has_value(false) {}
    };

    struct stack_item
    {
        typedef typename trie_node::children_type::const_iterator child_pos_type;
        const trie_node* node;
        child_pos_type child_pos;

        stack_item(const trie_node* _node, const child_pos_type& _child_pos) :
            node(_node), child_pos(_child_pos) {}
    };

    typedef std::vector<stack_item> node_stack_type;

public:

    /**
     * Not implemented.
     */
    trie_map() = delete;

    /**
     * Constructor.
     *
     * @param null_value null value to return when the find method fails to
     *                   find a matching entry.
     */
    trie_map(value_type null_value);

    const_iterator begin() const;

    const_iterator end() const;

    /**
     * Insert a new key-value pair.
     *
     * @param key pointer to the first character of a character array that
     *            stores key value.
     * @param len length of the character array storing the key.
     * @param value value to associate with the key.
     */
    void insert(const char_type* key, size_type len, const value_type& value);

    /**
     * Erase a key and the value associated with it.
     *
     * @param key pointer to the first character of a character array that
     *            stores key value.
     * @param len length of the character array storing the key.
     *
     * @return true if a key is erased, false otherwise.
     */
    bool erase(const char_type* key, size_type len);

    /**
     * Find a value associated with a specified string key.
     *
     * @param input pointer to a C-style string whose value represents the key
     *              to match.
     * @param len length of the matching string value.
     *
     * @return value associated with the key, or the null value in case the
     *         key is not found.
     */
    value_type find(const char_type* input, size_type len) const;

    /**
     * Retrieve all key-value pairs whose keys start with specified prefix.
     * You can also retrieve all key-value pairs by passing a null prefix and
     * a length of zero.
     *
     * @param prefix pointer to a C-style string whose value represents the
     *               prefix to match.
     * @param len length of the prefix value to match.
     *
     * @return list of all matching key-value pairs sorted by the key in
     *         ascending order.
     */
    std::vector<key_value_type> prefix_search(const char_type* prefix, size_type len) const;

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
        trie_node& node, const char_type* key, const char_type* key_end, const value_type& value);

    const trie_node* find_prefix_node(
        const trie_node& node, const char_type* prefix, const char_type* prefix_end) const;

    void fill_child_node_items(
        std::vector<key_value_type>& items, buffer_type& buffer, const trie_node& node) const;

    void count_values(size_type& n, const trie_node& node) const;

private:
    value_type m_null_value;
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
public:
    typedef _KeyTrait key_trait_type;
    typedef typename key_trait_type::string_type string_type;
    typedef typename key_trait_type::buffer_type buffer_type;
    typedef typename key_trait_type::char_type   char_type;
    typedef _ValueT value_type;
    typedef size_t size_type;
    typedef std::pair<string_type, value_type> key_value_type;

    /**
     * Single key-value entry.  Caller must provide at compile time a static
     * array of these entries.
     */
    struct entry
    {
        const char_type* key;
        size_type keylen;
        value_type value;

        entry(const char_type* _key, size_type _keylen, value_type _value) :
            key(_key), keylen(_keylen), value(_value) {}
    };

private:
    struct trie_node
    {
        char_type key;
        const value_type* value;

        std::deque<trie_node*> children;

        trie_node(char_type _key) : key(_key), value(nullptr) {}
    };

    typedef std::deque<trie_node> node_pool_type;
    typedef std::vector<uintptr_t> packed_type;
    typedef std::deque<value_type> value_store_type;
    typedef std::vector<std::tuple<size_t, char_type>> child_offsets_type;

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
    packed_trie_map(const entry* entries, size_type entry_size, value_type null_value);

    /**
     * Constructor to allow construction of an instance from the content of a
     * mdds::trie_map instance.
     *
     * @param other mdds::trie_map instance to build content from.
     */
    packed_trie_map(const trie_map<key_trait_type, value_type>& other);

    /**
     * Find a value associated with a specified string key.
     *
     * @param input pointer to a C-style string whose value represents the key
     *              to match.
     * @param len length of the matching string value.
     *
     * @return value associated with the key, or the null value in case the
     *         key is not found.
     */
    value_type find(const char_type* input, size_type len) const;

    /**
     * Retrieve all key-value pairs whose keys start with specified prefix.
     * You can also retrieve all key-value pairs by passing a null prefix and
     * a length of zero.
     *
     * @param prefix pointer to a C-style string whose value represents the
     *               prefix to match.
     * @param len length of the prefix value to match.
     *
     * @return list of all matching key-value pairs sorted by the key in
     *         ascending order.
     */
    std::vector<key_value_type> prefix_search(const char_type* prefix, size_type len) const;

    /**
     * Return the number of entries in the map.
     *
     * @return the number of entries in the map.
     */
    size_type size() const;

private:
    void traverse_range(
        trie_node& root, node_pool_type& node_pool, const entry* start, const entry* end,
        size_type pos);

    size_type compact_node(const trie_node& node);
    size_type compact_node(const typename trie_map<_KeyTrait, _ValueT>::trie_node& node);

    void push_child_offsets(size_type offset, const child_offsets_type& child_offsets);

    void compact(const trie_node& root);
    void compact(const typename trie_map<_KeyTrait, _ValueT>::trie_node& root);

    const uintptr_t* find_prefix_node(
        const uintptr_t* p, const char_type* prefix, const char_type* prefix_end) const;

    void fill_child_node_items(
        std::vector<key_value_type>& items, buffer_type& buffer, const uintptr_t* p) const;

#ifdef MDDS_TRIE_MAP_DEBUG
    void dump_node(buffer_type& buffer, const trie_node& node) const;
    void dump_trie(const trie_node& root) const;
    void dump_packed_trie(const std::vector<uintptr_t>& packed) const;
#endif

private:
    value_type m_null_value;
    size_type m_entry_size;

    value_store_type m_value_store;
    packed_type m_packed;
};

}

#include "trie_map_def.inl"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
