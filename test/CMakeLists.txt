# SPDX-FileCopyrightText: 2025 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

add_library(test-global STATIC EXCLUDE_FROM_ALL test_global.cpp)

target_include_directories(test-global PUBLIC
    include
)

add_executable(ref-pair-test-main EXCLUDE_FROM_ALL
    ref_pair_test.cpp
)

add_executable(version-test-main EXCLUDE_FROM_ALL
    version_test.cpp
)

target_link_libraries(ref-pair-test-main PRIVATE test-global)
target_link_libraries(version-test-main PRIVATE test-global)

add_test(ref-pair-test-main ref-pair-test-main)
add_test(version-test-main version-test-main)

add_dependencies(check
    ref-pair-test-main
    version-test-main
)

add_subdirectory(flat_segment_tree)
add_subdirectory(multi_type_vector)
add_subdirectory(multi_type_matrix)
add_subdirectory(point_quad_tree)
add_subdirectory(rtree)
add_subdirectory(segment_tree)
add_subdirectory(sorted_string_map)
add_subdirectory(trie_map)
