/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.
#include "test_global_rtree.hpp"

using mdds::rtree;
using std::cout;
using std::endl;

void rtree_test_square_distance()
{
    MDDS_TEST_FUNC_SCOPE;

    using mdds::detail::rtree::calc_square_distance;

    {
        // 1 dimensional unsigned
        using rt_type = rtree<uint16_t, std::string, tiny_trait_1d>;
        using point_type = rt_type::point_type;

        struct test_case
        {
            point_type p1;
            point_type p2;
            uint16_t expected;
        };

        std::vector<test_case> tcs = {
            {{3}, {5}, 4},
            {{9}, {2}, 49},
            {{0}, {0}, 0},
        };

        for (const test_case& tc : tcs)
        {
            cout << "p1: " << tc.p1.to_string() << "; p2: " << tc.p2.to_string() << endl;

            uint16_t dist = calc_square_distance<point_type>(tc.p1, tc.p2);
            TEST_ASSERT(dist == tc.expected);

            // Flip the value to make sure we still get the same result.
            dist = calc_square_distance<point_type>(tc.p2, tc.p1);
            TEST_ASSERT(dist == tc.expected);
        }
    }

    {
        // 2 dimensional unsigned
        using rt_type = rtree<uint16_t, std::string, tiny_trait_2d>;
        using point_type = rt_type::point_type;

        struct test_case
        {
            point_type p1;
            point_type p2;
            uint16_t expected;
        };

        std::vector<test_case> tcs = {
            {{0, 0}, {0, 0}, 0},
            {{0, 0}, {1, 1}, 2},
            {{0, 0}, {2, 2}, 8},
            {{3, 0}, {0, 4}, 25},
        };

        for (const test_case& tc : tcs)
        {
            cout << "p1: " << tc.p1.to_string() << "; p2: " << tc.p2.to_string() << endl;

            uint16_t dist = calc_square_distance<point_type>(tc.p1, tc.p2);
            TEST_ASSERT(dist == tc.expected);

            // Flip the value to make sure we still get the same result.
            dist = calc_square_distance<point_type>(tc.p2, tc.p1);
            TEST_ASSERT(dist == tc.expected);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
