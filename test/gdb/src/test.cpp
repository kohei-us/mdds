/* Test suite for gdb pretty printers.

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

// forward decls. of tests
void test_flat_segment_tree();
void test_multi_type_matrix();
void test_multi_type_vector();
void test_point_quad_tree();
void test_rtree();
void test_segment_tree();
void test_sorted_string_map();
void test_trie_map();

void stop()
{}

int main()
try
{
    test_flat_segment_tree();
    test_multi_type_matrix();
    test_multi_type_vector();
    test_point_quad_tree();
    test_rtree();
    test_segment_tree();
    test_sorted_string_map();
    test_trie_map();
    return 0;
}
catch (...)
{
    return -1;
}

// vim: set shiftwidth=4 softtabstop=4 expandtab:
