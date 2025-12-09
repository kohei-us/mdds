// SPDX-FileCopyrightText: 2019 David Tardon
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <string>

#include <mdds/segment_tree.hpp>

void stop();

using std::string;

using mdds::segment_tree;

void test_segment_tree()
{
    segment_tree<int, int> empty_st;

    segment_tree<int, int> st_int;
    st_int.insert(0, 10, 1);
    st_int.insert(5, 20, 2);
    st_int.insert(40, 50, 3);
    st_int.insert(-10, 20, 0);

    segment_tree<int, string> st_string;
    st_string.insert(20, 30, "My hovercraft is full of eels");

    st_int.build_tree();
    auto search_none = st_int.search(25);
    auto search_one = st_int.search(42);
    auto search_more = st_int.search(8);

    stop();
}

// vim: set shiftwidth=4 softtabstop=4 expandtab:
