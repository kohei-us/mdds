/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2020 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

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
