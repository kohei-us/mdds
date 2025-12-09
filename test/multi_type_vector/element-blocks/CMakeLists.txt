# SPDX-FileCopyrightText: 2025 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

set(TEST_NAME multi-type-vector-test-element-blocks)
set(TARGET_NAME ${TEST_NAME})

add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL
    test_main.cpp
    test_basic.cpp
    test_std_vector_bool.cpp
    test_store_types.cpp
    test_functions.cpp
)

add_executable(${TARGET_NAME}-aos EXCLUDE_FROM_ALL
    test_aos.cpp
    test_aos.cpp
    test_no_standard_blocks_aos.cpp
)

add_executable(${TARGET_NAME}-soa EXCLUDE_FROM_ALL
    test_soa.cpp
    test_soa.cpp
    test_no_standard_blocks_soa.cpp
)

add_executable(${TARGET_NAME}-util EXCLUDE_FROM_ALL
    test_util.cpp
)

target_link_libraries(${TARGET_NAME} PUBLIC test-global)
target_link_libraries(${TARGET_NAME}-aos PUBLIC test-global)
target_link_libraries(${TARGET_NAME}-soa PUBLIC test-global)
target_link_libraries(${TARGET_NAME}-util PUBLIC test-global)

add_test(${TEST_NAME} ${TARGET_NAME})
add_test(${TEST_NAME}-aos ${TARGET_NAME})
add_test(${TEST_NAME}-soa ${TARGET_NAME})
add_test(${TEST_NAME}-util ${TARGET_NAME})
add_dependencies(check ${TARGET_NAME})
