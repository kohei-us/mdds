/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2008-2023 Kohei Yoshida
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

#pragma once

#include <iostream>
#include <sstream>
#include <utility>
#include <cassert>
#include <type_traits>

#include "./node.hpp"
#include "./flat_segment_tree_itr.hpp"
#include "./global.hpp"

#ifdef MDDS_UNIT_TEST
#include <cstdio>
#include <vector>
#endif

namespace mdds {

template<typename Key, typename Value>
class flat_segment_tree
{
public:
    typedef Key key_type;
    typedef Value value_type;
    typedef size_t size_type;

    struct nonleaf_value_type
    {
        bool operator==(const nonleaf_value_type&) const noexcept
        {
            return true;
        }
    };

    struct leaf_value_type
    {
        value_type value;

        bool operator==(const leaf_value_type& r) const
            noexcept(noexcept(std::declval<value_type>() == std::declval<value_type>()))
        {
            return value == r.value;
        }

        bool operator!=(const leaf_value_type& r) const noexcept(noexcept(!operator==(r)))
        {
            return !operator==(r);
        }

        leaf_value_type() : value{}
        {}
    };

    using node = st::detail::node<key_type, leaf_value_type>;
    using node_ptr = typename node::node_ptr;
    using nonleaf_node = st::detail::nonleaf_node<key_type, nonleaf_value_type>;

private:
    friend struct ::mdds::fst::detail::forward_itr_handler<flat_segment_tree>;
    friend struct ::mdds::fst::detail::reverse_itr_handler<flat_segment_tree>;

    static constexpr bool nothrow_move_constructible_v =
        std::is_nothrow_move_constructible_v<std::vector<nonleaf_node>> &&
        std::is_nothrow_move_constructible_v<value_type>;

    static constexpr bool nothrow_swappable_v =
        std::is_nothrow_swappable_v<value_type> && std::is_nothrow_swappable_v<std::vector<nonleaf_node>>;

    static constexpr bool nothrow_move_assignable_v = nothrow_move_constructible_v && nothrow_swappable_v;

    static constexpr bool nothrow_eq_comparable_v =
        noexcept(std::declval<key_type>() == std::declval<key_type>()) &&
        noexcept(std::declval<leaf_value_type>() == std::declval<leaf_value_type>());

public:
    using const_segment_iterator = mdds::fst::detail::const_segment_iterator<flat_segment_tree>;

    class const_iterator : public ::mdds::fst::detail::const_iterator_base<
                               flat_segment_tree, ::mdds::fst::detail::forward_itr_handler<flat_segment_tree>>
    {
        typedef ::mdds::fst::detail::const_iterator_base<
            flat_segment_tree, ::mdds::fst::detail::forward_itr_handler<flat_segment_tree>>
            base_type;
        friend class flat_segment_tree;

        using base_type::get_parent;
        using base_type::get_pos;
        using base_type::is_end_pos;

    public:
        const_iterator() : base_type(nullptr, false)
        {}

        /**
         * Create a segment iterator that references the same segment the source
         * iterator references the start key of.
         */
        const_segment_iterator to_segment() const;

    private:
        explicit const_iterator(const typename base_type::fst_type* _db, bool _end) : base_type(_db, _end)
        {}

        explicit const_iterator(const typename base_type::fst_type* _db, const node* p) : base_type(_db, p)
        {}
    };

    class const_reverse_iterator : public ::mdds::fst::detail::const_iterator_base<
                                       flat_segment_tree, ::mdds::fst::detail::reverse_itr_handler<flat_segment_tree>>
    {
        typedef ::mdds::fst::detail::const_iterator_base<
            flat_segment_tree, ::mdds::fst::detail::reverse_itr_handler<flat_segment_tree>>
            base_type;
        friend class flat_segment_tree;

    public:
        const_reverse_iterator() : base_type(nullptr, false)
        {}

    private:
        explicit const_reverse_iterator(const typename base_type::fst_type* _db, bool _end) : base_type(_db, _end)
        {}
    };

    class const_segment_range_type
    {
        node_ptr m_left_leaf;
        node_ptr m_right_leaf;

    public:
        const_segment_range_type(node_ptr left_leaf, node_ptr right_leaf);

        const_segment_iterator begin() const;
        const_segment_iterator end() const;
    };

    /**
     * Return an iterator that points to the first leaf node that corresponds
     * with the start position of the first segment.
     *
     * @return immutable iterator that points to the first leaf node that
     *         corresponds with the start position of the first segment.
     */
    const_iterator begin() const
    {
        return const_iterator(this, false);
    }

    /**
     * Return an iterator that points to the position past the last leaf node
     * that corresponds with the end position of the last segment.
     *
     * @return immutable iterator that points to the position past last leaf
     *         node that corresponds with the end position of the last
     *         segment.
     */
    const_iterator end() const
    {
        return const_iterator(this, true);
    }

    /**
     * Return an iterator that points to the last leaf node that corresponds
     * with the end position of the last segment.  This iterator moves in the
     * reverse direction of a normal iterator.
     *
     * @return immutable reverse iterator that points to the last leaf node
     *         that corresponds with the end position of the last segment.
     */
    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(this, false);
    }

    /**
     * Return an iterator that points to the position past the first leaf node
     * that corresponds with the start position of the first segment. This
     * iterator moves in the reverse direction of a normal iterator.
     *
     * @return immutable reverse iterator that points to the position past
     *         first leaf node that corresponds with the start position of the
     *         first segment.
     */
    const_reverse_iterator rend() const
    {
        return const_reverse_iterator(this, true);
    }

    /**
     * Return an immutable iterator that points to the first segment stored in
     * the tree.  It iterates through the segments one segment at a time.
     * Each iterator value consists of <code>start</code>, <code>end</code>,
     * and <code>value</code> members that correspond with the start and end
     * positions of a segment and the value of that segment, respectively.
     *
     * @return immutable iterator that points to the first segment stored in
     *         the tree.
     */
    const_segment_iterator begin_segment() const;

    /**
     * Return an immutable iterator that points to the position past the last
     * segment stored in the tree.  It iterates through the segments one
     * segment at a time.  Each iterator value consists of <code>start</code>,
     * <code>end</code>, and <code>value</code> members that correspond with
     * the start and end positions of a segment and the value of that segment,
     * respectively.
     *
     * @return immutable iterator that points to the position past the last
     *         segment stored in the tree.
     */
    const_segment_iterator end_segment() const;

    /**
     * Return a range object that provides a begin iterator and an end sentinel.
     */
    const_segment_range_type segment_range() const;

    flat_segment_tree() = delete;

    /**
     * Constructor that takes minimum and maximum keys and the value to be
     * used for the initial segment.
     *
     * @param min_val minimum allowed key value for the entire series of
     *                segments.
     * @param max_val maximum allowed key value for the entires series of
     *                segments.
     * @param init_val value to be used for the initial segment. This value
     *                 will also be used for empty segments.
     */
    flat_segment_tree(key_type min_val, key_type max_val, value_type init_val);

    /**
     * Copy constructor only copies the leaf nodes.
     */
    flat_segment_tree(const flat_segment_tree& r);

    /**
     * Move constructor.
     *
     * @warning The source instance will not be usable after the move
     *          construction.
     */
    flat_segment_tree(flat_segment_tree&& other) noexcept(nothrow_move_constructible_v);

    ~flat_segment_tree();

    /**
     * Copy assignment operator.
     *
     * @param other Source instance to copy from.
     *
     * @note It only copies the leaf nodes.
     */
    flat_segment_tree<Key, Value>& operator=(const flat_segment_tree& other);

    /**
     * Move assignment operator.
     *
     * @param other Source instance to move from.
     */
    flat_segment_tree<Key, Value>& operator=(flat_segment_tree&& other) noexcept(nothrow_move_assignable_v);

    /**
     * Swap the content of the tree with another instance.
     *
     * @param other instance of flat_segment_tree to swap content with.
     */
    void swap(flat_segment_tree& other) noexcept(nothrow_swappable_v);

    /**
     * Remove all stored segments except for the initial segment. The minimum
     * and maximum keys and the default value will be retained after the call
     * returns.  This call will also remove the tree.
     */
    void clear();

    /**
     * Insert a new segment into the tree.  It searches for the point of
     * insertion from the first leaf node.
     *
     * @param start_key start value of the segment being inserted.  The value
     *              is inclusive.
     * @param end_key end value of the segment being inserted.  The value is
     *            not inclusive.
     * @param val value associated with this segment.
     *
     * @return pair of const_iterator corresponding to the start position of
     *         the inserted segment, and a boolean value indicating whether or
     *         not the insertion has modified the tree.
     */
    std::pair<const_iterator, bool> insert_front(key_type start_key, key_type end_key, value_type val)
    {
        return insert_segment_impl(std::move(start_key), std::move(end_key), std::move(val), true);
    }

    /**
     * Insert a new segment into the tree.  Unlike the insert_front()
     * counterpart, this method searches for the point of insertion from the
     * last leaf node toward the first.
     *
     * @param start_key start value of the segment being inserted.  The value
     *              is inclusive.
     * @param end_key end value of the segment being inserted.  The value is
     *            not inclusive.
     * @param val value associated with this segment.
     *
     * @return pair of const_iterator corresponding to the start position of
     *         the inserted segment, and a boolean value indicating whether or
     *         not the insertion has modified the tree.
     */
    std::pair<const_iterator, bool> insert_back(key_type start_key, key_type end_key, value_type val)
    {
        return insert_segment_impl(std::move(start_key), std::move(end_key), std::move(val), false);
    }

    /**
     * Insert a new segment into the tree at or after specified point of
     * insertion.
     *
     * @param pos specified insertion point
     * @param start_key start value of the segment being inserted.  The value
     *              is inclusive.
     * @param end_key end value of the segment being inserted.  The value is
     *            not inclusive.
     * @param val value associated with this segment.
     *
     * @return pair of const_iterator corresponding to the start position of
     *         the inserted segment, and a boolean value indicating whether or
     *         not the insertion has modified the tree.
     */
    std::pair<const_iterator, bool> insert(const_iterator pos, key_type start_key, key_type end_key, value_type val);

    /**
     * Remove a segment specified by the start and end key values, and shift
     * the remaining segments (i.e. those segments that come after the removed
     * segment) to left.  Note that the start and end positions of the segment
     * being removed <b>must</b> be within the base segment span.
     *
     * @param start_key start position of the segment being removed.
     * @param end_key end position of the segment being removed.
     */
    void shift_left(key_type start_key, key_type end_key);

    /**
     * Shift all segments that occur at or after the specified start position
     * to right by the size specified.
     *
     * @param pos position where the right-shift occurs.
     * @param size amount of shift (must be greater than 0)
     * @param skip_start_node if true, and the specified position is at an
     *                        existing node position, that node will
     *                        <i>not</i> be shifted.  This argument has no
     *                        effect if the position specified does not
     *                        coincide with any of the existing nodes.
     */
    void shift_right(key_type pos, key_type size, bool skip_start_node);

    /**
     * Perform leaf-node search for a value associated with a key.
     *
     * @param key key value
     * @param value value associated with key specified gets stored upon
     *              successful search.
     * @param start_key pointer to a variable where the start key value of the
     *                  segment that contains the key gets stored upon
     *                  successful search.
     * @param end_key pointer to a variable where the end key value of the
     *                segment that contains the key gets stored upon
     *                successful search.
     * @return a pair of const_iterator corresponding to the start position of
     *         the segment containing the key, and a boolean value indicating
     *         whether or not the search has been successful.
     *
     */
    std::pair<const_iterator, bool> search(
        key_type key, value_type& value, key_type* start_key = nullptr, key_type* end_key = nullptr) const;

    /**
     * Perform leaf-node search for a value associated with a key.
     *
     * @param pos position from which the search should start.  When the
     *            position is invalid, it falls back to the normal search.
     * @param key key value.
     * @param value value associated with key specified gets stored upon
     *              successful search.
     * @param start_key pointer to a variable where the start key value of the
     *                  segment that contains the key gets stored upon
     *                  successful search.
     * @param end_key pointer to a variable where the end key value of the
     *                segment that contains the key gets stored upon
     *                successful search.
     * @return a pair of const_iterator corresponding to the start position of
     *         the segment containing the key, and a boolean value indicating
     *         whether or not the search has been successful.
     */
    std::pair<const_iterator, bool> search(
        const_iterator pos, key_type key, value_type& value, key_type* start_key = nullptr,
        key_type* end_key = nullptr) const;

    /**
     * Perform leaf-node search for a value associated with a key.
     *
     * @param key Key value to perform search for.
     *
     * @return Iterator position associated with the start position of the
     *         segment containing the key, or end iterator position upon search
     *         failure.
     */
    const_iterator search(key_type key) const;

    /**
     * Perform leaf-node search for a value associated with a key.
     *
     * @param pos Position from which the search should start if valid. In case
     *            of an invalid position, it falls back to a search starting
     *            with the first position.
     * @param key Key value to perform search for.
     *
     * @return Iterator position associated with the start position of the
     *         segment containing the key, or end iterator position if the
     *         search has failed.
     */
    const_iterator search(const_iterator pos, key_type key) const;

    /**
     * Perform tree search for a value associated with a key.  This method
     * assumes that the tree is valid.  Call is_tree_valid() to find out
     * whether the tree is valid, and build_tree() to build a new tree in case
     * it's not.
     *
     * @param key key value
     * @param value value associated with key specified gets stored upon
     *              successful search.
     * @param start_key pointer to a variable where the start key value of the
     *                  segment that contains the key gets stored upon
     *                  successful search.
     * @param end_key pointer to a variable where the end key value of the
     *                segment that contains the key gets stored upon
     *                successful search.
     * @return a pair of const_iterator corresponding to the start position of
     *         the segment containing the key, and a boolean value indicating
     *         whether or not the search has been successful.
     */
    std::pair<const_iterator, bool> search_tree(
        key_type key, value_type& value, key_type* start_key = nullptr, key_type* end_key = nullptr) const;

    /**
     * Perform tree search for a value associated with a key.  The tree must be
     * valid before performing the search, else the search will fail.
     *
     * @param key Key value to perform search for.
     *
     * @return Iterator position associated with the start position of the
     *         segment containing the key, or end iterator position if the
     *         search has failed.
     */
    const_iterator search_tree(key_type key) const;

    /**
     * Build a tree of non-leaf nodes based on the values stored in the leaf
     * nodes.  The tree must be valid before you can call the search_tree()
     * method.
     */
    void build_tree();

    /**
     * @return true if the tree is valid, otherwise false.  The tree must be
     *         valid before you can call the search_tree() method.
     */
    bool valid_tree() const noexcept
    {
        return m_valid_tree;
    }

    /**
     * Equality between two flat_segment_tree instances is evaluated by
     * comparing the keys and the values of the leaf nodes only.  Neither the
     * non-leaf nodes nor the validity of the tree is evaluated.
     */
    bool operator==(const flat_segment_tree& other) const noexcept(nothrow_eq_comparable_v);

    bool operator!=(const flat_segment_tree& other) const noexcept(nothrow_eq_comparable_v)
    {
        return !operator==(other);
    }

    const key_type& min_key() const noexcept
    {
        return m_left_leaf->key;
    }

    const key_type& max_key() const noexcept
    {
        return m_right_leaf->key;
    }

    const value_type& default_value() const noexcept
    {
        return m_init_val;
    }

    /**
     * Return the number of leaf nodes.
     *
     * @return number of leaf nodes.
     */
    size_type leaf_size() const
        noexcept(noexcept(st::detail::count_leaf_nodes<size_type>(m_left_leaf.get(), m_right_leaf.get())))
    {
        return st::detail::count_leaf_nodes<size_type>(m_left_leaf.get(), m_right_leaf.get());
    }

#ifdef MDDS_UNIT_TEST
    const nonleaf_node* get_root_node() const
    {
        return m_root_node;
    }

    struct tree_dumper_traits
    {
        using leaf_type = node;
        using nonleaf_type = nonleaf_node;
        using tree_type = flat_segment_tree;

        struct to_string
        {
            to_string(const tree_type&)
            {}

            std::string operator()(const leaf_type& leaf) const
            {
                return leaf.to_string();
            }

            std::string operator()(const nonleaf_type& nonleaf) const
            {
                return nonleaf.to_string();
            }
        };
    };

    void dump_tree() const
    {
        using ::std::cout;
        using ::std::endl;

        if (!m_valid_tree)
            assert(!"attempted to dump an invalid tree!");

        size_t node_count = mdds::st::detail::tree_dumper<tree_dumper_traits>::dump(std::cout, *this, m_root_node);
        size_t node_instance_count = node::get_instance_count();
        size_t leaf_count = leaf_size();

        cout << "tree node count = " << node_count << "; node instance count = " << node_instance_count
             << "; leaf node count = " << leaf_count << endl;

        assert(leaf_count == node_instance_count);
    }

    void dump_leaf_nodes() const
    {
        using ::std::cout;
        using ::std::endl;

        cout << "------------------------------------------" << endl;

        node_ptr cur_node = m_left_leaf;
        long node_id = 0;
        while (cur_node)
        {
            cout << "  node " << node_id++ << ": key = " << cur_node->key << "; value = " << cur_node->value_leaf.value
                 << endl;
            cur_node = cur_node->next;
        }
        cout << endl << "  node instance count = " << node::get_instance_count() << endl;
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
        {
            // Start from the left-most node, and traverse right.
            node* cur_node = m_left_leaf.get();
            typename ::std::vector<key_type>::const_iterator itr = key_values.begin(), itr_end = key_values.end();
            for (; itr != itr_end; ++itr)
            {
                if (!cur_node)
                    // Position past the right-mode node.  Invalid.
                    return false;

                if (cur_node->key != *itr)
                    // Key values differ.
                    return false;

                cur_node = cur_node->next.get();
            }

            if (cur_node)
                // At this point, we expect the current node to be at the position
                // past the right-most node, which is nullptr.
                return false;
        }

        {
            // Start from the right-most node, and traverse left.
            node* cur_node = m_right_leaf.get();
            typename ::std::vector<key_type>::const_reverse_iterator itr = key_values.rbegin(),
                                                                     itr_end = key_values.rend();
            for (; itr != itr_end; ++itr)
            {
                if (!cur_node)
                    // Position past the left-mode node.  Invalid.
                    return false;

                if (cur_node->key != *itr)
                    // Key values differ.
                    return false;

                cur_node = cur_node->prev.get();
            }

            if (cur_node)
                // Likewise, we expect the current position to be past the
                // left-most node, in which case the node value is nullptr.
                return false;
        }

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
        node* cur_node = m_left_leaf.get();
        node* end_node = m_right_leaf.get();
        typename ::std::vector<value_type>::const_iterator itr = values.begin(), itr_end = values.end();
        for (; itr != itr_end; ++itr)
        {
            if (cur_node == end_node || !cur_node)
                return false;

            if (cur_node->value_leaf.value != *itr)
                // Key values differ.
                return false;

            cur_node = cur_node->next.get();
        }

        if (cur_node != end_node)
            // At this point, we expect the current node to be at the end of
            // range.
            return false;

        return true;
    }
#endif

private:
    const_iterator search_by_key_impl(const node* start_pos, key_type key) const;

    const node* search_tree_for_leaf_node(key_type key) const;

    void append_new_segment(key_type start_key)
    {
        if (m_right_leaf->prev->key == start_key)
        {
            m_right_leaf->prev->value_leaf.value = m_init_val;
            return;
        }

#ifdef MDDS_UNIT_TEST
        // The start position must come after the position of the last node
        // before the right-most node.
        assert(m_right_leaf->prev->key < start_key);
#endif

        if (m_right_leaf->prev->value_leaf.value == m_init_val)
            // The existing segment has the same value.  No need to insert a
            // new segment.
            return;

        node_ptr new_node(new node);
        new_node->key = start_key;
        new_node->value_leaf.value = m_init_val;
        new_node->prev = m_right_leaf->prev;
        new_node->next = m_right_leaf;
        m_right_leaf->prev->next = new_node;
        m_right_leaf->prev = std::move(new_node);
        m_valid_tree = false;
    }

    ::std::pair<const_iterator, bool> insert_segment_impl(
        key_type start_key, key_type end_key, value_type val, bool forward);

    ::std::pair<const_iterator, bool> insert_to_pos(
        node_ptr start_pos, key_type start_key, key_type end_key, value_type val);

    ::std::pair<const_iterator, bool> search_impl(
        const node* pos, key_type key, value_type& value, key_type* start_key, key_type* end_key) const;

    const node* get_insertion_pos_leaf_reverse(const key_type& key, const node* start_pos) const;

    /**
     * Find a node with the largest value whose key equals or less than a
     * specified key, starting with a specific node.
     *
     * @pre The caller must ensure that the key equals or greater than the min
     *      key.
     *
     * @note If the key exceeds the max key value, it will return
     *       <code>nullptr</code>.
     */
    const node* get_insertion_pos_leaf(const key_type& key, const node* start_pos) const;

    static void shift_leaf_key_left(node_ptr& begin_node, node_ptr& end_node, key_type shift_value)
    {
        node* cur_node_p = begin_node.get();
        node* end_node_p = end_node.get();
        while (cur_node_p != end_node_p)
        {
            cur_node_p->key -= shift_value;
            cur_node_p = cur_node_p->next.get();
        }
    }

    static void shift_leaf_key_right(node_ptr& cur_node, node_ptr& end_node, key_type shift_value)
    {
        key_type end_node_key = end_node->key;
        while (cur_node.get() != end_node.get())
        {
            cur_node->key += shift_value;
            if (cur_node->key < end_node_key)
            {
                // The node is still in-bound.  Keep shifting.
                cur_node = cur_node->next;
                continue;
            }

            // This node has been pushed outside the end node position.
            // Remove all nodes that follows, and connect the previous node
            // with the end node.

            node_ptr last_node = cur_node->prev;
            while (cur_node.get() != end_node.get())
            {
                node_ptr next_node = cur_node->next;
                disconnect_all_nodes(cur_node.get());
                cur_node = std::move(next_node);
            }
            last_node->next = end_node;
            end_node->prev = std::move(last_node);
            return;
        }
    }

    void destroy();

    /**
     * Check and optionally adjust the start and end key values if one of them
     * is out-of-bound.
     *
     * @return true if the start and end key values are valid, either with or
     *         without adjustments, otherwise false.
     */
    bool adjust_segment_range(key_type& start_key, key_type& end_key) const;

private:
    std::vector<nonleaf_node> m_nonleaf_node_pool;

    const nonleaf_node* m_root_node;
    node_ptr m_left_leaf;
    node_ptr m_right_leaf;
    value_type m_init_val;
    bool m_valid_tree;
};

template<typename Key, typename Value>
void swap(flat_segment_tree<Key, Value>& left, flat_segment_tree<Key, Value>& right) noexcept(
    std::is_nothrow_swappable_v<flat_segment_tree<Key, Value>>)
{
    left.swap(right);
}

} // namespace mdds

#include "flat_segment_tree_def.inl"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
