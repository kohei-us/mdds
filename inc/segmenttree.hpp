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

#ifndef __SEGMENTTREE_HPP__
#define __SEGMENTTREE_HPP__

#include "node.hpp"

#include <vector>
#include <list>
#include <set>
#include <iostream>
#include <boost/ptr_container/ptr_vector.hpp>

namespace mdds {

template<typename _Key, typename _Data>
class segment_tree
{
public:
    typedef _Key        key_type;
    typedef _Data       data_type;
    typedef ::std::list<data_type*> data_chain_type;

private:
    struct segment_data
    {
        key_type    begin_key;
        key_type    end_key;
        data_type*  pdata;

        segment_data(key_type _beg, key_type _end, data_type* p) :
            begin_key(_beg), end_key(_end), pdata(p) {}
    };
    typedef ::boost::ptr_vector<segment_data> data_array_type;

public:
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

    struct node : public node_base
    {
        union {
            nonleaf_value_type  value_nonleaf;
            leaf_value_type     value_leaf;
        };

        node(bool _is_leaf) :
            node_base(_is_leaf)
        {
            if (_is_leaf)
                value_leaf.data_chain = NULL;
            else
                value_nonleaf.data_labels = NULL;
        }

        node(const node& r) :
            node_base(r)
        {
        }

        virtual ~node()
        {
            if (is_leaf)
                delete value_leaf.data_chain;
            else
                delete value_nonleaf.data_labels;
        }

        bool equals(const node& r) const
        {
            if (is_leaf != r.is_leaf)
                return false;

            return true;
        }

        virtual void fill_nonleaf_value(const node_base_ptr& left_node, const node_base_ptr& right_node)
        {
            // Parent node should carry the range of all of its child nodes.
            if (left_node)
                value_nonleaf.low  = left_node->is_leaf ? get_node(left_node)->value_leaf.key : get_node(left_node)->value_nonleaf.low;
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
                        value_nonleaf.high = get_node(right_node->right)->value_leaf.key;
                    else
                        value_nonleaf.high = get_node(right_node)->value_leaf.key;
                }
                else
                {
                    value_nonleaf.high = get_node(right_node)->value_nonleaf.high;
                }
            }
            else
                value_nonleaf.high = left_node->is_leaf ? get_node(left_node)->value_leaf.key : get_node(left_node)->value_nonleaf.high;
        }

        virtual node_base* create_new(bool leaf) const
        {
            return new node(leaf);
        }

        virtual node_base* clone() const
        {
            return new node(*this);
        }

#if UNIT_TEST
        virtual void dump_value() const
        {
            using ::std::cout;
            if (is_leaf)
            {
                cout << "[" << value_leaf.key << "]";
            }
            else
            {
                cout << "[" << value_nonleaf.low << "-" << value_nonleaf.high << ")";
                if (value_nonleaf.data_labels)
                {
                    cout << " { ";
                    typename data_chain_type::const_iterator itr, itr_beg = value_nonleaf.data_labels->begin(), itr_end = value_nonleaf.data_labels->end();
                    for (itr = itr_beg; itr != itr_end; ++itr)
                    {
                        if (itr != itr_beg)
                            cout << ", ";
                        cout << (*itr)->name;
                    }
                    cout << " }";
                }
            }
            cout << " ";
        }
#endif
    };

    segment_tree();
    ~segment_tree();

    bool is_tree_valid() const { return m_valid_tree; }

    void build_tree();

    void insert(key_type begin_key, key_type end_key, data_type* pdata);

    bool search(key_type point, data_chain_type& data_chain) const;

