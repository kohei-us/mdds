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

#include "mdds/global.hpp"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <memory>
#include <cassert>
#include <algorithm>

namespace mdds {

namespace detail { namespace rtree {

template<typename _Key, typename _BBox>
_Key calc_linear_intersection(size_t dim, const _BBox& bb1, const _BBox& bb2)
{
    using key_type = _Key;

    key_type start1 = bb1.start.d[dim], end1 = bb1.end.d[dim];
    key_type start2 = bb2.start.d[dim], end2 = bb2.end.d[dim];

    // Ensure that start1 < start2.

    if (start1 > start2)
    {
        std::swap(start1, start2);
        std::swap(end1, end2);
    }

    assert(start1 < start2);

    if (end1 < start2)
    {
        // 1 : |------|
        // 2 :           |-------|

        // These two are not intersected at all. Bail out.
        return key_type();
    }

    if (end1 < end2)
    {
        // 1 : |---------|
        // 2 :      |----------|

        return end1 - start2;
    }

    // 1 : |--------------|
    // 2 :      |-----|

    return end2 - start2;
}

template<typename _Key, typename _BBox, size_t _Dim>
_Key calc_intersection(const _BBox& bb1, const _BBox& bb2)
{
    static_assert(_Dim > 0, "Dimension cannot be zero.");

    using key_type = _Key;

    key_type total_volume = calc_linear_intersection<_Key,_BBox>(0, bb1, bb2);
    if (!total_volume)
        return key_type();

    for (size_t dim = 1; dim < _Dim; ++dim)
    {
        key_type segment_len = calc_linear_intersection<_Key,_BBox>(dim, bb1, bb2);
        if (!segment_len)
            return key_type();

        total_volume *= segment_len;
    }

    return total_volume;
}

template<typename _Key, typename _BBox, size_t _Dim>
bool enlarge_box_to_fit(_BBox& parent, const _BBox& child)
{
    bool enlarged = false;

    for (size_t dim = 0; dim < _Dim; ++dim)
    {
        if (child.start.d[dim] < parent.start.d[dim])
        {
            parent.start.d[dim] = child.start.d[dim];
            enlarged = true;
        }

        if (parent.end.d[dim] < child.end.d[dim])
        {
            parent.end.d[dim] = child.end.d[dim];
            enlarged = true;
        }
    }

    return enlarged;
}

template<typename _Key, typename _BBox, size_t _Dim>
_Key calc_area(const _BBox& bb)
{
    static_assert(_Dim > 0, "Dimension cannot be zero.");
    using key_type = _Key;

    key_type area = bb.end.d[0] - bb.start.d[0];
    for (size_t dim = 1; dim < _Dim; ++dim)
        area *= bb.end.d[dim] - bb.start.d[dim];

    return area;
}

/**
 * Area enlargement is calculated by calculating the area of the enlarged
 * box subtracted by the area of the original box prior to the enlargement.
 *
 * @param bb_host bounding box of the area receiving the new object.
 * @param bb_guest bounding of the new object being inserted.
 *
 * @return quantity of the area enlargement.
 */
template<typename _Key, typename _BBox, size_t _Dim>
_Key calc_area_enlargement(const _BBox& bb_host, const _BBox& bb_guest)
{
    static_assert(_Dim > 0, "Dimension cannot be zero.");
    using key_type = _Key;
    using bounding_box = _BBox;

    // Calculate the original area.
    key_type original_area = calc_area<_Key,_BBox,_Dim>(bb_host);

    // Enlarge the box for the new object if needed.
    bounding_box bb_host_enlarged = bb_host; // make a copy.
    bool enlarged = enlarge_box_to_fit<_Key,_BBox,_Dim>(bb_host_enlarged, bb_guest);
    if (!enlarged)
        // Area enlargement did not take place.
        return key_type();

    key_type enlarged_area = calc_area<_Key,_BBox,_Dim>(bb_host_enlarged);

    return enlarged_area - original_area;
}

}}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::point::point()
{
    // Initialize the point values with the key type's default value.
    key_type* p = d;
    key_type* p_end = p + dimensions;

    for (; p != p_end; ++p)
        *p = key_type{};
}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::point::point(std::initializer_list<key_type> vs)
{
    // Initialize the point values with the key type's default value.
    key_type* dst = d;
    key_type* dst_end = dst + dimensions;

    for (const key_type& v : vs)
    {
        if (dst == dst_end)
            throw std::range_error("number of elements exceeds the dimension size.");

        *dst = v;
        ++dst;
    }
}

template<typename _Key, typename _Value, size_t _Dim>
std::string
rtree<_Key,_Value,_Dim>::point::to_string() const
{
    std::ostringstream os;

    os << "(";
    for (size_t i = 0; i < dimensions; ++i)
    {
        if (i > 0)
            os << ", ";
        os << d[i];
    }
    os << ")";

    return os.str();
}

template<typename _Key, typename _Value, size_t _Dim>
bool rtree<_Key,_Value,_Dim>::point::operator== (const point& other) const
{
    const key_type* left = d;
    const key_type* right = other.d;
    const key_type* left_end = left + dimensions;

    for (; left != left_end; ++left, ++right)
    {
        if (*left != *right)
            return false;
    }

    return true;
}

template<typename _Key, typename _Value, size_t _Dim>
bool rtree<_Key,_Value,_Dim>::point::operator!= (const point& other) const
{
    return !operator== (other);
}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::bounding_box::bounding_box() {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::bounding_box::bounding_box(const point& start, const point& end) :
    start(start), end(end) {}

template<typename _Key, typename _Value, size_t _Dim>
std::string
rtree<_Key,_Value,_Dim>::bounding_box::to_string() const
{
    std::ostringstream os;
    os << start.to_string() << " - " << end.to_string();
    return os.str();
}

template<typename _Key, typename _Value, size_t _Dim>
bool rtree<_Key,_Value,_Dim>::bounding_box::operator== (const bounding_box& other) const
{
    return start == other.start && end == other.end;
}

template<typename _Key, typename _Value, size_t _Dim>
bool rtree<_Key,_Value,_Dim>::bounding_box::operator!= (const bounding_box& other) const
{
    return !operator== (other);
}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::node_store::node_store() :
    type(node_type::unspecified), parent(nullptr), node_ptr(nullptr), count(0) {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::node_store::node_store(node_store&& r) :
    type(r.type),
    box(r.box),
    parent(r.parent),
    node_ptr(r.node_ptr),
    count(r.count)
{
    r.type = node_type::unspecified;
    r.parent = nullptr;
    r.node_ptr = nullptr;
    r.count = 0;
}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::node_store::node_store(node_type type, const bounding_box& box, node* node_ptr) :
    type(type), box(box), parent(nullptr), node_ptr(node_ptr), count(0) {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::node_store::~node_store()
{
    if (node_ptr)
    {
        switch (type)
        {
            case node_type::directory_leaf:
                delete static_cast<directory_node*>(node_ptr);
                break;
            case node_type::value:
                delete static_cast<value_node*>(node_ptr);
                break;
            case node_type::unspecified:
            default:
                throw std::logic_error("node::~node: unknown node type!");
        }
    }
}

template<typename _Key, typename _Value, size_t _Dim>
typename rtree<_Key,_Value,_Dim>::node_store
rtree<_Key,_Value,_Dim>::node_store::create_directory_node()
{
    node_store ret(node_type::directory_leaf, bounding_box(), new directory_node);
    return ret;
}

template<typename _Key, typename _Value, size_t _Dim>
typename rtree<_Key,_Value,_Dim>::node_store
rtree<_Key,_Value,_Dim>::node_store::create_value_node(const bounding_box& box, value_type v)
{
    node_store ret(node_type::value, box, new value_node(std::move(v)));
    return ret;
}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::node::node() {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::node::~node() {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::value_node::value_node(value_type value) :
    value(std::move(value)) {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::value_node::~value_node() {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::directory_node::directory_node() {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::directory_node::~directory_node() {}

template<typename _Key, typename _Value, size_t _Dim>
void rtree<_Key,_Value,_Dim>::directory_node::insert(node_store&& ns)
{
    children.push_back(std::move(ns));
}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::rtree() : m_root(node_store::create_directory_node()) {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::~rtree()
{
}

template<typename _Key, typename _Value, size_t _Dim>
void rtree<_Key,_Value,_Dim>::insert(const point& start, const point& end, value_type value)
{
    std::cout << __FILE__ << "#" << __LINE__ << " (rtree:insert): start=" << start.to_string() << "; end=" << end.to_string() << std::endl;
    bounding_box bb(start, end);
    node_store* ns = find_node_for_insertion(bb);
    assert(ns);

    if (!ns->has_capacity())
    {
        // TODO : implement the "split tree".
        throw std::runtime_error("TODO: implement the 'split tree' algorithm.");
    }

    assert(ns->type == node_type::directory_leaf);
    directory_node* dir = static_cast<directory_node*>(ns->node_ptr);

    // Insert the new value to this node.
    node_store new_ns = node_store::create_value_node(bb, std::move(value));
    new_ns.parent = ns;
    dir->insert(std::move(new_ns));
    ++ns->count;

    if (ns->count == 1)
        ns->box = bb;
    else
        detail::rtree::enlarge_box_to_fit<key_type,bounding_box,dimensions>(ns->box, bb);

    std::cout << __FILE__ << "#" << __LINE__ << " (rtree:insert): ns count = " << ns->count << std::endl;
    std::cout << __FILE__ << "#" << __LINE__ << " (rtree:insert): ns box = " << ns->box.to_string() << std::endl;

    bb = ns->box; // grab the parent bounding box.

    // Propagate the bounding box update up the tree all the way to the root.
    for (ns = ns->parent; ns; ns = ns->parent)
    {
        std::cout << __FILE__ << "#" << __LINE__ << " (rtree:insert): ns count = " << ns->count << std::endl;
        std::cout << __FILE__ << "#" << __LINE__ << " (rtree:insert): ns box = " << ns->box.to_string() << std::endl;

        assert(ns->count > 0);
        detail::rtree::enlarge_box_to_fit<key_type,bounding_box,dimensions>(ns->box, bb);
    }
}

template<typename _Key, typename _Value, size_t _Dim>
const typename rtree<_Key,_Value,_Dim>::bounding_box&
rtree<_Key,_Value,_Dim>::get_total_extent() const
{
    return m_root.box;
}

template<typename _Key, typename _Value, size_t _Dim>
typename rtree<_Key,_Value,_Dim>::node_store*
rtree<_Key,_Value,_Dim>::find_node_for_insertion(const bounding_box& bb)
{
    node_store* dst = &m_root;

    while (true) // TODO : do we need to set the max iteration limit?
    {
        if (dst->type == node_type::directory_leaf)
            return dst;

        assert(dst->type == node_type::directory_nonleaf);

        directory_node* dir = static_cast<directory_node*>(dst->node_ptr);
        std::vector<node_store>& children = dir->children;

        // If this non-leaf directory contains at least one leaf directory,
        // pick the entry with minimum overlap increase.  If all of its child
        // nodes are non-leaf directories, then pick the entry with minimum
        // area enlargement.

        auto it = std::find_if(children.cbegin(), children.cend(),
            [](const node_store& ns) -> bool
            {
                return ns.type == node_type::directory_leaf;
            }
        );

        bool has_leaf_dir = it != children.cend();

        if (has_leaf_dir)
        {
            // Compare the amounts of overlap increase.

            key_type min_cost = key_type();
            dst = nullptr;

            for (node_store& ns : children)
            {
                directory_node* dir = static_cast<directory_node*>(ns.node_ptr);
                key_type cost = calc_overlap_cost(bb, *dir);

                if (cost < min_cost || !dst)
                {
                    dst = &ns;
                    min_cost = cost;
                }
            }

            // TODO: resolve ties by picking the one with less area
            // enlargement, then the one with the smallest area rectangle.

            continue;
        }

        // Compare the costs of area enlargements.
        key_type min_cost = key_type();
        dst = nullptr;

        for (node_store& ns : children)
        {
            key_type cost = detail::rtree::calc_area_enlargement<_Key,bounding_box,_Dim>(ns.box, bb);
            if (cost < min_cost || !dst)
            {
                dst = &ns;
                min_cost = cost;
            }

            // TODO: resolve ties by picking the with on the smallest area
            // rectangle.
        }
    }

    throw std::runtime_error("TODO: descend into sub-trees.");
}

template<typename _Key, typename _Value, size_t _Dim>
typename rtree<_Key,_Value,_Dim>::key_type
rtree<_Key,_Value,_Dim>::calc_overlap_cost(
    const bounding_box& bb, const directory_node& dir) const
{
    key_type overlap_cost = key_type();

    for (const node_store& ns : dir.children)
        overlap_cost += detail::rtree::calc_intersection<_Key,bounding_box,_Dim>(ns.box, bb);

    return overlap_cost;
}

} // namespace mdds

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

