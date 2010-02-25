/*************************************************************************
 *
 * Copyright (c) 2008-2009 Kohei Yoshida
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

#ifndef __MDDS_FLATSEGMENTTREE_HXX__
#define __MDDS_FLATSEGMENTTREE_HXX__

#include <iostream>
#include <utility>
#include <cassert>

#include "node.hxx"

#ifdef UNIT_TEST
#include <vector>
#endif

namespace mdds {

template<typename _Key, typename _Value>
class flat_segment_tree
{
public:
    typedef _Key    key_type;
    typedef _Value  value_type;

    struct nonleaf_value_type
    {
        key_type low;   /// low range value (inclusive)
        key_type high;  /// high range value (non-inclusive)
    };

    struct leaf_value_type
    {
        key_type    key;
        value_type  value;
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

        virtual ~node()
        {
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

        virtual node_base* create_new(bool leaf) const
        {
            return new node(leaf);
        }
    };

private:
    class const_iterator_base
    {
    public:
        typedef flat_segment_tree<key_type, value_type> fst_type;

        explicit const_iterator_base(const fst_type* _db, bool _end, bool _forward) : 
            m_db(_db), m_pos(NULL), m_end_pos(_end), m_forward(_forward)
        {
            if (!_db)
                return;

            if (m_forward)
            {
                // forward direction
                m_pos = _end ? get_node(_db->m_right_leaf) : get_node(_db->m_left_leaf);
            }
            else
            {
                // reverse direction
                m_pos = _end ? get_node(_db->m_left_leaf) : get_node(_db->m_right_leaf);
            }
        }

        const_iterator_base(const const_iterator_base& r) :
            m_db(r.m_db), m_pos(r.m_pos), m_end_pos(r.m_end_pos), m_forward(r.m_forward) {}

        const_iterator_base& operator=(const const_iterator_base& r)
        {
            m_db = r.m_db;
            m_pos = r.m_pos;
            return *this;
        }

        const ::std::pair<key_type, value_type>* operator++()
        {
            assert(m_pos);
            if (m_forward)
            {
                if (m_pos == get_node(m_db->m_right_leaf))
                    m_end_pos = true;
                else
                    m_pos = get_node(m_pos->right);
            }
            else
            {
                if (m_pos == get_node(m_db->m_left_leaf))
                    m_end_pos = true;
                else
                    m_pos = get_node(m_pos->left);
            }

            return operator->();
        }

        const ::std::pair<key_type, value_type>* operator--()
        {
            assert(m_pos);
            if (m_end_pos)
                m_end_pos = false;
            else
                m_pos = m_forward ? get_node(m_pos->left) : get_node(m_pos->right);

            return operator->();
        }

        bool operator==(const const_iterator_base& r) const
        {
            return (m_end_pos == r.m_end_pos) && (m_pos == r.m_pos);
        }

        bool operator!=(const const_iterator_base& r) const
        {
            return !operator==(r);
        }

        const ::std::pair<key_type, value_type>& operator*()
        {
            return get_current_node_pair();
        }

        const ::std::pair<key_type, value_type>* operator->()
        {
            return &get_current_node_pair();
        }

    private:
        const ::std::pair<key_type, value_type>& get_current_node_pair()
        {
            m_current_pair = ::std::pair<key_type, value_type>(m_pos->value_leaf.key, m_pos->value_leaf.value);
            return m_current_pair;
        }

        const fst_type* m_db;
        const typename fst_type::node* m_pos;
        ::std::pair<key_type, value_type> m_current_pair;
        bool            m_end_pos:1;
        bool            m_forward:1;
    };

public:
    class const_iterator : public const_iterator_base
    {
        friend class flat_segment_tree;
    public:
        const_iterator() :
            const_iterator_base(NULL, false, true) {}

    private:
        explicit const_iterator(const typename const_iterator_base::fst_type* _db, bool _end) : 
            const_iterator_base(_db, _end, true) {}
    };

    class const_reverse_iterator : public const_iterator_base
    {
        friend class flat_segment_tree;
    public:
        const_reverse_iterator() :
            const_iterator_base(NULL, false, false) {}
    private:
        explicit const_reverse_iterator(const typename const_iterator_base::fst_type* _db, bool _end) : 
            const_iterator_base(_db, _end, false) {}
    };

    const_iterator begin() const
    {
        return const_iterator(this, false);
    }

    const_iterator end() const
    {
        return const_iterator(this, true);
    }

    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(this, false);
    }

    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(this, true);
    }

    /** 
     * Get a pointer of concrete node type from the base pointer.
     *
     * @param base_node base node pointer (ref-counted)
     * 
     * @return raw pointer of concrete node type
     */
    static node* get_node(const node_base_ptr& base_node)
    {
        return static_cast<node*>(base_node.get());
    }

    flat_segment_tree(key_type min_val, key_type max_val, value_type init_val) :
        m_root_node(static_cast<node*>(NULL)),
        m_left_leaf(new node(true)),
        m_right_leaf(new node(true)),
        m_init_val(init_val),
        m_valid_tree(false)
    {
        // we need to create two end nodes during initialization.
        get_node(m_left_leaf)->value_leaf.key = min_val;
        get_node(m_left_leaf)->value_leaf.value = init_val;
        m_left_leaf->right = m_right_leaf;

        get_node(m_right_leaf)->value_leaf.key = max_val;
        m_right_leaf->left = m_left_leaf;
    }

    ~flat_segment_tree()
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
        clear_tree(m_root_node);
        disconnect_node(m_root_node.get());
    }

    /** 
     * Insert a new segment into the tree.
     *
     * @param start_key start value of the segment being inserted.  The value 
     *              is inclusive.
     * @param end_key end value of the segment being inserted.  The value is 
     *            not inclusive.
     * @param val value associated with this segment.
     */
    void insert_segment(key_type start_key, key_type end_key, value_type val);

    /** 
     * Remove a segment specified by the start and end key values, and shift 
     * the remaining segments (i.e. those segments that come after the removed
     * segment) to left.  Note that the start and end positions of the segment 
     * being removed <b>must</b> be within the base segment span.
     *
     * @param start start position of the segment being removed.
     * @param end end position of the segment being removed. 
     */
    void shift_segment_left(key_type start_key, key_type end_key);

    /** 
     * Shift all segments that occur at or after the specified start position 
     * to right by the size specified.
     *
     * @param pos position where the right-shift occurs.
     * @param size amount of shift (must be greater than 0)
     */
    void shift_segment_right(key_type pos, key_type size, bool skip_start_node);

    bool search(key_type key, value_type& value, key_type* start_key = NULL, key_type* end_key = NULL) const
    {
        if (key < get_node(m_left_leaf)->value_leaf.key || get_node(m_right_leaf)->value_leaf.key <= key)
            // key value is out-of-bound.
            return false;

        const node* pos = get_insertion_pos_leaf(key, get_node(m_left_leaf));
        if (pos->value_leaf.key == key)
        {
            value = pos->value_leaf.value;
            if (start_key)
                *start_key = pos->value_leaf.key;
            if (end_key && pos->right)
                *end_key = get_node(pos->right)->value_leaf.key;
            return true;
        }
        else if (pos->left && get_node(pos->left)->value_leaf.key < key)
        {
            value = get_node(pos->left)->value_leaf.value;
            if (start_key)
                *start_key = get_node(pos->left)->value_leaf.key;
            if (end_key)
                *end_key = pos->value_leaf.key;
            return true;
        }

        return false;
    }

    bool search_tree(key_type key, value_type& value, key_type* start_key = NULL, key_type* end_key = NULL) const
    {
        if (!m_root_node || !m_valid_tree)
        {    
            // either tree has not been built, or is in an invalid state.
            return false;
        }

        if (key < get_node(m_left_leaf)->value_leaf.key || get_node(m_right_leaf)->value_leaf.key <= key)
        {    
            // key value is out-of-bound.
            return false;
        }

        // Descend down the tree through the last non-leaf layer.

        node* cur_node = get_node(m_root_node);
        while (true)
        {
            if (cur_node->left)
            {
                if (cur_node->left->is_leaf)
                    break;

                const nonleaf_value_type& v = get_node(cur_node->left)->value_nonleaf;
                if (v.low <= key && key < v.high)
                {    
                    cur_node = get_node(cur_node->left);
                    continue;
                }
            }
            else
            {    
                // left child node can't be missing !
                return false;
            }

            if (cur_node->right)
            {
                const nonleaf_value_type& v = get_node(cur_node->right)->value_nonleaf;
                if (v.low <= key && key < v.high)
                {    
                    cur_node = get_node(cur_node->right);
                    continue;
                }
            }
            return false;
        }

        assert(cur_node->left->is_leaf && cur_node->right->is_leaf);

        key_type key1 = get_node(cur_node->left)->value_leaf.key;
        key_type key2 = get_node(cur_node->right)->value_leaf.key;

        if (key1 <= key && key < key2)
        {
            cur_node = get_node(cur_node->left);
        }
        else if (key2 <= key && key < cur_node->value_nonleaf.high)
        {
            cur_node = get_node(cur_node->right);
        }
        else
            cur_node = NULL;

        if (!cur_node)
        {    
            return false;
        }

        value = cur_node->value_leaf.value;
        if (start_key)
            *start_key = cur_node->value_leaf.key;

        if (end_key)
        {
            assert(cur_node->right);
            if (cur_node->right)
                *end_key = get_node(cur_node->right)->value_leaf.key;
            else
                // This should never happen, but just in case....
                *end_key = get_node(m_right_leaf)->value_leaf.key;
        }

        return true;
    }

    void build_tree()
    {
        if (!m_left_leaf)
            return;

        clear_tree(m_root_node);
        m_root_node = ::mdds::build_tree(m_left_leaf);
        m_valid_tree = true;
    }

    bool is_tree_valid() const
    {
        return m_valid_tree;
    }

#ifdef UNIT_TEST
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

        node_base_ptr cur_node = m_left_leaf;
        long node_id = 0;
        while (cur_node)
        {
            cout << "  node " << node_id++ << ": key = " << get_node(cur_node)->value_leaf.key
                << "; value = " << get_node(cur_node)->value_leaf.value 
                << endl;
            cur_node = cur_node->right;
        }
        cout << endl << "  node instance count = " << node_base::get_instance_count() << endl;
    }

    /** 
     * Verify keys in the leaf nodes.
     *
     * @param key_values vector containing key values in the left-to-right 
     *                   order, including the key value of the rightmost leaf
     *                   node.
     */
    bool verify_keys(const ::std::vector<key_type>& key_values) const
    {
        node* cur_node = get_node(m_left_leaf);
        typename ::std::vector<key_type>::const_iterator itr = key_values.begin(), itr_end = key_values.end();
        for (; itr != itr_end; ++itr)
        {
            if (!cur_node)
                // Position past the right-mode node.  Invalid.
                return false;

            if (cur_node->value_leaf.key != *itr)
                // Key values differ.
                return false;

            cur_node = get_node(cur_node->right);
        }

        if (cur_node)
            // At this point, we expect the current node to be at the position
            // past the right-most node, which is NULL.
            return false;

        return true;
    }

    /** 
     * Verify values in the leaf nodes.
     *
     * @param values vector containing values to verify against, in the 
     *               left-to-right order, <i>not</i> including the value of
     *               the rightmost leaf node.
     */
    bool verify_values(const ::std::vector<value_type>& values) const
    {
        node* cur_node = get_node(m_left_leaf);
        node* end_node = get_node(m_right_leaf);
        typename ::std::vector<value_type>::const_iterator itr = values.begin(), itr_end = values.end();
        for (; itr != itr_end; ++itr)
        {
            if (cur_node == end_node || !cur_node)
                return false;

            if (cur_node->value_leaf.value != *itr)
                // Key values differ.
                return false;

            cur_node = get_node(cur_node->right);
        }

        if (cur_node != end_node)
            // At this point, we expect the current node to be at the end of 
            // range.
            return false;

        return true;
    }
#endif

private:
    flat_segment_tree();

    void append_new_segment(key_type start_key)
    {
        if (get_node(m_right_leaf->left)->value_leaf.key == start_key)
        {
            get_node(m_right_leaf->left)->value_leaf.value = m_init_val;
            return;
        }

#ifdef UNIT_TEST
        // The start position must come after the position of the last node 
        // before the right-most node.
        assert(get_node(m_right_leaf->left)->value_leaf.key < start_key);        
#endif

        if (get_node(m_right_leaf->left)->value_leaf.value == m_init_val)
            // The existing segment has the same value.  No need to insert a 
            // new segment.
            return;

        node_base_ptr new_node(new node(true));
        get_node(new_node)->value_leaf.key   = start_key;
        get_node(new_node)->value_leaf.value = m_init_val;
        new_node->left = m_right_leaf->left;
        new_node->right = m_right_leaf;
        m_right_leaf->left->right = new_node;
        m_right_leaf->left = new_node;
        m_valid_tree = false;
    }

    node_base_ptr get_insertion_pos_leaf(key_type key, const node_base_ptr& start_pos) const
    {
        node_base_ptr cur_node = start_pos;
        while (cur_node)
        {
            if (key <= get_node(cur_node)->value_leaf.key)
            {
                // Found the insertion position.
                return cur_node;
            }
            cur_node = cur_node->right;
        }
        return node_base_ptr();
    }

    const node* get_insertion_pos_leaf(key_type key, const node* start_pos) const
    {
        const node* cur_node = start_pos;
        while (cur_node)
        {
            if (key <= cur_node->value_leaf.key)
            {
                // Found the insertion position.
                return cur_node;
            }
            cur_node = get_node(cur_node->right);
        }
        return NULL;
    }

    static void shift_leaf_key_left(node_base_ptr& begin_node, node_base_ptr& end_node, key_type shift_value)
    {
        node* cur_node_p = get_node(begin_node);
        node* end_node_p = get_node(end_node);
        while (cur_node_p != end_node_p)
        {
            cur_node_p->value_leaf.key -= shift_value;
            cur_node_p = get_node(cur_node_p->right);
        }
    }

    static void shift_leaf_key_right(node_base_ptr& cur_node, node_base_ptr& end_node, key_type shift_value)
    {
        key_type end_node_key = get_node(end_node)->value_leaf.key;
        while (get_node(cur_node) != get_node(end_node))
        {
            get_node(cur_node)->value_leaf.key += shift_value;
            if (get_node(cur_node)->value_leaf.key < end_node_key)
            {
                // The node is still in-bound.  Keep shifting.
                cur_node = cur_node->right;
                continue;
            }

            // This node has been pushed outside the end node position.
            // Remove all nodes that follows, and connect the previous node
            // with the end node.

            node_base_ptr last_node = cur_node->left;
            while (get_node(cur_node) != get_node(end_node))
            {
                node_base_ptr next_node = cur_node->right;
                disconnect_node(cur_node);
                cur_node = next_node;
            }
            last_node->right = end_node;
            end_node->left = last_node;
            return;
        }
    }

private:
    node_base_ptr   m_root_node;
    node_base_ptr   m_left_leaf;
    node_base_ptr   m_right_leaf;
    value_type      m_init_val;
    bool            m_valid_tree;
};

template<typename _Key, typename _Value>
void flat_segment_tree<_Key, _Value>::insert_segment(key_type start_key, key_type end_key, value_type val)
{
    if (end_key < get_node(m_left_leaf)->value_leaf.key || start_key > get_node(m_right_leaf)->value_leaf.key)
        // The new segment does not overlap the current interval.
        return;

    if (start_key < get_node(m_left_leaf)->value_leaf.key)
        // The start value should not be smaller than the current minimum.
        start_key = get_node(m_left_leaf)->value_leaf.key;

    if (end_key > get_node(m_right_leaf)->value_leaf.key)
        // The end value should not be larger than the current maximum.
        end_key = get_node(m_right_leaf)->value_leaf.key;

    if (start_key >= end_key)
        return;

    // Find the node with value that either equals or is greater than the
    // start value.

    node_base_ptr start_pos = get_insertion_pos_leaf(start_key, m_left_leaf);
    if (!start_pos)
        // Insertion position not found.  Bail out.
        return;

    node_base_ptr end_pos = get_insertion_pos_leaf(end_key, start_pos);
    if (!end_pos)
        end_pos = m_right_leaf;

    node_base_ptr new_start_node;
    value_type old_value;

    // Set the start node.

    if (get_node(start_pos)->value_leaf.key == start_key)
    {
        // Re-use the existing node, but save the old value for later.

        if (start_pos->left && get_node(start_pos->left)->value_leaf.value == val)
        {
            // Extend the existing segment.
            old_value = get_node(start_pos)->value_leaf.value;
            new_start_node = start_pos->left;
        }
        else
        {
            // Update the value of the existing node.
            old_value = get_node(start_pos)->value_leaf.value;
            get_node(start_pos)->value_leaf.value = val;
            new_start_node = start_pos;
        }
    }
    else if (get_node(start_pos->left)->value_leaf.value == val)
    {
        // Extend the existing segment.
        old_value = get_node(start_pos->left)->value_leaf.value;
        new_start_node = start_pos->left;
    }
    else
    {
        // Insert a new node before the insertion position node.
        node_base_ptr new_node(new node(true));
        get_node(new_node)->value_leaf.key = start_key;
        get_node(new_node)->value_leaf.value = val;
        new_start_node = new_node;

        node_base_ptr left_node = start_pos->left;
        old_value = get_node(left_node)->value_leaf.value;

        // Link to the left node.
        link_nodes(left_node, new_node);

        // Link to the right node.
        link_nodes(new_node, start_pos);
    }

    node_base_ptr cur_node = new_start_node->right;
    while (cur_node != end_pos)
    {
        // Disconnect the link between the current node and the previous node.
        cur_node->left->right.reset();
        cur_node->left.reset();
        old_value = get_node(cur_node)->value_leaf.value;

        cur_node = cur_node->right;
    }

    // Set the end node.

    if (get_node(end_pos)->value_leaf.key == end_key)
    {
        // The new segment ends exactly at the end node position.

        if (end_pos->right && get_node(end_pos)->value_leaf.value == val)
        {
            // Remove this node, and connect the new start node with the 
            // node that comes after this node.
            new_start_node->right = end_pos->right;
            if (end_pos->right)
                end_pos->right->left = new_start_node;
            disconnect_node(end_pos.get());
        }
        else
        {
            // Just link the new segment to this node.
            new_start_node->right = end_pos;
            end_pos->left = new_start_node;
        }
    }
    else if (old_value == val)
    {
        link_nodes(new_start_node, end_pos);
    }
    else
    {
        // Insert a new node before the insertion position node.
        node_base_ptr new_node(new node(true));
        get_node(new_node)->value_leaf.key = end_key;
        get_node(new_node)->value_leaf.value = old_value;

        // Link to the left node.
        link_nodes(new_start_node, new_node);

        // Link to the right node.
        link_nodes(new_node, end_pos);
    }

    m_valid_tree = false;
}