#if UNIT_TEST
    void dump_tree() const
    {
        using ::std::cout;
        using ::std::endl;

        if (!m_valid_tree)
            assert(!"attempted to dump an invalid tree!");

        size_t node_count = ::mdds::dump_tree(m_root_node);
        size_t node_instance_count = node_base::get_instance_count();

        cout << "tree node count = " << node_count << "    node instance count = " << node_instance_count << endl;
        assert(node_count == node_instance_count);
    }

    void dump_leaf_nodes() const
    {
        using ::std::cout;
        using ::std::endl;

        cout << "------------------------------------------" << endl;

        node* p = get_node(m_left_leaf);
        while (p)
        {
            print_leaf_value(p->value_leaf);
            p = get_node(p->right);
        }
        cout << endl << "  node instance count = " << node_base::get_instance_count() << endl;
    }

    struct leaf_node_check
    {
        key_type key;
        data_chain_type data_chain;
    };

    bool verify_leaf_nodes(const ::std::vector<leaf_node_check>& checks) const
    {
        node* cur_node = get_node(m_left_leaf);
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

                typename data_chain_type::const_iterator itr1 = itr->data_chain.begin();
                typename data_chain_type::const_iterator itr1_end = itr->data_chain.end();
                typename data_chain_type::const_iterator itr2 = cur_node->value_leaf.data_chain->begin();
                typename data_chain_type::const_iterator itr2_end = cur_node->value_leaf.data_chain->end();
                for (; itr1 != itr1_end; ++itr1, ++itr2)
                {
                    if (itr2 == itr2_end)
                        // Data chain in the node finished early.
                        return false;

                    if (*itr1 != *itr2)
                        // Data pointers differ.
                        return false;
                }
                if (itr2 != itr2_end)
                    // There are more data pointers in the node.
                    return false;
            }

            cur_node = get_node(cur_node->right);
        }

        if (cur_node)
            // At this point, we expect the current node to be at the position
            // past the right-most node, which is NULL.
            return false;

        return true;
    }
#endif

private:

    static node* get_node(const node_base_ptr& base_node)
    { 
        return static_cast<node*>(base_node.get());
    }

    static void create_leaf_node_instances(const ::std::vector<key_type>& keys, node_base_ptr& left, node_base_ptr& right);

    /** 
     * Descend the tree from the root node, and mark appropriate nodes, both 
     * leaf and non-leaf, based on segment's end points. 
     */
    void descend_tree_and_mark(node* pnode, const segment_data& data);

    void build_leaf_nodes();

    void descend_tree_for_search(key_type point, const node* pnode, data_chain_type& data_chain) const;

#if UNIT_TEST
    static void print_leaf_value(const leaf_value_type& v)
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

private:
    data_array_type m_segment_data;

    node_base_ptr   m_root_node;
    node_base_ptr   m_left_leaf;
    node_base_ptr   m_right_leaf;
    bool m_valid_tree:1;
};

template<typename _Key, typename _Data>
segment_tree<_Key, _Data>::segment_tree() :
    m_valid_tree(false)
{
}

