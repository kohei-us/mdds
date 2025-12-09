// SPDX-FileCopyrightText: 2019 David Tardon
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <string>

#include <mdds/rtree.hpp>

void stop();

using mdds::rtree;

void test_rtree()
{
    using tree_type = rtree<int, std::string>;

    tree_type empty_tree;

    tree_type tree;
    tree.insert({{0, 1}, {2, 4}}, "a");
    tree.insert({{-3, 3}, {5, 8}}, "bc");
    tree.insert({{-2, 1}, {3, 6}}, "d");

    auto search_empty = tree.search({{0, 0}, {0, 0}}, tree_type::search_type::match);
    auto search_one = tree.search({{0, 1}, {2, 4}}, tree_type::search_type::match);
    auto search_more = tree.search({{0, 0}, {1, 5}}, tree_type::search_type::overlap);

    auto iter_begin = search_one.begin();
    auto iter_end = search_one.end();
    (void)iter_begin;
    (void)iter_end;

    stop();
}

// vim: set shiftwidth=4 softtabstop=4 expandtab:
