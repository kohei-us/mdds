/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.
#include "test_global_rtree.hpp"

using mdds::rtree;
using std::cout;
using std::endl;

void rtree_test_forced_reinsertion()
{
    MDDS_TEST_FUNC_SCOPE;

    using rt_type = rtree<int16_t, std::string, tiny_trait_2d_forced_reinsertion>;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = false;

    rt_type tree;

    for (int16_t i = 0; i < 6; ++i)
    {
        int16_t w = 1;
        std::ostringstream os;
        os << "foo" << i;
        tree.insert({{i, i}, {int16_t(i + w), int16_t(i + w)}}, os.str());
    }

    TEST_ASSERT(tree.size() == 6);
    tree.check_integrity(check_props);

    tree.clear();
    TEST_ASSERT(tree.empty());
    TEST_ASSERT(tree.size() == 0);
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
    TEST_ASSERT(tree.size() == 25);

    export_tree(tree, "rtree-test-forced-reinsertion");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
