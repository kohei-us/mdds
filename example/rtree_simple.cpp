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

#include <string>
#include <iostream>

int main() try
{
    // key values are of type double, and we are storing std::string as a
    // value for each spatial object.  By default, tree becomes 2-dimensional
    // object store unless otherwise specified.
    using rt_type = mdds::rtree<double, std::string>;

    rt_type tree;

    tree.insert({{0.0, 0.0}, {15.0, 20.0}}, "first rectangle data");

    rt_type::extent_type bounds({-2.0, -1.0}, {1.0, 2.0});
    std::cout << "inserting value for " << bounds.to_string() << std::endl;
    tree.insert(bounds, "second rectangle data");

    bounds.start.d[0] = -1.0; // Change the first dimension value of the start rectangle point.
    bounds.end.d[1] += 1.0; // Increment the second dimension value of the end rectangle point.
    std::cout << "inserting value for " << bounds.to_string() << std::endl;
    tree.insert(bounds, "third rectangle data");

    tree.insert({5.0, 6.0}, "first point data");

    {
        // Search for all objects that overlap with a (4, 4) - (7, 7) rectangle.
        auto results = tree.search({{4.0, 4.0}, {7.0, 7.0}}, rt_type::search_type::overlap);

        for (const std::string& v : results)
            std::cout << "value: " << v << std::endl;
    }

    {
        // Search for all objects whose bounding rectangles are exactly (4, 4) - (7, 7).
        auto results = tree.search({{4.0, 4.0}, {7.0, 7.0}}, rt_type::search_type::match);
        std::cout << "number of results: " << std::distance(results.begin(), results.end()) << std::endl;
    }

    {
        // Search for all objects whose bounding rectangles are exactly (0, 0) - (15, 20).
        auto results = tree.search({{0.0, 0.0}, {15.0, 20.0}}, rt_type::search_type::match);
        std::cout << "number of results: " << std::distance(results.begin(), results.end()) << std::endl;

        std::cout << "value: " << *results.begin() << std::endl;

        std::cout << "--" << std::endl;

        auto it = results.begin();
        std::cout << "value: " << *it << std::endl;
        std::cout << "extent: " << it.extent().to_string() << std::endl;
        std::cout << "depth: " << it.depth() << std::endl;
    }

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
