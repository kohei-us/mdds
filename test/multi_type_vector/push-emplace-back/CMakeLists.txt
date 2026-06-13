# SPDX-FileCopyrightText: 2025 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

foreach(VARIANT aos soa)
    set(TARGET_NAME multi-type-vector-test-push-emplace-back-${VARIANT})
    add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL test_${VARIANT}.cpp)
    target_link_libraries(${TARGET_NAME} PUBLIC test-global)
    target_include_directories(${TARGET_NAME} PUBLIC . tc ../../../include)
    add_test(${TARGET_NAME} ${TARGET_NAME})
    add_dependencies(check ${TARGET_NAME})
endforeach()
