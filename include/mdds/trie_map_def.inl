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

namespace mdds {

template<typename _KeyTrait, typename _ValueT>
trie_map<_KeyTrait,_ValueT>::trie_map() {}

template<typename _KeyTrait, typename _ValueT>
typename trie_map<_KeyTrait,_ValueT>::const_iterator
trie_map<_KeyTrait,_ValueT>::begin() const
{
    if (m_root.children.empty())
        // empty container
        return end();

    // Push the root node.
    key_buffer_type buf;
    node_stack_type node_stack;
    node_stack.emplace_back(&m_root, m_root.children.begin());

    // Push root's first child node.
    auto it = node_stack.back().child_pos;
    const_iterator::push_child_node_to_stack(node_stack, buf, it);

    // In theory there should always be at least one value node along the
    // left-most branch.

    while (!node_stack.back().node->has_value)
    {
        auto this_it = node_stack.back().child_pos;
        const_iterator::push_child_node_to_stack(node_stack, buf, this_it);
    }

    return const_iterator(
        std::move(node_stack), std::move(buf), trie::detail::iterator_type::normal);
}

template<typename _KeyTrait, typename _ValueT>
typename trie_map<_KeyTrait,_ValueT>::const_iterator
trie_map<_KeyTrait,_ValueT>::end() const
{
    node_stack_type node_stack;
    node_stack.emplace_back(&m_root, m_root.children.end());
    return const_iterator(
        std::move(node_stack), key_buffer_type(), trie::detail::iterator_type::end);
}

template<typename _KeyTrait, typename _ValueT>
void trie_map<_KeyTrait,_ValueT>::insert(const key_type& key, const value_type& value)
{
    using ktt = key_trait_type;

    key_buffer_type buf = ktt::to_key_buffer(key);
    const key_unit_type* p = ktt::buffer_data(buf);
    size_t n = ktt::buffer_size(buf);
    const key_unit_type* p_end = p + n;
    insert_into_tree(m_root, p, p_end, value);
}

template<typename _KeyTrait, typename _ValueT>
void trie_map<_KeyTrait,_ValueT>::insert(
    const key_unit_type* key, size_type len, const value_type& value)
{
    const key_unit_type* key_end = key + len;
    insert_into_tree(m_root, key, key_end, value);
}

template<typename _KeyTrait, typename _ValueT>
bool trie_map<_KeyTrait,_ValueT>::erase(const key_unit_type* key, size_type len)
{
    const key_unit_type* key_end = key + len;

    node_stack_type node_stack;
    find_prefix_node_with_stack(node_stack, m_root, key, key_end);

    if (node_stack.empty() || !node_stack.back().node->has_value)
        // Nothing is erased.
        return false;

    const trie_node* node = node_stack.back().node;
    trie_node* node_mod = const_cast<trie_node*>(node);
    node_mod->has_value = false;

    // If this is a leaf node, remove it, and keep removing its parents until
    // we reach a parent node that still has at least one child node.

    while (!node->has_value && node->children.empty() && node_stack.size() > 1)
    {
        node_stack.pop_back();
        auto& si = node_stack.back();

        const_cast<trie_node*>(si.node)->children.erase(si.child_pos);
        node = si.node;
    }

    return true;
}

template<typename _KeyTrait, typename _ValueT>
void trie_map<_KeyTrait,_ValueT>::insert_into_tree(
    trie_node& node, const key_unit_type* key, const key_unit_type* key_end,
    const value_type& value)
{
    if (key == key_end)
    {
        node.value = value;
        node.has_value = true;
        return;
    }

    key_unit_type c = *key;

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
    const trie_node& node, const key_unit_type* prefix, const key_unit_type* prefix_end) const
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
void trie_map<_KeyTrait,_ValueT>::find_prefix_node_with_stack(
    node_stack_type& node_stack,
    const trie_node& node, const key_unit_type* prefix, const key_unit_type* prefix_end) const
{
    if (prefix == prefix_end)
    {
        // Right node is found.
        node_stack.emplace_back(&node, node.children.begin());
        return;
    }

    auto it = node.children.find(*prefix);
    if (it == node.children.end())
        return;

    node_stack.emplace_back(&node, it);

    ++prefix;
    find_prefix_node_with_stack(node_stack, it->second, prefix, prefix_end);
}

template<typename _KeyTrait, typename _ValueT>
void trie_map<_KeyTrait,_ValueT>::count_values(size_type& n, const trie_node& node) const
{
    if (node.has_value)
        ++n;

    std::for_each(node.children.begin(), node.children.end(),
        [&](const typename trie_node::children_type::value_type& v)
        {
            count_values(n, v.second);
        }
    );
}

template<typename _KeyTrait, typename _ValueT>
typename trie_map<_KeyTrait,_ValueT>::const_iterator
trie_map<_KeyTrait,_ValueT>::find(const key_type& key) const
{
    using ktt = key_trait_type;
    key_buffer_type buf = ktt::to_key_buffer(key);
    const key_unit_type* p = ktt::buffer_data(buf);
    size_t n = ktt::buffer_size(buf);

    return find(p, n);
}

template<typename _KeyTrait, typename _ValueT>
typename trie_map<_KeyTrait,_ValueT>::const_iterator
trie_map<_KeyTrait,_ValueT>::find(const key_unit_type* input, size_type len) const
{
    const key_unit_type* input_end = input + len;
    node_stack_type node_stack;
    find_prefix_node_with_stack(node_stack, m_root, input, input_end);
    if (node_stack.empty() || !node_stack.back().node->has_value)
        // Specified key doesn't exist.
        return end();

    // Build the key value from the stack.
    key_buffer_type buf;
    auto end = node_stack.end();
    --end;  // Skip the node with value which doesn't store a key element.
    std::for_each(node_stack.begin(), end,
        [&](const stack_item& si)
        {
            using ktt = key_trait_type;
            ktt::push_back(buf, si.child_pos->first);
        }
    );

    return const_iterator(
        std::move(node_stack), std::move(buf), trie::detail::iterator_type::normal);
}

template<typename _KeyTrait, typename _ValueT>
typename trie_map<_KeyTrait,_ValueT>::search_results
trie_map<_KeyTrait,_ValueT>::prefix_search(const key_type& key) const
{
    using ktt = key_trait_type;
    key_buffer_type buf = ktt::to_key_buffer(key);
    const key_unit_type* p = ktt::buffer_data(buf);
    size_t n = ktt::buffer_size(buf);

    return prefix_search(p, n);
}

template<typename _KeyTrait, typename _ValueT>
typename trie_map<_KeyTrait,_ValueT>::search_results
trie_map<_KeyTrait,_ValueT>::prefix_search(const key_unit_type* prefix, size_type len) const
{
    using ktt = key_trait_type;

    const key_unit_type* prefix_end = prefix + len;
    std::vector<key_value_type> matches;

    const trie_node* node = find_prefix_node(m_root, prefix, prefix_end);
    return search_results(node, ktt::to_key_buffer(prefix, len));
}

template<typename _KeyTrait, typename _ValueT>
typename trie_map<_KeyTrait,_ValueT>::size_type
trie_map<_KeyTrait,_ValueT>::size() const
{
    size_type n = 0;
    count_values(n, m_root);
    return n;
}

template<typename _KeyTrait, typename _ValueT>
void trie_map<_KeyTrait,_ValueT>::clear()
{
    m_root.children.clear();
    m_root.has_value = false;
}

template<typename _KeyTrait, typename _ValueT>
typename trie_map<_KeyTrait,_ValueT>::packed_type
trie_map<_KeyTrait,_ValueT>::pack() const
{
    return packed_type(*this);
}

#ifdef MDDS_TRIE_MAP_DEBUG

template<typename _KeyTrait, typename _ValueT>
void packed_trie_map<_KeyTrait,_ValueT>::dump_node(
    key_buffer_type& buffer, const trie_node& node) const
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
            const trie_node& this_node = *p;
            ktt::push_back(buffer, this_node.key);
            dump_node(buffer, this_node);
            ktt::pop_back(buffer);
        }
    );
}

template<typename _KeyTrait, typename _ValueT>
void packed_trie_map<_KeyTrait,_ValueT>::dump_trie(const trie_node& root) const
{
    key_buffer_type buffer;
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
            key_unit_type key = packed[i];
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
    const entry* p = start;
    const entry* range_start = start;
    const entry* range_end = nullptr;
    key_unit_type range_char = 0;
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
        key_unit_type c = p->key[pos];

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
    std::vector<std::tuple<size_t,key_unit_type>> child_offsets;
    child_offsets.reserve(node.children.size());

    // Process child nodes first.
    std::for_each(node.children.begin(), node.children.end(),
        [&](const trie_node* p)
        {
            const trie_node& child_node = *p;
            size_type child_offset = compact_node(child_node);
            child_offsets.emplace_back(child_offset, child_node.key);
        }
    );

    // Process this node.
    size_type offset = m_packed.size();
    if (node.value)
    {
        m_value_store.push_back(*node.value);  // copy the value object.
        m_packed.push_back(uintptr_t(&m_value_store.back()));
    }
    else
        m_packed.push_back(uintptr_t(0));

    push_child_offsets(offset, child_offsets);
    return offset;
}

template<typename _KeyTrait, typename _ValueT>
typename packed_trie_map<_KeyTrait,_ValueT>::size_type
packed_trie_map<_KeyTrait,_ValueT>::compact_node(
    const typename trie_map<_KeyTrait, _ValueT>::trie_node& node)
{
    using node_type = typename trie_map<_KeyTrait, _ValueT>::trie_node;

    std::vector<std::tuple<size_t,key_unit_type>> child_offsets;
    child_offsets.reserve(node.children.size());

    // Process child nodes first.
    std::for_each(node.children.begin(), node.children.end(),
        [&](const typename node_type::children_type::value_type& v)
        {
            char key = v.first;
            const node_type& child_node = v.second;
            size_type child_offset = compact_node(child_node);
            child_offsets.emplace_back(child_offset, key);
        }
    );

    // Process this node.
    size_type offset = m_packed.size();
    if (node.has_value)
    {
        m_value_store.push_back(node.value);  // copy the value object.
        m_packed.push_back(uintptr_t(&m_value_store.back()));
        ++m_entry_size;
    }
    else
        m_packed.push_back(uintptr_t(0));

    push_child_offsets(offset, child_offsets);
    return offset;
}

template<typename _KeyTrait, typename _ValueT>
void packed_trie_map<_KeyTrait,_ValueT>::push_child_offsets(
    size_type offset, const child_offsets_type& child_offsets)
{
    m_packed.push_back(uintptr_t(child_offsets.size()*2));

    std::for_each(child_offsets.begin(), child_offsets.end(),
        [&](const std::tuple<size_t,key_unit_type>& v)
        {
            key_unit_type key = std::get<1>(v);
            size_t child_offset = std::get<0>(v);
            m_packed.push_back(key);
            m_packed.push_back(offset-child_offset);
        }
    );
}

template<typename _KeyTrait, typename _ValueT>
void packed_trie_map<_KeyTrait,_ValueT>::compact(const trie_node& root)
{
    packed_type init(size_t(1), uintptr_t(0));
    m_packed.swap(init);
    assert(m_packed.size() == 1);

    size_t root_offset = compact_node(root);
    m_packed[0] = root_offset;
}

template<typename _KeyTrait, typename _ValueT>
void packed_trie_map<_KeyTrait,_ValueT>::compact(
    const typename trie_map<_KeyTrait, _ValueT>::trie_node& root)
{
    packed_type init(size_t(1), uintptr_t(0));
    m_packed.swap(init);
    assert(m_packed.size() == 1);

    size_t root_offset = compact_node(root);
    m_packed[0] = root_offset;
}

template<typename _KeyTrait, typename _ValueT>
packed_trie_map<_KeyTrait,_ValueT>::packed_trie_map(
    const entry* entries, size_type entry_size) : m_entry_size(entry_size)
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
packed_trie_map<_KeyTrait,_ValueT>::packed_trie_map(
    const trie_map<key_trait_type, value_type>& other) : m_entry_size(0)
{
    compact(other.m_root);
}

template<typename _KeyTrait, typename _ValueT>
typename packed_trie_map<_KeyTrait,_ValueT>::const_iterator
packed_trie_map<_KeyTrait,_ValueT>::begin() const
{
    using ktt = key_trait_type;

    node_stack_type node_stack = get_root_stack();

    const stack_item* si = &node_stack.back();
    if (si->child_pos == si->child_end)
        // empty container.
        return const_iterator(std::move(node_stack), key_buffer_type());

    const uintptr_t* node_pos = si->node_pos;
    const uintptr_t* child_pos = si->child_pos;
    const uintptr_t* child_end = si->child_end;
    const uintptr_t* p = child_pos;

    // Follow the root node's left-most child.
    key_buffer_type buf;
    key_unit_type c = *p;
    ktt::push_back(buf, c);
    ++p;
    size_t offset = *p;

    node_pos -= offset;  // jump to the child node.
    p = node_pos;
    ++p;
    size_t index_size = *p;
    ++p;
    child_pos = p;
    child_end = child_pos + index_size;

    // Push this child node onto the stack.
    node_stack.emplace_back(node_pos, child_pos, child_end);

    const value_type* pv = reinterpret_cast<const value_type*>(*node_pos);
    while (!pv)
    {
        // Keep following the left child node until we reach a node with value.
        const_iterator::push_child_node_to_stack(node_stack, buf, node_stack.back().child_pos);
        pv = reinterpret_cast<const value_type*>(*node_stack.back().node_pos);
    }

    return const_iterator(std::move(node_stack), std::move(buf), *pv);
}

template<typename _KeyTrait, typename _ValueT>
typename packed_trie_map<_KeyTrait,_ValueT>::const_iterator
packed_trie_map<_KeyTrait,_ValueT>::end() const
{
    node_stack_type node_stack = get_root_stack();
    node_stack.back().child_pos = node_stack.back().child_end;
    return const_iterator(std::move(node_stack), key_buffer_type());
}

template<typename _KeyTrait, typename _ValueT>
typename packed_trie_map<_KeyTrait,_ValueT>::node_stack_type
packed_trie_map<_KeyTrait,_ValueT>::get_root_stack() const
{
    assert(!m_packed.empty());
    size_t root_offset = m_packed[0];
    assert(root_offset < m_packed.size());
    const uintptr_t* p = m_packed.data() + root_offset;
    const uintptr_t* node_pos = p;
    ++p;
    size_t index_size = *p;
    ++p;
    const uintptr_t* child_pos = p;
    const uintptr_t* child_end = child_pos + index_size;

    node_stack_type node_stack;
    node_stack.emplace_back(node_pos, child_pos, child_end);

    return node_stack;
}

template<typename _KeyTrait, typename _ValueT>
typename packed_trie_map<_KeyTrait,_ValueT>::const_iterator
packed_trie_map<_KeyTrait,_ValueT>::find(const key_type& key) const
{
    using ktt = key_trait_type;
    key_buffer_type buf = ktt::to_key_buffer(key);
    const key_unit_type* p = ktt::buffer_data(buf);
    size_t n = ktt::buffer_size(buf);

    return find(p, n);
}

template<typename _KeyTrait, typename _ValueT>
typename packed_trie_map<_KeyTrait,_ValueT>::const_iterator
packed_trie_map<_KeyTrait,_ValueT>::find(const key_unit_type* input, size_type len) const
{
    if (m_packed.empty())
        return end();

    const key_unit_type* key_end = input + len;
    size_t root_offset = m_packed[0];
    assert(root_offset < m_packed.size());
    const uintptr_t* root = m_packed.data() + root_offset;

    node_stack_type node_stack;
    find_prefix_node_with_stack(node_stack, root, input, key_end);
    if (node_stack.empty() || !node_stack.back().node_pos)
        return end();

    const stack_item& si = node_stack.back();
    const value_type* pv = reinterpret_cast<const value_type*>(*si.node_pos);
    if (!pv)
        return end();

    // Build the key value from the stack.
    key_buffer_type buf;
    auto end = node_stack.end();
    --end;  // Skip the node with value which doesn't store a key element.
    std::for_each(node_stack.begin(), end,
        [&](const stack_item& this_si)
        {
            using ktt = key_trait_type;
            ktt::push_back(buf, *this_si.child_pos);
        }
    );

    return const_iterator(std::move(node_stack), std::move(buf), *pv);
}

template<typename _KeyTrait, typename _ValueT>
typename packed_trie_map<_KeyTrait,_ValueT>::search_results
packed_trie_map<_KeyTrait,_ValueT>::prefix_search(const key_type& key) const
{
    using ktt = key_trait_type;
    key_buffer_type buf = ktt::to_key_buffer(key);
    const key_unit_type* p = ktt::buffer_data(buf);
    size_t n = ktt::buffer_size(buf);

    return prefix_search(p, n);
}

template<typename _KeyTrait, typename _ValueT>
typename packed_trie_map<_KeyTrait,_ValueT>::search_results
packed_trie_map<_KeyTrait,_ValueT>::prefix_search(const key_unit_type* prefix, size_type len) const
{
    using ktt = key_trait_type;

    if (m_packed.empty())
        return search_results(nullptr, key_buffer_type());

    const key_unit_type* prefix_end = prefix + len;

    size_t root_offset = m_packed[0];
    assert(root_offset < m_packed.size());
    const uintptr_t* root = m_packed.data() + root_offset;

    const uintptr_t* node = find_prefix_node(root, prefix, prefix_end);
    key_buffer_type buf = ktt::to_key_buffer(prefix, len);
    return search_results(node, std::move(buf));
}

template<typename _KeyTrait, typename _ValueT>
typename packed_trie_map<_KeyTrait,_ValueT>::size_type
packed_trie_map<_KeyTrait,_ValueT>::size() const
{
    return m_entry_size;
}

template<typename _KeyTrait, typename _ValueT>
const uintptr_t* packed_trie_map<_KeyTrait,_ValueT>::find_prefix_node(
    const uintptr_t* p, const key_unit_type* prefix, const key_unit_type* prefix_end) const
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
        key_unit_type node_key = *p_this;
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
void packed_trie_map<_KeyTrait,_ValueT>::find_prefix_node_with_stack(
    node_stack_type& node_stack,
    const uintptr_t* p, const key_unit_type* prefix, const key_unit_type* prefix_end) const
{
    if (prefix == prefix_end)
    {
        size_t index_size = *(p+1);
        const uintptr_t* child_pos = p+2;
        const uintptr_t* child_end = child_pos + index_size;
        node_stack.emplace_back(p, child_pos, child_end);
        return;
    }

    const uintptr_t* p0 = p; // store the head offset position of this node.

    // Find the child node with a matching key character.

    ++p;
    size_t index_size = *p;
    size_t n = index_size / 2;
    ++p;

    if (!n)
    {
        // This is a leaf node.
        node_stack.emplace_back(nullptr, nullptr, nullptr);
        return;
    }

    const uintptr_t* child_end = p + index_size;

    for (size_type low = 0, high = n-1; low <= high; )
    {
        size_type i = (low + high) / 2;

        const uintptr_t* child_pos = p + i*2;
        key_unit_type node_key = *child_pos;
        size_t offset = *(child_pos+1);

        if (*prefix == node_key)
        {
            // Match found!
            node_stack.emplace_back(p0, child_pos, child_end);
            const uintptr_t* p_child = p0 - offset;
            ++prefix;
            find_prefix_node_with_stack(node_stack, p_child, prefix, prefix_end);
            return;
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

    node_stack.emplace_back(nullptr, nullptr, nullptr);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
