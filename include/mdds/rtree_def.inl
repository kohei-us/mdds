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

namespace mdds {

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
    type(node_type::unspecified), parent(nullptr), node_ptr(nullptr), count(0), leaf(true) {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::node_store::node_store(node_store&& r) :
    type(r.type),
    box(r.box),
    parent(r.parent),
    node_ptr(r.node_ptr),
    count(r.count),
    leaf(r.leaf)
{
    r.type = node_type::unspecified;
    r.parent = nullptr;
    r.node_ptr = nullptr;
    r.count = 0;
    r.leaf = true;
}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::node_store::node_store(node_type type, const bounding_box& box, node* node_ptr) :
    type(type), box(box), parent(nullptr), node_ptr(node_ptr), count(0), leaf(true) {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::node_store::~node_store()
{
    if (node_ptr)
    {
        switch (type)
        {
            case node_type::directory:
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
    node_store ret(node_type::directory, bounding_box(), new directory_node);
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

    assert(ns->type == node_type::directory);
    directory_node* dir = static_cast<directory_node*>(ns->node_ptr);

    // Insert the new value to this node.
    node_store new_ns = node_store::create_value_node(bb, std::move(value));
    new_ns.parent = ns;
    dir->insert(std::move(new_ns));
    ++ns->count;

    if (ns->count == 1)
        ns->box = bb;
    else
        expand_box_to_fit(ns->box, bb);

    std::cout << __FILE__ << "#" << __LINE__ << " (rtree:insert): ns count = " << ns->count << std::endl;
    std::cout << __FILE__ << "#" << __LINE__ << " (rtree:insert): ns box = " << ns->box.to_string() << std::endl;

    bb = ns->box; // grab the parent bounding box.

    // Propagate the bounding box update up the tree all the way to the root.
    for (ns = ns->parent; ns; ns = ns->parent)
    {
        std::cout << __FILE__ << "#" << __LINE__ << " (rtree:insert): ns count = " << ns->count << std::endl;
        std::cout << __FILE__ << "#" << __LINE__ << " (rtree:insert): ns box = " << ns->box.to_string() << std::endl;

        assert(ns->count > 0);
        expand_box_to_fit(ns->box, bb);
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
    if (dst->leaf)
        return dst;

    throw std::runtime_error("TODO: descend into sub-trees.");
}

template<typename _Key, typename _Value, size_t _Dim>
void rtree<_Key,_Value,_Dim>::expand_box_to_fit(bounding_box& parent, const bounding_box& child) const
{
    for (size_t dim = 0; dim < dimensions; ++dim)
    {
        if (child.start.d[dim] < parent.start.d[dim])
            parent.start.d[dim] = child.start.d[dim];

        if (parent.end.d[dim] < child.end.d[dim])
            parent.end.d[dim] = child.end.d[dim];
    }
}

} // namespace mdds

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

