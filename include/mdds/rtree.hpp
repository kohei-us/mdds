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

namespace mdds {

template<typename _Key, typename _Value, size_t _Dim = 2>
class rtree
{
    static const size_t dimensions = _Dim;

    using key_type = _Key;
    using value_type = _Value;

    struct point
    {
        key_type d[dimensions];

        point();
    };

    struct bounding_box
    {
        point start;
        point end;

        bounding_box();
    };

    enum class node_type { unspecified, directory, value };

    struct node
    {
        node_type type;
        bounding_box box;

        std::vector<node*> store;

        node() = delete;
        node(node_type type);
        ~node();
    };

    struct value_node : public node
    {
        value_type value;

        value_node();
        ~value_node();
    };

    struct directory_node : public node
    {
        directory_node();
        ~directory_node();
    };

public:
    rtree();
    ~rtree();

    rtree(const rtree&) = delete;
    rtree& operator= (const rtree&) = delete;

private:
    directory_node* m_root;
};

}

#include "rtree_def.inl"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */