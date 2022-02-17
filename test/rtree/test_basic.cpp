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

    std::unordered_map<std::string, rt_type::extent_type> expected_values = {
        {"test", {{0, 0}, {2, 2}}},
        {"more test", {{-2, 1}, {3, 6}}},
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
    std::vector<rt_type::point_type> pts = {
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
    tree.insert({{-2, -2}, {2, 2}}, "erase me");
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

    tree.insert({{0, 0}, {2, 2}}, "erase me");
    tree.insert({{-10, -4}, {0, 0}}, "erase me");
    rt_type::extent_type expected_bb({-10, -4}, {2, 2});
    assert(tree.extent() == expected_bb);
    assert(tree.size() == 2);

    res = ctree.search({-5, -2}, search_type::overlap);
    n = std::distance(res.begin(), res.end());
    assert(n == 1);
    it = res.begin();
    tree.erase(it);
    assert(!tree.empty()); // there should be one value stored in the tree.
    assert(tree.size() == 1);
    expected_bb = {{0, 0}, {2, 2}};
    assert(tree.extent() == expected_bb);

    tree.check_integrity(check_props);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
