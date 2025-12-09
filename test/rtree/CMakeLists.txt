# SPDX-FileCopyrightText: 2025 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

add_executable(rtree-test EXCLUDE_FROM_ALL
    test_basic.cpp
    test_copy.cpp
    test_directory_node_split.cpp
    test_erase_directories.cpp
    test_exact_search_by_extent.cpp
    test_exact_search_by_point.cpp
    test_forced_reinsertion.cpp
    test_geometry.cpp
    test_intersection.cpp
    test_main.cpp
    test_main.cpp
    test_move.cpp
    test_node_split.cpp
    test_point_objects.cpp
    test_square_distance.cpp
)

target_link_libraries(rtree-test PRIVATE test-global)

add_executable(rtree-test-bulkload EXCLUDE_FROM_ALL
    test_bulkload_main.cpp
)

target_link_libraries(rtree-test-bulkload PRIVATE test-global)

add_test(rtree-test rtree-test)
add_test(rtree-test-bulkload rtree-test-bulkload)

add_dependencies(check
    rtree-test
    rtree-test-bulkload
)
