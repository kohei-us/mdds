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
    typedef ::std::set<data_type*>  data_set_type;

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
        data_set_type* data_labels;
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
        }

        node(const node& r) :
            node_base(r)
        {
        }

        virtual ~node()
        {
            if (is_leaf)
                delete value_leaf.data_chain;
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
                cout << "(" << value_leaf.key << ")";
            }
            else
            {
                cout << "(" << value_nonleaf.low << "-" << value_nonleaf.high << ")";
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
#endif

private:

    static node* get_node(const node_base_ptr& base_node)
    { 
        return static_cast<node*>(base_node.get());
    }

    static void create_leaf_node_instances(const ::std::vector<key_type>& keys, node_base_ptr& left, node_base_ptr& right);

    void descend_tree_and_mark(node* pnode, const segment_data& data);

    void build_leaf_nodes();

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
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::build_leaf_nodes()
{
    using namespace std;

#if UNIT_TEST
    cout << "-------------------------------------" << endl;
#endif

    disconnect_leaf_nodes(m_left_leaf.get(), m_right_leaf.get());

    // In 1st pass, collect unique end-point values and sort them.
    vector<key_type> keys_uniq;
    keys_uniq.reserve(m_segment_data.size()*2);
    typename data_array_type::const_iterator itr, itr_beg = m_segment_data.begin(), itr_end = m_segment_data.end();
    for (itr = itr_beg; itr != itr_end; ++itr)
    {
#if UNIT_TEST
        cout << itr->pdata->name << ": " << itr->begin_key << "," << itr->end_key << endl;
#endif
        keys_uniq.push_back(itr->begin_key);
        keys_uniq.push_back(itr->end_key);
    }

    // sort and remove duplicates.
    sort(keys_uniq.begin(), keys_uniq.end());
    keys_uniq.erase(unique(keys_uniq.begin(), keys_uniq.end()), keys_uniq.end());

#if UNIT_TEST
    // debug output.
    cout << "unique keys: ";
    copy(keys_uniq.begin(), keys_uniq.end(), ostream_iterator<key_type>(cout, " "));
    cout << endl;
#endif

    // Create leaf nodes with the unique end-point values.
    create_leaf_node_instances(keys_uniq, m_left_leaf, m_right_leaf);

#if UNIT_TEST
    // debug output.
    {
        cout << "forward: ";
        node* p = get_node(m_left_leaf);
        while (p)
        {
            cout << p->value_leaf.key << " ";
            p = get_node(p->right);
        }
        cout << endl;

        cout << "backward: ";
        p = get_node(m_right_leaf);
        while (p)
        {
            cout << p->value_leaf.key << " ";
            p = get_node(p->left);
        }
        cout << endl;
    }
#endif

    // In 2nd pass, "insert" each segment.
    for (itr = itr_beg; itr != itr_end; ++itr)
    {
        key_type key_beg = itr->begin_key;
        key_type key_end = itr->end_key;
        data_type* pdata = itr->pdata;

        node* p = get_node(m_left_leaf);
        while (p)
        {
            if (p->value_leaf.key == key_beg)
            {
                // Insertion of begin point.
                leaf_value_type& v = p->value_leaf;
                if (!v.data_chain)
                    v.data_chain = new data_chain_type;
                v.data_chain->push_back(pdata);
            }
            else if (p->value_leaf.key == key_end)
            {
                // For insertion of the end point, insert data pointer to the
                // previous node _only when_ the value of the previous node
                // doesn't equal the begin point value.
                node* pprev = get_node(p->left);
                if (pprev && pprev->value_leaf.key != key_beg)
                {
                    leaf_value_type& v = pprev->value_leaf;
                    if (!v.data_chain)
                        v.data_chain = new data_chain_type;
                    v.data_chain->push_back(pdata);
                }
            }
            p = get_node(p->right);
        }
    }

#if UNIT_TEST
    // debug output
    {
        node* p = get_node(m_left_leaf);
        while (p)
        {
            print_leaf_value(p->value_leaf);
            p = get_node(p->right);
        }
    }
#endif
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

}

#endif
