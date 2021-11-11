/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2020 Kohei Yoshida
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

#include <mdds/rtree.hpp>

#include <iostream>
#include <fstream>

// Make the node capacity intentionally small.
struct tiny_trait_2d
{
    constexpr static size_t dimensions = 2;
    constexpr static size_t min_node_size = 2;
    constexpr static size_t max_node_size = 5;
    constexpr static size_t max_tree_depth = 100;

    constexpr static bool enable_forced_reinsertion = true;
    constexpr static size_t reinsertion_size = 2;
};

using rt_type = mdds::rtree<int, int, tiny_trait_2d>;

// 2D rectangle with the top-left position (x, y), width and height.
struct rect
{
    int x;
    int y;
    int w;
    int h;
};

std::vector<rect> rects =
{
    {  3538,  9126, 1908,  1908 },
    { 34272, 52053, 2416,  2543 },
    { 32113,  9761, 2416,   638 },
    { 16493, 16747, 7369,  2289 },
    { 29192, 23732, 3432,  2035 },
    { 35797, 17000, 1781,   892 },
    { 15857, 29319, 2162,  1654 },
    {  5825, 24239, 3559,  8512 },
    {  9127, 46846, 2543,  1019 },
    {  7094, 54338, 5210,   892 },
    { 18779, 39734, 3813, 10417 },
    { 32749, 35923, 2289,  2924 },
    { 26018, 31098,  257,  2797 },
    {  6713, 37066, 2924,  1146 },
    { 19541,  3157, 3305,  1146 },
    { 21953, 10904, 4448,   892 },
    { 15984, 24240, 5210,  1273 },
    {  8237, 15350, 2670,  2797 },
    { 17001, 13826, 4067,  1273 },
    { 30970, 13826, 3940,   765 },
    {  9634,  6587, 1654,  1781 },
    { 38464, 47099,  511,  1400 },
    { 20556, 54085, 1400,  1527 },
    { 37575, 24113, 1019,   765 },
    { 20429, 21064, 1146,  1400 },
    { 31733,  4427, 2543,   638 },
    {  2142, 27161, 1273,  7369 },
    {  3920, 43289, 8131,  1146 },
    { 14714, 34272, 1400,  4956 },
    { 38464, 41258, 1273,  1273 },
    { 35542, 45703,  892,  1273 },
    { 25891, 50783, 1273,  5083 },
    { 35415, 28431, 2924,  1781 },
    { 15476,  7349, 1908,   765 },
    { 12555, 11159, 1654,  2035 },
    { 11158, 21445, 1908,  2416 },
    { 23350, 28049, 3432,   892 },
    { 28684, 15985, 2416,  4321 },
    { 24620, 21953, 1654,   638 },
    { 30208, 30716, 2670,  2162 },
    { 26907, 44179, 2797,  4067 },
    { 21191, 35416, 2162,  1019 },
    { 27668, 38717,  638,  3178 },
    {  3666, 50528, 2035,  1400 },
    { 15349, 48750, 2670,  1654 },
    { 28430,  7221, 2162,   892 },
    {  4808,  3158, 2416,  1273 },
    { 38464,  3666, 1527,  1781 },
    {  2777, 20937, 2289,  1146 },
    { 38209,  9254, 1908,  1781 },
    {  2269, 56497, 2289,   892 },
};

void load_tree()
{
    rt_type tree;

    // Insert the rectangle objects into the tree.
    int value = 0;
    for (const auto& rect : rects)
        tree.insert({{rect.x, rect.y}, {rect.x + rect.w, rect.y + rect.h}}, value++);

    // Export the tree structure as a SVG for visualization.
    std::string tree_svg = tree.export_tree(rt_type::export_tree_type::extent_as_svg);
    std::ofstream fout("bounds2.svg");
    fout << tree_svg;
}

void bulkload_tree()
{
    rt_type::bulk_loader loader;

    // Insert the rectangle objects into the tree.
    int value = 0;
    for (const auto& rect : rects)
        loader.insert({{rect.x, rect.y}, {rect.x + rect.w, rect.y + rect.h}}, value++);

    // Start bulk-loading the tree.
    rt_type tree = loader.pack();

    // Export the tree structure as a SVG for visualization.
    std::string tree_svg = tree.export_tree(rt_type::export_tree_type::extent_as_svg);
    std::ofstream fout("bounds2-bulkload.svg");
    fout << tree_svg;
}

int main() try
{
    load_tree();
    bulkload_tree();

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
