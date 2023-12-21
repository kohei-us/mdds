/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2020-2023 Kohei Yoshida
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

//!code-start: header
#include <mdds/segment_tree.hpp>
#include <string>
#include <iostream>
//!code-end: header

//!code-start: type
using tree_type = mdds::segment_tree<long, std::string>;
//!code-end: type

//!code-start: search-and-print
void search_and_print(const tree_type& tree, long pos)
{
    auto results = tree.search(pos);

    std::cout << "--" << std::endl;
    std::cout << "search at " << pos << std::endl;
    std::cout << "number of results: " << results.size() << std::endl;

    for (const auto& [start, end, value] : results)
        std::cout << "range: [" << start << ":" << end << "); value: " << value << std::endl;
}
//!code-end: search-and-print

void erase_by_iterator(tree_type tree)
{
    //!code-start: erase-by-iterator
    auto results = tree.search(5);
    for (auto it = results.begin(); it != results.end(); ++it)
    {
        if (it->value == "A")
        {
            tree.erase(it);
            break;
        }
    }
    //!code-end: erase-by-iterator

    //!code-start: search-5-after-erase-by-iterator
    search_and_print(tree, 5);
    //!code-end: search-5-after-erase-by-iterator
}

void erase_by_predicate(tree_type tree)
{
    //!code-start: erase-by-predicate
    auto pred = [](long start, long end, const std::string& /*value*/)
    {
        return start <= 5 && 5 < end;
    };

    auto n = tree.erase_if(pred);

    std::cout << "--" << std::endl;
    std::cout << n << " segments have been removed" << std::endl;
    //!code-end: erase-by-predicate

    //!code-start: search-5-after-erase-by-predicate
    search_and_print(tree, 5);
    //!code-end: search-5-after-erase-by-predicate
}

int main() try
{
    //!code-start: insert
    tree_type tree;

    tree.insert(0, 10, "A");
    tree.insert(2, 15, "B");
    tree.insert(-2, 5, "C");
    tree.insert(5, 22, "D");
    tree.insert(20, 35, "E");
    //!code-end: insert

    //!code-start: build
    tree.build_tree();
    //!code-end: build

    //!code-start: search-5
    search_and_print(tree, 5);
    //!code-end: search-5

    //!code-start: search-0
    search_and_print(tree, 0);
    //!code-end: search-0

    erase_by_iterator(tree);
    erase_by_predicate(tree);
}
catch (...)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
