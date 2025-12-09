/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.
#include "test_global_rtree.hpp"

using mdds::rtree;
using std::cout;
using std::endl;

void rtree_test_basic_search()
{
    MDDS_TEST_FUNC_SCOPE;

    using rt_type = rtree<int16_t, std::string>;
    using search_type = rt_type::search_type;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = true;

    rt_type tree;
    const rt_type& ctree = tree;
    rt_type::extent_type expected_bb;

    tree.insert({{0, 0}, {2, 2}}, "test");
    expected_bb = {{0, 0}, {2, 2}};
    TEST_ASSERT(tree.extent() == expected_bb);
    TEST_ASSERT(tree.size() == 1);

    tree.insert({{3, 3}, {5, 5}}, "test again");
    expected_bb = {{0, 0}, {5, 5}};
    TEST_ASSERT(tree.extent() == expected_bb);
    TEST_ASSERT(tree.size() == 2);

    tree.insert({{-2, 1}, {3, 6}}, "more test");
    expected_bb = {{-2, 0}, {5, 6}};
    TEST_ASSERT(tree.extent() == expected_bb);
    TEST_ASSERT(tree.size() == 3);

    tree.check_integrity(check_props);

    // Verify the search method works.

    rt_type::const_search_results res = ctree.search({1, 1}, search_type::overlap);

    auto it = res.cbegin(), it_end = res.cend();

    size_t n = std::distance(it, it_end);
    TEST_ASSERT(n == 2);

    std::unordered_map<std::string, rt_type::extent_type> expected_values = {
        {"test", {{0, 0}, {2, 2}}},
        {"more test", {{-2, 1}, {3, 6}}},
    };

    for (; it != it_end; ++it)
    {
        cout << "bounding box: " << it.extent().to_string() << "; value: " << *it << "; depth: " << it.depth() << endl;
        auto itv = expected_values.find(*it);
        TEST_ASSERT(itv != expected_values.end());
        TEST_ASSERT(itv->second == it.extent());
        TEST_ASSERT(it.depth() == 1);
    }

    // Perform an out-of-bound search by point.
    std::vector<rt_type::point_type> pts = {
        {-10, -10},
        {1, 7},
        {6, 3},
    };

    for (const rt_type::point_type& pt : pts)
    {
        res = ctree.search(pt, search_type::overlap);
        TEST_ASSERT(res.cbegin() == res.cend());
    }
}

void rtree_test_basic_erase()
{
    MDDS_TEST_FUNC_SCOPE;

    using rt_type = rtree<int16_t, std::string>;
    using search_type = rt_type::search_type;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = true;

    rt_type tree;
    const rt_type& ctree = tree;
    tree.insert({{-2, -2}, {2, 2}}, "erase me");
    TEST_ASSERT(!tree.empty());
    TEST_ASSERT(tree.size() == 1);

    rt_type::const_search_results res = ctree.search({0, 0}, search_type::overlap);

    size_t n = std::distance(res.begin(), res.end());
    TEST_ASSERT(n == 1);

    rt_type::const_iterator it = res.begin();
    TEST_ASSERT(it != res.end());

    tree.erase(it);
    TEST_ASSERT(tree.empty());
    TEST_ASSERT(tree.size() == 0);
    TEST_ASSERT(rt_type::extent_type() == tree.extent());

    tree.insert({{0, 0}, {2, 2}}, "erase me");
    tree.insert({{-10, -4}, {0, 0}}, "erase me");
    rt_type::extent_type expected_bb({-10, -4}, {2, 2});
    TEST_ASSERT(tree.extent() == expected_bb);
    TEST_ASSERT(tree.size() == 2);

    res = ctree.search({-5, -2}, search_type::overlap);
    n = std::distance(res.begin(), res.end());
    TEST_ASSERT(n == 1);
    it = res.begin();
    tree.erase(it);
    TEST_ASSERT(!tree.empty()); // there should be one value stored in the tree.
    TEST_ASSERT(tree.size() == 1);
    expected_bb = {{0, 0}, {2, 2}};
    TEST_ASSERT(tree.extent() == expected_bb);

    tree.check_integrity(check_props);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
