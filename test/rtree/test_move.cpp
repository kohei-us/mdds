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

void rtree_test_move()
{
    stack_printer __stack_printer__("::rtree_test_move");

    using rt_type = rtree<int16_t, std::string, tiny_trait_2d_forced_reinsertion>;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = false;

    rt_type tree;

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

    // moved via constructor.
    rt_type tree_moved(std::move(tree));
    tree.check_integrity(check_props);
    tree_moved.check_integrity(check_props);
    assert(tree.empty());
    assert(tree.size() == 0);
    assert(tree_moved.size() == 25);

    // moved via assignment operator.
    rt_type tree_moved_2;
    tree_moved_2.check_integrity(check_props);
    tree_moved_2 = std::move(tree_moved);
    tree_moved.check_integrity(check_props);
    tree_moved_2.check_integrity(check_props);
    assert(tree_moved.empty());
    assert(tree_moved.size() == 0);
    assert(tree_moved_2.size() == 25);
}

/**
 * Make sure we can 1) store objects that are only movable but not copyable,
 * and 2) the whole tree can still be moved.
 */
void rtree_test_move_custom_type()
{
    stack_printer __stack_printer__("::rtree_test_move_custom_type");

    using rt_type = rtree<double, only_movable, tiny_trait_2d_forced_reinsertion>;
    using point_type = rt_type::point_type;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = false;

    {
        // Make sure you can actually move an object of this type.
        only_movable v1(1.5);
        only_movable v2(std::move(v1));
    }

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
        tree.insert({i.start, i.end}, only_movable(i.value));

    assert(tree.size() == inputs.size());

    tree.check_integrity(check_props);

    export_tree(tree, "rtree-test-move-custom-type");

    // Now move the tree.
    rt_type tree_moved = std::move(tree);
    tree.check_integrity(check_props);
    tree_moved.check_integrity(check_props);
    assert(tree.empty());
    assert(tree_moved.size() == inputs.size());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
