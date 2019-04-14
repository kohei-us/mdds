/* Test code for mdds::segment_tree.

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
