/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.
#include "test_global_rtree.hpp"

using mdds::rtree;
using std::cout;
using std::endl;

void rtree_test_node_split()
{
    MDDS_TEST_FUNC_SCOPE;

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
        tree.insert({{i, i}, {int16_t(i + w), int16_t(i + w)}}, os.str());
    }

    TEST_ASSERT(tree.size() == 6);

    cout << tree.export_tree(rt_type::export_tree_type::formatted_node_properties) << endl;

    size_t count_values = 0;
    size_t count_leaf = 0;
    size_t count_nonleaf = 0;

    auto walker = [&](const rt_type::node_properties& np) {
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
            default:;
        }
    };

    tree.walk(walker);

    TEST_ASSERT(count_values == 6);
    TEST_ASSERT(count_leaf == 2);
    TEST_ASSERT(count_nonleaf == 1);

    tree.check_integrity(check_props);

    // Adding two more entries will cause one of the leaf directory nodes
    // below the root node to split.

    for (int16_t i = 6; i < 8; ++i)
    {
        int16_t w = 1;
        std::ostringstream os;
        os << "bar" << i;
        tree.insert({{i, i}, {int16_t(i + w), int16_t(i + w)}}, os.str());
    }

    TEST_ASSERT(tree.size() == 8);
    tree.check_integrity(check_props);

    // Count all the nodes again.
    count_values = 0;
    count_leaf = 0;
    count_nonleaf = 0;

    tree.walk(walker);

    TEST_ASSERT(count_values == 8);
    TEST_ASSERT(count_leaf == 3);
    TEST_ASSERT(count_nonleaf == 1);

    // Erase the entry at (0, 0).  There should be only one match.  Erasing
    // this entry will cause the node to be underfilled.

    rt_type::const_search_results res = ctree.search({0, 0}, search_type::overlap);
    auto it = res.cbegin();
    TEST_ASSERT(it != res.cend());
    TEST_ASSERT(std::distance(it, res.cend()) == 1);
    tree.erase(it);

    TEST_ASSERT(tree.size() == 7);
    tree.check_integrity(check_props);

    // Count all the nodes again.
    count_values = 0;
    count_leaf = 0;
    count_nonleaf = 0;

    tree.walk(walker);

    TEST_ASSERT(count_values == 7);
    TEST_ASSERT(count_leaf == 2);
    TEST_ASSERT(count_nonleaf == 1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
