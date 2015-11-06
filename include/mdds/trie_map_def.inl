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

#include <cassert>
#include <algorithm>

#ifdef MDDS_TRIE_MAP_DEBUG
#include <iostream>
#endif

namespace mdds { namespace draft {

namespace detail {

#ifdef MDDS_TRIE_MAP_DEBUG

template<typename _NodeT>
void dump_node(std::string& buffer, const _NodeT& node)
{
    using namespace std;
    using node_type = _NodeT;

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

template<typename _NodeT>
void dump_trie(const _NodeT& root)
{
    std::string buffer;
    dump_node(buffer, root);
}

template<typename _ValueT>
void dump_packed_trie(const std::vector<uintptr_t>& packed)
{
    using namespace std;

    using value_type = _ValueT;

    cout << "packed size: " << packed.size() << endl;

    size_t n = packed.size();
    size_t i = 0;
    cout << i << ": root node offset: " << packed[i] << endl;
    ++i;

    while (i < n)
    {
        const value_type* value = reinterpret_cast<const value_type*>(packed[i]);
        cout << i << ": node value pointer: " << value;
        if (value)
            cout << ", value: " << *value;
        cout << endl;
        ++i;

        size_t index_size = packed[i];
        cout << i << ": index size: " << index_size << endl;
        ++i;
        index_size /= 2;

        for (size_t j = 0; j < index_size; ++j)
        {
            char key = packed[i];
            cout << i << ": key: " << key << endl;
            ++i;
            size_t offset = packed[i];
            cout << i << ": offset: " << offset << endl;
            ++i;
        }
    }
}

#endif

template<typename _NodeT, typename _EntryT>
void traverse_range(
    _NodeT& root, const _EntryT* start, const _EntryT* end, size_t pos)
{
    using namespace std;
    using entry = _EntryT;

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

}

template<typename _ValueT>
trie_map<_ValueT>::trie_map(
    const entry* entries, size_type entry_size, value_type null_value) :
    m_null_value(null_value)
{
    const entry* p = entries;
    const entry* p_end = p + entry_size;

    node_type root(0);
    detail::traverse_range(root, p, p_end, 0);
#if defined(MDDS_TRIE_MAP_DEBUG) && defined(MDDS_TREI_MAP_DEBUG_DUMP_TRIE)
    detail::dump_trie(root);
#endif
    compact(root);
}

template<typename _ValueT>
void trie_map<_ValueT>::compact(const node_type& root)
{
    packed_type init(size_t(1), uintptr_t(0));
    m_packed.swap(init);

    size_t root_offset = compact_node(root);
    m_packed[0] = root_offset;

#if defined(MDDS_TRIE_MAP_DEBUG) && defined(MDDS_TREI_MAP_DEBUG_DUMP_PACKED)
    detail::dump_packed_trie<value_type>(m_packed);
#endif
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
            m_packed.push_back(offset-child_offset);
        }
    );

    return offset;
}

#ifdef MDDS_TRIE_MAP_DEBUG

template<typename _ValueT>
void trie_map<_ValueT>::dump() const
{
    if (m_packed.empty())
        return;

    std::string buffer;
    size_t root_offset = m_packed[0];
    const uintptr_t* p = m_packed.data() + root_offset;
    dump_compact_trie_node(buffer, p);
}

template<typename _ValueT>
void trie_map<_ValueT>::dump_compact_trie_node(std::string& buffer, const uintptr_t* p) const
{
    using namespace std;

    const uintptr_t* p0 = p; // store the head offset position of this node.

    const value_type* v = reinterpret_cast<const value_type*>(*p);
    if (v)
        cout << buffer << ":" << *v << endl;

    ++p;
    size_t index_size = *p;
    size_t n = index_size / 2;
    ++p;
    for (size_t i = 0; i < n; ++i)
    {
        char key = *p++;
        size_t offset = *p++;
        buffer.push_back(key);
        const uintptr_t* p_child = p0 - offset;
        dump_compact_trie_node(buffer, p_child);
        buffer.pop_back();
    }
}

#endif

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
