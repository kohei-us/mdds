/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2015 Kohei Yoshida
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

#include <deque>
#include <vector>
#include <string>

namespace mdds { namespace draft {

/**
 * An immutable trie container that packs its content into a contiguous
 * array to achieve both space efficiency and lookup performance.  The user
 * of this data structure must provide a pre-constructed list of key-value
 * entries that are sorted by the key in ascending order.
 */
template<typename _ValueT>
class packed_trie_map
{
    typedef std::vector<uintptr_t> packed_type;
public:
    typedef _ValueT value_type;
    typedef size_t size_type;
    typedef std::pair<std::string, value_type> key_value_type;

    /**
     * Single key-value entry.  Caller must provide at compile time a static
     * array of these entries.
     */
    struct entry
    {
        const char* key;
        size_type keylen;
        value_type value;
    };

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
     * Find a value associated with a specified string key.
     *
     * @param input pointer to a C-style string whose value represents the key
     *              to match.
     * @param len length of the matching string value.
     *
     * @return value associated with the key, or the null value in case the
     *         key is not found.
     */
    value_type find(const char* input, size_type len) const;

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
    std::vector<key_value_type> prefix_search(const char* prefix, size_type len) const;

#ifdef MDDS_TRIE_MAP_DEBUG
    void dump() const;
#endif

private:
    const uintptr_t* find_prefix_node(
        const uintptr_t* p, const char* prefix, const char* prefix_end) const;

    void fill_child_node_items(
        std::vector<key_value_type>& items, std::string& buffer, const uintptr_t* p) const;

#ifdef MDDS_TRIE_MAP_DEBUG
    void dump_compact_trie_node(std::string& buffer, const uintptr_t* p) const;
#endif

private:
    value_type m_null_value;

    packed_type m_packed;
};

}}

#include "trie_map_def.inl"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
