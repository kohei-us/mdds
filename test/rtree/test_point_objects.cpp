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

void rtree_test_point_objects()
{
    stack_printer __stack_printer__("::rtree_test_point_objects");

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
            assert(std::distance(results.begin(), results.end()) == 1);
            double expected = x * y;
            auto it = results.begin();
            assert(expected == *it);
        }
    }

    // Test an extent-based search on point data.
    auto results = ctree.search({{0, 0}, {3, 3}}, search_type::overlap);
    size_t n_results = std::distance(results.cbegin(), results.cend());
    assert(n_results == 16);

    export_tree(tree, "rtree-test-point-objects");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
