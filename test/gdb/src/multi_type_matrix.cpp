/* Test code for mdds::multi_type_matrix.

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

#include <mdds/multi_type_matrix.hpp>

void stop();

using mdds::multi_type_matrix;

void test_multi_type_matrix()
{
    typedef mdds::multi_type_matrix<mdds::mtm::std_string_traits> mtm_type;

    mtm_type empty_mtm;

    mtm_type mtm(4, 2);
    mtm.set(1, 0, 1);
    mtm.set(2, 0, 2);
    mtm.set(3, 0, std::string("a"));
    mtm.set(0, 1, std::string("b"));
    mtm.set(1, 1, true);
    mtm.set(3, 1, 0.5);

    stop();
}

// vim: set shiftwidth=4 softtabstop=4 expandtab:
