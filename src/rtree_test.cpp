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

#include <mdds/rtree.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

#include "test_global.hpp"

using namespace mdds;
using namespace std;

struct tiny_trait
{
    constexpr static const size_t dimensions = 2;
    constexpr static const size_t min_node_size = 2;
    constexpr static const size_t max_node_size = 5;
    constexpr static const size_t max_tree_depth = 100;
};

void rtree_test_intersection()
{
    stack_printer __stack_printer__("::rtree_test_intersection");
    using rt_type = rtree<int16_t, std::string>;
    using bounding_box = rt_type::bounding_box;
    using detail::rtree::calc_linear_intersection;
    using detail::rtree::calc_intersection;

    struct check
    {
        bounding_box bb1;
        bounding_box bb2;
        int16_t expected_length1;
        int16_t expected_length2;
    };

    std::vector<check> checks =
    {
        { { {0, 0}, {3, 6} }, { {1, 2}, {7, 5} }, 2, 3 },
        { { {3, 2}, {7, 10} }, { {1, 10}, {10, 11} }, 4, 0 },
        { { {3, 2}, {7, 10} }, { {1, 9}, {10, 11} }, 4, 1 },
        { { {3, 2}, {7, 6} }, { {5, 4}, {11, 8} }, 2, 2 },
        { { {-2, -8}, {2, -5} }, { {0, -10}, {8, -1} }, 2, 3 },
        { { {2, 2}, {20, 12} }, { {5, 6}, {16, 9} }, 11, 3 },
    };

    for (const check& c : checks)
    {
        int16_t length1 = calc_linear_intersection<int16_t,bounding_box>(0, c.bb1, c.bb2);
        assert(length1 == c.expected_length1);
        int16_t length2 = calc_linear_intersection<int16_t,bounding_box>(1, c.bb1, c.bb2);
        assert(length2 == c.expected_length2);

        int16_t area = calc_intersection<int16_t,bounding_box,2>(c.bb1, c.bb2);
        int16_t expected_area = c.expected_length1 * c.expected_length2;
        assert(area == expected_area);

        // Swap the boxes and run the same tests. We should get the same results.

        length1 = calc_linear_intersection<int16_t,bounding_box>(0, c.bb2, c.bb1);
        assert(length1 == c.expected_length1);
        length2 = calc_linear_intersection<int16_t,bounding_box>(1, c.bb2, c.bb1);
        assert(length2 == c.expected_length2);

        area = calc_intersection<int16_t,bounding_box,2>(c.bb2, c.bb1);
        assert(area == expected_area);
    }
}

void rtree_test_area_enlargement()
{
    stack_printer __stack_printer__("::rtree_test_area_enlargement");
    using rt_type = rtree<int16_t, std::string>;
    using bounding_box = rt_type::bounding_box;
    using detail::rtree::calc_area_enlargement;

    struct check
    {
        bounding_box host;
        bounding_box guest;
        int16_t expected_area;
    };

    std::vector<check> checks =
    {
        { {{0,0}, {2,2}}, {{ 2, 2}, { 3, 3}},  5 }, // 3x3 - 2x2 = 5
        { {{0,0}, {2,2}}, {{ 0, 1}, { 1, 2}},  0 }, // no enlargement
        { {{0,0}, {3,3}}, {{-3, 1}, { 7, 2}}, 21 }, // 10x3 - 3x3 = 21
        { {{0,0}, {0,0}}, {{-1,-1}, { 0, 0}},  1 }, // 1x1 = 1
        { {{0,0}, {1,1}}, {{-4,-3}, {-3,-2}}, 19 }, // 5x4 - 1x1 = 19
    };

    for (const check& c : checks)
    {
        int16_t area = calc_area_enlargement<int16_t,bounding_box,2u>(c.host, c.guest);
        assert(area == c.expected_area);
    }
}

void rtree_test_basic_search()
{
    stack_printer __stack_printer__("::rtree_test_basic_search");
    using rt_type = mdds::rtree<int16_t, std::string>;

    rt_type tree;
    rt_type::bounding_box expected_bb;

    tree.insert({0, 0}, {2, 2}, "test");
    expected_bb = {{0, 0}, {2, 2}};
    assert(tree.get_root_extent() == expected_bb);

    tree.insert({3, 3}, {5, 5}, "test again");
    expected_bb = {{0, 0}, {5, 5}};
    assert(tree.get_root_extent() == expected_bb);

    tree.insert({-2, 1}, {3, 6}, "more test");
    expected_bb = {{-2, 0}, {5, 6}};
    assert(tree.get_root_extent() == expected_bb);

    // Verify the search method works.

    rt_type::const_search_results res = tree.search({1, 1});

    auto it = res.cbegin(), it_end = res.cend();

    size_t n = std::distance(it, it_end);
    assert(n == 2);

    std::unordered_map<std::string, rt_type::bounding_box> expected_values =
    {
        { "test",      {{ 0, 0}, {2, 2}} },
        { "more test", {{-2, 1}, {3, 6}} },
    };

    for (; it != it_end; ++it)
    {
        cout << "bounding box: " << it->box.to_string() << "; value: " << it->value << endl;
        auto itv = expected_values.find(it->value);
        assert(itv != expected_values.end());
        assert(itv->second == it->box);
    }

    // Perform an out-of-bound search by point.
    std::vector<rt_type::point> pts =
    {
        {-10, -10},
        {1, 7},
        {6, 3},
    };

    for (const rt_type::point& pt : pts)
    {
        res = tree.search(pt);
        assert(res.cbegin() == res.cend());
    }
}

void rtree_test_basic_erase()
{
    stack_printer __stack_printer__("::rtree_test_basic_erase");
    using rt_type = mdds::rtree<int16_t, std::string>;

    rt_type tree;
    tree.insert({-2,-2}, {2,2}, "erase me");
    assert(!tree.empty());

    rt_type::const_search_results res = tree.search({0,0});

    size_t n = std::distance(res.begin(), res.end());
    assert(n == 1);

    rt_type::const_iterator it = res.begin();
    assert(it != res.end());

    tree.erase(it);
    assert(tree.empty());
    assert(rt_type::bounding_box() == tree.get_root_extent());

    tree.insert({0,0}, {2,2}, "erase me");
    tree.insert({-10,-4}, {0,0}, "erase me");
    rt_type::bounding_box expected_bb({-10,-4}, {2,2});
    assert(tree.get_root_extent() == expected_bb);

    res = tree.search({-5,-2});
    n = std::distance(res.begin(), res.end());
    assert(n == 1);
    it = res.begin();
    tree.erase(it);
    assert(!tree.empty()); // there should be one value stored in the tree.
    expected_bb = {{0,0}, {2,2}};
    assert(tree.get_root_extent() == expected_bb);
}

void rtree_test_node_split()
{
    stack_printer __stack_printer__("::rtree_test_node_split");
    using rt_type = rtree<int16_t, std::string, tiny_trait>;

    rt_type tree;

    for (int16_t i = 0; i < 6; ++i)
    {
        int16_t w = 1;
        tree.insert({i, i}, {int16_t(i+w), int16_t(i+w)}, "foo");
    }
}

int main(int argc, char** argv)
{
    rtree_test_intersection();
    rtree_test_area_enlargement();
    rtree_test_basic_search();
    rtree_test_basic_erase();
    rtree_test_node_split();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

