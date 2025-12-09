# SPDX-FileCopyrightText: 2025 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

set(TEST_NAME multi-type-vector-test-custom-trait-aos)
set(TARGET_NAME ${TEST_NAME})

add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL
    test_main.cpp
    test_loop_unrolling.cpp
)

target_link_libraries(${TARGET_NAME} PUBLIC test-global)
target_include_directories(${TARGET_NAME} PUBLIC
    ../tc
    ${PROJECT_SOURCE_DIR}/src/include
)

add_test(${TEST_NAME} ${TARGET_NAME})
add_dependencies(check ${TARGET_NAME})
