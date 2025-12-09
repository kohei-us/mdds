/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.
#include "test_global_rtree.hpp"

using mdds::rtree;
using std::cout;
using std::endl;

void rtree_test_intersection()
{
    MDDS_TEST_FUNC_SCOPE;

    using rt_type = rtree<int16_t, std::string>;
    using bounding_box = rt_type::extent_type;
    using mdds::detail::rtree::calc_intersection;
    using mdds::detail::rtree::calc_linear_intersection;

    struct check
    {
        bounding_box bb1;
        bounding_box bb2;
        int16_t expected_length1;
        int16_t expected_length2;
    };

    std::vector<check> checks = {
        // bounding box 1           bounding box 2
        {{{0, 0}, {3, 6}}, {{1, 2}, {7, 5}}, 2, 3},       {{{3, 2}, {7, 10}}, {{1, 10}, {10, 11}}, 4, 0},
        {{{3, 2}, {7, 10}}, {{1, 9}, {10, 11}}, 4, 1},    {{{3, 2}, {7, 6}}, {{5, 4}, {11, 8}}, 2, 2},
        {{{-2, -8}, {2, -5}}, {{0, -10}, {8, -1}}, 2, 3}, {{{2, 2}, {20, 12}}, {{5, 6}, {16, 9}}, 11, 3},
        {{{0, 0}, {6, 6}}, {{0, 0}, {2, 3}}, 2, 3},
    };

    for (const check& c : checks)
    {
        int16_t length1 = calc_linear_intersection<bounding_box>(0, c.bb1, c.bb2);
        TEST_ASSERT(length1 == c.expected_length1);
        int16_t length2 = calc_linear_intersection<bounding_box>(1, c.bb1, c.bb2);
        TEST_ASSERT(length2 == c.expected_length2);

        int16_t area = calc_intersection<bounding_box>(c.bb1, c.bb2);
        int16_t expected_area = c.expected_length1 * c.expected_length2;
        TEST_ASSERT(area == expected_area);

        // Swap the boxes and run the same tests. We should get the same results.

        length1 = calc_linear_intersection<bounding_box>(0, c.bb2, c.bb1);
        TEST_ASSERT(length1 == c.expected_length1);
        length2 = calc_linear_intersection<bounding_box>(1, c.bb2, c.bb1);
        TEST_ASSERT(length2 == c.expected_length2);

        area = calc_intersection<bounding_box>(c.bb2, c.bb1);
        TEST_ASSERT(area == expected_area);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
