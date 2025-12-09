/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.
#include "test_global_rtree.hpp"

using mdds::rtree;
using std::cout;
using std::endl;

void rtree_test_area_enlargement()
{
    MDDS_TEST_FUNC_SCOPE;

    using rt_type = rtree<int16_t, std::string>;
    using bounding_box = rt_type::extent_type;
    using mdds::detail::rtree::calc_area_enlargement;

    struct check
    {
        bounding_box host;
        bounding_box guest;
        int16_t expected_area;
    };

    std::vector<check> checks = {
        {{{0, 0}, {2, 2}}, {{2, 2}, {3, 3}}, 5}, // 3x3 - 2x2 = 5
        {{{0, 0}, {2, 2}}, {{0, 1}, {1, 2}}, 0}, // no enlargement
        {{{0, 0}, {3, 3}}, {{-3, 1}, {7, 2}}, 21}, // 10x3 - 3x3 = 21
        {{{0, 0}, {0, 0}}, {{-1, -1}, {0, 0}}, 1}, // 1x1 = 1
        {{{0, 0}, {1, 1}}, {{-4, -3}, {-3, -2}}, 19}, // 5x4 - 1x1 = 19
    };

    for (const check& c : checks)
    {
        int16_t area = calc_area_enlargement(c.host, c.guest);
        TEST_ASSERT(area == c.expected_area);
    }
}

void rtree_test_center_point()
{
    MDDS_TEST_FUNC_SCOPE;

    using rt_type = rtree<int16_t, std::string, tiny_trait_2d>;
    using mdds::detail::rtree::get_center_point;
    using extent_type = rt_type::extent_type;
    using point_type = rt_type::point_type;

    struct test_case
    {
        extent_type extent;
        point_type expected;
    };

    std::vector<test_case> tcs = {
        {{{0, 0}, {2, 2}}, {1, 1}},
        {{{-2, -4}, {2, 4}}, {0, 0}},
        {{{3, 5}, {8, 10}}, {5, 7}},
    };

    for (const test_case& tc : tcs)
    {
        cout << "extent: " << tc.extent.to_string() << endl;
        auto pt = get_center_point(tc.extent);
        TEST_ASSERT(pt == tc.expected);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
