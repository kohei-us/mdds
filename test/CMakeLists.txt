# SPDX-FileCopyrightText: 2025 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

add_library(test-global STATIC EXCLUDE_FROM_ALL test_global.cpp)

target_include_directories(test-global PUBLIC
    include
)

add_subdirectory(flat_segment_tree)
add_subdirectory(multi_type_vector)
add_subdirectory(multi_type_matrix)
add_subdirectory(point_quad_tree)
add_subdirectory(rtree)
add_subdirectory(segment_tree)
add_subdirectory(sorted_string_map)
add_subdirectory(trie_map)
