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
    m_root(0)
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

    size_t n = std::distance(start, end);

    const entry* p = start;
    const entry* range_start = start;
    const entry* range_end = nullptr;
    char range_char = 0;
    size_t range_count = 0;

    for (; p != end; ++p)
    {
        if (pos == p->keylen)
        {
            root.value = p->value;
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

            root.children.push_back(make_unique<node_type>(range_char));
            traverse_range(*root.children.back(), range_start, range_end, pos+1);
            range_start = range_end;
            range_char = range_start->key[pos];
            range_end = nullptr;
            range_count = 1;
        }

        for (size_t i = 0; i < pos; ++i)
            cout << " ";
        cout << n << ":" << pos << ":" << range_count << ": " << c << endl;
    }

    if (range_count)
    {
        assert(range_char);
        root.children.push_back(make_unique<node_type>(range_char));
        traverse_range(*root.children.back(), range_start, end, pos+1);
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

    if (node.children.empty())
    {
        // This is a leaf node.
        cout << buffer << endl;
        return;
    }

    std::for_each(node.children.begin(), node.children.end(),
        [&](const std::unique_ptr<node_type>& p)
        {
            node_type& node = *p;
            buffer.push_back(node.key);
            dump_node(buffer, node);
            buffer.pop_back();
        }
    );
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