template<typename _Key, typename _Value>
void flat_segment_tree<_Key, _Value>::shift_segment_left(key_type start_key, key_type end_key)
{
    if (start_key >= end_key)
        return;

    key_type left_leaf_key = get_node(m_left_leaf)->value_leaf.key;
    key_type right_leaf_key = get_node(m_right_leaf)->value_leaf.key;
    if (start_key < left_leaf_key || end_key < left_leaf_key)
        // invalid key value
        return;

    if (start_key > right_leaf_key || end_key > right_leaf_key)
        // invalid key value.
        return;

    node_base_ptr node_pos;
    if (left_leaf_key == start_key)
        node_pos = m_left_leaf;
    else
        // Get the first node with a key value equal to or greater than the 
        // start key value.  But we want to skip the leftmost node.
        node_pos = get_insertion_pos_leaf(start_key, m_left_leaf->right);

    if (!node_pos)
        return;

    key_type segment_size = end_key - start_key;

    if (node_pos == m_right_leaf)
    {
        // The segment being removed begins after the last node before the 
        // right-most node.

        if (right_leaf_key <= end_key)
        {
            // The end position equals or is past the right-most node.
            append_new_segment(start_key);
        }
        else
        {
            // The end position stops before the right-most node.  Simply 
            // append the blank segment to the end.
            append_new_segment(right_leaf_key - segment_size);
        }
        return;
    }

    if (end_key < get_node(node_pos)->value_leaf.key)
    {
        // The removed segment does not overlap with any nodes.  Simply 
        // shift the key values of those nodes that come after the removed
        // segment.
        shift_leaf_key_left(node_pos, m_right_leaf, segment_size);
        append_new_segment(right_leaf_key - segment_size);
        m_valid_tree = false;
        return;
    }

    // Move the first node to the starting position, and from there search
    // for the first node whose key value is greater than the end value.
    get_node(node_pos)->value_leaf.key = start_key;
    node_base_ptr start_pos = node_pos;
    node_pos = node_pos->right;
    value_type last_seg_value = get_node(start_pos)->value_leaf.value;
    while (get_node(node_pos) != get_node(m_right_leaf) && get_node(node_pos)->value_leaf.key <= end_key)
    {
        last_seg_value = get_node(node_pos)->value_leaf.value;
        node_base_ptr next = node_pos->right;
        disconnect_node(node_pos.get());
        node_pos = next;
    }

    get_node(start_pos)->value_leaf.value = last_seg_value;
    start_pos->right = node_pos;
    node_pos->left = start_pos;
    if (start_pos->left && get_node(start_pos->left)->value_leaf.value == get_node(start_pos)->value_leaf.value)
    {
        // Removing a segment resulted in two consecutive segments with
        // identical value. Combine them by removing the 2nd redundant
        // node.
        start_pos->left->right = start_pos->right;
        start_pos->right->left = start_pos->left;
        disconnect_node(start_pos.get());
    }

    shift_leaf_key_left(node_pos, m_right_leaf, segment_size);
    m_valid_tree = false;

    // Insert at the end a new segment with the initial base value, for 
    // the length of the removed segment.
    append_new_segment(right_leaf_key - segment_size);
}

template<typename _Key, typename _Value>
void flat_segment_tree<_Key, _Value>::shift_segment_right(key_type pos, key_type size, bool skip_start_node)
{
    if (size <= 0)
        return;

    if (pos < get_node(m_left_leaf)->value_leaf.key || get_node(m_right_leaf)->value_leaf.key <= pos)
        // specified position is out-of-bound
        return;

    if (get_node(m_left_leaf)->value_leaf.key == pos)
    {
        // Position is at the leftmost node.  Shift all the other nodes, 
        // and insert a new node at (pos + size) position.
        node_base_ptr cur_node = m_left_leaf->right;
        shift_leaf_key_right(cur_node, m_right_leaf, size);

        if (get_node(m_left_leaf)->value_leaf.value != m_init_val)
        {
            // The leftmost leaf node has a non-initial value.  We need to
            // insert a new node to carry that value after the shift.
            node_base_ptr new_node(new node(true));
            get_node(new_node)->value_leaf.key = pos + size;
            get_node(new_node)->value_leaf.value = get_node(m_left_leaf)->value_leaf.value;
            get_node(m_left_leaf)->value_leaf.value = m_init_val;
            new_node->left = m_left_leaf;
            new_node->right = m_left_leaf->right;
            m_left_leaf->right = new_node;
        }

        m_valid_tree = false;
        return;
    }

    // Get the first node with a key value equal to or greater than the
    // start key value.  But we want to skip the leftmost node.
    node_base_ptr cur_node = get_insertion_pos_leaf(pos, m_left_leaf->right);

    // If the point of insertion is at an existing node position, don't 
    // shift that node but start with the one after it if that's
    // requested.
    if (skip_start_node && cur_node && get_node(cur_node)->value_leaf.key == pos)
        cur_node = cur_node->right;

    if (!cur_node)
        return;

    shift_leaf_key_right(cur_node, m_right_leaf, size);
    m_valid_tree = false;
}

}

#endif
