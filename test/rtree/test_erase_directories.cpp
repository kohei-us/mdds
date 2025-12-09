/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.
#include "test_global_rtree.hpp"

using mdds::rtree;
using std::cout;
using std::endl;

void rtree_test_erase_directories()
{
    MDDS_TEST_FUNC_SCOPE;

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

    TEST_ASSERT(tree.size() == 25);
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
            TEST_ASSERT(n == 1);

            tree.erase(it);

            TEST_ASSERT(tree.size() == --expected_size);
            tree.check_integrity(check_props);

            res = ctree.search({x2, y2}, search_type::overlap);
            n = std::distance(res.begin(), res.end());
            TEST_ASSERT(n == 0);
        }
    }

    TEST_ASSERT(tree.empty());
    TEST_ASSERT(tree.size() == 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
