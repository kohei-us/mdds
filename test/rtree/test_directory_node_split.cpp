/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2021 Kohei Yoshida
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

#include "test_global.hpp" // This must be the first header to be included.
#include "test_global_rtree.hpp"

using mdds::rtree;
using std::cout;
using std::endl;

void rtree_test_directory_node_split()
{
    stack_printer __stack_printer__("::rtree_test_directory_node_split");
    using rt_type = rtree<int16_t, std::string, tiny_trait_2d>;
    using search_type = rt_type::search_type;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = true;

    rt_type tree;
    const rt_type& ctree = tree;
    using point = rt_type::point_type;
    using bounding_box = rt_type::extent_type;

    for (int16_t x = 0; x < 10; ++x)
    {
        for (int16_t y = 0; y < 10; ++y)
        {
            std::ostringstream os;
            os << "(x=" << x << ",y=" << y << ")";
            std::string v = os.str();
            int16_t xe = x + 1, ye = y + 1;
            point s({x, y}), e({xe, ye});
            bounding_box bb(s, e);
            cout << "Inserting value '" << v << "' to {" << bb.to_string() << "} ..." << endl;
            tree.insert({s, e}, std::move(v));
            tree.check_integrity(check_props);
        }
    }

    assert(tree.size() == 100);

    // All value nodes in this tree should be at depth 4 (root having the
    // depth of 0).  Just check a few of them.

    std::vector<point> pts = {
        {5, 5},
        {2, 3},
        {7, 9},
    };

    for (const point& pt : pts)
    {
        auto res = ctree.search(pt, search_type::overlap);
        auto it = res.cbegin();
        assert(it != res.cend());
        assert(it.depth() == 4);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
