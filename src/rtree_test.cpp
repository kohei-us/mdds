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
    constexpr static size_t dimensions = 2;
    constexpr static size_t min_node_size = 2;
    constexpr static size_t max_node_size = 5;
    constexpr static size_t max_tree_depth = 100;
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
        // bounding box 1           bounding box 2
        { { { 0,  0}, { 3,   6} }, { {1,   2}, { 7,  5} },  2, 3 },
        { { { 3,  2}, { 7,  10} }, { {1,  10}, {10, 11} },  4, 0 },
        { { { 3,  2}, { 7,  10} }, { {1,   9}, {10, 11} },  4, 1 },
        { { { 3,  2}, { 7,   6} }, { {5,   4}, {11,  8} },  2, 2 },
        { { {-2, -8}, { 2,  -5} }, { {0, -10}, { 8, -1} },  2, 3 },
        { { { 2,  2}, {20,  12} }, { {5,   6}, {16,  9} }, 11, 3 },
        { { { 0,  0}, { 6,   6} }, { {0,   0}, { 2,  3} },  2, 3 },
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

    tree.check_integrity(rt_type::output_mode_type::none);

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

    tree.check_integrity(rt_type::output_mode_type::none);
}

void rtree_test_node_split()
{
    stack_printer __stack_printer__("::rtree_test_node_split");
    using rt_type = rtree<int16_t, std::string, tiny_trait>;

    rt_type tree;

    // Inserting 6 entries should cause the root directory node to split.
    // After the split, the root node should become a non-leaf directory
    // storing two leaf directory nodes as its children.

    for (int16_t i = 0; i < 6; ++i)
    {
        int16_t w = 1;
        std::ostringstream os;
        os << "foo" << i;
        tree.insert({i, i}, {int16_t(i+w), int16_t(i+w)}, os.str());
    }

    size_t count_values = 0;
    size_t count_leaf = 0;
    size_t count_nonleaf = 0;

    auto walker = [&](const rt_type::node_properties& np)
    {
        switch (np.type)
        {
            case rt_type::node_type::value:
                ++count_values;
                break;
            case rt_type::node_type::directory_leaf:
                ++count_leaf;
                break;
            case rt_type::node_type::directory_nonleaf:
                ++count_nonleaf;
                break;
            default:
                ;
        }
    };

    tree.walk(walker);

    assert(count_values == 6);
    assert(count_leaf == 2);
    assert(count_nonleaf == 1);

    tree.check_integrity(rt_type::output_mode_type::none);

    // Adding two more entries will cause one of the leaf directory nodes
    // below the root node to split.

    for (int16_t i = 6; i < 8; ++i)
    {
        int16_t w = 1;
        std::ostringstream os;
        os << "bar" << i;
        tree.insert({i, i}, {int16_t(i+w), int16_t(i+w)}, os.str());
    }

    tree.check_integrity(rt_type::output_mode_type::none);

    // Count all the nodes again.
    count_values = 0;
    count_leaf = 0;
    count_nonleaf = 0;

    tree.walk(walker);

    assert(count_values == 8);
    assert(count_leaf == 3);
    assert(count_nonleaf == 1);

    // Erase the entry at (0, 0).  There should be only one match.  Erasing
    // this entry will cause the node to be underfilled.

    rt_type::const_search_results res = tree.search({0,0});
    auto it = res.cbegin();
    assert(it != res.cend());
    assert(std::distance(it, res.cend()) == 1);
    tree.erase(it);

    tree.check_integrity(rt_type::output_mode_type::none);

    // Count all the nodes again.
    count_values = 0;
    count_leaf = 0;
    count_nonleaf = 0;

    tree.walk(walker);

    assert(count_values == 7);
    assert(count_leaf == 2);
    assert(count_nonleaf == 1);
}

void rtree_test_directory_node_split()
{
    stack_printer __stack_printer__("::rtree_test_directory_node_split");
    using rt_type = rtree<int16_t, std::string, tiny_trait>;

    rt_type tree;
    using point = rt_type::point;
    using bounding_box = rt_type::bounding_box;

    for (int16_t x = 0; x < 10; ++x)
    {
        for (int16_t y = 0; y < 10; ++y)
        {
            std::ostringstream os;
            os << "(x=" << x << ",y=" << y << ")";
            std::string v = os.str();
            int16_t xe = x + 1, ye = y + 1;
            point s({x,y}), e({xe,ye});
            bounding_box bb(s, e);
            cout << "Inserting value '" << v << "' to {" << bb.to_string() << "} ..." << endl;
            tree.insert(s, e, v);
            tree.check_integrity(rt_type::output_mode_type::none);
        }
    }
}

int main(int argc, char** argv)
{
    rtree_test_intersection();
    rtree_test_area_enlargement();
    rtree_test_basic_search();
    rtree_test_basic_erase();
    rtree_test_node_split();
    rtree_test_directory_node_split();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

