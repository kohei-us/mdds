/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2018 Kohei Yoshida
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

#ifndef INCLUDED_MDDS_RTREE_HPP
#define INCLUDED_MDDS_RTREE_HPP

#include <deque>
#include <vector>
#include <cstdlib>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <functional>

namespace mdds {

namespace detail { namespace rtree {

struct default_rtree_trait
{
    /**
     * Number of dimensions in bounding rectangles.
     */
    constexpr static size_t dimensions = 2;

    /**
     * Minimum number of child nodes that must be present in each directory
     * node.  Exception is the root node, which is allowed to have less than
     * the minimum number of nodes, but only when it's a leaf directory node.
     */
    constexpr static size_t min_node_size = 40;

    /**
     * Maximum number of child nodes that each directory node is allowed to
     * have.  There are no exceptions to this rule.
     */
    constexpr static size_t max_node_size = 100;

    /**
     * Maximum depth a tree is allowed to have.
     */
    constexpr static size_t max_tree_depth = 100;

    /**
     * A flag to determine whether or not to perform forced reinsertion when a
     * directory node overflows, before attempting to split the node.
     */
    constexpr static bool enable_forced_reinsertion = true;

    /**
     * Number of nodes to get re-inserted during forced reinsertion.  This
     * should be roughly 30% of max_node_size + 1, and should not be greater
     * than max_node_size - min_node_size + 1.
     */
    constexpr static size_t reinsertion_size = 30;
};

struct integrity_check_properties
{
    /**
     * When true, the integrity check will throw an exception on the first
     * validation failure.  When false, it will run through the entire tree
     * and report all encountered validation failures then throw an exception
     * if there is at least one failure.
     */
    bool throw_on_first_error = true;

    /**
     * When true, a node containing children less than the minimum node size
     * will be treated as an error.
     */
    bool error_on_min_node_size = true;
};

enum class node_type { unspecified, directory_leaf, directory_nonleaf, value };

enum class export_tree_type
{
    /**
     * Textural representation of a tree structure.  Indent levels represent
     * depths, and each line represents a single node.
     */
    formatted_node_properties,

    /**
     * The extents of all directory and value nodes are exported as Wavefront
     * .obj format.  Only 2 dimensional trees are supported for now.
     *
     * For a 2-dimensional tree, each depth is represented by a 2D plane
     * filled with rectangles representing the extents of either value or
     * directory nodes at that depth level.  The depth planes are then stacked
     * vertically.
     */
    extent_as_obj,

    /**
     * The extents of all directory and value nodes are exported as a scalable
     * vector graphics (SVG) format.  Only 2 dimensional trees are supported.
     */
    extent_as_svg
};

enum class search_type
{
    /**
     * Pick up all objects whose extents overlap with the specified search
     * extent.
     */
    overlap,

    /**
     * Pick up all objects whose extents exactly match the specified search
     * extent.
     */
    match,
};

template<typename _NodePtrT>
struct ptr_to_string;

}}

template<typename _Key, typename _Value, typename _Trait = detail::rtree::default_rtree_trait>
class rtree
{
    using trait_type = _Trait;

    constexpr static size_t max_dist_size = trait_type::max_node_size - trait_type::min_node_size * 2 + 2;

public:
    using key_type = _Key;
    using value_type = _Value;

    struct point_type
    {
        key_type d[trait_type::dimensions];

        point_type();
        point_type(std::initializer_list<key_type> vs);

        std::string to_string() const;

        bool operator== (const point_type& other) const;
        bool operator!= (const point_type& other) const;
    };

    struct extent_type
    {
        point_type start;
        point_type end;

        extent_type();
        extent_type(const point_type& _start, const point_type& _end);

        std::string to_string() const;

        bool is_point() const;

        bool operator== (const extent_type& other) const;
        bool operator!= (const extent_type& other) const;