template<typename _Key, typename _Data>
segment_tree<_Key, _Data>::~segment_tree()
{
    disconnect_leaf_nodes(m_left_leaf.get(), m_right_leaf.get());
    clear_tree(m_root_node.get());
    disconnect_node(m_root_node.get());
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::build_tree()
{
    if (m_valid_tree)
        // Nothing to do.
        return;

    build_leaf_nodes();
    clear_tree(m_root_node.get());
    m_root_node = ::mdds::build_tree(m_left_leaf);
    
    // Start "inserting" all segments from the root.
    typename data_array_type::const_iterator itr, 
        itr_beg = m_segment_data.begin(), itr_end = m_segment_data.end();

    for (itr = itr_beg; itr != itr_end; ++itr)
        descend_tree_and_mark(get_node(m_root_node), *itr);

    m_valid_tree = true;
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::descend_tree_and_mark(node* pnode, const segment_data& data)
{
    if (!pnode)
        return;

    // TODO: Build auxiliary table to keep track of marked nodes for each line
    // segment.  We need this to support segment deletion.

    if (pnode->is_leaf)
    {
        // This is a leaf node.
        if (pnode->value_leaf.key == data.begin_key)
        {
            // Insertion of begin point.
            leaf_value_type& v = pnode->value_leaf;
            if (!v.data_chain)
                v.data_chain = new data_chain_type;
            v.data_chain->push_back(data.pdata);
        }
        else if (pnode->value_leaf.key == data.end_key)
        {
            // For insertion of the end point, insert data pointer to the
            // previous node _only when_ the value of the previous node
            // doesn't equal the begin point value.
            node* pprev = get_node(pnode->left);
            if (pprev && pprev->value_leaf.key != data.begin_key)
            {
                leaf_value_type& v = pprev->value_leaf;
                if (!v.data_chain)
                    v.data_chain = new data_chain_type;
                v.data_chain->push_back(data.pdata);
            }
        }
        return;
    }
    
    if (data.end_key < pnode->value_nonleaf.low || pnode->value_nonleaf.high <= data.begin_key)
        return;

    nonleaf_value_type& v = pnode->value_nonleaf;
    if (data.begin_key <= v.low && v.high < data.end_key)
    {
        // mark this non-leaf node and stop.
        if (!v.data_labels)
            v.data_labels = new data_chain_type;
        v.data_labels->push_back(data.pdata);
        return;
    }

    descend_tree_and_mark(get_node(pnode->left), data);
    descend_tree_and_mark(get_node(pnode->right), data);
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::build_leaf_nodes()
{
    using namespace std;

    disconnect_leaf_nodes(m_left_leaf.get(), m_right_leaf.get());

    // In 1st pass, collect unique end-point values and sort them.
    vector<key_type> keys_uniq;
    keys_uniq.reserve(m_segment_data.size()*2);
    typename data_array_type::const_iterator itr, itr_beg = m_segment_data.begin(), itr_end = m_segment_data.end();
    for (itr = itr_beg; itr != itr_end; ++itr)
    {
        keys_uniq.push_back(itr->begin_key);
        keys_uniq.push_back(itr->end_key);
    }

    // sort and remove duplicates.
    sort(keys_uniq.begin(), keys_uniq.end());
    keys_uniq.erase(unique(keys_uniq.begin(), keys_uniq.end()), keys_uniq.end());

    create_leaf_node_instances(keys_uniq, m_left_leaf, m_right_leaf);
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::create_leaf_node_instances(const ::std::vector<key_type>& keys, node_base_ptr& left, node_base_ptr& right)
{
    if (keys.empty() || keys.size() < 2)
        // We need at least two keys in order to build tree.
        return;

    typename ::std::vector<key_type>::const_iterator itr = keys.begin(), itr_end = keys.end();

    // left-most node
    left.reset(new node(true));
    get_node(left)->value_leaf.key = *itr;

    // move on to next.
    left->right.reset(new node(true));
    node_base_ptr prev_node = left;
    node_base_ptr cur_node = left->right;
    cur_node->left = prev_node;

    for (++itr; itr != itr_end; ++itr)
    {
        get_node(cur_node)->value_leaf.key = *itr;

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
void segment_tree<_Key, _Data>::insert(key_type begin_key, key_type end_key, data_type* pdata)
{
    if (begin_key >= end_key)
        return;

    m_segment_data.push_back(new segment_data(begin_key, end_key, pdata));
    m_valid_tree = false;
}

template<typename _Key, typename _Data>
bool segment_tree<_Key, _Data>::search(key_type point, data_chain_type& data_chain) const
{
    data_chain_type result;
    if (!m_root_node.get())
        // Tree doesn't exist.
        return false;

    descend_tree_for_search(point, get_node(m_root_node), result);
    result.swap(data_chain);
    return true;
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::descend_tree_for_search(key_type point, const node* pnode, data_chain_type& data_chain) const
{
    if (!pnode)
        // This should never happen, but just in case.
        return;

    if (pnode->is_leaf)
    {
        // TODO: Pick up the data.
        return;
    }

    const nonleaf_value_type& v = pnode->value_nonleaf;
    if (point < v.low || v.high <= point)
        // Query point is out-of-range.
        return;

    // TODO: Pick up the data.

    // Check the left child node first, then the right one.
    node* pchild = get_node(pnode->left);
    assert(pchild->is_leaf == pnode->right->is_leaf);
    if (pchild->is_leaf)
    {
        // The child node are leaf nodes.
        const leaf_value_type& vleft = pchild->value_leaf;
        const leaf_value_type& vright = get_node(pnode->right)->value_leaf;
        if (point < vleft.key)
        {
            // Out-of-range.  Nothing more to do.
            return;
        }

        if (vright.key < point)
            // Follow the right node.
            pchild = get_node(pnode->right);
    }
    else
    {
        const nonleaf_value_type& vleft = pchild->value_nonleaf;
        if (point < vleft.low)
        {
            // Out-of-range.  Nothing more to do.
            return;
        }
        if (vleft.high <= point)
            // Follow the right child.
            pchild = get_node(pnode->right);

        assert(pchild->value_nonleaf.low <= point && point < pchild->value_nonleaf.high);
    }
    descend_tree_for_search(point, pchild, data_chain);
}

}

#endif
