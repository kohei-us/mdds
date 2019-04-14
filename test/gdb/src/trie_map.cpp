/* Test code for mdds::trie_map.

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

#include <mdds/global.hpp>
#include <mdds/trie_map.hpp>

void stop();

namespace trie = mdds::trie;

using mdds::trie_map;

void test_trie_map()
{
    trie_map<trie::std_string_trait, int> empty_tm;

    trie_map<trie::std_string_trait, int> tm_int;
    tm_int.insert("a", 13);
    tm_int.insert("aa", 10);
    tm_int.insert("ab", 3);
    tm_int.insert("b", 7);

    trie_map<trie::std_string_trait, std::string> tm_str;
    tm_str.insert("a", "13");
    tm_str.insert("aa", "10");
    tm_str.insert("ab", "3");
    tm_str.insert("b", "7");

    stop();
}

// vim: set shiftwidth=4 softtabstop=4 expandtab:
