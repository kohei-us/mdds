/*************************************************************************
 *
 * Copyright (c) 2010 Kohei Yoshida
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

#ifndef __MDDS_SEGMENTTREE_HPP__
#define __MDDS_SEGMENTTREE_HPP__

#include "node.hpp"

#include <vector>
#include <list>
#include <iostream>
#include <unordered_map>

#include <boost/ptr_container/ptr_map.hpp>

#ifdef UNIT_TEST
#include <sstream>
#define private public // We need to expose the private members during unit test.
#endif

namespace mdds {

template<typename _Key, typename _Data>
class segment_tree
{
public:
    typedef _Key        key_type;
    typedef _Data       data_type;
    typedef ::std::vector<const data_type*> data_chain_type;
    typedef ::std::vector<const data_type*> search_result_type;

private:

    typedef ::std::unordered_map<data_type*, ::std::pair<key_type, key_type> > segment_map_type;

#ifdef UNIT_TEST
    struct segment_data
    {
        key_type    begin_key;
        key_type    end_key;
        data_type*  pdata;

        segment_data(key_type _beg, key_type _end, data_type* p) :
            begin_key(_beg), end_key(_end), pdata(p) {}

        bool operator==(const segment_data& r) const
        {
            return begin_key == r.begin_key && end_key == r.end_key && pdata == r.pdata;
        }

        bool operator!=(const segment_data& r) const
        {
            return !operator==(r);
        }
    };

    struct segment_map_printer : public ::std::unary_function< ::std::pair<data_type*, ::std::pair<key_type, key_type> >, void>
    {
        void operator() (const ::std::pair<data_type*, ::std::pair<key_type, key_type> >& r) const
        {
            using namespace std;
            cout << r.second.first << "-" << r.second.second << ": " << r.first->name << endl;
        }
    };
#endif

    struct nonleaf_value_type
    {
        key_type low;   /// low range value (inclusive)
        key_type high;  /// high range value (non-inclusive)
        data_chain_type* data_labels;
    };

    struct leaf_value_type
    {
        key_type key;
        data_chain_type* data_chain;
    };

    struct node;
    typedef ::boost::intrusive_ptr<node> node_ptr;

    struct node : public node_base<node_ptr, node>
    {
        union {
            nonleaf_value_type  value_nonleaf;
            leaf_value_type     value_leaf;
        };

        node(bool _is_leaf) :
            node_base<node_ptr, node>(_is_leaf)
        {
            if (_is_leaf)
                value_leaf.data_chain = NULL;
            else
                value_nonleaf.data_labels = NULL;
        }

        node(const node& r) :
            node_base<node_ptr, node>(r)
        {
        }

        void dispose()
        {
            if (this->is_leaf)
                delete value_leaf.data_chain;
            else
                delete value_nonleaf.data_labels;
        }

        bool equals(const node& r) const
        {
            if (this->is_leaf != r.is_leaf)
                return false;

            return true;
        }

        void fill_nonleaf_value(const node_ptr& left_node, const node_ptr& right_node)
        {
            // Parent node should carry the range of all of its child nodes.
            if (left_node)
                value_nonleaf.low  = left_node->is_leaf ? left_node->value_leaf.key : left_node->value_nonleaf.low;
            else
                // Having a left node is prerequisite.
                return;

            if (right_node)
            {
                if (right_node->is_leaf)
                {
                    // When the child nodes are leaf nodes, the upper bound
                    // must be the value of the node that comes after the
                    // right leaf node (if such node exists).

                    if (right_node->right)
                        value_nonleaf.high = right_node->right->value_leaf.key;
                    else
                        value_nonleaf.high = right_node->value_leaf.key;
                }
                else
                {
                    value_nonleaf.high = right_node->value_nonleaf.high;
                }
            }
            else
                value_nonleaf.high = left_node->is_leaf ? left_node->value_leaf.key : left_node->value_nonleaf.high;
        }

#ifdef UNIT_TEST
        void dump_value() const
        {
            ::std::cout << print();
        }

        ::std::string print() const
        {
            ::std::ostringstream os;
            if (this->is_leaf)
            {
                os << "[" << value_leaf.key << "]";
            }
            else
            {
                os << "[" << value_nonleaf.low << "-" << value_nonleaf.high << ")";
                if (value_nonleaf.data_labels)
                {
                    os << " { ";
                    typename data_chain_type::const_iterator itr, itr_beg = value_nonleaf.data_labels->begin(), itr_end = value_nonleaf.data_labels->end();
                    for (itr = itr_beg; itr != itr_end; ++itr)
                    {
                        if (itr != itr_beg)
                            os << ", ";
                        os << (*itr)->name;
                    }
                    os << " }";
                }
            }
            os << " ";
            return os.str();
        }

        struct printer : public ::std::unary_function<const node*, void>
        {
            void operator() (const node* p) const
            {
                ::std::cout << p->print() << " ";
            }
        };
#endif
    };

public:
    segment_tree();
    segment_tree(const segment_tree& r);
    ~segment_tree();

    /** 
     * Equality between two segment_tree instances is evaluated by comparing 
     * the segments that they store.  The trees are not compared.
     */
    bool operator==(const segment_tree& r) const;

    bool operator!=(const segment_tree& r) const
    {
        return !operator==(r);
    }

    /** 
     * Check whether or not the internal tree is in a valid state.  The tree 
     * must be valid in order to perform searches. 
     * 
     * @return true if the tree is valid, false otherwise.
     */
    bool is_tree_valid() const { return m_valid_tree; }

    /** 
     * Build or re-build tree based on the current set of segments.
     */
    void build_tree();

    /** 
     * Insert a new segment.
     *  
     * @param begin_key begin point of the segment.  The value is inclusive.
     * @param end_key end point of the segment.  The value is non-inclusive. 
     * @param pdata pointer to the data instance associated with this segment. 
     *               Note that <i>the caller must manage the life cycle of the
     *               data instance</i>.
     */
    bool insert(key_type begin_key, key_type end_key, data_type* pdata);

    /** 
     * Search the tree and collect all segments that include a specified 
     * point. 
     *  
     * @param point specified point value 
     * @param result doubly-linked list of data instances associated with 
     *                   the segments that include the specified point.
     *                   <i>Note that the search result gets appended to the
     *                   list; the list will not get emptied on each
     *                   search.</i>  It is caller's responsibility to empty
     *                   the list before passing it to this method in case the
     *                   caller so desires.
     *  
     * @return true if the search is performed successfully, false if the 
     *         search has ended prematurely due to error conditions.
     */
    bool search(key_type point, search_result_type& result) const;

    /** 
     * Remove a segment by the data pointer.  This will <i>not</i> invalidate 
     * the tree; however, if you have removed lots of segments, you might want 
     * to re-build the tree to shrink its size. 
     */
    void remove(data_type* pdata);

    /** 
     * Remove all segments data.
     */
    void clear();

    /** 
     * Return the number of segments currently stored in this container.
     */
    size_t size() const;

    /** 
     * Return whether or not the container stores any segments or none at all.
     */
    bool empty() const;

#ifdef UNIT_TEST
    void dump_tree() const;
    void dump_leaf_nodes() const;
    void dump_segment_data() const;
    bool verify_node_lists() const;

    struct leaf_node_check
    {
        key_type key;
        data_chain_type data_chain;
    };

    bool verify_leaf_nodes(const ::std::vector<leaf_node_check>& checks) const;

    /** 
     * Verify the validity of the segment data array.
     *  
     * @param checks null-terminated array of expected values.  The last item 
     *               must have a NULL pdata value to terminate the array.
     */
    bool verify_segment_data(const segment_map_type& checks) const;
#endif

private:

    typedef ::std::vector<node*> node_list_type;
    typedef ::boost::ptr_map<data_type*, node_list_type> data_node_map_type;

    static void create_leaf_node_instances(const ::std::vector<key_type>& keys, node_ptr& left, node_ptr& right);

    /** 
     * Descend the tree from the root node, and mark appropriate nodes, both 
     * leaf and non-leaf, based on segment's end points.  When marking nodes, 
     * record their positions as a list of node pointers. 
     */
    void descend_tree_and_mark(node* pnode, data_type* pdata, key_type begin_key, key_type end_key, node_list_type* plist);

    void build_leaf_nodes();
    void descend_tree_for_search(key_type point, const node* pnode, search_result_type& data_chain) const;
    void append_search_result(search_result_type& data_chain, const data_chain_type* node_data) const;

    /** 
     * Go through the list of nodes, and remove the specified data pointer 
     * value from the nodes.
     */
    void remove_data_from_nodes(node_list_type* plist, const data_type* pdata);
    void remove_data_from_chain(data_chain_type& chain, const data_type* pdata);

    void clear_all_nodes();

#ifdef UNIT_TEST
    static bool has_data_pointer(const node_list_type& node_list, const data_type* pdata);
    static void print_leaf_value(const leaf_value_type& v);
#endif

private:
    segment_map_type m_segment_data;

    /** 
     * For each data pointer, it keeps track of all nodes, leaf or non-leaf, 
     * that stores the data pointer label.  This data is used when removing 
     * segments by the data pointer value. 
     */
    data_node_map_type m_tagged_node_map;

    node_ptr   m_root_node;
    node_ptr   m_left_leaf;
    node_ptr   m_right_leaf;
    bool m_valid_tree:1;
};

template<typename _Key, typename _Data>
segment_tree<_Key, _Data>::segment_tree() :
    m_valid_tree(false)
{
}

template<typename _Key, typename _Data>
segment_tree<_Key, _Data>::segment_tree(const segment_tree& r) :
    m_segment_data(r.m_segment_data),
    m_valid_tree(r.m_valid_tree)
{
    if (m_valid_tree)
        build_tree();
}

template<typename _Key, typename _Data>
segment_tree<_Key, _Data>::~segment_tree()
{
    clear_all_nodes();
}

template<typename _Key, typename _Data>
bool segment_tree<_Key, _Data>::operator==(const segment_tree& r) const
{
    if (m_valid_tree != r.m_valid_tree)
        return false;

    // First, we need to re-organize the segment data so that they are sorted
    // by the data pointer values.

    typename segment_map_type::const_iterator itr1 = m_segment_data.begin(), itr1_end = m_segment_data.end();
    typename segment_map_type::const_iterator itr2 = r.m_segment_data.begin(), itr2_end = r.m_segment_data.end();

    for (; itr1 != itr1_end; ++itr1, ++itr2)
    {
        if (itr2 == itr2_end)
            return false;

        if (itr1->first != itr2->first)
            return false;

        if (itr1->second != itr2->second)
            return false;
    }
    if (itr2 != itr2_end)
        return false;

    return true;
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::build_tree()
{
    build_leaf_nodes();
    clear_tree(m_root_node.get());
    m_root_node = ::mdds::build_tree<node_ptr, node>(m_left_leaf);
    
    // Start "inserting" all segments from the root.
    typename segment_map_type::const_iterator itr, 
        itr_beg = m_segment_data.begin(), itr_end = m_segment_data.end();

    data_node_map_type tagged_node_map;
    for (itr = itr_beg; itr != itr_end; ++itr)
    {
        data_type* pdata = itr->first;
        ::std::pair<typename data_node_map_type::iterator, bool> r = 
            tagged_node_map.insert(pdata, new node_list_type);
        node_list_type* plist = r.first->second;
        plist->reserve(10);

        descend_tree_and_mark(m_root_node.get(), pdata, itr->second.first, itr->second.second, plist);
    }

    m_tagged_node_map.swap(tagged_node_map);
    m_valid_tree = true;
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::descend_tree_and_mark(
    node* pnode, data_type* pdata, key_type begin_key, key_type end_key, node_list_type* plist)
{
    if (!pnode)
        return;

    if (pnode->is_leaf)
    {
        // This is a leaf node.
        if (pnode->value_leaf.key == begin_key)
        {
            // Insertion of begin point.
            leaf_value_type& v = pnode->value_leaf;
            if (!v.data_chain)
                v.data_chain = new data_chain_type;
            v.data_chain->push_back(pdata);
            plist->push_back(pnode);
        }
        else if (begin_key <= pnode->value_leaf.key && pnode->value_leaf.key < end_key)
        {
            leaf_value_type& v = pnode->value_leaf;
            if (!v.data_chain)
                v.data_chain = new data_chain_type;
            v.data_chain->push_back(pdata);
            plist->push_back(pnode);
        }
        return;
    }
    
    if (end_key < pnode->value_nonleaf.low || pnode->value_nonleaf.high <= begin_key)
        return;

    nonleaf_value_type& v = pnode->value_nonleaf;
    if (begin_key <= v.low && v.high < end_key)
    {
        // mark this non-leaf node and stop.
        if (!v.data_labels)
            v.data_labels = new data_chain_type;
        v.data_labels->push_back(pdata);
        plist->push_back(pnode);
        return;
    }

    descend_tree_and_mark(pnode->left.get(), pdata, begin_key, end_key, plist);
    descend_tree_and_mark(pnode->right.get(), pdata, begin_key, end_key, plist);
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::build_leaf_nodes()
{
    using namespace std;

    disconnect_leaf_nodes(m_left_leaf.get(), m_right_leaf.get());

    // In 1st pass, collect unique end-point values and sort them.
    vector<key_type> keys_uniq;
    keys_uniq.reserve(m_segment_data.size()*2);
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

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::create_leaf_node_instances(const ::std::vector<key_type>& keys, node_ptr& left, node_ptr& right)
{
    if (keys.empty() || keys.size() < 2)
        // We need at least two keys in order to build tree.
        return;

    typename ::std::vector<key_type>::const_iterator itr = keys.begin(), itr_end = keys.end();

    // left-most node
    left.reset(new node(true));
    left->value_leaf.key = *itr;

    // move on to next.
    left->right.reset(new node(true));
    node_ptr prev_node = left;
    node_ptr cur_node = left->right;
    cur_node->left = prev_node;

    for (++itr; itr != itr_end; ++itr)
    {
        cur_node->value_leaf.key = *itr;

        // move on to next
        cur_node->right.reset(new node(true));
        prev_node = cur_node;
        cur_node = cur_node->right;
        cur_node->left = prev_node;
    }

    // Remove the excess node.
    prev_node->right.reset();
    right = prev_node;
}

template<typename _Key, typename _Data>
bool segment_tree<_Key, _Data>::insert(key_type begin_key, key_type end_key, data_type* pdata)
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

template<typename _Key, typename _Data>
bool segment_tree<_Key, _Data>::search(key_type point, search_result_type& result) const
{
    if (!m_valid_tree)
        // Tree is invalidated.
        return false;

    if (!m_root_node.get())
        // Tree doesn't exist.
        return false;

    descend_tree_for_search(point, m_root_node.get(), result);
    return true;
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::remove(data_type* pdata)
{
    using namespace std;

    typename data_node_map_type::iterator itr = m_tagged_node_map.find(pdata);
    if (itr != m_tagged_node_map.end())
    {
        // Tagged node list found.  Remove all the tags from the tree nodes.
        node_list_type* plist = itr->second;
        if (!plist)
            return;

        remove_data_from_nodes(plist, pdata);

        // Remove the tags associated with this pointer from the data set.
        m_tagged_node_map.erase(itr);
    }

    // Remove from the segment data array.
    m_segment_data.erase(pdata);
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::clear()
{
    m_tagged_node_map.clear();
    m_segment_data.clear();
    clear_all_nodes();
    m_valid_tree = false;
}

template<typename _Key, typename _Data>
size_t segment_tree<_Key, _Data>::size() const
{
    return m_segment_data.size();
}

template<typename _Key, typename _Data>
bool segment_tree<_Key, _Data>::empty() const
{
    return m_segment_data.empty();
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::remove_data_from_nodes(node_list_type* plist, const data_type* pdata)
{
    typename node_list_type::iterator itr = plist->begin(), itr_end = plist->end();
    for (; itr != itr_end; ++itr)
    {
        data_chain_type* chain = NULL;
        node* p = *itr;
        if (p->is_leaf)
            chain = p->value_leaf.data_chain;
        else
            chain = p->value_nonleaf.data_labels;

        if (!chain)
            continue;

        remove_data_from_chain(*chain, pdata);
    }
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::remove_data_from_chain(data_chain_type& chain, const data_type* pdata)
{
    typename data_chain_type::iterator itr = ::std::find(chain.begin(), chain.end(), pdata);
    if (itr != chain.end())
    {
        *itr = chain.back();
        chain.pop_back();
    }
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::clear_all_nodes()
{
    disconnect_leaf_nodes(m_left_leaf.get(), m_right_leaf.get());
    clear_tree(m_root_node.get());
    disconnect_node(m_root_node.get());
    m_left_leaf.reset();
    m_right_leaf.reset();
    m_root_node.reset();
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::descend_tree_for_search(key_type point, const node* pnode, search_result_type& data_chain) const
{
    if (!pnode)
        // This should never happen, but just in case.
        return;

    if (pnode->is_leaf)
    {
        append_search_result(data_chain, pnode->value_leaf.data_chain);
        return;
    }

    const nonleaf_value_type& v = pnode->value_nonleaf;
    if (point < v.low || v.high <= point)
        // Query point is out-of-range.
        return;

    append_search_result(data_chain, pnode->value_nonleaf.data_labels);

    // Check the left child node first, then the right one.
    node* pchild = pnode->left.get();
    if (!pchild)
        return;

    assert(pnode->right.get() ? pchild->is_leaf == pnode->right->is_leaf : true);
    if (pchild->is_leaf)
    {
        // The child node are leaf nodes.
        const leaf_value_type& vleft = pchild->value_leaf;
        if (point < vleft.key)
        {
            // Out-of-range.  Nothing more to do.
            return;
        }

        if (pnode->right.get())
        {
            const leaf_value_type& vright = pnode->right->value_leaf;    
            if (vright.key <= point)
                // Follow the right node.
                pchild = pnode->right.get();
        }
    }
    else
    {
        const nonleaf_value_type& vleft = pchild->value_nonleaf;
        if (point < vleft.low)
        {
            // Out-of-range.  Nothing more to do.
            return;
        }
        if (vleft.high <= point && pnode->right.get())
            // Follow the right child.
            pchild = pnode->right.get();

        assert(pchild->value_nonleaf.low <= point && point < pchild->value_nonleaf.high);
    }
    descend_tree_for_search(point, pchild, data_chain);
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::append_search_result(search_result_type& data_chain, const data_chain_type* node_data) const
{
    if (!node_data)
        return;

    typename data_chain_type::const_iterator itr = node_data->begin(), itr_end = node_data->end();
    for (; itr != itr_end; ++itr)
        data_chain.push_back(*itr);
}

#ifdef UNIT_TEST
template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::dump_tree() const
{
    using ::std::cout;
    using ::std::endl;

    if (!m_valid_tree)
        assert(!"attempted to dump an invalid tree!");

    cout << "dump tree ------------------------------------------------------" << endl;
    size_t node_count = ::mdds::dump_tree<node_ptr>(m_root_node.get());
    size_t node_instance_count = node_base<node_ptr, node>::get_instance_count();

    cout << "tree node count = " << node_count << "    node instance count = " << node_instance_count << endl;
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::dump_leaf_nodes() const
{
    using ::std::cout;
    using ::std::endl;

    cout << "dump leaf nodes ------------------------------------------------" << endl;

    node* p = m_left_leaf.get();
    while (p)
    {
        print_leaf_value(p->value_leaf);
        p = p->right.get();
    }
    cout << "  node instance count = " << node_base<node_ptr, node>::get_instance_count() << endl;
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::dump_segment_data() const
{
    using namespace std;
    cout << "dump segment data ----------------------------------------------" << endl;

    segment_map_printer func;
    for_each(m_segment_data.begin(), m_segment_data.end(), func);
}

template<typename _Key, typename _Data>
bool segment_tree<_Key, _Data>::verify_node_lists() const
{
    using namespace std;

    typename data_node_map_type::const_iterator 
        itr = m_tagged_node_map.begin(), itr_end = m_tagged_node_map.end();
    for (; itr != itr_end; ++itr)
    {
        // Print stored nodes.
        cout << "node list " << itr->first->name << ": ";
        const node_list_type* plist = itr->second;
        assert(plist);
        typename node::printer func;
        for_each(plist->begin(), plist->end(), func);
        cout << endl;

        // Verify that all of these nodes have the data pointer.
        if (!has_data_pointer(*plist, itr->first))
            return false;
    }
    return true;
}

template<typename _Key, typename _Data>
bool segment_tree<_Key, _Data>::verify_leaf_nodes(const ::std::vector<leaf_node_check>& checks) const
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
                // The data chain should be empty (i.e. the pointer should be NULL).
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

            ::std::vector<const data_type*> test1, test2;
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

        cur_node = cur_node->right.get();
    }

    if (cur_node)
        // At this point, we expect the current node to be at the position
        // past the right-most node, which is NULL.
        return false;

    return true;
}

template<typename _Key, typename _Data>
bool segment_tree<_Key, _Data>::verify_segment_data(const segment_map_type& checks) const
{
    typename segment_map_type::const_iterator itr1 = checks.begin(), itr1_end = checks.end();
    typename segment_map_type::const_iterator itr2 = m_segment_data.begin(), itr2_end = m_segment_data.end();
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

template<typename _Key, typename _Data>
bool segment_tree<_Key, _Data>::has_data_pointer(const node_list_type& node_list, const data_type* pdata)
{
    using namespace std;

    typename node_list_type::const_iterator
        itr = node_list.begin(), itr_end = node_list.end();

    for (; itr != itr_end; ++itr)
    {
        // Check each node, and make sure each node has the pdata pointer
        // listed.
        const node* pnode = *itr;
        const data_chain_type* chain = NULL;
        if (pnode->is_leaf)
            chain = pnode->value_leaf.data_chain;
        else
            chain = pnode->value_nonleaf.data_labels;

        if (!chain)
            return false;
        
        if (find(chain->begin(), chain->end(), pdata) == chain->end())
            return false;
    }
    return true;
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::print_leaf_value(const leaf_value_type& v)
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

}

#endif
