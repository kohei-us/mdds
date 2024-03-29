/* Test code for mdds::sorted_string_map.

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

#include <mdds/sorted_string_map.hpp>

void stop();

using mdds::sorted_string_map;

void test_sorted_string_map()
{
    sorted_string_map<int> empty_ssmap(nullptr, 0, 0);

    sorted_string_map<int>::entry_type entries[] = {
        {"aaaa", 1},
        {"bbb", 2},
        {"cc", 3},
        {"d", 4},
    };
    sorted_string_map<int> ssmap_int(entries, std::size(entries), 0);

    stop();
}

// vim: set shiftwidth=4 softtabstop=4 expandtab:
