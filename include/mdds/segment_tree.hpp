/*************************************************************************
 *
 * Copyright (c) 2010-2015 Kohei Yoshida
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

#include "./node.hpp"
#include "./global.hpp"

#include <vector>
#include <deque>
#include <iostream>
#include <map>
#include <unordered_map>
#include <memory>
#include <sstream>

namespace mdds {

/**
 * Segment tree is a data structure designed for storing one-dimensional
 * intervals or segments, either overlapping or non-overlapping.
 * It is useful for detecting all the segments that contain a specific point.
 *
 * Each segment has start and end positions where the start position is
 * inclusive while the end position is not. This segment tree implementation
 * allows associating a value with each segment so that you can use it as an
 * associative container.
 *
 * @tparam KeyT Key type.  The key type must be copyable.  If it's moveable
 *         then it gets moved instead of copied where possible.  Additionally,
 *         the key type must support the @p <, @p <= and @p == operators.
 *         To use to_string(), the key type must support the ostream operator.
 * @tparam ValueT Value type.  The value type does not need to be copyable but
 *         must be at least moveable.  Additionally, the value type must
 *         support the @p == operator.  To use to_string(), the value type
 *         must support the ostream operator.
 */
template<typename KeyT, typename ValueT>
class segment_tree
{
public:
    /** The key type must be copyable, and may be moveable. */
    using key_type = KeyT;
    /** The value type must be either copyable or moveable. */
    using value_type = ValueT;
    using size_type = std::size_t;

private:
    struct segment_type
    {
        key_type start;
        key_type end;
        value_type value;

        segment_type();
        segment_type(key_type _start, key_type _end, value_type _value);
        segment_type(const segment_type&) = default;
        segment_type(segment_type&&) = default;

        segment_type& operator=(const segment_type& r) = default;
        segment_type& operator=(segment_type&& r) = default;
        bool operator<(const segment_type& r) const;
        bool operator==(const segment_type& r) const;
        bool operator!=(const segment_type& r) const;
    };

    using segment_store_type = std::deque<segment_type>;
    using value_pos_type = typename segment_store_type::size_type;
    using value_chain_type = std::vector<value_pos_type>;

    using node_chain_type = std::vector<st::detail::node_base*>;
    using value_to_nodes_type = std::map<value_pos_type, node_chain_type>;

    struct nonleaf_value_type
    {
        std::unique_ptr<value_chain_type> data_chain;

        nonleaf_value_type()
        {}
        nonleaf_value_type(const nonleaf_value_type& r)
        {
            if (r.data_chain)
                data_chain = std::make_unique<value_chain_type>(*r.data_chain);
        }

        bool operator==(const nonleaf_value_type& r) const
        {
            return data_chain == r.data_chain;
        }

        ~nonleaf_value_type()
        {}
    };

    struct leaf_value_type
    {
        std::unique_ptr<value_chain_type> data_chain;

        leaf_value_type()
        {}
        leaf_value_type(const leaf_value_type& r)
        {
            if (r.data_chain)
                data_chain = std::make_unique<value_chain_type>(*r.data_chain);
        }

        bool operator==(const leaf_value_type& r) const
        {
            return data_chain == r.data_chain;
        }

        ~leaf_value_type()
        {}
    };

public:
    using node = st::detail::node<key_type, leaf_value_type>;
    using node_ptr = typename node::node_ptr;
    using nonleaf_node = typename st::detail::nonleaf_node<key_type, nonleaf_value_type>;

private:
    class search_result_inserter;

    /**
     * This base class takes care of collecting data chain pointers during
     * tree descend for search.
     */
    class search_results_base
    {
        friend class search_result_inserter;

    public:
        typedef std::vector<value_chain_type*> res_chains_type;
        typedef std::shared_ptr<res_chains_type> res_chains_ptr;

    protected:
        search_results_base(const segment_store_type& segment_store) : m_segment_store(&segment_store)
        {}

        search_results_base(const search_results_base& r)
            : m_segment_store(r.m_segment_store), mp_res_chains(r.mp_res_chains)
        {}

        bool empty() const;

        size_type size() const;

        void push_back_chain(value_chain_type* chain);

        const segment_store_type* get_segment_store() const
        {
            return m_segment_store;
        }

        const res_chains_ptr& get_res_chains() const
        {
            return mp_res_chains;
        }

    private:
        const segment_store_type* m_segment_store = nullptr;
        res_chains_ptr mp_res_chains;
    };

    class const_iterator_base
    {
        friend class segment_tree;

    protected:
        typedef typename search_results_base::res_chains_type res_chains_type;
        typedef typename search_results_base::res_chains_ptr res_chains_ptr;

        const_iterator_base(const segment_store_type* segment_store, const res_chains_ptr& p)
            : m_segment_store(segment_store), mp_res_chains(p), m_end_pos(true)
        {}

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = const segment_tree::segment_type;
        using pointer = value_type*;
        using reference = value_type&;
        using difference_type = std::ptrdiff_t;

        const_iterator_base() = default;
        const_iterator_base(const const_iterator_base& r) = default;
        const_iterator_base& operator=(const const_iterator_base& r) = default;

        value_type* operator++();
        value_type* operator--();

        bool operator==(const const_iterator_base& r) const;
        bool operator!=(const const_iterator_base& r) const;

        value_type& operator*()
        {
            return cur_value();
        }

        const value_type* operator->()
        {
            return &cur_value();
        }

    protected:
        void move_to_front();
        void move_to_end();

    private:
        value_type& cur_value()
        {
            auto pos = *m_cur_pos_in_chain;
            return (*m_segment_store)[pos];
        }

        size_type cur_pos() const
        {
            return *m_cur_pos_in_chain;
        }

        const segment_store_type* m_segment_store = nullptr;
        res_chains_ptr mp_res_chains;
        typename res_chains_type::const_iterator m_cur_chain;
        typename value_chain_type::const_iterator m_cur_pos_in_chain;
        bool m_end_pos = true;
    };

    class search_result_inserter
    {
    public:
        search_result_inserter(search_results_base& results) : m_results(results)
        {}
        void operator()(value_chain_type* node_data)
        {
            if (!node_data)
                return;

            m_results.push_back_chain(node_data);
        }

    private:
        search_results_base& m_results;
    };

public:
    class search_results : public search_results_base
    {
        friend class segment_tree;

        typedef typename search_results_base::res_chains_type res_chains_type;
        typedef typename search_results_base::res_chains_ptr res_chains_ptr;

        search_results(const segment_store_type& segment_tree) : search_results_base(segment_tree)
        {}

    public:
        class const_iterator : public const_iterator_base
        {
            friend class segment_tree<KeyT, ValueT>::search_results;

        private:
            const_iterator(const segment_store_type* segment_store, const res_chains_ptr& p)
                : const_iterator_base(segment_store, p)
            {}

        public:
            const_iterator() : const_iterator_base()
            {}

            const_iterator(const const_iterator& r) : const_iterator_base(r)
            {}

            const_iterator& operator=(const const_iterator& r)
            {
                const_iterator_base::operator=(r);
                return *this;
            }
        };

        /**
         * Check if this results pool is empty or not.
         *
         * @return true if this results pool is empty, otherwise false.
         */
        bool empty() const
        {
            return search_results_base::empty();
        }

        /**
         * Count the number of results in this results pool.
         *
         * @return number of results in this results pool.
         */
        size_type size() const
        {
            return search_results_base::size();
        }

        typename search_results::const_iterator begin() const
        {
            typename search_results::const_iterator it(
                search_results_base::get_segment_store(), search_results_base::get_res_chains());
            it.move_to_front();
            return it;
        }

        typename search_results::const_iterator end() const
        {
            typename search_results::const_iterator it(
                search_results_base::get_segment_store(), search_results_base::get_res_chains());
            it.move_to_end();
            return it;
        }
    };

    segment_tree();
    segment_tree(const segment_tree& r);
    segment_tree(segment_tree&& r) noexcept = default;
    ~segment_tree();

    segment_tree& operator=(const segment_tree& r);
    segment_tree& operator=(segment_tree&& r);

    /**
     * Check equality with another instance.
     *
     * @note Equality between two segment_tree instances is evaluated
     * by comparing the logically stored segments only; the tree parts of the
     * structures are not compared.
     */
    bool operator==(const segment_tree& r) const;

    bool operator!=(const segment_tree& r) const;

    /**
     * Check whether or not the internal tree is in a valid state.  The tree
     * must be valid in order to perform searches.
     *
     * @return true if the tree is valid, false otherwise.
     */
    bool valid_tree() const noexcept
    {
        return m_valid_tree;
    }

    /**
     * Build or re-build a tree based on the current set of segments.
     *
     * @note Building a tree with no logical stored segments will not result in
     *       a valid tree.
     */
    void build_tree();

    /**
     * Insert a new segment.  Duplicate segments are allowed.
     *
     * @param start_key start key of a segment.  The value is inclusive.
     * @param end_key end key of a segment.  The value is non-inclusive.
     *                It must be greater than the start key.
     * @param value value to associate with this segment.
     */
    void insert(key_type start_key, key_type end_key, value_type value);

    /**
     * Search the tree to find all the segments that contain a specified point.
     *
     * @param point A point to search the tree with.
     *
     * @return Results object containing the segments that contain the specified
     *         point.
     *
     * @note You need to have a valid tree prior to calling this method.  Call
     *       build_tree() to build one.  To check if you have a valid tree, call
     *       valid_tree() and see if it returns true.
     *
     * @note A point value can be any value allowed by the key_type.
     *       If the point is outside of the range of the tree, empty results
     *       will be returned.
     */
    search_results search(const key_type& point) const;

    /**
     * Remove a segment referenced by an iterator.  Calling this method will
     * <em>not</em> invalidate the tree; however, you might want to re-build the
     * tree to compact its size if you have removed a large number of segments.
     *
     * @param pos Position that references the segment to be removed.
     */
    void erase(const typename search_results::const_iterator& pos);

    /**
     * Remove all segments that satisfy a predicate.
     *
     * The predicate must take three parameters that are a start position, end
     * position and a value of a segment in this order.  The first two
     * parameters are of the same type as the key_type while the last parameter
     * is of the same type as the value_type.
     *
     * @code{.cpp}
     * // Given int as key_type and std::string as value_type, this predicate removes
     * // all the segments that 1) contain 5 and 2) store a value of "A".
     * auto pred = [](int start, int end, const std::string& value) {
     *     return start <= 5 && 5 < end && value == "A";
     * };
     * @endcode
     *
     * @param pred Predicate that tests each segment.  A segment that evaluates
     *             true by the predicate gets removed.
     *
     * @return Number of erased elements.
     */
    template<typename Pred>
    size_type erase_if(Pred pred);

    /**
     * Exchange the content of the tree with that of another.
     *
     * @param r Another tree instance to swap the contents with.
     */
    void swap(segment_tree& r) noexcept;

    /**
     * Remove all segments data.
     */
    void clear();

    /**
     * Return the number of segments currently stored in this container.
     */
    size_type size() const;

    /**
     * Return whether or not the container stores any segments or none at all.
     */
    bool empty() const;

    /**
     * Return the number of leaf nodes.
     *
     * @return number of leaf nodes.
     */
    size_type leaf_size() const;

    /**
     * Create a string representation of the internal state of a tree.
     *
     * @return String representation of the internal state of a tree.
     */
    std::string to_string() const;

    /**
     * Create a sorted sequence of unique boundary keys.  A boundary key is a
     * key that is either the start or the end key of a stored segment.
     *
     * @return A sorted sequence of unique boundary keys.
     */
    std::vector<key_type> boundary_keys() const;

    struct integrity_check_properties
    {
        struct leaf_node
        {
            key_type key = {};
            std::vector<value_type> value_chain;
        };

        std::vector<leaf_node> leaf_nodes;
    };

    void check_integrity(const integrity_check_properties& props) const;

private:
    static void create_leaf_node_instances(std::vector<key_type> keys, node_ptr& left, node_ptr& right);

    /**
     * Descend the tree from the root node, and mark appropriate nodes, both
     * leaf and non-leaf, based on segment's end points.  When marking nodes,
     * record their positions as a list of node pointers.
     */
    void descend_tree_and_mark(
        st::detail::node_base* pnode, value_pos_type value, const key_type& start_key, const key_type& end_key,
        node_chain_type& nodelist);

    void build_leaf_nodes();

    /**
     * Go through the list of nodes, and remove the specified data pointer
     * value from the nodes.
     */
    void remove_data_from_nodes(node_chain_type& nodelist, value_pos_type value);
    void remove_data_from_chain(value_chain_type& chain, value_pos_type value);
    void remove_value_pos(size_type pos);

    void clear_all_nodes();

private:
    struct tree_dumper_traits
    {
        using leaf_type = node;
        using nonleaf_type = nonleaf_node;
        using tree_type = segment_tree;

        struct to_string
        {
            const tree_type& tree;

            to_string(const tree_type& _tree);
            std::string operator()(const leaf_type& leaf) const;
            std::string operator()(const nonleaf_type& nonleaf) const;
        };
    };

    std::vector<nonleaf_node> m_nonleaf_node_pool;

    /**
     * Storage for inserted segments.  Note that real values are only stored
     * here. The rest of the structure only stores indices into this storage to
     * find the values when needed.
     */
    segment_store_type m_segment_store;

    /**
     * For each segment index, it keeps track of all nodes, leaf or non-leaf,
     * that are marked with that index.  This is used for removing segments by
     * value.
     */
    value_to_nodes_type m_tagged_nodes_map;

    nonleaf_node* m_root_node;
    node_ptr m_left_leaf;
    node_ptr m_right_leaf;
    bool m_valid_tree;
};

} // namespace mdds

#include "segment_tree_def.inl"
