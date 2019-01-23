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

#include "test_global.hpp" // This must be the first header to be included.
#include "test_global_rtree.hpp"

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>

using namespace mdds;
using namespace std;

void rtree_test_intersection()
{
    stack_printer __stack_printer__("::rtree_test_intersection");
    using rt_type = rtree<int16_t, std::string>;
    using bounding_box = rt_type::extent_type;
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
        int16_t length1 = calc_linear_intersection<bounding_box>(0, c.bb1, c.bb2);
        assert(length1 == c.expected_length1);
        int16_t length2 = calc_linear_intersection<bounding_box>(1, c.bb1, c.bb2);
        assert(length2 == c.expected_length2);

        int16_t area = calc_intersection<bounding_box>(c.bb1, c.bb2);
        int16_t expected_area = c.expected_length1 * c.expected_length2;
        assert(area == expected_area);

        // Swap the boxes and run the same tests. We should get the same results.

        length1 = calc_linear_intersection<bounding_box>(0, c.bb2, c.bb1);
        assert(length1 == c.expected_length1);
        length2 = calc_linear_intersection<bounding_box>(1, c.bb2, c.bb1);
        assert(length2 == c.expected_length2);

        area = calc_intersection<bounding_box>(c.bb2, c.bb1);
        assert(area == expected_area);
    }
}

void rtree_test_square_distance()
{
    stack_printer __stack_printer__("::rtree_test_square_distance");
    using detail::rtree::calc_square_distance;

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

        std::vector<test_case> tcs =
        {
            { {3}, {5},  4 },
            { {9}, {2}, 49 },
            { {0}, {0},  0 },
        };

        for (const test_case& tc : tcs)
        {
            cout << "p1: " << tc.p1.to_string() << "; p2: " << tc.p2.to_string() << endl;

            uint16_t dist = calc_square_distance<point_type>(tc.p1, tc.p2);
            assert(dist == tc.expected);

            // Flip the value to make sure we still get the same result.
            dist = calc_square_distance<point_type>(tc.p2, tc.p1);
            assert(dist == tc.expected);
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

        std::vector<test_case> tcs =
        {
            { {0, 0}, {0, 0},  0 },
            { {0, 0}, {1, 1},  2 },
            { {0, 0}, {2, 2},  8 },
            { {3, 0}, {0, 4},  25 },
        };

        for (const test_case& tc : tcs)
        {
            cout << "p1: " << tc.p1.to_string() << "; p2: " << tc.p2.to_string() << endl;

            uint16_t dist = calc_square_distance<point_type>(tc.p1, tc.p2);
            assert(dist == tc.expected);

            // Flip the value to make sure we still get the same result.
            dist = calc_square_distance<point_type>(tc.p2, tc.p1);
            assert(dist == tc.expected);
        }
    }
}

void rtree_test_center_point()
{
    stack_printer __stack_printer__("::rtree_test_center_point");
    using rt_type = rtree<int16_t, std::string, tiny_trait_2d>;
    using detail::rtree::get_center_point;
    using extent_type = rt_type::extent_type;
    using point_type = rt_type::point_type;

    struct test_case
    {
        extent_type extent;
        point_type expected;
    };

    std::vector<test_case> tcs =
    {
        { {{0, 0}, {2, 2}}, {1, 1} },
        { {{-2, -4}, {2, 4}}, {0, 0} },
        { {{3, 5}, {8, 10}}, {5, 7} },
    };

    for (const test_case& tc : tcs)
    {
        cout << "extent: " << tc.extent.to_string() << endl;
        auto pt = get_center_point(tc.extent);
        assert(pt == tc.expected);
    }
}

void rtree_test_area_enlargement()
{
    stack_printer __stack_printer__("::rtree_test_area_enlargement");
    using rt_type = rtree<int16_t, std::string>;
    using bounding_box = rt_type::extent_type;
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
        int16_t area = calc_area_enlargement(c.host, c.guest);
        assert(area == c.expected_area);
    }
}

void rtree_test_basic_search()
{
    stack_printer __stack_printer__("::rtree_test_basic_search");
    using rt_type = rtree<int16_t, std::string>;
    using search_type = rt_type::search_type;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = true;

    rt_type tree;
    const rt_type& ctree = tree;
    rt_type::extent_type expected_bb;

    tree.insert({{0, 0}, {2, 2}}, "test");
    expected_bb = {{0, 0}, {2, 2}};
    assert(tree.extent() == expected_bb);
    assert(tree.size() == 1);

    tree.insert({{3, 3}, {5, 5}}, "test again");
    expected_bb = {{0, 0}, {5, 5}};
    assert(tree.extent() == expected_bb);
    assert(tree.size() == 2);

    tree.insert({{-2, 1}, {3, 6}}, "more test");
    expected_bb = {{-2, 0}, {5, 6}};
    assert(tree.extent() == expected_bb);
    assert(tree.size() == 3);

    tree.check_integrity(check_props);

    // Verify the search method works.

    rt_type::const_search_results res = ctree.search({1, 1}, search_type::overlap);

    auto it = res.cbegin(), it_end = res.cend();

    size_t n = std::distance(it, it_end);
    assert(n == 2);

    std::unordered_map<std::string, rt_type::extent_type> expected_values =
    {
        { "test",      {{ 0, 0}, {2, 2}} },
        { "more test", {{-2, 1}, {3, 6}} },
    };

    for (; it != it_end; ++it)
    {
        cout << "bounding box: " << it.extent().to_string() << "; value: " << *it << "; depth: " << it.depth() << endl;
        auto itv = expected_values.find(*it);
        assert(itv != expected_values.end());
        assert(itv->second == it.extent());
        assert(it.depth() == 1);
    }

    // Perform an out-of-bound search by point.
    std::vector<rt_type::point_type> pts =
    {
        {-10, -10},
        {1, 7},
        {6, 3},
    };

    for (const rt_type::point_type& pt : pts)
    {
        res = ctree.search(pt, search_type::overlap);
        assert(res.cbegin() == res.cend());
    }
}

void rtree_test_basic_erase()
{
    stack_printer __stack_printer__("::rtree_test_basic_erase");
    using rt_type = rtree<int16_t, std::string>;
    using search_type = rt_type::search_type;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = true;

    rt_type tree;
    const rt_type& ctree = tree;
    tree.insert({{-2,-2}, {2,2}}, "erase me");
    assert(!tree.empty());
    assert(tree.size() == 1);

    rt_type::const_search_results res = ctree.search({0, 0}, search_type::overlap);

    size_t n = std::distance(res.begin(), res.end());
    assert(n == 1);

    rt_type::const_iterator it = res.begin();
    assert(it != res.end());

    tree.erase(it);
    assert(tree.empty());
    assert(tree.size() == 0);
    assert(rt_type::extent_type() == tree.extent());

    tree.insert({{0,0}, {2,2}}, "erase me");
    tree.insert({{-10,-4}, {0,0}}, "erase me");
    rt_type::extent_type expected_bb({-10,-4}, {2,2});
    assert(tree.extent() == expected_bb);
    assert(tree.size() == 2);

    res = ctree.search({-5, -2}, search_type::overlap);
    n = std::distance(res.begin(), res.end());
    assert(n == 1);
    it = res.begin();
    tree.erase(it);
    assert(!tree.empty()); // there should be one value stored in the tree.
    assert(tree.size() == 1);
    expected_bb = {{0,0}, {2,2}};
    assert(tree.extent() == expected_bb);

    tree.check_integrity(check_props);
}

void rtree_test_node_split()
{
    stack_printer __stack_printer__("::rtree_test_node_split");
    using rt_type = rtree<int16_t, std::string, tiny_trait_2d>;
    using search_type = rt_type::search_type;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = true;

    rt_type tree;
    const rt_type& ctree = tree;

    // Inserting 6 entries should cause the root directory node to split.
    // After the split, the root node should become a non-leaf directory
    // storing two leaf directory nodes as its children.

    for (int16_t i = 0; i < 6; ++i)
    {
        int16_t w = 1;
        std::ostringstream os;
        os << "foo" << i;
        tree.insert({{i, i}, {int16_t(i+w), int16_t(i+w)}}, os.str());
    }

    assert(tree.size() == 6);

    cout << tree.export_tree(rt_type::export_tree_type::formatted_node_properties) << endl;

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

    tree.check_integrity(check_props);

    // Adding two more entries will cause one of the leaf directory nodes
    // below the root node to split.

    for (int16_t i = 6; i < 8; ++i)
    {
        int16_t w = 1;
        std::ostringstream os;
        os << "bar" << i;
        tree.insert({{i, i}, {int16_t(i+w), int16_t(i+w)}}, os.str());
    }

    assert(tree.size() == 8);
    tree.check_integrity(check_props);

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

    rt_type::const_search_results res = ctree.search({0, 0}, search_type::overlap);
    auto it = res.cbegin();
    assert(it != res.cend());
    assert(std::distance(it, res.cend()) == 1);
    tree.erase(it);

    assert(tree.size() == 7);
    tree.check_integrity(check_props);

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
            point s({x,y}), e({xe,ye});
            bounding_box bb(s, e);
            cout << "Inserting value '" << v << "' to {" << bb.to_string() << "} ..." << endl;
            tree.insert({s, e}, std::move(v));
            tree.check_integrity(check_props);
        }
    }

    assert(tree.size() == 100);

    // All value nodes in this tree should be at depth 4 (root having the
    // depth of 0).  Just check a few of them.

    std::vector<point> pts =
    {
        { 5, 5 },
        { 2, 3 },
        { 7, 9 },
    };

    for (const point& pt : pts)
    {
        auto res = ctree.search(pt, search_type::overlap);
        auto it = res.cbegin();
        assert(it != res.cend());
        assert(it.depth() == 4);
    }
}

