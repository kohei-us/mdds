/* Test code for mdds::multi_type_vector.

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

#include <mdds/multi_type_vector.hpp>
#include <mdds/multi_type_vector_trait.hpp>

void stop();

using mdds::multi_type_vector;

void test_multi_type_vector()
{
    typedef mdds::multi_type_vector<mdds::mtv::element_block_func> mtv_type;

    mtv_type empty_mtv;

    mtv_type mtv_int;
    mtv_int.push_back<int>(1);
    mtv_int.push_back<int>(2);
    mtv_int.push_back<int>(3);
    mtv_int.push_back<int>(4);

    mtv_type mtv_string;
    mtv_string.push_back<std::string>("ab");
    mtv_string.push_back<std::string>("c");

    mtv_type mtv_int_string(mtv_int);
    mtv_int_string.push_back<std::string>("ab");
    mtv_int_string.push_back<std::string>("c");
    mtv_int_string.push_back<int>(7);
    mtv_int_string.push_back<int>(8);

    mtv_type mtv_default(2);

    auto mtv_iter_begin = mtv_int_string.begin();
    auto mtv_iter_next = ++mtv_int_string.begin();
    auto mtv_iter_end = mtv_int_string.end();
    (void)mtv_iter_begin;
    (void)mtv_iter_next;
    (void)mtv_iter_end;

    auto mtv_iter_cbegin = mtv_int_string.cbegin();
    auto mtv_iter_cend = mtv_int_string.cend();
    (void)mtv_iter_cbegin;
    (void)mtv_iter_cend;

    auto mtv_iter_rbegin = mtv_int_string.rbegin();
    auto mtv_iter_rend = mtv_int_string.rend();
    (void)mtv_iter_rbegin;
    (void)mtv_iter_rend;

    auto mtv_iter_default = mtv_default.begin();
    (void)mtv_iter_default;

    stop();
}

// vim: set shiftwidth=4 softtabstop=4 expandtab:
