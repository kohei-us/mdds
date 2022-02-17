/* Test code for mdds::point_quad_tree.

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

#include <mdds/point_quad_tree.hpp>

void stop();

using mdds::point_quad_tree;

void test_point_quad_tree()
{
    point_quad_tree<int, int> empty_pqt;

    point_quad_tree<int, int> pqt_int;
    pqt_int.insert(10, 20, 1);
    pqt_int.insert(15, 8, 2);
    pqt_int.insert(25, 12, 3);
    pqt_int.insert(15, 12, 4);

    point_quad_tree<unsigned, std::string> pqt_string;
    pqt_string.insert(25, 32, "a");
    pqt_string.insert(5, 45, "b");
    pqt_string.insert(52, 10, "c");
    pqt_string.insert(80, 5, "d");
    pqt_string.insert(40, 50, "e");
    pqt_string.insert(10, 10, "f");

    auto pqt_search_empty = pqt_int.search_region(0, 2, 3, 4);
    auto pqt_search_one = pqt_int.search_region(0, 15, 10, 25);
    auto pqt_search_more = pqt_int.search_region(0, 10, 15, 25);

    stop();
}

// vim: set shiftwidth=4 softtabstop=4 expandtab:
