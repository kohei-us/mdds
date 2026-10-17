# SPDX-FileCopyrightText: 2025 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

add_executable(multi-type-matrix-test-main EXCLUDE_FROM_ALL
    test_main.cpp
)

add_executable(multi-type-matrix-test-walk EXCLUDE_FROM_ALL
    test_walk.cpp
)

add_executable(multi-type-matrix-test-custom EXCLUDE_FROM_ALL
    test_custom.cpp
)

target_link_libraries(multi-type-matrix-test-main PRIVATE test-global)
target_link_libraries(multi-type-matrix-test-walk PRIVATE test-global)
target_link_libraries(multi-type-matrix-test-custom PRIVATE test-global)

add_test(multi-type-matrix-test-main multi-type-matrix-test-main)
add_test(multi-type-matrix-test-walk multi-type-matrix-test-walk)
add_test(multi-type-matrix-test-custom multi-type-matrix-test-custom)

add_dependencies(check
    multi-type-matrix-test-main
    multi-type-matrix-test-walk
    multi-type-matrix-test-custom
)
