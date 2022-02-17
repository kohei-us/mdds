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

namespace __st {

template<typename T, typename _Inserter>
void descend_tree_for_search(typename T::key_type point, const __st::node_base* pnode, _Inserter& result)
{
    typedef typename T::node leaf_node;
    typedef typename T::nonleaf_node nonleaf_node;

    typedef typename T::nonleaf_value_type nonleaf_value_type;
    typedef typename T::leaf_value_type leaf_value_type;

    if (!pnode)
        // This should never happen, but just in case.
        return;

    if (pnode->is_leaf)
    {
        result(static_cast<const leaf_node*>(pnode)->value_leaf.data_chain);
        return;
    }

    const nonleaf_node* pnonleaf = static_cast<const nonleaf_node*>(pnode);
    const nonleaf_value_type& v = pnonleaf->value_nonleaf;
    if (point < v.low || v.high <= point)
        // Query point is out-of-range.
        return;

    result(v.data_chain);

    // Check the left child node first, then the right one.
    __st::node_base* pchild = pnonleaf->left;
    if (!pchild)
        return;

    assert(pnonleaf->right ? pchild->is_leaf == pnonleaf->right->is_leaf : true);

    if (pchild->is_leaf)
    {
        // The child node are leaf nodes.
        const leaf_value_type& vleft = static_cast<const leaf_node*>(pchild)->value_leaf;
        if (point < vleft.key)
        {
            // Out-of-range.  Nothing more to do.
            return;
        }

        if (pnonleaf->right)
        {
            assert(pnonleaf->right->is_leaf);
            const leaf_value_type& vright = static_cast<const leaf_node*>(pnonleaf->right)->value_leaf;
            if (vright.key <= point)
                // Follow the right node.
                pchild = pnonleaf->right;
        }
    }
    else
    {
        // This child nodes are non-leaf nodes.

        const nonleaf_value_type& vleft = static_cast<const nonleaf_node*>(pchild)->value_nonleaf;

        if (point < vleft.low)
        {
            // Out-of-range.  Nothing more to do.
            return;
        }
        if (vleft.high <= point && pnonleaf->right)
            // Follow the right child.
            pchild = pnonleaf->right;

        assert(
            static_cast<const nonleaf_node*>(pchild)->value_nonleaf.low <= point &&
            point < static_cast<const nonleaf_node*>(pchild)->value_nonleaf.high);
    }

    descend_tree_for_search<T, _Inserter>(point, pchild, result);
}

} // namespace __st

template<typename _Key, typename _Value>
segment_tree<_Key, _Value>::segment_tree() : m_root_node(nullptr), m_valid_tree(false)
{}

template<typename _Key, typename _Value>
segment_tree<_Key, _Value>::segment_tree(const segment_tree& r)
    : m_segment_data(r.m_segment_data), m_root_node(nullptr), m_valid_tree(r.m_valid_tree)
{
    if (m_valid_tree)
        build_tree();
}

template<typename _Key, typename _Value>
segment_tree<_Key, _Value>::~segment_tree()
{
    clear_all_nodes();
}

template<typename _Key, typename _Value>
bool segment_tree<_Key, _Value>::operator==(const segment_tree& r) const
{
    if (m_valid_tree != r.m_valid_tree)
        return false;

    // Sort the data by key values first.
    sorted_segment_map_type seg1(m_segment_data.begin(), m_segment_data.end());
    sorted_segment_map_type seg2(r.m_segment_data.begin(), r.m_segment_data.end());
    typename sorted_segment_map_type::const_iterator itr1 = seg1.begin(), itr1_end = seg1.end();
    typename sorted_segment_map_type::const_iterator itr2 = seg2.begin(), itr2_end = seg2.end();

    for (; itr1 != itr1_end; ++itr1, ++itr2)
    {
        if (itr2 == itr2_end)
            return false;

        if (*itr1 != *itr2)
            return false;
    }
    if (itr2 != itr2_end)
        return false;

    return true;
}

template<typename _Key, typename _Value>
void segment_tree<_Key, _Value>::build_tree()
{
    build_leaf_nodes();
    m_nonleaf_node_pool.clear();

    // Count the number of leaf nodes.
    size_t leaf_count = __st::count_leaf_nodes(m_left_leaf.get(), m_right_leaf.get());

    // Determine the total number of non-leaf nodes needed to build the whole tree.
    size_t nonleaf_count = __st::count_needed_nonleaf_nodes(leaf_count);

    m_nonleaf_node_pool.resize(nonleaf_count);

    mdds::__st::tree_builder<segment_tree> builder(m_nonleaf_node_pool);
    m_root_node = builder.build(m_left_leaf);

    // Start "inserting" all segments from the root.
    typename segment_map_type::const_iterator itr, itr_beg = m_segment_data.begin(), itr_end = m_segment_data.end();

    data_node_map_type tagged_node_map;
    for (itr = itr_beg; itr != itr_end; ++itr)
    {
        value_type pdata = itr->first;
        auto r =
            tagged_node_map.insert(typename data_node_map_type::value_type(pdata, std::make_unique<node_list_type>()));

        node_list_type* plist = r.first->second.get();
        plist->reserve(10);

        descend_tree_and_mark(m_root_node, pdata, itr->second.first, itr->second.second, plist);
    }

    m_tagged_node_map.swap(tagged_node_map);
    m_valid_tree = true;
}

template<typename _Key, typename _Value>
void segment_tree<_Key, _Value>::descend_tree_and_mark(
    __st::node_base* pnode, value_type pdata, key_type begin_key, key_type end_key, node_list_type* plist)
{
    if (!pnode)
        return;

    if (pnode->is_leaf)
    {
        // This is a leaf node.
        node* pleaf = static_cast<node*>(pnode);
        if (begin_key <= pleaf->value_leaf.key && pleaf->value_leaf.key < end_key)
        {
            leaf_value_type& v = pleaf->value_leaf;
            if (!v.data_chain)
                v.data_chain = new data_chain_type;
            v.data_chain->push_back(pdata);
            plist->push_back(pnode);
        }
        return;
    }

    nonleaf_node* pnonleaf = static_cast<nonleaf_node*>(pnode);
    if (end_key < pnonleaf->value_nonleaf.low || pnonleaf->value_nonleaf.high <= begin_key)
        return;

    nonleaf_value_type& v = pnonleaf->value_nonleaf;
    if (begin_key <= v.low && v.high < end_key)
    {
        // mark this non-leaf node and stop.
        if (!v.data_chain)
            v.data_chain = new data_chain_type;
        v.data_chain->push_back(pdata);
        plist->push_back(pnode);
        return;
    }

    descend_tree_and_mark(pnonleaf->left, pdata, begin_key, end_key, plist);
    descend_tree_and_mark(pnonleaf->right, pdata, begin_key, end_key, plist);
}

template<typename _Key, typename _Value>
void segment_tree<_Key, _Value>::build_leaf_nodes()
{
    using namespace std;

    disconnect_leaf_nodes(m_left_leaf.get(), m_right_leaf.get());

    // In 1st pass, collect unique end-point values and sort them.
    vector<key_type> keys_uniq;
    keys_uniq.reserve(m_segment_data.size() * 2);
    typename segment_map_type::const_iterator itr, itr_beg = m_segment_data.begin(), itr_end = m_segment_data.end();
    for (itr = itr_beg; itr != itr_end; ++itr)
    {
        keys_uniq.push_back(itr->second.first);
        keys_uniq.push_back(itr->second.second);
    }

    // sort and remove duplicates.
    sort(keys_uniq.begin(), keys_uniq.end());
    keys_uniq.erase(unique(keys_uniq.begin(), keys_uniq.end()), keys_uniq.end());

    create_leaf_node_instances(keys_uniq, m_left_leaf, m_right_leaf);
}

template<typename _Key, typename _Value>
void segment_tree<_Key, _Value>::create_leaf_node_instances(
    const ::std::vector<key_type>& keys, node_ptr& left, node_ptr& right)
{
    if (keys.empty() || keys.size() < 2)
        // We need at least two keys in order to build tree.
        return;

    typename ::std::vector<key_type>::const_iterator itr = keys.begin(), itr_end = keys.end();

    // left-most node
    left.reset(new node);
    left->value_leaf.key = *itr;

    // move on to next.
    left->next.reset(new node);
    node_ptr prev_node = left;
    node_ptr cur_node = left->next;
    cur_node->prev = prev_node;

    for (++itr; itr != itr_end; ++itr)
    {
        cur_node->value_leaf.key = *itr;

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

template<typename _Key, typename _Value>
bool segment_tree<_Key, _Value>::insert(key_type begin_key, key_type end_key, value_type pdata)
{
    if (begin_key >= end_key)
        return false;

    if (m_segment_data.find(pdata) != m_segment_data.end())
        // Insertion of duplicate data is not allowed.
        return false;

    ::std::pair<key_type, key_type> range;
    range.first = begin_key;
    range.second = end_key;
    m_segment_data.insert(typename segment_map_type::value_type(pdata, range));

    m_valid_tree = false;
    return true;
}

template<typename _Key, typename _Value>
bool segment_tree<_Key, _Value>::search(key_type point, search_results_type& result) const
{
    if (!m_valid_tree)
        // Tree is invalidated.
        return false;

    if (!m_root_node)
        // Tree doesn't exist.  Since the tree is flagged valid, this means no
        // segments have been inserted.
        return true;

    search_result_vector_inserter result_inserter(result);
    typedef segment_tree<_Key, _Value> tree_type;
    __st::descend_tree_for_search<tree_type, search_result_vector_inserter>(point, m_root_node, result_inserter);
    return true;
}

template<typename _Key, typename _Value>
typename segment_tree<_Key, _Value>::search_results segment_tree<_Key, _Value>::search(key_type point) const
{
    search_results result;
    if (!m_valid_tree || !m_root_node)
        return result;

    search_result_inserter result_inserter(result);
    typedef segment_tree<_Key, _Value> tree_type;
    __st::descend_tree_for_search<tree_type, search_result_inserter>(point, m_root_node, result_inserter);

    return result;
}

template<typename _Key, typename _Value>
void segment_tree<_Key, _Value>::search(key_type point, search_results_base& result) const
{
    if (!m_valid_tree || !m_root_node)
        return;

    search_result_inserter result_inserter(result);
    typedef segment_tree<_Key, _Value> tree_type;
    __st::descend_tree_for_search<tree_type>(point, m_root_node, result_inserter);
}

template<typename _Key, typename _Value>
void segment_tree<_Key, _Value>::remove(value_type value)
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

template<typename _Key, typename _Value>
void segment_tree<_Key, _Value>::clear()
{
    m_tagged_node_map.clear();
    m_segment_data.clear();
    clear_all_nodes();
    m_valid_tree = false;
}

template<typename _Key, typename _Value>
size_t segment_tree<_Key, _Value>::size() const
{
    return m_segment_data.size();
}

template<typename _Key, typename _Value>
bool segment_tree<_Key, _Value>::empty() const
{
    return m_segment_data.empty();
}

template<typename _Key, typename _Value>
size_t segment_tree<_Key, _Value>::leaf_size() const
{
    return __st::count_leaf_nodes(m_left_leaf.get(), m_right_leaf.get());
}

template<typename _Key, typename _Value>
void segment_tree<_Key, _Value>::remove_data_from_nodes(node_list_type* plist, const value_type pdata)
{
    typename node_list_type::iterator itr = plist->begin(), itr_end = plist->end();
    for (; itr != itr_end; ++itr)
    {
        data_chain_type* chain = nullptr;
        __st::node_base* p = *itr;
        if (p->is_leaf)
            chain = static_cast<node*>(p)->value_leaf.data_chain;
        else
            chain = static_cast<nonleaf_node*>(p)->value_nonleaf.data_chain;

        if (!chain)
            continue;

        remove_data_from_chain(*chain, pdata);
    }
}

template<typename _Key, typename _Value>
void segment_tree<_Key, _Value>::remove_data_from_chain(data_chain_type& chain, const value_type pdata)
{
    typename data_chain_type::iterator itr = ::std::find(chain.begin(), chain.end(), pdata);
    if (itr != chain.end())
    {
        *itr = chain.back();
        chain.pop_back();
    }
}

template<typename _Key, typename _Value>
void segment_tree<_Key, _Value>::clear_all_nodes()
{
    disconnect_leaf_nodes(m_left_leaf.get(), m_right_leaf.get());
    m_nonleaf_node_pool.clear();
    m_left_leaf.reset();
    m_right_leaf.reset();
    m_root_node = nullptr;
}

#ifdef MDDS_UNIT_TEST
template<typename _Key, typename _Value>
void segment_tree<_Key, _Value>::dump_tree() const
{
    using ::std::cout;
    using ::std::endl;

    if (!m_valid_tree)
        assert(!"attempted to dump an invalid tree!");

    cout << "dump tree ------------------------------------------------------" << endl;
    size_t node_count = mdds::__st::tree_dumper<node, nonleaf_node>::dump(m_root_node);
    size_t node_instance_count = node::get_instance_count();

    cout << "tree node count = " << node_count << "    node instance count = " << node_instance_count << endl;
}

template<typename _Key, typename _Value>
void segment_tree<_Key, _Value>::dump_leaf_nodes() const
{
    using ::std::cout;
    using ::std::endl;

    cout << "dump leaf nodes ------------------------------------------------" << endl;

    node* p = m_left_leaf.get();
    while (p)
    {
        print_leaf_value(p->value_leaf);
        p = p->next.get();
    }
    cout << "  node instance count = " << node::get_instance_count() << endl;
}

template<typename _Key, typename _Value>
void segment_tree<_Key, _Value>::dump_segment_data() const
{
    using namespace std;
    cout << "dump segment data ----------------------------------------------" << endl;

    segment_map_printer func;
    for_each(m_segment_data.begin(), m_segment_data.end(), func);
}

template<typename _Key, typename _Value>
bool segment_tree<_Key, _Value>::verify_node_lists() const
{
    using namespace std;

    typename data_node_map_type::const_iterator itr = m_tagged_node_map.begin(), itr_end = m_tagged_node_map.end();
    for (; itr != itr_end; ++itr)
    {
        // Print stored nodes.
        cout << "node list " << itr->first->name << ": ";
        const node_list_type* plist = itr->second.get();
        assert(plist);
        node_printer func;
        for_each(plist->begin(), plist->end(), func);
        cout << endl;

        // Verify that all of these nodes have the data pointer.
        if (!has_data_pointer(*plist, itr->first))
            return false;
    }
    return true;
}

template<typename _Key, typename _Value>
bool segment_tree<_Key, _Value>::verify_leaf_nodes(const ::std::vector<leaf_node_check>& checks) const
{
    using namespace std;

    node* cur_node = m_left_leaf.get();
    typename ::std::vector<leaf_node_check>::const_iterator itr = checks.begin(), itr_end = checks.end();
    for (; itr != itr_end; ++itr)
    {
        if (!cur_node)
            // Position past the right-mode node.  Invalid.
            return false;

        if (cur_node->value_leaf.key != itr->key)
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

template<typename _Key, typename _Value>
bool segment_tree<_Key, _Value>::verify_segment_data(const segment_map_type& checks) const
{
    // Sort the data by key values first.
    sorted_segment_map_type seg1(checks.begin(), checks.end());
    sorted_segment_map_type seg2(m_segment_data.begin(), m_segment_data.end());

    typename sorted_segment_map_type::const_iterator itr1 = seg1.begin(), itr1_end = seg1.end();
    typename sorted_segment_map_type::const_iterator itr2 = seg2.begin(), itr2_end = seg2.end();
    for (; itr1 != itr1_end; ++itr1, ++itr2)
    {
        if (itr2 == itr2_end)
            return false;

        if (*itr1 != *itr2)
            return false;
    }
    if (itr2 != itr2_end)
        return false;

    return true;
}

template<typename _Key, typename _Value>
bool segment_tree<_Key, _Value>::has_data_pointer(const node_list_type& node_list, const value_type pdata)
{
    using namespace std;

    typename node_list_type::const_iterator itr = node_list.begin(), itr_end = node_list.end();

    for (; itr != itr_end; ++itr)
    {
        // Check each node, and make sure each node has the pdata pointer
        // listed.
        const __st::node_base* pnode = *itr;
        const data_chain_type* chain = nullptr;
        if (pnode->is_leaf)
            chain = static_cast<const node*>(pnode)->value_leaf.data_chain;
        else
            chain = static_cast<const nonleaf_node*>(pnode)->value_nonleaf.data_chain;

        if (!chain)
            return false;

        if (find(chain->begin(), chain->end(), pdata) == chain->end())
            return false;
    }
    return true;
}

template<typename _Key, typename _Value>
void segment_tree<_Key, _Value>::print_leaf_value(const leaf_value_type& v)
{
    using namespace std;
    cout << v.key << ": { ";
    if (v.data_chain)
    {
        const data_chain_type* pchain = v.data_chain;
        typename data_chain_type::const_iterator itr, itr_beg = pchain->begin(), itr_end = pchain->end();
        for (itr = itr_beg; itr != itr_end; ++itr)
        {
            if (itr != itr_beg)
                cout << ", ";
            cout << (*itr)->name;
        }
    }
    cout << " }" << endl;
}
#endif

} // namespace mdds
