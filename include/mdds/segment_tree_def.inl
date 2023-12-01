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

#include <algorithm>

namespace mdds {

namespace st { namespace detail {

template<typename T, typename InserterT>
void descend_tree_for_search(typename T::key_type point, const st::detail::node_base* pnode, InserterT& result)
{
    assert(pnode);

    typedef typename T::node leaf_node;
    typedef typename T::nonleaf_node nonleaf_node;

    if (pnode->is_leaf)
    {
        result(static_cast<const leaf_node*>(pnode)->value_leaf.data_chain.get());
        return;
    }

    const nonleaf_node* pnonleaf = static_cast<const nonleaf_node*>(pnode);
    if (point < pnonleaf->low || pnonleaf->high <= point)
        // Query point is out-of-range.
        return;

    result(pnonleaf->value_nonleaf.data_chain.get());

    // Check the left child node first, then the right one.
    st::detail::node_base* pchild = pnonleaf->left;
    if (!pchild)
        return;

    assert(pnonleaf->right ? pchild->is_leaf == pnonleaf->right->is_leaf : true);

    if (pchild->is_leaf)
    {
        // The child node are leaf nodes.
        if (point < static_cast<const leaf_node*>(pchild)->key)
        {
            // Out-of-range.  Nothing more to do.
            return;
        }

        if (pnonleaf->right)
        {
            assert(pnonleaf->right->is_leaf);
            if (static_cast<const leaf_node*>(pnonleaf->right)->key <= point)
                // Follow the right node.
                pchild = pnonleaf->right;
        }
    }
    else
    {
        // This child nodes are non-leaf nodes.

        const auto* pleaf_left = static_cast<const nonleaf_node*>(pchild);

        if (point < pleaf_left->low)
        {
            // Out-of-range.  Nothing more to do.
            return;
        }
        if (pleaf_left->high <= point && pnonleaf->right)
            // Follow the right child.
            pchild = pnonleaf->right;

        assert(
            static_cast<const nonleaf_node*>(pchild)->low <= point &&
            point < static_cast<const nonleaf_node*>(pchild)->high);
    }

    descend_tree_for_search<T, InserterT>(point, pchild, result);
}

}} // namespace st::detail

template<typename KeyT, typename ValueT>
segment_tree<KeyT, ValueT>::segment_tree() : m_root_node(nullptr), m_valid_tree(false)
{}

template<typename KeyT, typename ValueT>
segment_tree<KeyT, ValueT>::segment_tree(const segment_tree& r)
    : m_segment_data(r.m_segment_data), m_root_node(nullptr), m_valid_tree(r.m_valid_tree)
{
    if (m_valid_tree)
        build_tree();
}

template<typename KeyT, typename ValueT>
segment_tree<KeyT, ValueT>::~segment_tree()
{
    clear_all_nodes();
}

template<typename KeyT, typename ValueT>
bool segment_tree<KeyT, ValueT>::operator==(const segment_tree& r) const
{
    if (m_valid_tree != r.m_valid_tree)
        return false;

    // std::unordered_map's equality should not depend on the order of the stored
    // key-value pairs according to the standard.
    return m_segment_data == r.m_segment_data;
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::build_tree()
{
    build_leaf_nodes();
    m_nonleaf_node_pool.clear();

    // Count the number of leaf nodes.
    size_t leaf_count = st::detail::count_leaf_nodes(m_left_leaf.get(), m_right_leaf.get());

    // Determine the total number of non-leaf nodes needed to build the whole tree.
    size_t nonleaf_count = st::detail::count_needed_nonleaf_nodes(leaf_count);

    m_nonleaf_node_pool.resize(nonleaf_count);

    mdds::st::detail::tree_builder<segment_tree> builder(m_nonleaf_node_pool);
    m_root_node = builder.build(m_left_leaf);

    // Star t "inserting" all segments from the root.

    data_node_map_type tagged_node_map;

    for (const auto& v : m_segment_data)
    {
        value_type value = v.first;
        auto r = tagged_node_map.insert({value, std::make_unique<node_list_type>()});
        assert(r.second);

        node_list_type* plist = r.first->second.get();
        plist->reserve(10);

        descend_tree_and_mark(m_root_node, value, v.second.first, v.second.second, plist);
    }

    m_tagged_node_map.swap(tagged_node_map);
    m_valid_tree = true;
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::descend_tree_and_mark(
    st::detail::node_base* pnode, value_type value, key_type begin_key, key_type end_key, node_list_type* plist)
{
    if (!pnode)
        return;

    if (pnode->is_leaf)
    {
        // This is a leaf node.
        node* pleaf = static_cast<node*>(pnode);
        if (begin_key <= pleaf->key && pleaf->key < end_key)
        {
            leaf_value_type& v = pleaf->value_leaf;
            if (!v.data_chain)
                v.data_chain = std::make_unique<data_chain_type>();
            v.data_chain->push_back(value);
            plist->push_back(pnode);
        }
        return;
    }

    nonleaf_node* pnonleaf = static_cast<nonleaf_node*>(pnode);
    if (end_key < pnonleaf->low || pnonleaf->high <= begin_key)
        return;

    if (begin_key <= pnonleaf->low && pnonleaf->high < end_key)
    {
        // mark this non-leaf node and stop.
        nonleaf_value_type& v = pnonleaf->value_nonleaf;

        if (!v.data_chain)
            v.data_chain = std::make_unique<data_chain_type>();
        v.data_chain->push_back(value);
        plist->push_back(pnode);
        return;
    }

    descend_tree_and_mark(pnonleaf->left, value, begin_key, end_key, plist);
    descend_tree_and_mark(pnonleaf->right, value, begin_key, end_key, plist);
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::build_leaf_nodes()
{
    disconnect_leaf_nodes(m_left_leaf.get(), m_right_leaf.get());

    // Collect all boundary keys.
    std::vector<key_type> keys;
    keys.reserve(m_segment_data.size() * 2);

    for (const auto& v : m_segment_data)
    {
        keys.push_back(v.second.first);
        keys.push_back(v.second.second);
    }

    create_leaf_node_instances(std::move(keys), m_left_leaf, m_right_leaf);
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::create_leaf_node_instances(std::vector<key_type> keys, node_ptr& left, node_ptr& right)
{
    if (keys.empty() || keys.size() < 2)
        // We need at least two keys in order to build tree.
        return;

    // sort and remove duplicates.
    std::sort(keys.begin(), keys.end());
    keys.erase(std::unique(keys.begin(), keys.end()), keys.end());

    auto it = keys.cbegin();

    // left-most node
    left.reset(new node);
    left->key = *it;

    // move on to next.
    left->next.reset(new node);
    node_ptr prev_node = left;
    node_ptr cur_node = left->next;
    cur_node->prev = prev_node;

    for (++it; it != keys.cend(); ++it)
    {
        cur_node->key = *it;

        // move on to next
        cur_node->next.reset(new node);
        prev_node = cur_node;
        cur_node = cur_node->next;
        cur_node->prev = prev_node;
    }

    // Remove the excess node.
    prev_node->next.reset();
    right = prev_node;
}

template<typename KeyT, typename ValueT>
bool segment_tree<KeyT, ValueT>::insert(key_type begin_key, key_type end_key, value_type value)
{
    if (begin_key >= end_key)
        return false;

    if (m_segment_data.find(value) != m_segment_data.end())
        // Insertion of duplicate data is not allowed.
        return false;

    ::std::pair<key_type, key_type> range;
    range.first = begin_key;
    range.second = end_key;
    m_segment_data.insert(typename segment_map_type::value_type(value, range));

    m_valid_tree = false;
    return true;
}

template<typename KeyT, typename ValueT>
typename segment_tree<KeyT, ValueT>::search_results segment_tree<KeyT, ValueT>::search(key_type point) const
{
    search_results result;
    if (!m_valid_tree || !m_root_node)
        return result;

    search_result_inserter result_inserter(result);
    typedef segment_tree<KeyT, ValueT> tree_type;
    st::detail::descend_tree_for_search<tree_type, search_result_inserter>(point, m_root_node, result_inserter);

    return result;
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::remove(value_type value)
{
    using namespace std;

    typename data_node_map_type::iterator itr = m_tagged_node_map.find(value);
    if (itr != m_tagged_node_map.end())
    {
        // Tagged node list found.  Remove all the tags from the tree nodes.
        node_list_type* plist = itr->second.get();
        if (!plist)
            return;

        remove_data_from_nodes(plist, value);

        // Remove the tags associated with this pointer from the data set.
        m_tagged_node_map.erase(itr);
    }

    // Remove from the segment data array.
    m_segment_data.erase(value);
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::clear()
{
    m_tagged_node_map.clear();
    m_segment_data.clear();
    clear_all_nodes();
    m_valid_tree = false;
}

template<typename KeyT, typename ValueT>
size_t segment_tree<KeyT, ValueT>::size() const
{
    return m_segment_data.size();
}

template<typename KeyT, typename ValueT>
bool segment_tree<KeyT, ValueT>::empty() const
{
    return m_segment_data.empty();
}

template<typename KeyT, typename ValueT>
size_t segment_tree<KeyT, ValueT>::leaf_size() const
{
    return st::detail::count_leaf_nodes(m_left_leaf.get(), m_right_leaf.get());
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::remove_data_from_nodes(node_list_type* plist, const value_type value)
{
    typename node_list_type::iterator itr = plist->begin(), itr_end = plist->end();
    for (; itr != itr_end; ++itr)
    {
        data_chain_type* chain = nullptr;
        st::detail::node_base* p = *itr;
        if (p->is_leaf)
            chain = static_cast<node*>(p)->value_leaf.data_chain.get();
        else
            chain = static_cast<nonleaf_node*>(p)->value_nonleaf.data_chain.get();

        if (!chain)
            continue;

        remove_data_from_chain(*chain, value);
    }
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::remove_data_from_chain(data_chain_type& chain, const value_type value)
{
    typename data_chain_type::iterator itr = ::std::find(chain.begin(), chain.end(), value);
    if (itr != chain.end())
    {
        *itr = chain.back();
        chain.pop_back();
    }
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::clear_all_nodes()
{
    disconnect_leaf_nodes(m_left_leaf.get(), m_right_leaf.get());
    m_nonleaf_node_pool.clear();
    m_left_leaf.reset();
    m_right_leaf.reset();
    m_root_node = nullptr;
}

#ifdef MDDS_UNIT_TEST
template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::dump_tree() const
{
    using ::std::cout;
    using ::std::endl;

    if (!m_valid_tree)
        assert(!"attempted to dump an invalid tree!");

    cout << "dump tree ------------------------------------------------------" << endl;
    size_t node_count = mdds::st::detail::tree_dumper<node, nonleaf_node>::dump(m_root_node);
    size_t node_instance_count = node::get_instance_count();

    cout << "tree node count = " << node_count << "    node instance count = " << node_instance_count << endl;
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::dump_leaf_nodes() const
{
    using ::std::cout;
    using ::std::endl;

    cout << "dump leaf nodes ------------------------------------------------" << endl;

    node* p = m_left_leaf.get();
    while (p)
    {
        print_leaf_value(*p);
        p = p->next.get();
    }
    cout << "  node instance count = " << node::get_instance_count() << endl;
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::dump_segment_data() const
{
    std::cout << "dump segment data ----------------------------------------------" << std::endl;

    for (const auto& v : m_segment_data)
        std::cout << v.second.first << "-" << v.second.second << ": " << v.first << std::endl;
}

template<typename KeyT, typename ValueT>
bool segment_tree<KeyT, ValueT>::verify_node_lists() const
{
    using namespace std;

    typename data_node_map_type::const_iterator itr = m_tagged_node_map.begin(), itr_end = m_tagged_node_map.end();
    for (; itr != itr_end; ++itr)
    {
        // Print stored nodes.
        cout << "node list " << itr->first->name << ": ";
        const node_list_type* plist = itr->second.get();
        assert(plist);

        for (const st::detail::node_base* p : *plist)
        {
            if (p->is_leaf)
                std::cout << static_cast<const node*>(p)->to_string() << " ";
            else
                std::cout << static_cast<const nonleaf_node*>(p)->to_string() << " ";
        }
        cout << endl;

        // Verify that all of these nodes have the data pointer.
        if (!has_data_pointer(*plist, itr->first))
            return false;
    }
    return true;
}

template<typename KeyT, typename ValueT>
bool segment_tree<KeyT, ValueT>::verify_leaf_nodes(const ::std::vector<leaf_node_check>& checks) const
{
    using namespace std;

    node* cur_node = m_left_leaf.get();
    typename ::std::vector<leaf_node_check>::const_iterator itr = checks.begin(), itr_end = checks.end();
    for (; itr != itr_end; ++itr)
    {
        if (!cur_node)
            // Position past the right-mode node.  Invalid.
            return false;

        if (cur_node->key != itr->key)
            // Key values differ.
            return false;

        if (itr->data_chain.empty())
        {
            if (cur_node->value_leaf.data_chain)
                // The data chain should be empty (i.e. the pointer should be nullptr).
                return false;
        }
        else
        {
            if (!cur_node->value_leaf.data_chain)
                // This node should have data pointers!
                return false;

            data_chain_type chain1 = itr->data_chain;
            data_chain_type chain2 = *cur_node->value_leaf.data_chain;

            if (chain1.size() != chain2.size())
                return false;

            ::std::vector<value_type> test1, test2;
            test1.reserve(chain1.size());
            test2.reserve(chain2.size());
            copy(chain1.begin(), chain1.end(), back_inserter(test1));
            copy(chain2.begin(), chain2.end(), back_inserter(test2));

            // Sort both arrays before comparing them.
            sort(test1.begin(), test1.end());
            sort(test2.begin(), test2.end());

            if (test1 != test2)
                return false;
        }

        cur_node = cur_node->next.get();
    }

    if (cur_node)
        // At this point, we expect the current node to be at the position
        // past the right-most node, which is nullptr.
        return false;

    return true;
}

template<typename KeyT, typename ValueT>
bool segment_tree<KeyT, ValueT>::has_data_pointer(const node_list_type& node_list, const value_type value)
{
    using namespace std;

    typename node_list_type::const_iterator itr = node_list.begin(), itr_end = node_list.end();

    for (; itr != itr_end; ++itr)
    {
        // Check each node, and make sure each node has the value pointer
        // listed.
        const st::detail::node_base* pnode = *itr;
        const data_chain_type* chain = nullptr;
        if (pnode->is_leaf)
            chain = static_cast<const node*>(pnode)->value_leaf.data_chain.get();
        else
            chain = static_cast<const nonleaf_node*>(pnode)->value_nonleaf.data_chain.get();

        if (!chain)
            return false;

        if (find(chain->begin(), chain->end(), value) == chain->end())
            return false;
    }
    return true;
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::print_leaf_value(const node& n)
{
    cout << n.to_string() << ": {";

    if (n.value_leaf.data_chain)
    {
        for (const auto& v : *n.value_leaf.data_chain)
            std::cout << v << ", ";
    }
    cout << "}" << endl;
}
#endif

} // namespace mdds
