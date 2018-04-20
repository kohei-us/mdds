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

#include <stdexcept>

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
rtree<_Key,_Value,_Dim>::bounding_box::bounding_box() {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::node::node(node_type type) : type(type) {}

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
rtree<_Key,_Value,_Dim>::value_node::value_node() : node(node_type::value) {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::value_node::~value_node() {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::directory_node::directory_node() : node(node_type::directory) {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::directory_node::~directory_node() {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::rtree() : m_root(new directory_node) {}

template<typename _Key, typename _Value, size_t _Dim>
rtree<_Key,_Value,_Dim>::~rtree()
{
    delete m_root;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

