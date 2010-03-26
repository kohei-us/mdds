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
#include <iostream>
#include <boost/ptr_container/ptr_vector.hpp>

namespace mdds {

template<typename _Key, typename _Data>
class segment_tree
{
public:
    typedef _Key        key_type;
    typedef _Data       data_type;;

    struct nonleaf_value_type
    {
    };

    struct leaf_value_type
    {
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
        }

        node(const node& r) :
            node_base(r)
        {
            if (is_leaf)
            {
                value_leaf.key = r.value_leaf.key;
                value_leaf.value = r.value_leaf.value;
            }
            else
            {
                value_nonleaf.low = r.value_nonleaf.low;
                value_nonleaf.high = r.value_nonleaf.high;
            }
        }

        virtual ~node()
        {
        }

        bool equals(const node& r) const
        {
            if (is_leaf != r.is_leaf)
                return false;

            if (is_leaf)
            {
                if (value_leaf.key != r.value_leaf.key)
                    return false;
                if (value_leaf.value != r.value_leaf.value)
                    return false;
            }
            else
            {
                if (value_nonleaf.low != r.value_nonleaf.low)
                    return false;
                if (value_nonleaf.high != r.value_nonleaf.high)
                    return false;
            }

            return true;
        }

        virtual void fill_nonleaf_value(const node_base_ptr& left_node, const node_base_ptr& right_node)
        {
#if 0
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
#endif
        }

        virtual void dump_value() const
        {
#if 0
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
#endif
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
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::build_tree()
{
    using namespace std;

    vector<key_type> keys_uniq;
    keys_uniq.reserve(m_segment_data.size()*2);
    typename data_array_type::const_iterator itr = m_segment_data.begin(), itr_end = m_segment_data.end();
    for (; itr != itr_end; ++itr)
    {
        cout << itr->begin_key << "," << itr->end_key << ": " << itr->pdata << endl;
        keys_uniq.push_back(itr->begin_key);
        keys_uniq.push_back(itr->end_key);
    }

    // sort and remove duplicates.
    sort(keys_uniq.begin(), keys_uniq.end());
    keys_uniq.erase(unique(keys_uniq.begin(), keys_uniq.end()), keys_uniq.end());

    copy(keys_uniq.begin(), keys_uniq.end(), ostream_iterator<key_type>(cout, " "));
    cout << endl;
}

template<typename _Key, typename _Data>
void segment_tree<_Key, _Data>::insert(key_type begin_key, key_type end_key, data_type* pdata)
{
    m_segment_data.push_back(new segment_data(begin_key, end_key, pdata));
}

}

#endif
