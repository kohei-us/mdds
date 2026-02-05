# SPDX-FileCopyrightText: 2025 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0048 NEW)

set(MDDS_API_VERSION_MAJOR 3)
set(MDDS_API_VERSION_MINOR 0)
set(MDDS_API_VERSION "${MDDS_API_VERSION_MAJOR}.${MDDS_API_VERSION_MINOR}")
message(STATUS "API version: ${MDDS_API_VERSION}")

project(mdds VERSION 3.2.0 LANGUAGES CXX)
set(MDDS_VERSION "${mdds_VERSION}")
set(MDDS_VERSION_MAJOR ${mdds_VERSION_MAJOR})
set(MDDS_VERSION_MINOR ${mdds_VERSION_MINOR})
set(MDDS_VERSION_MICRO ${mdds_VERSION_PATCH})  # NB: PATCH -> MICRO
message(STATUS "mdds version: ${MDDS_VERSION}")
set(CMAKE_CXX_STANDARD 17)

find_package(Boost)
find_package(Threads)

enable_testing()
set(CMAKE_CTEST_COMMAND ctest -V)

message(STATUS "size of void pointer: ${CMAKE_SIZEOF_VOID_P}")

add_compile_definitions(
    MDDS_UNIT_TEST
    MDDS_DEBUG_NODE_BASE
    SIZEOF_VOID_P=${CMAKE_SIZEOF_VOID_P}
)

if(MSVC)
    add_definitions(/wd4251 /wd4275 /wd4244 /MP)
endif(MSVC)

configure_file(
    ${PROJECT_SOURCE_DIR}/include/mdds/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/mdds/version.hpp
)

include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${Boost_INCLUDE_DIR}
)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/include/mdds
    ${CMAKE_CURRENT_BINARY_DIR}/include/mdds
    DESTINATION include/mdds-${MDDS_API_VERSION}
    FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.inl"
)

add_subdirectory(misc)
add_subdirectory(test)
add_subdirectory(tools)
