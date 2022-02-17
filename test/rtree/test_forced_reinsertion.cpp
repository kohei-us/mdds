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

void rtree_test_forced_reinsertion()
{
    stack_printer __stack_printer__("::rtree_test_forced_reinsertion");

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

    assert(tree.size() == 6);
    tree.check_integrity(check_props);

    tree.clear();
    assert(tree.empty());
    assert(tree.size() == 0);
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
    assert(tree.size() == 25);

    export_tree(tree, "rtree-test-forced-reinsertion");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
