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

template<typename _ValueT>
struct trie_node
{
    typedef _ValueT value_type;

    char key;
    const value_type* value;

    std::deque<trie_node> children;

    trie_node(char _key) : key(_key), value(nullptr) {}
};

template<typename _ValueT>
class trie_map
{
    typedef std::vector<uintptr_t> packed_type;
public:
    typedef _ValueT value_type;
    typedef size_t size_type;
    typedef trie_node<value_type> node_type;

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
    trie_map(const entry* entries, size_type entry_size, value_type null_value);

    /**
     * Dump the content of the trie to stdout for debugging.
     */
    void dump_trie() const;

    void compact();

    void dump_compact_trie() const;

private:
    void traverse_range(node_type& root, const entry* start, const entry* end, size_t pos);
    void dump_node(std::string& buffer, const node_type& node) const;
    void dump_compact_trie_node(std::string& buffer, const uintptr_t* p) const;
    size_t compact_node(const node_type& node);

private:
    value_type m_null_value;
    size_type m_entry_size;

    node_type m_root;
    packed_type m_packed;
};

}}

#include "trie_map_def.inl"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
