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

void rtree_test_exact_search_by_point()
{
    stack_printer __stack_printer__("::rtree_test_exact_search_by_point");

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
    assert(n == 2);

    res = ctree.search({3, 3}, search_type::overlap);
    n = std::distance(res.begin(), res.end());
    assert(n == 2);

    res = ctree.search({2, 2}, search_type::overlap);
    n = std::distance(res.begin(), res.end());
    assert(n == 1);
    rt_type::const_iterator it = res.begin();
    assert(*it == 10.0);
    assert(it.extent() == extent_type({{0, 0}, {4, 4}}));

    res = ctree.search({1, 1}, search_type::match);
    n = std::distance(res.begin(), res.end());
    assert(n == 1);
    it = res.begin();
    assert(*it == 11.0);
    assert(it.extent().is_point());
    assert(it.extent().start == point_type({1, 1}));

    res = ctree.search({3, 3}, search_type::match);
    n = std::distance(res.begin(), res.end());
    assert(n == 1);
    it = res.begin();
    assert(*it == 33.0);
    assert(it.extent().is_point());
    assert(it.extent().start == point_type({3, 3}));

    res = ctree.search({{0, 0}, {4, 4}}, search_type::match);
    n = std::distance(res.begin(), res.end());
    assert(n == 1);
    it = res.begin();
    assert(*it == 10.0);
    assert(it.extent() == extent_type({{0, 0}, {4, 4}}));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
