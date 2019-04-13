/* Test code for mdds::flat_segment_tree.

   This file is part of mdds.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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
