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

    std::vector<input> inputs = {
        {{0.0, 0.0}, {1.0, 1.0}, 1.0},    {{2.0, 2.0}, {2.1, 2.1}, 2.4},    {{100.0, 80.0}, {101.0, 85.0}, 100.0},
        {{1.0, 75.0}, {2.0, 78.0}, 65.0}, {{1.0, 80.0}, {2.0, 82.0}, 68.0}, {{1.2, 1.0}, {2.2, 1.5}, 2.1},
        {{2.2, 2.2}, {2.3, 2.4}, 3.5},    {{3.0, 3.0}, {3.3, 3.4}, 3.8},    {{4.0, 4.0}, {8.3, 12.4}, 13.8},
        {{3.0, 5.0}, {4.3, 11.4}, 13.9},
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
