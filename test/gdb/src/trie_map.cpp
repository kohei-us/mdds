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

using mdds::packed_trie_map;
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

    auto tm_search_empty = tm_int.prefix_search("foo");
    auto tm_search_one = tm_int.prefix_search("b");
    auto tm_search_more = tm_int.prefix_search("a");

    auto tm_iter_begin = tm_int.begin();
    auto tm_iter_end = tm_int.end();
    auto tm_search_iter_begin = tm_search_one.begin();
    auto tm_search_iter_end = tm_search_one.end();
    auto tm_search_iter_empty = tm_search_empty.begin();

    packed_trie_map<trie::std_string_trait, int>::entry entries_int[] = {
        {MDDS_ASCII("a"), 13},
        {MDDS_ASCII("aa"), 10},
        {MDDS_ASCII("ab"), 3},
        {MDDS_ASCII("b"), 7},
    };
    packed_trie_map<trie::std_string_trait, int> empty_ptm(entries_int, 0);
    packed_trie_map<trie::std_string_trait, int> ptm_int(entries_int, MDDS_N_ELEMENTS(entries_int));
    packed_trie_map<trie::std_string_trait, std::string>::entry entries_str[] = {
        {MDDS_ASCII("a"), "13"},
        {MDDS_ASCII("aa"), "10"},
        {MDDS_ASCII("ab"), "3"},
        {MDDS_ASCII("b"), "7"},
    };
    packed_trie_map<trie::std_string_trait, std::string> ptm_str(entries_str, MDDS_N_ELEMENTS(entries_str));

    auto ptm_search_empty = ptm_int.prefix_search("foo");
    auto ptm_search_one = ptm_int.prefix_search("b");
    auto ptm_search_more = ptm_int.prefix_search("a");

    auto ptm_iter_begin = ptm_int.begin();
    auto ptm_iter_end = ptm_int.end();
    auto ptm_search_iter_begin = ptm_search_one.begin();
    auto ptm_search_iter_end = ptm_search_one.end();
    auto ptm_search_iter_empty = ptm_search_empty.begin();

    stop();
}

// vim: set shiftwidth=4 softtabstop=4 expandtab:
