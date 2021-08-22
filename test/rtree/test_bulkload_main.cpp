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

#include "test_global.hpp" // This must be the first header to be included.
#include "test_global_rtree.hpp"

#include <vector>
#include <fstream>
#include <unordered_map>

using namespace mdds;
using namespace std;

void rtree_test_bl_empty()
{
    stack_printer __stack_printer__("::rtree_test_bl_empty");
    using rt_type = rtree<int16_t, std::string>;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = false;

    // Load nothing.
    rt_type::bulk_loader loader;
    rt_type tree = loader.pack();
    assert(tree.empty());
    tree.check_integrity(check_props);
}

void rtree_test_bl_insert_points_move()
{
    stack_printer __stack_printer__("::rtree_test_bl_insert_points_move");
    using rt_type = rtree<int16_t, std::string, tiny_trait_2d_forced_reinsertion>;
    using key_type = rt_type::key_type;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = true;

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
    tree.check_integrity(check_props);
    export_tree(tree, "rtree-test-bl-insert-points-move");
}

void rtree_test_bl_insert_points_copy()
{
    stack_printer __stack_printer__("::rtree_test_bl_insert_points_copy");
    using rt_type = rtree<int16_t, std::string, tiny_trait_2d_forced_reinsertion>;
    using point_type = rt_type::point_type;
    using search_type = rt_type::search_type;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = true;

    struct kv
    {
        point_type point;
        std::string value;
    };

    std::vector<kv> values =
    {
        { {  0,    0}, "origin"      },
        { {125,  125}, "middle"      },
        { { 22,  987}, "somewhere"   },
        { {-34, -200}, "negative"    },
        { {  2,    3}, "near origin" },
    };

    // Insert less than max node size in order to test the packing
    // implementation that doesn't involve per-level packing.
    tiny_trait_2d_forced_reinsertion t;
    assert(values.size() <= t.max_node_size);

    for (size_t n_values = 1; n_values <= values.size(); ++n_values)
    {
        auto loader = rt_type::bulk_loader();

        // Insert specified number of value(s).
        for (size_t i = 0; i < n_values; ++i)
            loader.insert(values[i].point, values[i].value);

        // Populate and pack the tree.
        auto tree = loader.pack();
        tree.check_integrity(check_props);
        assert(tree.size() == n_values);

        // Make sure the inserted values are all there.
        for (size_t i = 0; i < n_values; ++i)
        {
            auto res = tree.search(values[i].point, search_type::match);
            assert(std::distance(res.begin(), res.end()) == 1);
            auto it = res.begin();
            assert(*it == values[i].value);

            // The values should all be the immediate children of the root
            // directory node.
            assert(it.depth() == 1);
        }
    }
}

void rtree_test_bl_insert_extents_move()
{
    stack_printer __stack_printer__("::rtree_test_bl_insert_extents_move");
    using rt_type = rtree<int16_t, only_movable, tiny_trait_2d_forced_reinsertion>;
    using extent_type = rt_type::extent_type;
    using search_type = rt_type::search_type;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = true;

    struct kv
    {
        int16_t x;
        int16_t y;
        int16_t w;
        int16_t h;
        double value;
    };

    std::vector<kv> values =
    {
        {  2142,  2777, 1781, 1273, 1.0 },
        {  5063,  2396,  765, 1019, 1.1 },
        {  1887,  4935, 1400,  892, 1.2 },
        {  4428,  4046, 1527, 1146, 1.3 },
        {  2268,  6713, 1146, 2162, 1.4 },
        {  7729,  7094, 2924, 1908, 1.5 },
        { 10396,  2014, 6480,  892, 1.6 },
        {  3158, 12302, 2035, 2289, 1.7 },
        { 10777, 11032, 3432, 2289, 1.8 },
        { 14334,  5063, 1781, 2797, 1.9 },
        {  8365,  4301, 3051, 1019, 2.0 },
        { 16619, 11794,  638,  511, 2.1 },
        { 15222,  9254,  892,  511, 2.2 },
        {  8111, 10142,  765,  638, 2.3 },
        {  6587, 10397,  765,  511, 2.4 },
        {  7475, 11793,  638, 1146, 2.5 },
        {  8746, 11285,  765,  892, 2.6 },
        { 16620,  3665,  511, 2162, 2.7 },
        {  6332, 14714, 2162, 1908, 2.8 },
        {  1634, 12048,  765, 5083, 2.9 },
        { 15349, 16238, 1400, 1400, 3.0 },
        { 11666, 14587, 1400, 1146, 3.1 },
    };

    for (size_t n_values = 5; n_values <= values.size(); ++n_values)
    {
        rt_type::bulk_loader loader = rt_type::bulk_loader();

        for (size_t i = 0; i < n_values; ++i)
        {
            const auto& v = values[i];
            extent_type extent{{v.x, v.y}, {int16_t(v.x+v.w), int16_t(v.y+v.h)}};
            only_movable vv(v.value);

            loader.insert(extent, std::move(vv));
        }

        auto tree = loader.pack();
        assert(tree.size() == n_values);
        tree.check_integrity(check_props);

        // Make sure the values are all there.
        for (size_t i = 0; i < n_values; ++i)
        {
            const auto& v = values[i];
            extent_type extent{{v.x, v.y}, {int16_t(v.x+v.w), int16_t(v.y+v.h)}};
            auto res = tree.search(extent, search_type::match);
            assert(std::distance(res.begin(), res.end()) == 1);
            assert(res.begin()->get() == v.value);
        }

        if (n_values == values.size())
            export_tree(tree, "rtree-test-bl-insert-extents-move");
    }
}

void rtree_test_bl_insert_extents_copy()
{
    stack_printer __stack_printer__("::rtree_test_bl_insert_extents_copy");
    using rt_type = rtree<int16_t, only_copyable, tiny_trait_2d_forced_reinsertion>;
    using extent_type = rt_type::extent_type;
    using search_type = rt_type::search_type;
    rt_type::integrity_check_properties check_props;
    check_props.throw_on_first_error = true;

    struct kv
    {
        int16_t x;
        int16_t y;
        int16_t w;
        int16_t h;
        double value;
    };

    std::vector<kv> values =
    {
        {  2142,  2777, 1781, 1273, 1.0 },
        {  5063,  2396,  765, 1019, 1.1 },
        {  1887,  4935, 1400,  892, 1.2 },
        {  4428,  4046, 1527, 1146, 1.3 },
        {  2268,  6713, 1146, 2162, 1.4 },
        {  7729,  7094, 2924, 1908, 1.5 },
        { 10396,  2014, 6480,  892, 1.6 },
        {  2904, 11286, 2035, 2289, 1.7 },
        { 11412, 10524, 3432, 2289, 1.8 },
        { 14334,  5063, 1781, 2797, 1.9 },
        {  8365,  4301, 3051, 1019, 2.0 },
        { 16619, 11794,  638,  511, 2.1 },
        { 15222,  9254,  892,  511, 2.2 },
        {  8111, 10142,  765,  638, 2.3 },
        {  6587, 10397,  765,  511, 2.4 },
        {  7475, 11793,  638, 1146, 2.5 },
        {  8746, 11285,  765,  892, 2.6 },
        { 16620,  3665,  511, 2162, 2.7 },
        {  1760, 17762, 2162, 1908, 2.8 },
        {  1634, 12048,  765, 5083, 2.9 },
        { 15349, 16238, 1400, 1400, 3.0 },
        { 11793, 13190, 1400, 1146, 3.1 },
        {  4174,  6078, 1019,  638, 3.2 },
        { 13191,  3412, 1781,  257, 3.3 },
        {  6714, 11413,  511,  765, 3.4 },
        {  2903, 14969,  892,  511, 3.5 },
        {  3919, 16366,  638,  511, 3.6 },
        {  4554, 15222,  892,  638, 3.7 },
        {  2904, 16238,  511,  892, 3.8 },
        {  1507, 20557, 6099,  257, 3.9 },
        {  4047,  7221, 1273,  384, 4.0 },
        { 12301,  3538,  638, 1908, 4.1 },
        { 12174,  6460, 1146,  511, 4.2 },
        { 13318,  4046, 1273,  384, 4.3 },
        { 16620,  6459, 1019,  638, 4.4 },
        { 14080,  8238,  511,  765, 4.5 },
        {  8365, 12555,  765,  384, 4.6 },
        { 16493,  7349,  511,  511, 4.7 },
        {  7603, 11031,  511,  384, 4.8 },
        { 13571, 13191,  638, 1527, 4.9 },
        { 15858, 13826, 2035, 2035, 5.0 },
        {  7094,  1380,  892, 3305, 5.1 },
        {  7094,  6332, 1400, 1400, 5.2 },
        { 10142,  8237, 2162,  384, 5.3 },
        { 13444,  9761,    4,  130, 5.4 },
    };

    for (size_t n_values = 22; n_values <= values.size(); ++n_values)
    {
        rt_type::bulk_loader loader = rt_type::bulk_loader();

        for (size_t i = 0; i < n_values; ++i)
        {
            const auto& v = values[i];
            extent_type extent{{v.x, v.y}, {int16_t(v.x+v.w), int16_t(v.y+v.h)}};
            only_copyable vv(v.value);

            loader.insert(extent, vv);
        }

        auto tree = loader.pack();
        assert(tree.size() == n_values);
        tree.check_integrity(check_props);

        // Make sure the values are all there.
        for (size_t i = 0; i < n_values; ++i)
        {
            const auto& v = values[i];
            extent_type extent{{v.x, v.y}, {int16_t(v.x+v.w), int16_t(v.y+v.h)}};
            auto res = tree.search(extent, search_type::match);
            assert(std::distance(res.begin(), res.end()) == 1);
            assert(res.begin()->get() == v.value);
        }

        if (n_values == values.size())
            export_tree(tree, "rtree-test-bl-insert-extents-copy");
    }
}

int main(int argc, char** argv)
{
    try
    {
        rtree_test_bl_empty();
        rtree_test_bl_insert_points_move();
        rtree_test_bl_insert_points_copy();
        rtree_test_bl_insert_extents_move();
        rtree_test_bl_insert_extents_copy();
    }
    catch (const std::exception& e)
    {
        cout << "Test failed: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

