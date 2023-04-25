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
