/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.
#include "test_global_rtree.hpp"

using mdds::rtree;
using std::cout;
using std::endl;

void rtree_test_point_objects()
{
    MDDS_TEST_FUNC_SCOPE;

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
            tree.insert({x, y}, x * y);
        }
    }

    tree.check_integrity(check_props);

    for (key_type x = 0; x < 10; ++x)
    {
        for (key_type y = 0; y < 10; ++y)
        {
            auto results = tree.search({x, y}, search_type::overlap);
            TEST_ASSERT(std::distance(results.begin(), results.end()) == 1);
            double expected = x * y;
            auto it = results.begin();
            TEST_ASSERT(expected == *it);
        }
    }

    // Test an extent-based search on point data.
    auto results = ctree.search({{0, 0}, {3, 3}}, search_type::overlap);
    size_t n_results = std::distance(results.cbegin(), results.cend());
    TEST_ASSERT(n_results == 16);

    export_tree(tree, "rtree-test-point-objects");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
