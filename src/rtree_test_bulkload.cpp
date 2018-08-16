/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2018 Kohei Yoshida
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

#include "test_global.hpp"
#include "test_global_rtree.hpp"

#include <vector>
#include <fstream>
#include <unordered_map>

using namespace mdds::draft;
using namespace std;

void rtree_test_bl_empty()
{
    stack_printer __stack_printer__("::rtree_test_bl_empty");
    using rt_type = rtree<int16_t, std::string>;
    using integrity_check_type = rt_type::integrity_check_type;

    // Load nothing.
    rt_type::bulk_loader loader;
    rt_type tree = loader.pack();
    assert(tree.empty());
    tree.check_integrity(integrity_check_type::whole_tree);
}

void rtree_test_bl_insert_points()
{
    stack_printer __stack_printer__("::rtree_test_bl_insert_points");
    using rt_type = rtree<int16_t, std::string, tiny_trait_2d_forced_reinsertion>;
    using integrity_check_type = rt_type::integrity_check_type;
    using key_type = rt_type::key_type;

    rt_type::bulk_loader loader;
    for (key_type x = 0; x < 20; ++x)
    {
        key_type yn = (x == 0) ? 19 : 20;
        for (key_type y = 0; y < yn; ++y)
        {
            std::ostringstream os;
            os << '(' << x << ',' << y << ')';
            loader.insert({x, y}, os.str());
        }
    }

    auto tree = loader.pack();
    assert(tree.size() == 399);
    tree.check_integrity(integrity_check_type::whole_tree);
}

int main(int argc, char** argv)
{
    rtree_test_bl_empty();
    rtree_test_bl_insert_points();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

