/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.
#include "test_global_rtree.hpp"

using mdds::rtree;
using std::cout;
using std::endl;

void rtree_test_exact_search_by_point()
{
    MDDS_TEST_FUNC_SCOPE;

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
    TEST_ASSERT(n == 2);

    res = ctree.search({3, 3}, search_type::overlap);
    n = std::distance(res.begin(), res.end());
    TEST_ASSERT(n == 2);

    res = ctree.search({2, 2}, search_type::overlap);
    n = std::distance(res.begin(), res.end());
    TEST_ASSERT(n == 1);
    rt_type::const_iterator it = res.begin();
    TEST_ASSERT(*it == 10.0);
    TEST_ASSERT(it.extent() == extent_type({{0, 0}, {4, 4}}));

    res = ctree.search({1, 1}, search_type::match);
    n = std::distance(res.begin(), res.end());
    TEST_ASSERT(n == 1);
    it = res.begin();
    TEST_ASSERT(*it == 11.0);
    TEST_ASSERT(it.extent().is_point());
    TEST_ASSERT(it.extent().start == point_type({1, 1}));

    res = ctree.search({3, 3}, search_type::match);
    n = std::distance(res.begin(), res.end());
    TEST_ASSERT(n == 1);
    it = res.begin();
    TEST_ASSERT(*it == 33.0);
    TEST_ASSERT(it.extent().is_point());
    TEST_ASSERT(it.extent().start == point_type({3, 3}));

    res = ctree.search({{0, 0}, {4, 4}}, search_type::match);
    n = std::distance(res.begin(), res.end());
    TEST_ASSERT(n == 1);
    it = res.begin();
    TEST_ASSERT(*it == 10.0);
    TEST_ASSERT(it.extent() == extent_type({{0, 0}, {4, 4}}));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
