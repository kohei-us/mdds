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
#include <memory>

#ifdef MDDS_TRIE_MAP_DEBUG
#include <iostream>
#endif

namespace mdds { namespace draft {

template<typename _KeyTrait, typename _ValueT>
trie_map<_KeyTrait,_ValueT>::trie_map(value_type null_value) :
    m_null_value(null_value) {}

template<typename _KeyTrait, typename _ValueT>
void trie_map<_KeyTrait,_ValueT>::insert(
    const char_type* key, size_type len, const value_type& value)
{
    const char_type* key_end = key + len;
    insert_into_tree(m_root, key, key_end, value);
}

template<typename _KeyTrait, typename _ValueT>
void trie_map<_KeyTrait,_ValueT>::insert_into_tree(
    trie_node& node, const char_type* key, const char_type* key_end,
    const value_type& value)
{
    if (key == key_end)
    {
        node.value = value;
        node.has_value = true;
        return;
    }

    char_type c = *key;

    auto it = node.children.lower_bound(c);
    if (it == node.children.end() || node.children.key_comp()(c, it->first))
    {
        // Insert a new node.
        it = node.children.insert(
            it, typename trie_node::children_type::value_type(c, trie_node()));
    }

    ++key;
    insert_into_tree(it->second, key, key_end, value);
}

template<typename _KeyTrait, typename _ValueT>
const typename trie_map<_KeyTrait,_ValueT>::trie_node*
trie_map<_KeyTrait,_ValueT>::find_prefix_node(
    const trie_node& node, const char_type* prefix, const char_type* prefix_end) const
{
    if (prefix == prefix_end)
        // Right node is found.
        return &node;

    auto it = node.children.find(*prefix);
    if (it == node.children.end())
        return nullptr;

    ++prefix;
    return find_prefix_node(it->second, prefix, prefix_end);
}

template<typename _KeyTrait, typename _ValueT>
typename trie_map<_KeyTrait,_ValueT>::value_type
trie_map<_KeyTrait,_ValueT>::find(const char_type* input, size_type len) const
{
    const char_type* input_end = input + len;
    const trie_node* node = find_prefix_node(m_root, input, input_end);
    if (!node || !node->has_value)
        return m_null_value;

    return node->value;
}

template<typename _KeyTrait, typename _ValueT>
std::vector<typename trie_map<_KeyTrait,_ValueT>::key_value_type>
trie_map<_KeyTrait,_ValueT>::prefix_search(const char_type* prefix, size_type len) const
{
    std::vector<key_value_type> matches;
    return matches;
}

template<typename _KeyTrait, typename _ValueT>
typename trie_map<_KeyTrait,_ValueT>::size_type
trie_map<_KeyTrait,_ValueT>::size() const
{
    return 0;
}

#ifdef MDDS_TRIE_MAP_DEBUG

template<typename _KeyTrait, typename _ValueT>
void packed_trie_map<_KeyTrait,_ValueT>::dump_node(
    buffer_type& buffer, const trie_node& node) const
{
    using namespace std;

    using ktt = key_trait_type;

    if (node.value)
    {
        // This node has value.
        cout << buffer << ":" << *static_cast<const value_type*>(node.value) << endl;
    }

    std::for_each(node.children.begin(), node.children.end(),
        [&](const trie_node* p)
        {
            const trie_node& node = *p;
            ktt::push_back(buffer, node.key);
            dump_node(buffer, node);
            ktt::pop_back(buffer);
        }
    );
}

template<typename _KeyTrait, typename _ValueT>
void packed_trie_map<_KeyTrait,_ValueT>::dump_trie(const trie_node& root) const
{
    buffer_type buffer;
    dump_node(buffer, root);
}

template<typename _KeyTrait, typename _ValueT>
void packed_trie_map<_KeyTrait,_ValueT>::dump_packed_trie(const std::vector<uintptr_t>& packed) const
{
    using namespace std;

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
            char_type key = packed[i];
            cout << i << ": key: " << key << endl;
            ++i;
            size_t offset = packed[i];
            cout << i << ": offset: " << offset << endl;
            ++i;
        }
    }
}

#endif

template<typename _KeyTrait, typename _ValueT>
void packed_trie_map<_KeyTrait,_ValueT>::traverse_range(
    trie_node& root,
    node_pool_type& node_pool,
    const typename packed_trie_map<_KeyTrait,_ValueT>::entry* start,
    const typename packed_trie_map<_KeyTrait,_ValueT>::entry* end,
    size_type pos)
{
    using entry = typename packed_trie_map<_KeyTrait,_ValueT>::entry;

    const entry* p = start;
    const entry* range_start = start;
    const entry* range_end = nullptr;
    char_type range_char = 0;
    size_type range_count = 0;

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
        char_type c = p->key[pos];

        if (!range_char)
            range_char = c;
        else if (range_char != c)
        {
            // End of current character range.
            range_end = p;

            node_pool.emplace_back(range_char);
            root.children.push_back(&node_pool.back());
            traverse_range(*root.children.back(), node_pool, range_start, range_end, pos+1);
            range_start = range_end;
            range_char = range_start->key[pos];
            range_end = nullptr;
            range_count = 1;
        }
    }

    if (range_count)
    {
        assert(range_char);
        node_pool.emplace_back(range_char);
        root.children.push_back(&node_pool.back());
        traverse_range(*root.children.back(), node_pool, range_start, end, pos+1);
    }
}

template<typename _KeyTrait, typename _ValueT>
typename packed_trie_map<_KeyTrait,_ValueT>::size_type
packed_trie_map<_KeyTrait,_ValueT>::compact_node(const trie_node& node)
{
    std::vector<std::tuple<size_t,char_type>> child_offsets;
    child_offsets.reserve(node.children.size());

    // Process child nodes first.
    std::for_each(node.children.begin(), node.children.end(),
        [&](const trie_node* p)
        {
            const trie_node& node = *p;
            size_t child_offset = compact_node(node);
            child_offsets.emplace_back(child_offset, node.key);
        }
    );

    // Process this node.
    size_t offset = m_packed.size();
    if (node.value)
    {
        m_value_store.push_back(*node.value);  // copy the value object.
        m_packed.push_back(uintptr_t(&m_value_store.back()));
    }
    else
        m_packed.push_back(uintptr_t(0));

    m_packed.push_back(uintptr_t(child_offsets.size()*2));

    std::for_each(child_offsets.begin(), child_offsets.end(),
        [&](const std::tuple<size_t,char_type>& v)
        {
            char_type key = std::get<1>(v);
            size_t child_offset = std::get<0>(v);
            m_packed.push_back(key);
            m_packed.push_back(offset-child_offset);
        }
    );

    return offset;
}

template<typename _KeyTrait, typename _ValueT>
void packed_trie_map<_KeyTrait,_ValueT>::compact(const trie_node& root)
{
    packed_type init(size_t(1), uintptr_t(0));
    m_packed.swap(init);

    size_t root_offset = compact_node(root);
    m_packed[0] = root_offset;
}

template<typename _KeyTrait, typename _ValueT>
packed_trie_map<_KeyTrait,_ValueT>::packed_trie_map(
    const entry* entries, size_type entry_size, value_type null_value) :
    m_null_value(null_value), m_entry_size(entry_size)
{
    const entry* p = entries;
    const entry* p_end = p + entry_size;

    // Populate the normal tree first.
    trie_node root(0);
    node_pool_type node_pool;
    traverse_range(root, node_pool, p, p_end, 0);
#if defined(MDDS_TRIE_MAP_DEBUG) && defined(MDDS_TREI_MAP_DEBUG_DUMP_TRIE)
    dump_trie(root);
#endif

    // Compact the trie into a packed array.
    compact(root);
#if defined(MDDS_TRIE_MAP_DEBUG) && defined(MDDS_TREI_MAP_DEBUG_DUMP_PACKED)
    dump_packed_trie(m_packed);
#endif
}

template<typename _KeyTrait, typename _ValueT>
typename packed_trie_map<_KeyTrait,_ValueT>::value_type
packed_trie_map<_KeyTrait,_ValueT>::find(const char_type* input, size_type len) const
{
    if (m_packed.empty())
        return m_null_value;

    const char_type* key_end = input + len;
    size_t root_offset = m_packed[0];
    const uintptr_t* root = m_packed.data() + root_offset;

    const uintptr_t* node = find_prefix_node(root, input, key_end);
    if (!node)
        return m_null_value;

    const value_type* pv = reinterpret_cast<const value_type*>(*node);
    return pv ? *pv : m_null_value;
}

template<typename _KeyTrait, typename _ValueT>
std::vector<typename packed_trie_map<_KeyTrait,_ValueT>::key_value_type>
packed_trie_map<_KeyTrait,_ValueT>::prefix_search(const char_type* prefix, size_type len) const
{
    using ktt = key_trait_type;

    std::vector<key_value_type> matches;

    if (m_packed.empty())
        return matches;

    const char_type* prefix_end = prefix + len;

    size_t root_offset = m_packed[0];
    const uintptr_t* root = m_packed.data() + root_offset;

    const uintptr_t* node = find_prefix_node(root, prefix, prefix_end);
    if (!node)
        return matches;

    // Fill all its child nodes.
    buffer_type buffer = ktt::init_buffer(prefix, len);
    fill_child_node_items(matches, buffer, node);
    return matches;
}

template<typename _KeyTrait, typename _ValueT>
typename packed_trie_map<_KeyTrait,_ValueT>::size_type
packed_trie_map<_KeyTrait,_ValueT>::size() const
{
    return m_entry_size;
}

template<typename _KeyTrait, typename _ValueT>
const uintptr_t* packed_trie_map<_KeyTrait,_ValueT>::find_prefix_node(
    const uintptr_t* p, const char_type* prefix, const char_type* prefix_end) const
{
    if (prefix == prefix_end)
        return p;

    const uintptr_t* p0 = p; // store the head offset position of this node.

    // Find the child node with a matching key character.

    ++p;
    size_t index_size = *p;
    size_t n = index_size / 2;
    ++p;

    if (!n)
        // This is a leaf node.
        return nullptr;

    for (size_type low = 0, high = n-1; low <= high; )
    {
        size_type i = (low + high) / 2;

        const uintptr_t* p_this = p + i*2;
        char_type node_key = *p_this;
        size_t offset = *(p_this+1);

        if (*prefix == node_key)
        {
            // Match found!
            const uintptr_t* p_child = p0 - offset;
            ++prefix;
            return find_prefix_node(p_child, prefix, prefix_end);
        }

        if (low == high)
            // No more child node key to test. Bail out.
            break;

        if (high - low == 1)
        {
            // Only two more child keys left.
            if (i == low)
                low = high;
            else
            {
                assert(i == high);
                high = low;
            }
        }
        else if (*prefix < node_key)
            // Move on to the lower sub-group.
            high = i;
        else
            // Move on to the higher sub-group.
            low = i;
    }

    return nullptr;
}

template<typename _KeyTrait, typename _ValueT>
void packed_trie_map<_KeyTrait,_ValueT>::fill_child_node_items(
    std::vector<key_value_type>& items, buffer_type& buffer, const uintptr_t* p) const
{
    using ktt = key_trait_type;

    const uintptr_t* p0 = p; // store the head offset position of this node.

    const value_type* v = reinterpret_cast<const value_type*>(*p);
    if (v)
        items.push_back(key_value_type(ktt::to_string(buffer), *v));

    ++p;
    size_t index_size = *p;
    size_t n = index_size / 2;
    ++p;
    for (size_t i = 0; i < n; ++i)
    {
        char_type key = *p++;
        size_t offset = *p++;
        ktt::push_back(buffer, key);
        const uintptr_t* p_child = p0 - offset;
        fill_child_node_items(items, buffer, p_child);
        ktt::pop_back(buffer);
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
