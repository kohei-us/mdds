/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.
#include "test_global_rtree.hpp"

using mdds::rtree;
using std::cout;
using std::endl;

void rtree_test_exact_search_by_extent()
{
    MDDS_TEST_FUNC_SCOPE;

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
        TEST_ASSERT(n == 3);

        cres = ctree.search({{1, 1}, {3, 3}}, search_type::match);
        n = std::distance(cres.begin(), cres.end());
        TEST_ASSERT(n == 1);
        auto it = cres.cbegin();
        TEST_ASSERT(*it == 1.2);
        TEST_ASSERT(it.extent() == extent_type({{1, 1}, {3, 3}}));
        TEST_ASSERT(it.depth() == 1);
    }

    {
        // Mutable search
        rt_type::search_results res = tree.search({{1, 1}, {3, 3}}, search_type::overlap);
        size_t n = std::distance(res.begin(), res.end());
        TEST_ASSERT(n == 3);

        // Modify the values.
        for (double& v : res)
            v += 1.0;

        res = tree.search({{1, 1}, {3, 3}}, search_type::match);
        n = std::distance(res.begin(), res.end());
        TEST_ASSERT(n == 1);
        TEST_ASSERT(*res.begin() == 2.2); // The value should be updated.
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
