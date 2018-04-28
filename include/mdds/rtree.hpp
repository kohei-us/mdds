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

template<typename _Key, typename _Value, size_t _Dim = 2>
class rtree
{
    static const size_t min_node_size = 40;
    static const size_t max_node_size = 100;

public:
    static const size_t dimensions = _Dim;
    using key_type = _Key;
    using value_type = _Value;

    struct point
    {
        key_type d[dimensions];

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

        node_store();
        node_store(node_store&& r);
        node_store(node_type type, const bounding_box& box, node* node_ptr);
        ~node_store();

        static node_store create_directory_node();
        static node_store create_value_node(const bounding_box& box, value_type v);

        node_store(const node_store&) = delete;

        bool has_capacity() const
        {
            if (type != node_type::directory_leaf)
                return false;

            return count < max_node_size;
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

    rtree();
    ~rtree();

    rtree(const rtree&) = delete;
    rtree& operator= (const rtree&) = delete;

    void insert(const point& start, const point& end, value_type value);

    const bounding_box& get_total_extent() const;

private:
    node_store* find_node_for_insertion(const bounding_box& bb);

    key_type calc_overlap_cost(directory_node& dir) const;

private:
    node_store m_root;
};

}

#include "rtree_def.inl"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
