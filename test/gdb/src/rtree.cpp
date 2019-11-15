/* Test code for mdds::rtree.

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
