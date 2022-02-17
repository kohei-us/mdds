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

void rtree_test_erase_directories()
{
    stack_printer __stack_printer__("::rtree_test_erase_directories");
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

    assert(tree.size() == 25);
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
            assert(n == 1);

            tree.erase(it);

            assert(tree.size() == --expected_size);
            tree.check_integrity(check_props);

            res = ctree.search({x2, y2}, search_type::overlap);
            n = std::distance(res.begin(), res.end());
            assert(n == 0);
        }
    }

    assert(tree.empty());
    assert(tree.size() == 0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
