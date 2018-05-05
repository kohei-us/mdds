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

#include <vector>
#include <cstdlib>
#include <string>

namespace mdds {

namespace detail { namespace rtree {

struct default_rtree_trait
{
    constexpr static const size_t dimensions = 2;
    constexpr static const size_t min_node_size = 40;
    constexpr static const size_t max_node_size = 100;
    constexpr static const size_t max_tree_depth = 100;
};

}}

template<typename _Key, typename _Value, typename _Trait = detail::rtree::default_rtree_trait>
class rtree
{
    using trait_type = _Trait;

public:
    using key_type = _Key;
    using value_type = _Value;

    struct point
    {
        key_type d[trait_type::dimensions];

        point();
        point(std::initializer_list<key_type> vs);

        std::string to_string() const;

        bool operator== (const point& other) const;
        bool operator!= (const point& other) const;
    };

    struct bounding_box
    {
        point start;
        point end;

        bounding_box();
        bounding_box(const point& start, const point& end);

        std::string to_string() const;

        bool operator== (const bounding_box& other) const;
        bool operator!= (const bounding_box& other) const;

        bool contains(const point& pt) const;
    };

private:
    enum class node_type { unspecified, directory_leaf, directory_nonleaf, value };

    struct node;

    struct node_store
    {
        node_type type;
        bounding_box box;
        node_store* parent;
        node* node_ptr;
        size_t count;

        node_store(const node_store&) = delete;

        node_store();
        node_store(node_store&& r);
        node_store(node_type type, const bounding_box& box, node* node_ptr);
        ~node_store();

        static node_store create_directory_node();
        static node_store create_value_node(const bounding_box& box, value_type v);

        node_store& operator= (node_store&& other);

        bool has_capacity() const;
        void swap(node_store& other);
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
        value_node(value_type value);
        ~value_node();
    };

    /**
     * Directory node can either contain all value nodes, or all directory
     * nodes as its child nodes.
     */
    struct directory_node : public node
    {
        std::vector<node_store> children;

        directory_node();
        ~directory_node();

        void insert(node_store&& ns);
    };

public:

    class const_iterator;

    class const_search_results
    {
        friend class const_iterator;
        friend class rtree;

        using store_type = std::vector<const node_store*>;
        store_type m_store;

        void add_node_store(const node_store* ns);
    public:
        const_iterator cbegin() const;
        const_iterator cend() const;
        const_iterator begin() const;
        const_iterator end() const;
    };

    class const_iterator
    {
        friend class rtree;

        struct node
        {
            bounding_box box;
            value_type value;
        };

        using store_type = typename const_search_results::store_type;
        typename store_type::const_iterator m_pos;
        node m_cur_node;

        void update_current_node();

    public:
        const_iterator(typename store_type::const_iterator pos);

        // iterator traits
        typedef node value_type;
        typedef value_type* pointer;
        typedef value_type& reference;
        typedef std::ptrdiff_t difference_type;
        typedef std::bidirectional_iterator_tag iterator_category;

        bool operator== (const const_iterator& other) const;
        bool operator!= (const const_iterator& other) const;

        const_iterator& operator++();
        const_iterator operator++(int);
        const_iterator& operator--();
        const_iterator operator--(int);

        const value_type& operator*();
        const value_type* operator->();
    };

    rtree();
    ~rtree();

    rtree(const rtree&) = delete;
    rtree& operator= (const rtree&) = delete;

    void insert(const point& start, const point& end, value_type value);

    const_search_results search(const point& pt) const;

    void erase(const_iterator pos);

    const bounding_box& get_total_extent() const;

private:
    node_store* find_node_for_insertion(const bounding_box& bb);

    key_type calc_overlap_cost(const bounding_box& bb, const directory_node& dir) const;

    void search_descend(const point& pt, const node_store& ns, const_search_results& results) const;

    void shrink_tree_upward(node_store* ns, const bounding_box& bb_affected);

private:
    node_store m_root;
};

}

#include "rtree_def.inl"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
