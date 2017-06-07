cmake_minimum_required(VERSION 2.8)

project(mdds)

option(BOOST_INCLUDE_DIR "path to boost header include directory.")

add_definitions(-DMDDS_UNIT_TEST)
add_definitions(-DMDDS_DEBUG_NODE_BASE)
include_directories(${PROJECT_SOURCE_DIR}/include ${BOOST_INCLUDE_DIR})

add_executable(flat_segment_tree_test ${PROJECT_SOURCE_DIR}/src/flat_segment_tree_test.cpp)

