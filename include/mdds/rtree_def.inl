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
rtree<_Key,_Value,_Dim>::bounding_box::bounding_box() {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::bounding_box::bounding_box(const point& start, const point& end) :
    start(start), end(end) {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::node::node(node_type type) : type(type), parent(nullptr) {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::node::node(node_type type, const bounding_box& bb) :
    type(type), box(bb), parent(nullptr) {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::node::~node()
{
    for (node* p : store)
    {
        switch (p->type)
        {
            case node_type::directory:
                delete static_cast<directory_node*>(p);
                break;
            case node_type::value:
                delete static_cast<value_node*>(p);
                break;
            case node_type::unspecified:
            default:
                throw std::logic_error("node::~node: unknown node type!");
        }
    }
}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::value_node::value_node(const bounding_box& bb, value_type value) :
    node(node_type::value), value(std::move(value))
{

}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::value_node::~value_node() {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::directory_node::directory_node() : node(node_type::directory) {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::directory_node::~directory_node() {}

template<typename _Key, typename _Value, size_t _Dim>
void rtree<_Key,_Value,_Dim>::directory_node::insert(node* p)
{
    p->parent = this;
    store.push_back(p);

    throw std::runtime_error("TODO: propagate the bbox adjustment upward.");
}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::rtree() : m_root(new directory_node) {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::~rtree()
{
    delete m_root;
}

template<typename _Key, typename _Value, size_t _Dim>
void rtree<_Key,_Value,_Dim>::insert(const point& start, const point& end, value_type value)
{
    std::cout << __FILE__ << "#" << __LINE__ << " (rtree:insert): start=" << start.to_string() << "; end=" << end.to_string() << std::endl;
    bounding_box bb(start, end);
    directory_node* dir = find_node_for_insertion(bb);
    if (!dir->has_capacity())
        throw std::runtime_error("WIP");

    // Insert the new value to this node.
    dir->insert(new value_node(bb, std::move(value)));

    // Propagate the bounding box adjustment upward.

}

template<typename _Key, typename _Value, size_t _Dim>
typename rtree<_Key,_Value,_Dim>::directory_node*
rtree<_Key,_Value,_Dim>::find_node_for_insertion(const bounding_box& bb)
{
    directory_node* dst = m_root;
    if (dst->is_leaf())
        return dst;

    throw std::runtime_error("WIP");
}

} // namespace mdds

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