void rtree_test_erase_directories()
{
    stack_printer __stack_printer__("::rtree_test_erase_directories");
    using rt_type = rtree<int16_t, std::string, tiny_trait_2d>;
    using search_type = rt_type::search_type;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = true;

    rt_type tree;
    const rt_type& ctree = tree;
    using point = rt_type::point_type;
    using bounding_box = rt_type::extent_type;

    for (int16_t x = 0; x < 5; ++x)
    {
        for (int16_t y = 0; y < 5; ++y)
        {
            std::ostringstream os;
            int16_t x2 = x * 2;
            int16_t y2 = y * 2;
            os << "(x=" << x2 << ",y=" << y2 << ")";
            std::string v = os.str();
            int16_t xe = x2 + 2, ye = y2 + 2;
            point s({x2, y2}), e({xe, ye});
            bounding_box bb(s, e);
            tree.insert({s, e}, std::move(v));
        }
    }

    assert(tree.size() == 25);
    tree.check_integrity(check_props);
    cout << tree.export_tree(rt_type::export_tree_type::formatted_node_properties) << endl;

    size_t expected_size = 25;

    for (int16_t x = 0; x < 5; ++x)
    {
        for (int16_t y = 0; y < 5; ++y)
        {
            int16_t x2 = x * 2 + 1;
            int16_t y2 = y * 2 + 1;

            cout << "erase at (" << x2 << ", " << y2 << ")" << endl;

            auto res = ctree.search({x2, y2}, search_type::overlap);
            auto it = res.begin(), ite = res.end();
            size_t n = std::distance(it, ite);
            assert(n == 1);

            tree.erase(it);

            assert(tree.size() == --expected_size);
            tree.check_integrity(check_props);

            res = ctree.search({x2, y2}, search_type::overlap);
            n = std::distance(res.begin(), res.end());
            assert(n == 0);
        }
    }

    assert(tree.empty());
    assert(tree.size() == 0);
}

void rtree_test_forced_reinsertion()
{
    stack_printer __stack_printer__("::rtree_test_forced_reinsertion");

    using rt_type = rtree<int16_t, std::string, tiny_trait_2d_forced_reinsertion>;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = false;

    rt_type tree;

    for (int16_t i = 0; i < 6; ++i)
    {
        int16_t w = 1;
        std::ostringstream os;
        os << "foo" << i;
        tree.insert({{i, i}, {int16_t(i+w), int16_t(i+w)}}, os.str());
    }

    assert(tree.size() == 6);
    tree.check_integrity(check_props);

    tree.clear();
    assert(tree.empty());
    assert(tree.size() == 0);
    tree.check_integrity(check_props);

    for (int16_t x = 0; x < 5; ++x)
    {
        for (int16_t y = 0; y < 5; ++y)
        {
            std::ostringstream os;
            int16_t x2 = x * 2;
            int16_t y2 = y * 2;
            os << "(x=" << x2 << ",y=" << y2 << ")";
            std::string v = os.str();
            int16_t xe = x2 + 2, ye = y2 + 2;
            tree.insert({{x2, y2}, {xe, ye}}, std::move(v));
        }
    }

    tree.check_integrity(check_props);
    assert(tree.size() == 25);

    export_tree(tree, "rtree-test-forced-reinsertion");
}

