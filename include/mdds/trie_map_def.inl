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

#include "mdds/global.hpp"

#include <iostream>
#include <cassert>
#include <algorithm>

namespace mdds { namespace draft {

template<typename _ValueT>
trie_map<_ValueT>::trie_map(
    const entry* entries, size_type entry_size, value_type null_value) :
    m_null_value(null_value), m_entry_size(entry_size), m_root(0)
{
    const entry* p = entries;
    const entry* p_end = p + entry_size;

    traverse_range(m_root, p, p_end, 0);
}

template<typename _ValueT>
void trie_map<_ValueT>::traverse_range(
    node_type& root, const entry* start, const entry* end, size_t pos)
{
    using namespace std;

    const entry* p = start;
    const entry* range_start = start;
    const entry* range_end = nullptr;
    char range_char = 0;
    size_t range_count = 0;

    for (; p != end; ++p)
    {
        if (pos > p->keylen)
            continue;

        if (pos == p->keylen)
        {
            root.value = &p->value;
            continue;
        }

        ++range_count;
        char c = p->key[pos];

        if (!range_char)
            range_char = c;
        else if (range_char != c)
        {
            // End of current character range.
            range_end = p;

            root.children.emplace_back(range_char);
            traverse_range(root.children.back(), range_start, range_end, pos+1);
            range_start = range_end;
            range_char = range_start->key[pos];
            range_end = nullptr;
            range_count = 1;
        }
    }

    if (range_count)
    {
        assert(range_char);
        root.children.emplace_back(range_char);
        traverse_range(root.children.back(), range_start, end, pos+1);
    }
}

template<typename _ValueT>
void trie_map<_ValueT>::dump_trie()
{
    std::string buffer;
    dump_node(buffer, m_root);
}

template<typename _ValueT>
void trie_map<_ValueT>::dump_node(std::string& buffer, const node_type& node)
{
    using namespace std;

    if (node.value)
    {
        // This node has value.
        cout << buffer << ":" << *node.value << endl;
    }

    std::for_each(node.children.begin(), node.children.end(),
        [&](const node_type& node)
        {
            buffer.push_back(node.key);
            dump_node(buffer, node);
            buffer.pop_back();
        }
    );
}

template<typename _ValueT>
void trie_map<_ValueT>::compact()
{
    using namespace std;

    packed_type init(size_t(1), uintptr_t(0));
    m_packed.swap(init);

    size_t root_offset = compact_node(m_root);
    m_packed[0] = root_offset;

    cout << "packed size: " << m_packed.size() << endl;

    size_t n = m_packed.size();
    size_t i = 0;
    cout << i << ": root node offset: " << m_packed[i] << endl;
    ++i;

    while (i < n)
    {
        const value_type* value = reinterpret_cast<const value_type*>(m_packed[i]);
        cout << i << ": node value pointer: " << value;
        if (value)
            cout << ", value: " << *value;
        cout << endl;
        ++i;

        size_t index_size = m_packed[i];
        cout << i << ": index size: " << index_size << endl;
        ++i;
        index_size /= 2;

        for (size_t j = 0; j < index_size; ++j)
        {
            char key = m_packed[i];
            cout << i << ": key: " << key << endl;
            ++i;
            size_t offset = m_packed[i];
            cout << i << ": offset: " << offset << endl;
            ++i;
        }
    }
}

template<typename _ValueT>
size_t trie_map<_ValueT>::compact_node(const node_type& node)
{
    std::vector<std::tuple<size_t,char>> child_offsets;
    child_offsets.reserve(node.children.size());

    // Process child nodes first.
    std::for_each(node.children.begin(), node.children.end(),
        [&](const node_type& node)
        {
            size_t child_offset = compact_node(node);
            child_offsets.emplace_back(child_offset, node.key);
        }
    );

    // Process this node.
    size_t offset = m_packed.size();
    m_packed.push_back(uintptr_t(node.value));
    m_packed.push_back(uintptr_t(child_offsets.size()*2));

    std::for_each(child_offsets.begin(), child_offsets.end(),
        [&](const std::tuple<size_t,char>& v)
        {
            char key = std::get<1>(v);
            size_t child_offset = std::get<0>(v);
            m_packed.push_back(key);
            m_packed.push_back(child_offset);
        }
    );

    return offset;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
