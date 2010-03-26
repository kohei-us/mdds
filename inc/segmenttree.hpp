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

    struct nonleaf_value_type
    {
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
        }

        virtual void dump_value() const
        {
        }

        virtual node_base* create_new(bool leaf) const
        {
            return new node(leaf);
        }

        virtual node_base* clone() const
        {
            return new node(*this);
        }
    };

    segment_tree();
    ~segment_tree();

    bool is_tree_valid() const { return m_valid_tree; }

    void build_tree();

    void insert(key_type begin_key, key_type end_key, data_type* pdata);

private:
    static node* get_node(const node_base_ptr& base_node)
    { 
        return static_cast<node*>(base_node.get());
    }

    static void build_leaf_node(const ::std::vector<key_type>& keys, node_base_ptr& left, node_base_ptr& right);

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
    // Go through all leaf nodes, and disconnect their links.
    node_base* cur_node = m_left_leaf.get();
    do
    {
        node_base* next_node = cur_node->right.get();
        disconnect_node(cur_node);
        cur_node = next_node;
    }
    while (cur_node != m_right_leaf.get());

    disconnect_node(m_right_leaf.get());
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::build_tree()
{
    using namespace std;

    // In 1st pass, collect unique end-point values and sort them.
    vector<key_type> keys_uniq;
    keys_uniq.reserve(m_segment_data.size()*2);
    typename data_array_type::const_iterator itr, itr_beg = m_segment_data.begin(), itr_end = m_segment_data.end();
    for (itr = itr_beg; itr != itr_end; ++itr)
    {
        cout << itr->begin_key << "," << itr->end_key << ": " << itr->pdata << endl;
        keys_uniq.push_back(itr->begin_key);
        keys_uniq.push_back(itr->end_key);
    }

    // sort and remove duplicates.
    sort(keys_uniq.begin(), keys_uniq.end());
    keys_uniq.erase(unique(keys_uniq.begin(), keys_uniq.end()), keys_uniq.end());

    // debug output.
    cout << "unique keys: ";
    copy(keys_uniq.begin(), keys_uniq.end(), ostream_iterator<key_type>(cout, " "));
    cout << endl;

    // Create leaf nodes with the unique end-point values.
    build_leaf_node(keys_uniq, m_left_leaf, m_right_leaf);

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
                leaf_value_type& v = p->value_leaf;
                if (!v.data_chain)
                    v.data_chain = new data_chain_type;
                v.data_chain->push_back(pdata);
            }
            else if (p->value_leaf.key == key_end)
            {
                // Insert data pointer to the previous node _only when_ the
                // value of the previous node doesn't equal the begin point
                // value.
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

    // debug output
    {
        node* p = get_node(m_left_leaf);
        while (p)
        {
            cout << p->value_leaf.key << ":{ ";
            if (p->value_leaf.data_chain)
            {
                const data_chain_type* pchain = p->value_leaf.data_chain;
                copy(pchain->begin(), pchain->end(), ostream_iterator<data_type*>(cout, " "));
            }
            cout << "}" << endl;
            p = get_node(p->right);
        }
    }
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::build_leaf_node(const ::std::vector<key_type>& keys, node_base_ptr& left, node_base_ptr& right)
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
}

}

#endif