void rtree_test_move()
{
    stack_printer __stack_printer__("::rtree_test_move");

    using rt_type = rtree<int16_t, std::string, tiny_trait_2d_forced_reinsertion>;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = false;

    rt_type tree;

    for (int16_t x = 0; x < 5; ++x)
    {
        for (int16_t y = 0; y < 5; ++y)
        {
            std::ostringstream os;
            int16_t x2 = x * 2;
            int16_t y2 = y * 2;
            os << "(x=" << x2 << ",y=" << y2 << ")";
            std::string v = os.str();
            int16_t xe = x2 + 2, ye = y2 + 2;
            tree.insert({{x2, y2}, {xe, ye}}, std::move(v));
        }
    }

    tree.check_integrity(check_props);
    assert(tree.size() == 25);

    // moved via constructor.
    rt_type tree_moved(std::move(tree));
    tree.check_integrity(check_props);
    tree_moved.check_integrity(check_props);
    assert(tree.empty());
    assert(tree.size() == 0);
    assert(tree_moved.size() == 25);

    // moved via assignment operator.
    rt_type tree_moved_2;
    tree_moved_2.check_integrity(check_props);
    tree_moved_2 = std::move(tree_moved);
    tree_moved.check_integrity(check_props);
    tree_moved_2.check_integrity(check_props);
    assert(tree_moved.empty());
    assert(tree_moved.size() == 0);
    assert(tree_moved_2.size() == 25);
}

/**
 * Make sure we can 1) store objects that are only movable but not copyable,
 * and 2) the whole tree can still be moved.
 */
void rtree_test_move_custom_type()
{
    stack_printer __stack_printer__("::rtree_test_move_custom_type");

    using rt_type = rtree<double, only_movable, tiny_trait_2d_forced_reinsertion>;
    using point_type = rt_type::point_type;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = false;

    {
        // Make sure you can actually move an object of this type.
        only_movable v1(1.5);
        only_movable v2(std::move(v1));
    }

    struct input
    {
        point_type start;
        point_type end;
        double value;
    };

    std::vector<input> inputs =
    {
        {    {0.0, 0.0},    {1.0, 1.0},   1.0 },
        {    {2.0, 2.0},    {2.1, 2.1},   2.4 },
        { {100.0, 80.0}, {101.0, 85.0}, 100.0 },
        {   {1.0, 75.0},   {2.0, 78.0},  65.0 },
        {   {1.0, 80.0},   {2.0, 82.0},  68.0 },
        {   {1.2,  1.0},    {2.2, 1.5},   2.1 },
        {   {2.2,  2.2},    {2.3, 2.4},   3.5 },
        {   {3.0,  3.0},    {3.3, 3.4},   3.8 },
        {   {4.0,  4.0},   {8.3, 12.4},  13.8 },
        {   {3.0,  5.0},   {4.3, 11.4},  13.9 },
    };

    rt_type tree;

    for (const input& i : inputs)
        tree.insert({i.start, i.end}, only_movable(i.value));

    assert(tree.size() == inputs.size());

    tree.check_integrity(check_props);

    export_tree(tree, "rtree-test-move-custom-type");

    // Now move the tree.
    rt_type tree_moved = std::move(tree);
    tree.check_integrity(check_props);
    tree_moved.check_integrity(check_props);
    assert(tree.empty());
    assert(tree_moved.size() == inputs.size());
}

void rtree_test_copy()
{
    stack_printer __stack_printer__("::rtree_test_copy");

    using rt_type = rtree<double, double, tiny_trait_2d_forced_reinsertion>;
    using point_type = rt_type::point_type;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = false;

    struct input
    {
        point_type start;
        point_type end;
        double value;
    };

    std::vector<input> inputs =
    {
        {    {0.0, 0.0},    {1.0, 1.0},   1.0 },
        {    {2.0, 2.0},    {2.1, 2.1},   2.4 },
        { {100.0, 80.0}, {101.0, 85.0}, 100.0 },
        {   {1.0, 75.0},   {2.0, 78.0},  65.0 },
        {   {1.0, 80.0},   {2.0, 82.0},  68.0 },
        {   {1.2,  1.0},    {2.2, 1.5},   2.1 },
        {   {2.2,  2.2},    {2.3, 2.4},   3.5 },
        {   {3.0,  3.0},    {3.3, 3.4},   3.8 },
        {   {4.0,  4.0},   {8.3, 12.4},  13.8 },
        {   {3.0,  5.0},   {4.3, 11.4},  13.9 },
    };

    rt_type tree;
    for (const input& i : inputs)
        tree.insert({i.start, i.end}, double(i.value));

    auto copied(tree);

    tree.check_integrity(check_props);
    copied.check_integrity(check_props);

    std::string str_src = tree.export_tree(rt_type::export_tree_type::formatted_node_properties);
    std::string str_dst = tree.export_tree(rt_type::export_tree_type::formatted_node_properties);

    assert(!str_src.empty() && str_src == str_dst);

    // Test the "copy via assignment" scenario too.
    auto copied_via_assign = tree;
    copied_via_assign.check_integrity(check_props);
    str_dst = copied_via_assign.export_tree(rt_type::export_tree_type::formatted_node_properties);

    assert(!str_src.empty() && str_src == str_dst);
}

void rtree_test_point_objects()
{
    stack_printer __stack_printer__("::rtree_test_point_objects");

    using rt_type = rtree<double, double, tiny_trait_2d_forced_reinsertion>;
    using key_type = rt_type::key_type;
    using search_type = rt_type::search_type;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = false;

    rt_type tree;
    const rt_type& ctree = tree;

    for (key_type x = 0; x < 10; ++x)
    {
        for (key_type y = 0; y < 10; ++y)
        {
            tree.insert({x, y}, x*y);
        }
    }

    tree.check_integrity(check_props);

    for (key_type x = 0; x < 10; ++x)
    {
        for (key_type y = 0; y < 10; ++y)
        {
            auto results = tree.search({x, y}, search_type::overlap);
            assert(std::distance(results.begin(), results.end()) == 1);
            double expected = x*y;
            auto it = results.begin();
            assert(expected == *it);
        }
    }

    // Test an extent-based search on point data.
    auto results = ctree.search({{0, 0}, {3, 3}}, search_type::overlap);
    size_t n_results = std::distance(results.cbegin(), results.cend());
    assert(n_results == 16);

    export_tree(tree, "rtree-test-point-objects");
}

/**
 * Make sure the rtree works with values that are only copyable (i.e. not
 * movable).
 */
void rtree_test_only_copyable()
{
    stack_printer __stack_printer__("::rtree_test_only_copyable");

    using rt_type = rtree<float, only_copyable, tiny_trait_2d_forced_reinsertion>;
    using search_type = rt_type::search_type;
    using extent_type = rt_type::extent_type;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = false;

    rt_type tree;
    const rt_type& ctree = tree;
    only_copyable v(11.2);
    tree.insert({{0, 0}, {2, 5}}, v);
    v.set(12.5);
    tree.insert({9, 9}, v);

    tree.check_integrity(check_props);

    {
        // Immutable search.
        auto cres = ctree.search({1, 1}, search_type::overlap);
        assert(std::distance(cres.begin(), cres.end()) == 1);
        assert(cres.begin()->get() == 11.2);

        cres = ctree.search({9, 9}, search_type::overlap);
        assert(std::distance(cres.cbegin(), cres.cend()) == 1);
        assert(cres.cbegin()->get() == 12.5);
    }

    {
        // Mutable search
        auto res = tree.search({9, 9}, search_type::match);
        assert(std::distance(res.begin(), res.end()) == 1);
        assert(res.begin()->get() == 12.5);
        auto it = res.begin();
        (*it).set(34.5);

        res = tree.search({9, 9}, search_type::match);
        assert(std::distance(res.begin(), res.end()) == 1);
        assert(res.begin()->get() == 34.5);
    }

    {
        // Erase the only object via mutable iterator.
        assert(tree.size() == 2);
        rt_type::search_results res = tree.search({{0, 0}, {100, 100}}, search_type::overlap);
        assert(std::distance(res.begin(), res.end()) == 2);

        res = tree.search({9, 9}, search_type::match);
        assert(std::distance(res.begin(), res.end()) == 1);
        tree.erase(res.begin());

        assert(tree.size() == 1);
        res = tree.search({{0, 0}, {100, 100}}, search_type::overlap);
        assert(std::distance(res.begin(), res.end()) == 1);
        auto it = res.begin();
        assert(it.extent() == extent_type({{0, 0}, {2, 5}}));
        assert(it->get() == 11.2);
    }
}

void rtree_test_exact_search_by_extent()
{
    stack_printer __stack_printer__("::rtree_test_exact_search_by_extent");

    using rt_type = rtree<double, double, tiny_trait_2d_forced_reinsertion>;
    using extent_type = rt_type::extent_type;
    using search_type = rt_type::search_type;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = false;

    rt_type tree;
    const rt_type& ctree = tree;
    tree.insert({{0, 0}, {2, 2}}, 1.1);
    tree.insert({{1, 1}, {3, 3}}, 1.2);
    tree.insert({{2, 2}, {4, 4}}, 1.3);
    tree.check_integrity(check_props);

    {
        // Immutable search
        rt_type::const_search_results cres = ctree.search({{1, 1}, {3, 3}}, search_type::overlap);
        size_t n = std::distance(cres.begin(), cres.end());
        assert(n == 3);

        cres = ctree.search({{1, 1}, {3, 3}}, search_type::match);
        n = std::distance(cres.begin(), cres.end());
        assert(n == 1);
        auto it = cres.cbegin();
        assert(*it == 1.2);
        assert(it.extent() == extent_type({{1, 1}, {3, 3}}));
        assert(it.depth() == 1);
    }

    {
        // Mutable search
        rt_type::search_results res = tree.search({{1, 1}, {3, 3}}, search_type::overlap);
        size_t n = std::distance(res.begin(), res.end());
        assert(n == 3);

        // Modify the values.
        for (double& v : res)
            v += 1.0;

        res = tree.search({{1, 1}, {3, 3}}, search_type::match);
        n = std::distance(res.begin(), res.end());
        assert(n == 1);
        assert(*res.begin() == 2.2); // The value should be updated.
    }
}

void rtree_test_exact_search_by_point()
{
    stack_printer __stack_printer__("::rtree_test_exact_search_by_point");

    using rt_type = rtree<double, double, tiny_trait_2d_forced_reinsertion>;
    using point_type = rt_type::point_type;
    using extent_type = rt_type::extent_type;
    using search_type = rt_type::search_type;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = false;

    rt_type tree;
    const rt_type& ctree = tree;
    tree.insert({{0, 0}, {4, 4}}, 10.0);
    tree.insert({1, 1}, 11.0);
    tree.insert({3, 3}, 33.0);
    tree.check_integrity(check_props);

    rt_type::const_search_results res = ctree.search({1, 1}, search_type::overlap);
    size_t n = std::distance(res.begin(), res.end());
    assert(n == 2);

    res = ctree.search({3, 3}, search_type::overlap);
    n = std::distance(res.begin(), res.end());
    assert(n == 2);

    res = ctree.search({2, 2}, search_type::overlap);
    n = std::distance(res.begin(), res.end());
    assert(n == 1);
    rt_type::const_iterator it = res.begin();
    assert(*it == 10.0);
    assert(it.extent() == extent_type({{0, 0}, {4, 4}}));

    res = ctree.search({1, 1}, search_type::match);
    n = std::distance(res.begin(), res.end());
    assert(n == 1);
    it = res.begin();
    assert(*it == 11.0);
    assert(it.extent().is_point());
    assert(it.extent().start == point_type({1, 1}));

    res = ctree.search({3, 3}, search_type::match);
    n = std::distance(res.begin(), res.end());
    assert(n == 1);
    it = res.begin();
    assert(*it == 33.0);
    assert(it.extent().is_point());
    assert(it.extent().start == point_type({3, 3}));

    res = ctree.search({{0, 0}, {4, 4}}, search_type::match);
    n = std::distance(res.begin(), res.end());
    assert(n == 1);
    it = res.begin();
    assert(*it == 10.0);
    assert(it.extent() == extent_type({{0, 0}, {4, 4}}));
}

int main(int argc, char** argv)
{
    try
    {
        rtree_test_intersection();
        rtree_test_square_distance();
        rtree_test_center_point();
        rtree_test_area_enlargement();
        rtree_test_basic_search();
        rtree_test_basic_erase();
        rtree_test_node_split();
        rtree_test_directory_node_split();
        rtree_test_erase_directories();
        rtree_test_forced_reinsertion();
        rtree_test_move();
        rtree_test_move_custom_type();
        rtree_test_copy();
        rtree_test_point_objects();
        rtree_test_only_copyable();
        rtree_test_exact_search_by_extent();
        rtree_test_exact_search_by_point();
    }
    catch (const std::exception& e)
    {
        cout << "Test failed: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    cout << "Test finished successfully!" << endl;

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

