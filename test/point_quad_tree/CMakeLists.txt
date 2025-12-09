# SPDX-FileCopyrightText: 2025 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

add_executable(point-quad-tree-test-main EXCLUDE_FROM_ALL
    test_main.cpp
)

target_link_libraries(point-quad-tree-test-main PRIVATE test-global)

add_test(point-quad-tree-test-main point-quad-tree-test-main)

add_dependencies(check
    point-quad-tree-test-main
)
