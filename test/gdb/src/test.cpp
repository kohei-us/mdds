// SPDX-FileCopyrightText: 2019 David Tardon
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
