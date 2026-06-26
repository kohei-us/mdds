# SPDX-FileCopyrightText: 2026 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

# COW is implemented for the SoA variant only for now; the AoS COW test will
# be added when AoS receives COW support.
set(TARGET_NAME multi-type-vector-test-cow-soa)
add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL test_soa.cpp)
target_link_libraries(${TARGET_NAME} PUBLIC test-global)
target_include_directories(${TARGET_NAME} PUBLIC . tc ../../../include)
add_test(${TARGET_NAME} ${TARGET_NAME})
add_dependencies(check ${TARGET_NAME})
