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

#include <mdds/flat_segment_tree.hpp>
#include <iostream>

int main() try
{
    //!code-start: type
    using fst_type = mdds::flat_segment_tree<long, int>;
    //!code-end: type

    //!code-start: instance
    // Define the begin and end points of the whole segment, and the default
    // value.
    fst_type db(0, 500, 0);
    //!code-end: instance

    //!code-start: insert-1
    db.insert_front(10, 20, 10);
    db.insert_back(50, 70, 15);
    //!code-end: insert-1

    //!code-start: insert-2
    db.insert_back(60, 65, 5);
    //!code-end: insert-2

    //!code-start: linear-search
    // Perform linear search.  This doesn't require the tree to be built ahead
    // of time.
    if (auto it = db.search(15); it != db.end())
    {
        auto segment = it.to_segment();
        std::cout << "The value at 15 is " << segment->value
            << ", and this segment spans from " << segment->start << " to "
            << segment->end << std::endl;
    }
    //!code-end: linear-search

    //!code-start: tree-search
    // Don't forget to build tree before calling search_tree().
    db.build_tree();

    // Perform tree search.  Tree search is generally a lot faster than linear
    // search, but requires the tree to be built ahead of time.
    if (auto it = db.search_tree(62); it != db.end())
    {
        auto segment = it.to_segment();
        std::cout << "The value at 62 is " << segment->value
            << ", and this segment spans from " << segment->start << " to "
            << segment->end << std::endl;
    }
    //!code-end: tree-search
}
catch (...)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
