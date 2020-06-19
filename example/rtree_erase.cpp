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

int main()
{
    using rt_type = mdds::rtree<int, std::string>;

    rt_type tree;

    // Insert multiple values at the same point.
    tree.insert({1, 1}, "A");
    tree.insert({1, 1}, "B");
    tree.insert({1, 1}, "C");
    tree.insert({1, 1}, "D");
    tree.insert({1, 1}, "E");

    // This should return all five values.
    auto results = tree.search({1, 1}, rt_type::search_type::match);

    for (const std::string& v : results)
        std::cout << v << std::endl;

    // Erase "C".
    for (auto it = results.begin(); it != results.end(); ++it)
    {
        if (*it == "C")
        {
            tree.erase(it);
            break; // This invalidates the iterator.  Bail out.
        }
    }

    std::cout << "'C' has been erased." << std::endl;

    // Now this should only return A, B, D and E.
    results = tree.search({1, 1}, rt_type::search_type::match);

    for (const std::string& v : results)
        std::cout << v << std::endl;

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