        /**
         * Determine whether or not the specified point lies within this
         * extent.
         *
         * @param pt point to query with.
         *
         * @return true if the point lies within this extent, or false
         *         otherwise.
         */
        bool contains(const point_type& pt) const;

        /**
         * Determine whether or not the specified extent lies <i>entirely</i>
         * within this extent.
         *
         * @param bb extent to query with.
         *
         * @return true if the specified extent lies entirely within this
         *         extent, or otherwise false.
         */
        bool contains(const extent_type& bb) const;

        /**
         * Determine whether or not the specified extent overlaps with this
         * extent either partially or fully.
         *
         * @param bb extent to query with.
         *
         * @return true if the specified extent overlaps with this extent, or
         *         otherwise false.
         */
        bool intersects(const extent_type& bb) const;

        /**
         * Determine whether or not another bounding box is within this
         * bounding box and shares a part of its boundaries.
         */
        bool contains_at_boundary(const extent_type& other) const;
    };

    using node_type = detail::rtree::node_type;
    using export_tree_type = detail::rtree::export_tree_type;
    using search_type = detail::rtree::search_type;
    using integrity_check_properties = detail::rtree::integrity_check_properties;

    struct node_properties
    {
        node_type type;
        extent_type extent;
    };

private:

    struct node;
    struct directory_node;

    /**
     * This class is intentionally only movable and non-copyable, to prevent
     * accidental copying of its object. To "copy" this class, you must use
     * its clone() method explicitly.
     */
    struct node_store
    {
        node_type type;
        extent_type extent;
        node_store* parent;
        node* node_ptr;
        size_t count;

        bool valid_pointer;

        node_store(const node_store&) = delete;
        node_store& operator= (const node_store&) = delete;

        node_store();
        node_store(node_store&& r);
        node_store(node_type _type, const extent_type& _extent, node* _node_ptr);
        ~node_store();

        node_store clone() const;

        static node_store create_leaf_directory_node();
        static node_store create_nonleaf_directory_node();
        static node_store create_value_node(const extent_type& extent, value_type&& v);
        static node_store create_value_node(const extent_type& extent, const value_type& v);

        node_store& operator= (node_store&& other);

        /**
         * Re-calculate the extent based on its current children.
         *
         * @return true if the extent has changed, false otherwise.
         */
        bool pack();

        /**
         * Re-calculate the extent of the parent nodes recursively all the way
         * up to the root node.
         */
        void pack_upward();

        bool is_directory() const;
        bool is_root() const;
        bool exceeds_capacity() const;

        void swap(node_store& other);

        /**
         * Have all its child nodes update their parent pointers if the memory
         * location of this node has been invalidated.  Run the tree
         * recursively until no more invalid pointers have been found.
         */
        void reset_parent_pointers_of_children();

        /**
         * Update its parent pointer and all its child nodes' parent pointers
         * if the memory location of this node has been invalidated. Run the
         * tree recursively until no more invalid pointers have been found.
         */
        void reset_parent_pointers();

        directory_node* get_directory_node();
        const directory_node* get_directory_node() const;

        bool erase_child(const node_store* p);
    };

    using dir_store_type = std::deque<node_store>;

    struct dir_store_segment
    {
        typename dir_store_type::iterator begin;
        typename dir_store_type::iterator end;
        size_t size;

        dir_store_segment() : size(0) {}

        dir_store_segment(
            typename dir_store_type::iterator _begin,
            typename dir_store_type::iterator _end,
            size_t _size) :
            begin(std::move(_begin)),
            end(std::move(_end)),
            size(_size) {}
    };

    struct distribution
    {
        dir_store_segment g1;
        dir_store_segment g2;

        distribution(size_t dist, dir_store_type& nodes)
        {
            g1.begin = nodes.begin();
            g1.end = g1.begin;
            g1.size = trait_type::min_node_size - 1 + dist;
            std::advance(g1.end, g1.size);

            g2.begin = g1.end;
            g2.end = nodes.end();
            g2.size = nodes.size() - g1.size;
        }
    };

    struct node
    {
        node();
        node(const node&) = delete;
        ~node();
    };

    struct value_node : public node
    {
        value_type value;

        value_node() = delete;
        value_node(value_type&& _value);
        value_node(const value_type& _value);
        ~value_node();
    };

    /**
     * Directory node can either contain all value nodes, or all directory
     * nodes as its child nodes.
     */
    struct directory_node : public node
    {
        dir_store_type children;

        directory_node(const directory_node&) = delete;
        directory_node& operator=(const directory_node&) = delete;

        directory_node();
        ~directory_node();

        bool erase(const node_store* p);

        node_store* get_child_with_minimal_overlap(const extent_type& bb);
        node_store* get_child_with_minimal_area_enlargement(const extent_type& bb);

        extent_type calc_extent() const;
        key_type calc_overlap_cost(const extent_type& bb) const;
        bool has_leaf_directory() const;
    };

    struct orphan_node_entry
    {
        node_store ns;
        size_t depth;
    };

    using orphan_node_entries_type = std::deque<orphan_node_entry>;

    struct insertion_point
    {
        node_store* ns;
        size_t depth;
    };

public:

    template<typename _NS>
    class search_results_base
    {
        friend class rtree;
    protected:
        using node_store_type = _NS;

        struct entry
        {
            node_store_type* ns;
            size_t depth;

            entry(node_store_type* _ns, size_t _depth);
        };

        using store_type = std::vector<entry>;
        store_type m_store;

        void add_node_store(node_store_type* ns, size_t depth);
    };

    class const_iterator;
    class iterator;
    class search_results;

    class const_search_results : public search_results_base<const node_store>
    {
        using base_type = search_results_base<const node_store>;
        using base_type::m_store;
    public:
        const_iterator cbegin() const;
        const_iterator cend() const;
        const_iterator begin() const;
        const_iterator end() const;
    };

    class search_results : public search_results_base<node_store>
    {
        using base_type = search_results_base<node_store>;
        using base_type::m_store;
    public:
        iterator begin();
        iterator end();
    };

    template<typename _SelfIter, typename _StoreIter, typename _ValueT>
    class iterator_base
    {
    public:
        using store_iterator_type = _StoreIter;
        using self_iterator_type = _SelfIter;

    protected:
        store_iterator_type m_pos;

        iterator_base(store_iterator_type pos);

    public:
        // iterator traits
        using value_type = _ValueT;
        using pointer = value_type*;
        using reference = value_type&;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;

        bool operator== (const self_iterator_type& other) const;
        bool operator!= (const self_iterator_type& other) const;

        self_iterator_type& operator++();
        self_iterator_type operator++(int);

        self_iterator_type& operator--();
        self_iterator_type operator--(int);

        const extent_type& extent() const;
        size_t depth() const;
    };

    class const_iterator : public iterator_base<
        const_iterator,
        typename const_search_results::store_type::const_iterator,
        const rtree::value_type>
    {
        using base_type = iterator_base<
            const_iterator,
            typename const_search_results::store_type::const_iterator,
            const rtree::value_type>;

        using store_type = typename const_search_results::store_type;
        using typename base_type::store_iterator_type;
        using base_type::m_pos;

        friend class rtree;

        const_iterator(store_iterator_type pos);
    public:
        using typename base_type::value_type;

        value_type& operator*() const;
        value_type* operator->() const;
    };

    class iterator : public iterator_base<
        iterator,
        typename search_results::store_type::iterator,
        rtree::value_type>
    {
        using base_type = iterator_base<
            iterator,
            typename search_results::store_type::iterator,
            rtree::value_type>;

        using store_type = typename const_search_results::store_type;
        using typename base_type::store_iterator_type;
        using base_type::m_pos;

        friend class rtree;

        iterator(store_iterator_type pos);
    public:
        using typename base_type::value_type;

        value_type& operator*();
        value_type* operator->();
    };

    /**
     * Loader optimized for loading a large number of value objects.  A
     * resultant tree will have a higher chance of being well balanced than if
     * the value objects were inserted individually into the tree.
     */
    class bulk_loader
    {
        dir_store_type m_store;

    public:
        bulk_loader();

        void insert(const extent_type& extent, value_type&& value);
        void insert(const extent_type& extent, const value_type& value);

        void insert(const point_type& position, value_type&& value);
        void insert(const point_type& position, const value_type& value);

        rtree pack();

    private:
        void pack_level(dir_store_type& store, size_t depth);

        void insert_impl(const extent_type& extent, value_type&& value);
        void insert_impl(const extent_type& extent, const value_type& value);
    };

    rtree();
    rtree(rtree&& other);
    rtree(const rtree& other);

private:
    rtree(node_store&& root); // only used internally by bulk_loader.

public:
    ~rtree();

    rtree& operator= (const rtree& other);

    rtree& operator= (rtree&& other);

    /**
     * Insert a new value associated with a bounding box.  The new value
     * object will be moved into the container.
     *
     * @param extent bounding box associated with the value.
     * @param value value being inserted.
     */
    void insert(const extent_type& extent, value_type&& value);

    /**
     * Insert a new value associated with a bounding box.  A copy of the new
     * value object will be placed into the container.
     *
     * @param extent bounding box associated with the value.
     * @param value value being inserted.
     */
    void insert(const extent_type& extent, const value_type& value);

    /**
     * Insert a new value associated with a point.  The new value object will
     * be moved into the container.
     *
     * @param position point associated with the value.
     * @param value value being inserted.
     */
    void insert(const point_type& position, value_type&& value);

    /**
     * Insert a new value associated with a point.  A copy of the new value
     * object will be placed into the container.
     *
     * @param position point associated with the value.
     * @param value value being inserted.
     */
    void insert(const point_type& position, const value_type& value);

    /**
     * Search the tree and collect all value objects whose extents either
     * contain the specified point, or exactly match the specified point.
     *
     * @param pt reference point to use for the search.
     * @param st search type that determines the satisfying condition of the
     *           search with respect to the reference point.
     *
     * @return collection of all value objects that satisfy the specified
     *         search condition.  This collection is immutable.
     */
    const_search_results search(const point_type& pt, search_type st) const;

    /**
     * Search the tree and collect all value objects whose extents either
     * contain the specified point, or exactly match the specified point.
     *
     * @param pt reference point to use for the search.
     * @param st search type that determines the satisfying condition of the
     *           search with respect to the reference point.
     *
     * @return collection of all value objects that satisfy the specified
     *         search condition.  This collection is mutable.
     */
    search_results search(const point_type& pt, search_type st);

    /**
     * Search the tree and collect all value objects whose extents either
     * overlaps with the specified extent, or exactly match the specified
     * extent.
     *
     * @param extent reference extent to use for the search.
     * @param st search type that determines the satisfying condition of the
     *           search with respect to the reference extent.
     *
     * @return collection of all value objects that satisfy the specified
     *         search condition.  This collection is immutable.
     */
    const_search_results search(const extent_type& extent, search_type st) const;

    /**
     * Search the tree and collect all value objects whose extents either
     * overlaps with the specified extent, or exactly match the specified
     * extent.
     *
     * @param extent reference extent to use for the search.
     * @param st search type that determines the satisfying condition of the
     *           search with respect to the reference extent.
     *
     * @return collection of all value objects that satisfy the specified
     *         search condition.  This collection is mutable.
     */
    search_results search(const extent_type& extent, search_type st);

    /**
     * Erase the value object referenced by the iterator passed to this
     * method.
     *
     * <p>The iterator object will become invalid if the call results in an
     * erasure of a value.</p>
     *
     * @param pos iterator that refernces the value object to erase.
     */
    void erase(const const_iterator& pos);

    /**
     * Erase the value object referenced by the iterator passed to this
     * method.
     *
     * <p>The iterator object will become invalid if the call results in an
     * erasure of a value.</p>
     *
     * @param pos iterator that refernces the value object to erase.
     */
    void erase(const iterator& pos);

    /**
     * Get the minimum bounding extent of the root node of the tree. The
     * extent returned from this method is the minimum extent that contains
     * the extents of all objects stored in the tree.
     *
     * @return immutable reference to the extent of the root node of the tree.
     */
    const extent_type& extent() const;

    /**
     * Check whether or not the tree stores any objects.
     *
     * @return true if the tree is empty, otherwise false.
     */
    bool empty() const;

    /**
     * Return the number of value nodes currently stored in the tree.
     *
     * @return number of value nodes currently in the tree.
     */
    size_t size() const;

    /**
     * Swap the content of the tree with another instance.
     *
     * @param other another instance to swap the content with.
     */
    void swap(rtree& other);

    /**
     * Empty the entire container.
     */
    void clear();

    /**
     * Walk down the entire tree depth first.
     *
     * @func function or function object that gets called at each node in the
     *       tree.
     */
    template<typename _Func>
    void walk(_Func func) const;

    /**
     * Check the integrity of the entire tree structure.
     *
     * @param props specify how the check is to be performed.
     *
     * @exception integrity_error if the integrity check fails.
     */
    void check_integrity(const integrity_check_properties& props) const;

    /**
     * Export the structure of a tree in textural format.
     *
     * @param mode specify the format in which to represent the structure of a
     *             tree.
     *
     * @return string representation of the tree structure.
     */
    std::string export_tree(export_tree_type mode) const;

private:

    void insert_impl(const point_type& start, const point_type& end, value_type&& value);
    void insert_impl(const point_type& start, const point_type& end, const value_type& value);

    void erase_impl(const node_store* ns, size_t depth);

    /**
     * Build and return a callable function object that you can call in order
     * to convert node's memory location to a normalized unique string still
     * representative of that node.
     */
    detail::rtree::ptr_to_string<const node_store*> build_ptr_to_string_map() const;

    std::string export_tree_formatted() const;
    std::string export_tree_extent_as_obj() const;
    std::string export_tree_extent_as_svg() const;

    void insert(node_store&& ns, std::unordered_set<size_t>* reinserted_depths);
    void insert_dir(node_store&& ns, size_t max_depth);

    /**
     * Split an overfilled node.  The node to split is expected to have
     * exactly M+1 child nodes where M is the maximum number of child nodes a
     * single node is allowed to have.
     *
     * @param ns node to split.
     */
    void split_node(node_store* ns);

    void perform_forced_reinsertion(node_store* ns, std::unordered_set<size_t>& reinserted_depth);

    /**
     * Determine the best dimension to split by, and sort the child nodes by
     * that dimension.
     *
     * @param children child nodes to sort.
     */
    void sort_dir_store_by_split_dimension(dir_store_type& children);

    void sort_dir_store_by_dimension(size_t dim, dir_store_type& children);

    size_t pick_optimal_distribution(dir_store_type& children) const;

    insertion_point find_leaf_directory_node_for_insertion(const extent_type& bb);
    node_store* find_nonleaf_directory_node_for_insertion(const extent_type& bb, size_t max_depth);

    template<typename _Func>
    void descend_with_func(_Func func) const;

    using search_condition_type = std::function<bool(const node_store&)>;

    template<typename _ResT>
    void search_descend(
        size_t depth, const search_condition_type& dir_cond, const search_condition_type& value_cond,
        typename _ResT::node_store_type& ns, _ResT& results) const;

    void shrink_tree_upward(node_store* ns, const extent_type& bb_affected);

private:
    node_store m_root;
};

} // namespace mdds

#include "rtree_def.inl"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
