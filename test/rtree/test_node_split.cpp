/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2021 Kohei Yoshida
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

using mdds::rtree;
using std::cout;
using std::endl;

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
        tree.insert({{i, i}, {int16_t(i + w), int16_t(i + w)}}, os.str());
    }

    assert(tree.size() == 6);

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
        tree.insert({{i, i}, {int16_t(i + w), int16_t(i + w)}}, os.str());
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
