/*************************************************************************
 *
 * Copyright (c) 2010-2023 Kohei Yoshida
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

namespace mdds {

template<typename Key, typename Value>
typename flat_segment_tree<Key, Value>::const_segment_iterator flat_segment_tree<
    Key, Value>::const_iterator::to_segment() const
{
    const node* pos = get_pos();
    const auto* parent = get_parent();
    if (!parent || is_end_pos() || !pos || !pos->next)
        return parent->end_segment();

    return const_segment_iterator(pos, pos->next.get());
}

template<typename Key, typename Value>
flat_segment_tree<Key, Value>::const_segment_range_type::const_segment_range_type(
    node_ptr left_leaf, node_ptr right_leaf)
    : m_left_leaf(left_leaf), m_right_leaf(right_leaf)
{}

template<typename Key, typename Value>
typename flat_segment_tree<Key, Value>::const_segment_iterator flat_segment_tree<
    Key, Value>::const_segment_range_type::begin() const
{
    return const_segment_iterator(m_left_leaf.get(), m_left_leaf->next.get());
}

template<typename Key, typename Value>
typename flat_segment_tree<Key, Value>::const_segment_iterator flat_segment_tree<
    Key, Value>::const_segment_range_type::end() const
{
    return const_segment_iterator(m_right_leaf.get(), nullptr);
}

template<typename Key, typename Value>
typename flat_segment_tree<Key, Value>::const_segment_iterator flat_segment_tree<Key, Value>::begin_segment() const
{
    return const_segment_iterator(m_left_leaf.get(), m_left_leaf->next.get());
}

template<typename Key, typename Value>
typename flat_segment_tree<Key, Value>::const_segment_iterator flat_segment_tree<Key, Value>::end_segment() const
{
    return const_segment_iterator(m_right_leaf.get(), nullptr);
}

template<typename Key, typename Value>
typename flat_segment_tree<Key, Value>::const_segment_range_type flat_segment_tree<Key, Value>::segment_range() const
{
    return const_segment_range_type(m_left_leaf, m_right_leaf);
}

template<typename Key, typename Value>
flat_segment_tree<Key, Value>::flat_segment_tree(key_type min_val, key_type max_val, value_type init_val)
    : m_root_node(nullptr), m_left_leaf(new node), m_right_leaf(new node), m_init_val(std::move(init_val)),
      m_valid_tree(false)
{
    // we need to create two end nodes during initialization.
    m_left_leaf->value_leaf.key = std::move(min_val);
    m_left_leaf->value_leaf.value = m_init_val; // copy
    m_left_leaf->next = m_right_leaf;

    m_right_leaf->value_leaf.key = std::move(max_val);
    m_right_leaf->prev = m_left_leaf;

    // We don't ever use the value of the right leaf node, but we need the
    // value to be always the same, to make it easier to check for
    // equality.
    m_right_leaf->value_leaf.value = value_type{};
}

template<typename Key, typename Value>
flat_segment_tree<Key, Value>::flat_segment_tree(const flat_segment_tree& r)
    : m_root_node(nullptr), m_left_leaf(new node(*r.m_left_leaf)), m_right_leaf(), m_init_val(r.m_init_val),
      m_valid_tree(false) // tree is invalid because we only copy the leaf nodes.
{
    // Copy all the leaf nodes from the original instance.
    node* src_node = r.m_left_leaf.get();
    node_ptr dest_node = m_left_leaf;
    while (true)
    {
        dest_node->next.reset(new node(*src_node->next));

        // Move on to the next source node.
        src_node = src_node->next.get();

        // Move on to the next destination node, and have the next node point
        // back to the previous node.
        node_ptr old_node = dest_node;
        dest_node = dest_node->next;
        dest_node->prev = old_node;

        if (src_node == r.m_right_leaf.get())
        {
            // Reached the right most leaf node.  We can stop here.
            m_right_leaf = dest_node;
            break;
        }
    }
}

template<typename Key, typename Value>
flat_segment_tree<Key, Value>::flat_segment_tree(flat_segment_tree&& other)
    : m_nonleaf_node_pool(std::move(other.m_nonleaf_node_pool)), m_root_node(other.m_root_node),
      m_left_leaf(other.m_left_leaf), m_right_leaf(other.m_right_leaf), m_init_val(other.m_init_val),
      m_valid_tree(other.m_valid_tree)
{
    // NB: boost::intrusive_ptr doesn't have move constructor
    other.m_left_leaf.reset();
    other.m_right_leaf.reset();
    other.m_root_node = nullptr;
    other.m_valid_tree = false;
}

template<typename Key, typename Value>
flat_segment_tree<Key, Value>::~flat_segment_tree()
{
    destroy();
}

template<typename Key, typename Value>
flat_segment_tree<Key, Value>& flat_segment_tree<Key, Value>::operator=(const flat_segment_tree& other)
{
    flat_segment_tree copy(other);
    swap(copy);
    return *this;
}

template<typename Key, typename Value>
flat_segment_tree<Key, Value>& flat_segment_tree<Key, Value>::operator=(flat_segment_tree&& other)
{
    flat_segment_tree moved(std::move(other));
    swap(moved);
    return *this;
}

template<typename Key, typename Value>
void flat_segment_tree<Key, Value>::swap(flat_segment_tree& other)
{
    m_nonleaf_node_pool.swap(other.m_nonleaf_node_pool);
    std::swap(m_root_node, other.m_root_node);
    std::swap(m_left_leaf, other.m_left_leaf);
    std::swap(m_right_leaf, other.m_right_leaf);
    std::swap(m_init_val, other.m_init_val);
    std::swap(m_valid_tree, other.m_valid_tree);
}

template<typename Key, typename Value>
void flat_segment_tree<Key, Value>::clear()
{
    // the border nodes should not be destroyed--add a ref to keep them alive
    node_ptr left(m_left_leaf);
    node_ptr right(m_right_leaf);

    // destroy the tree
    destroy();

    // and construct the default tree
    __st::link_nodes<flat_segment_tree>(m_left_leaf, m_right_leaf);
    m_left_leaf->value_leaf.value = m_init_val;
    m_valid_tree = false;
}

template<typename Key, typename Value>
::std::pair<typename flat_segment_tree<Key, Value>::const_iterator, bool> flat_segment_tree<
    Key, Value>::insert_segment_impl(key_type start_key, key_type end_key, value_type val, bool forward)
{
    typedef std::pair<typename flat_segment_tree<Key, Value>::const_iterator, bool> ret_type;

    if (!adjust_segment_range(start_key, end_key))
        return ret_type(const_iterator(this, true), false);

    // Find the node with value that either equals or is greater than the
    // start value.

    node_ptr start_pos;
    if (forward)
    {
        const node* p = get_insertion_pos_leaf(start_key, m_left_leaf.get());
        start_pos.reset(const_cast<node*>(p));
    }
    else
    {
        const node* p = get_insertion_pos_leaf_reverse(start_key, m_right_leaf.get());
        if (p)
            start_pos = p->next;
        else
            start_pos = m_left_leaf;
    }
    if (!start_pos)
    {
        // Insertion position not found.  Bail out.
        assert(!"Insertion position not found.  Bail out");
        return ret_type(const_iterator(this, true), false);
    }

    return insert_to_pos(start_pos, std::move(start_key), std::move(end_key), std::move(val));
}

template<typename Key, typename Value>
::std::pair<typename flat_segment_tree<Key, Value>::const_iterator, bool> flat_segment_tree<Key, Value>::insert_to_pos(
    node_ptr start_pos, key_type start_key, key_type end_key, value_type val)
{
    node_ptr end_pos;
    {
        const node* p = get_insertion_pos_leaf(end_key, start_pos.get());
        end_pos.reset(const_cast<node*>(p));
    }
    if (!end_pos)
        end_pos = m_right_leaf;

    node_ptr new_start_node;
    value_type old_value;

    // Set the start node.

    bool changed = false;

    if (start_pos->value_leaf.key == start_key)
    {
        // Re-use the existing node, but save the old value for later.

        if (start_pos->prev && start_pos->prev->value_leaf.value == val)
        {
            // Extend the existing segment.
            old_value = start_pos->value_leaf.value;
            new_start_node = start_pos->prev;
        }
        else
        {
            // Update the value of the existing node.
            old_value = start_pos->value_leaf.value;
            start_pos->value_leaf.value = val;
            new_start_node = start_pos;

            changed = (old_value != val);
        }
    }
    else if (start_pos->prev->value_leaf.value == val)
    {
        // Extend the existing segment.
        old_value = start_pos->prev->value_leaf.value;
        new_start_node = start_pos->prev;
    }
    else
    {
        // Insert a new node before the insertion position node.
        node_ptr new_node(new node);
        new_node->value_leaf.key = std::move(start_key);
        new_node->value_leaf.value = val;
        new_start_node = new_node;

        node_ptr left_node = start_pos->prev;
        old_value = left_node->value_leaf.value;

        // Link to the left node.
        __st::link_nodes<flat_segment_tree>(left_node, new_node);

        // Link to the right node.
        __st::link_nodes<flat_segment_tree>(new_node, start_pos);
        changed = true;
    }

    node_ptr cur_node = new_start_node->next;
    while (cur_node != end_pos)
    {
        // Disconnect the link between the current node and the previous node.
        cur_node->prev->next.reset();
        cur_node->prev.reset();
        old_value = cur_node->value_leaf.value;

        cur_node = cur_node->next;
        changed = true;
    }

    // Set the end node.

    if (end_pos->value_leaf.key == end_key)
    {
        // The new segment ends exactly at the end node position.

        if (end_pos->next && end_pos->value_leaf.value == val)
        {
            // Remove this node, and connect the new start node with the
            // node that comes after this node.
            new_start_node->next = end_pos->next;
            if (end_pos->next)
                end_pos->next->prev = new_start_node;
            disconnect_all_nodes(end_pos.get());
            changed = true;
        }
        else if (new_start_node->next != end_pos)
        {
            // Just link the new segment to this node.
            new_start_node->next = end_pos;
            end_pos->prev = new_start_node;
            changed = true;
        }
    }
    else if (old_value == val)
    {
        if (new_start_node->next != end_pos)
        {
            __st::link_nodes<flat_segment_tree>(new_start_node, end_pos);
            changed = true;
        }
    }
    else
    {
        // Insert a new node before the insertion position node.
        node_ptr new_node(new node);
        new_node->value_leaf.key = std::move(end_key);
        new_node->value_leaf.value = old_value;

        // Link to the left node.
        __st::link_nodes<flat_segment_tree>(new_start_node, new_node);

        // Link to the right node.
        __st::link_nodes<flat_segment_tree>(new_node, end_pos);
        changed = true;
    }

    if (changed)
        m_valid_tree = false;

    return ::std::pair<const_iterator, bool>(const_iterator(this, new_start_node.get()), changed);
}

template<typename Key, typename Value>
::std::pair<typename flat_segment_tree<Key, Value>::const_iterator, bool> flat_segment_tree<Key, Value>::insert(
    const const_iterator& pos, key_type start_key, key_type end_key, value_type val)
{
    const node* p = pos.get_pos();
    if (!p || this != pos.get_parent())
    {
        // Switch to normal insert.
        return insert_front(start_key, end_key, val);
    }

    assert(p->is_leaf);

    if (start_key < p->value_leaf.key)
    {
        // Specified position is already past the start key position.  Not good.
        return insert_front(start_key, end_key, val);
    }

    if (!adjust_segment_range(start_key, end_key))
    {
        typedef std::pair<typename flat_segment_tree<Key, Value>::const_iterator, bool> ret_type;
        return ret_type(const_iterator(this, true), false);
    }

    p = get_insertion_pos_leaf(start_key, p);
    node_ptr start_pos(const_cast<node*>(p));
    return insert_to_pos(start_pos, start_key, end_key, val);
}

template<typename Key, typename Value>
void flat_segment_tree<Key, Value>::shift_left(key_type start_key, key_type end_key)
{
    if (start_key >= end_key)
        return;

    key_type left_leaf_key = m_left_leaf->value_leaf.key;
    key_type right_leaf_key = m_right_leaf->value_leaf.key;
    if (start_key < left_leaf_key || end_key < left_leaf_key)
        // invalid key value
        return;

    if (start_key > right_leaf_key || end_key > right_leaf_key)
        // invalid key value.
        return;

    node_ptr node_pos;
    if (left_leaf_key == start_key)
        node_pos = m_left_leaf;
    else
    {
        // Get the first node with a key value equal to or greater than the
        // start key value.  But we want to skip the leftmost node.
        const node* p = get_insertion_pos_leaf(start_key, m_left_leaf->next.get());
        node_pos.reset(const_cast<node*>(p));
    }

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

    if (end_key < node_pos->value_leaf.key)
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
    node_pos->value_leaf.key = start_key;
    node_ptr start_pos = node_pos;
    node_pos = node_pos->next;
    value_type last_seg_value = start_pos->value_leaf.value;
    while (node_pos.get() != m_right_leaf.get() && node_pos->value_leaf.key <= end_key)
    {
        last_seg_value = node_pos->value_leaf.value;
        node_ptr next = node_pos->next;
        disconnect_all_nodes(node_pos.get());
        node_pos = next;
    }

    start_pos->value_leaf.value = last_seg_value;
    start_pos->next = node_pos;
    node_pos->prev = start_pos;
    if (start_pos->prev && start_pos->prev->value_leaf.value == start_pos->value_leaf.value)
    {
        // Removing a segment resulted in two consecutive segments with
        // identical value. Combine them by removing the 2nd redundant
        // node.
        start_pos->prev->next = start_pos->next;
        start_pos->next->prev = start_pos->prev;
        disconnect_all_nodes(start_pos.get());
    }

    shift_leaf_key_left(node_pos, m_right_leaf, segment_size);
    m_valid_tree = false;

    // Insert at the end a new segment with the initial base value, for
    // the length of the removed segment.
    append_new_segment(right_leaf_key - segment_size);
}

template<typename Key, typename Value>
void flat_segment_tree<Key, Value>::shift_right(key_type pos, key_type size, bool skip_start_node)
{
    if (size <= 0)
        return;

    if (pos < m_left_leaf->value_leaf.key || m_right_leaf->value_leaf.key <= pos)
        // specified position is out-of-bound
        return;

    if (m_left_leaf->value_leaf.key == pos)
    {
        // Position is at the leftmost node.  Shift all the other nodes,
        // and insert a new node at (pos + size) position.
        node_ptr cur_node = m_left_leaf->next;
        shift_leaf_key_right(cur_node, m_right_leaf, size);

        if (m_left_leaf->value_leaf.value != m_init_val && !skip_start_node)
        {
            if (size < m_right_leaf->value_leaf.key - m_left_leaf->value_leaf.key)
            {
                // The leftmost leaf node has a non-initial value.  We need to
                // insert a new node to carry that value after the shift.
                node_ptr new_node(new node);
                new_node->value_leaf.key = pos + size;
                new_node->value_leaf.value = m_left_leaf->value_leaf.value;
                m_left_leaf->value_leaf.value = m_init_val;
                new_node->prev = m_left_leaf;
                new_node->next = m_left_leaf->next;
                m_left_leaf->next->prev = new_node;
                m_left_leaf->next = new_node;
            }
            else
            {
                // We shifted out the whole range, so there would be no new
                // node inserted. Just set default value.
                m_left_leaf->value_leaf.value = m_init_val;
            }
        }

        m_valid_tree = false;
        return;
    }

    // Get the first node with a key value equal to or greater than the
    // start key value.  But we want to skip the leftmost node.
    const node* p = get_insertion_pos_leaf(pos, m_left_leaf->next.get());
    node_ptr cur_node(const_cast<node*>(p));

    // If the point of insertion is at an existing node position, don't
    // shift that node but start with the one after it if that's
    // requested.
    if (skip_start_node && cur_node && cur_node->value_leaf.key == pos)
        cur_node = cur_node->next;

    if (!cur_node)
        return;

    shift_leaf_key_right(cur_node, m_right_leaf, size);
    m_valid_tree = false;
}

template<typename Key, typename Value>
::std::pair<typename flat_segment_tree<Key, Value>::const_iterator, bool> flat_segment_tree<Key, Value>::search_impl(
    const node* pos, key_type key, value_type& value, key_type* start_key, key_type* end_key) const
{
    typedef ::std::pair<const_iterator, bool> ret_type;

    if (pos->value_leaf.key == key)
    {
        value = pos->value_leaf.value;
        if (start_key)
            *start_key = pos->value_leaf.key;
        if (end_key && pos->next)
            *end_key = pos->next->value_leaf.key;
        return ret_type(const_iterator(this, pos), true);
    }
    else if (pos->prev && pos->prev->value_leaf.key < key)
    {
        value = pos->prev->value_leaf.value;
        if (start_key)
            *start_key = pos->prev->value_leaf.key;
        if (end_key)
            *end_key = pos->value_leaf.key;
        return ret_type(const_iterator(this, pos->prev.get()), true);
    }

    return ret_type(const_iterator(this, true), false);
}

template<typename Key, typename Value>
::std::pair<typename flat_segment_tree<Key, Value>::const_iterator, bool> flat_segment_tree<Key, Value>::search(
    key_type key, value_type& value, key_type* start_key, key_type* end_key) const
{
    typedef ::std::pair<const_iterator, bool> ret_type;

    if (key < m_left_leaf->value_leaf.key || m_right_leaf->value_leaf.key <= key)
        // key value is out-of-bound.
        return ret_type(const_iterator(this, true), false);

    const node* pos = get_insertion_pos_leaf(key, m_left_leaf.get());
    return search_impl(pos, key, value, start_key, end_key);
}

template<typename Key, typename Value>
::std::pair<typename flat_segment_tree<Key, Value>::const_iterator, bool> flat_segment_tree<Key, Value>::search(
    const const_iterator& pos, key_type key, value_type& value, key_type* start_key, key_type* end_key) const
{
    typedef ::std::pair<const_iterator, bool> ret_type;

    if (key < m_left_leaf->value_leaf.key || m_right_leaf->value_leaf.key <= key)
        // key value is out-of-bound.
        return ret_type(const_iterator(this, true), false);

    const node* p = pos.get_pos();
    if (!p || this != pos.get_parent())
    {
        // Switch to normal search.
        return search(key, value, start_key, end_key);
    }

    assert(p->is_leaf);

    if (key < p->value_leaf.key)
    {
        // Specified position is already past the start key position.  Fall
        // back to normal search.
        return search(key, value, start_key, end_key);
    }

    p = get_insertion_pos_leaf(key, p);
    return search_impl(p, key, value, start_key, end_key);
}

template<typename Key, typename Value>
typename flat_segment_tree<Key, Value>::const_iterator flat_segment_tree<Key, Value>::search(key_type key) const
{
    return search_by_key_impl(m_left_leaf.get(), key);
}

template<typename Key, typename Value>
typename flat_segment_tree<Key, Value>::const_iterator flat_segment_tree<Key, Value>::search(
    const const_iterator& pos, key_type key) const
{
    return search_by_key_impl(pos.get_pos(), key);
}

template<typename Key, typename Value>
typename flat_segment_tree<Key, Value>::const_iterator flat_segment_tree<Key, Value>::search_by_key_impl(
    const node* start_pos, key_type key) const
{
    if (key < m_left_leaf->value_leaf.key || m_right_leaf->value_leaf.key <= key)
        // key value is out-of-bound.
        return const_iterator(this, true);

    const node* pos = get_insertion_pos_leaf(key, start_pos);
    if (!pos)
        return const_iterator(this, true);

    if (pos->value_leaf.key == key)
        return const_iterator(this, pos);
    else if (pos->prev && pos->prev->value_leaf.key < key)
        return const_iterator(this, pos->prev.get());

    return const_iterator(this, true);
}

template<typename Key, typename Value>
std::pair<typename flat_segment_tree<Key, Value>::const_iterator, bool> flat_segment_tree<Key, Value>::search_tree(
    key_type key, value_type& value, key_type* start_key, key_type* end_key) const
{
    using ret_type = std::pair<const_iterator, bool>;

    const node* dest_node = search_tree_for_leaf_node(key);
    if (!dest_node)
        return ret_type(const_iterator(this, true), false);

    value = dest_node->value_leaf.value;
    if (start_key)
        *start_key = dest_node->value_leaf.key;

    if (end_key)
    {
        assert(dest_node->next);
        if (dest_node->next)
            *end_key = dest_node->next->value_leaf.key;
        else
            // This should never happen, but just in case....
            *end_key = m_right_leaf->value_leaf.key;
    }

    return ret_type(const_iterator(this, dest_node), true);
}

template<typename Key, typename Value>
typename flat_segment_tree<Key, Value>::const_iterator flat_segment_tree<Key, Value>::search_tree(key_type key) const
{
    const node* dest_node = search_tree_for_leaf_node(key);
    if (!dest_node)
        return const_iterator(this, true);

    return const_iterator(this, dest_node);
}

template<typename Key, typename Value>
const typename flat_segment_tree<Key, Value>::node* flat_segment_tree<Key, Value>::search_tree_for_leaf_node(
    key_type key) const
{
    if (!m_root_node || !m_valid_tree)
    {
        // either tree has not been built, or is in an invalid state.
        return nullptr;
    }

    if (key < m_left_leaf->value_leaf.key || m_right_leaf->value_leaf.key <= key)
    {
        // key value is out-of-bound.
        return nullptr;
    }

    // Descend down the tree through the last non-leaf layer.

    const nonleaf_node* cur_node = m_root_node;
    while (true)
    {
        if (cur_node->left)
        {
            if (cur_node->left->is_leaf)
                break;

            const nonleaf_node* left_nonleaf = static_cast<const nonleaf_node*>(cur_node->left);
            const nonleaf_value_type& v = left_nonleaf->value_nonleaf;
            if (v.low <= key && key < v.high)
            {
                // Descend one level through the left child node.
                cur_node = left_nonleaf;
                continue;
            }
        }
        else
        {
            // left child node can't be missing !
            return nullptr;
        }

        if (cur_node->right)
        {
            assert(!cur_node->right->is_leaf);
            const nonleaf_node* right_nonleaf = static_cast<const nonleaf_node*>(cur_node->right);
            const nonleaf_value_type& v = right_nonleaf->value_nonleaf;
            if (v.low <= key && key < v.high)
            {
                // Descend one level through the right child node.
                cur_node = right_nonleaf;
                continue;
            }
        }
        return nullptr;
    }

    // Current node must be a non-leaf whose child nodes are leaf nodes.
    assert(cur_node->left->is_leaf && cur_node->right->is_leaf);

    const node* dest_node = nullptr;
    const node* leaf_left = static_cast<const node*>(cur_node->left);
    const node* leaf_right = static_cast<const node*>(cur_node->right);
    key_type key1 = leaf_left->value_leaf.key;
    key_type key2 = leaf_right->value_leaf.key;

    if (key1 <= key && key < key2)
    {
        dest_node = leaf_left;
    }
    else if (key2 <= key && key < cur_node->value_nonleaf.high)
    {
        dest_node = leaf_right;
    }

    if (!dest_node)
        return nullptr;

    return dest_node;
}

template<typename Key, typename Value>
void flat_segment_tree<Key, Value>::build_tree()
{
    if (!m_left_leaf)
        return;

    m_nonleaf_node_pool.clear();

    // Count the number of leaf nodes.
    size_t leaf_count = leaf_size();

    // Determine the total number of non-leaf nodes needed to build the whole tree.
    size_t nonleaf_count = __st::count_needed_nonleaf_nodes(leaf_count);

    m_nonleaf_node_pool.resize(nonleaf_count);
    mdds::__st::tree_builder<flat_segment_tree> builder(m_nonleaf_node_pool);
    m_root_node = builder.build(m_left_leaf);
    m_valid_tree = true;
}

template<typename Key, typename Value>
typename flat_segment_tree<Key, Value>::size_type flat_segment_tree<Key, Value>::leaf_size() const
{
    return __st::count_leaf_nodes(m_left_leaf.get(), m_right_leaf.get());
}

template<typename Key, typename Value>
bool flat_segment_tree<Key, Value>::operator==(const flat_segment_tree& other) const
{
    const node* n1 = m_left_leaf.get();
    const node* n2 = other.m_left_leaf.get();

    if ((!n1 && n2) || (n1 && !n2))
        // Either one of them is nullptr;
        return false;

    while (n1)
    {
        if (!n2)
            return false;

        if (!n1->equals(*n2))
            return false;

        n1 = n1->next.get();
        n2 = n2->next.get();
    }

    if (n2)
        // n1 is nullptr, but n2 is not.
        return false;

    // All leaf nodes are equal.
    return true;
}

template<typename Key, typename Value>
const typename flat_segment_tree<Key, Value>::node* flat_segment_tree<Key, Value>::get_insertion_pos_leaf_reverse(
    const key_type& key, const node* start_pos) const
{
    const node* cur_node = start_pos;
    while (cur_node)
    {
        if (cur_node->value_leaf.key < key)
        {
            // Found the insertion position.
            return cur_node;
        }
        cur_node = cur_node->prev.get();
    }
    return nullptr;
}

template<typename Key, typename Value>
const typename flat_segment_tree<Key, Value>::node* flat_segment_tree<Key, Value>::get_insertion_pos_leaf(
    const key_type& key, const node* start_pos) const
{
    assert(m_left_leaf->value_leaf.key <= key);

    const node* cur_node = start_pos;
    while (cur_node)
    {
        if (key <= cur_node->value_leaf.key)
        {
            // Found the insertion position.
            return cur_node;
        }
        cur_node = cur_node->next.get();
    }
    return nullptr;
}

template<typename Key, typename Value>
void flat_segment_tree<Key, Value>::destroy()
{
    disconnect_leaf_nodes(m_left_leaf.get(), m_right_leaf.get());
    m_nonleaf_node_pool.clear();
    m_root_node = nullptr;
}

template<typename Key, typename Value>
bool flat_segment_tree<Key, Value>::adjust_segment_range(key_type& start_key, key_type& end_key) const
{
    if (end_key <= start_key)
        // Invalid order of segment range.
        return false;

    if (end_key < m_left_leaf->value_leaf.key || m_right_leaf->value_leaf.key <= start_key)
        // The new segment does not overlap the current interval.
        return false;

    if (start_key < m_left_leaf->value_leaf.key)
        // The start value should not be smaller than the current minimum.
        start_key = m_left_leaf->value_leaf.key;

    if (m_right_leaf->value_leaf.key < end_key)
        // The end value should not be larger than the current maximum.
        end_key = m_right_leaf->value_leaf.key;

    return true;
}

} // namespace mdds
