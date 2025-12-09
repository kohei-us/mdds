// SPDX-FileCopyrightText: 2019 David Tardon
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <string>

#include <mdds/flat_segment_tree.hpp>

void stop();

using std::string;

using mdds::flat_segment_tree;

void test_flat_segment_tree()
{
    flat_segment_tree<int, int> default_fst(0, 100, 42);

    flat_segment_tree<int, int> fst_int(0, 100, 42);
    fst_int.insert_back(0, 20, 1);
    fst_int.insert_back(40, 60, 0);

    flat_segment_tree<int, string> fst_string(10, 50, "42");
    fst_string.insert_back(20, 30, "My hovercraft is full of eels");

    flat_segment_tree<int, int>::const_iterator fst_iter_singular;
    auto fst_iter1 = fst_int.begin();
    auto fst_iter2 = fst_iter1;
    ++fst_iter2;

    flat_segment_tree<int, int>::const_segment_iterator fst_seg_iter_singular;
    auto fst_seg_iter1 = fst_int.begin_segment();
    auto fst_seg_iter2 = fst_seg_iter1;
    ++fst_seg_iter2;

    stop();
}

// vim: set shiftwidth=4 softtabstop=4 expandtab:
