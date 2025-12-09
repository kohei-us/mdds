/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.
#include "test_global_rtree.hpp"

using mdds::rtree;
using std::cout;
using std::endl;

void rtree_test_directory_node_split()
{
    MDDS_TEST_FUNC_SCOPE;

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

    TEST_ASSERT(tree.size() == 100);

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
        TEST_ASSERT(it != res.cend());
        TEST_ASSERT(it.depth() == 4);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
