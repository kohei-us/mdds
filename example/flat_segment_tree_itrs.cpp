/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2020 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include <mdds/flat_segment_tree.hpp>
#include <iostream>

using fst_type = mdds::flat_segment_tree<long, int>;

void iterate_nodes(const fst_type& db)
{
    //!code-start: iterate-nodes
    for (auto it = db.begin(); it != db.end(); ++it)
    {
        std::cout << "key: " << it->first << "; value: " << it->second << std::endl;
    }
    //!code-end: iterate-nodes
}

void loop_nodes(const fst_type& db)
{
    //!code-start: loop-nodes
    for (const auto& node : db)
    {
        std::cout << "key: " << node.first << "; value: " << node.second << std::endl;
    }
    //!code-end: loop-nodes
}

void iterate_segments(const fst_type& db)
{
    //!code-start: iterate-segments
    for (auto it = db.begin_segment(); it != db.end_segment(); ++it)
    {
        std::cout << "start: " << it->start << "; end: " << it->end << "; value: " << it->value << std::endl;
    }
    //!code-end: iterate-segments
}

void loop_segments(const fst_type& db)
{
    //!code-start: loop-segments
    for (const auto& segment : db.segment_range())
    {
        std::cout << "start: " << segment.start << "; end: " << segment.end << "; value: " << segment.value << std::endl;
    }
    //!code-end: loop-segments
}

int main()
{
    fst_type db(0, 500, 0);

    db.insert_front(10, 20, 10);
    db.insert_back(50, 70, 15);
    db.insert_back(60, 65, 5);

    iterate_nodes(db);
    loop_nodes(db);
    iterate_segments(db);
    loop_segments(db);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
