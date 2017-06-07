cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0048 NEW)

project(mdds VERSION 1.2.3 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)

option(BOOST_INCLUDE_DIR "path to boost header include directory.")

add_definitions(-DMDDS_UNIT_TEST)
add_definitions(-DMDDS_DEBUG_NODE_BASE)
include_directories(${PROJECT_SOURCE_DIR}/include ${BOOST_INCLUDE_DIR})

add_executable(flat_segment_tree_test ${PROJECT_SOURCE_DIR}/src/flat_segment_tree_test.cpp)
add_executable(segment_tree_test ${PROJECT_SOURCE_DIR}/src/segment_tree_test.cpp)
add_executable(rectangle_set_test ${PROJECT_SOURCE_DIR}/src/rectangle_set_test.cpp)
add_executable(point_quad_tree_test ${PROJECT_SOURCE_DIR}/src/point_quad_tree_test.cpp)
add_executable(sorted_string_map_test ${PROJECT_SOURCE_DIR}/src/sorted_string_map_test.cpp)
add_executable(trie_map_test ${PROJECT_SOURCE_DIR}/src/trie_map_test.cpp)
add_executable(multi_type_matrix_test ${PROJECT_SOURCE_DIR}/src/multi_type_matrix_test.cpp)
add_executable(multi_type_matrix_test_walk ${PROJECT_SOURCE_DIR}/src/multi_type_matrix_test_walk.cpp)
add_executable(multi_type_vector_test_collection ${PROJECT_SOURCE_DIR}/src/multi_type_vector_test_collection.cpp)
add_executable(multi_type_vector_test_custom ${PROJECT_SOURCE_DIR}/src/multi_type_vector_test_custom.cpp)
add_executable(multi_type_vector_test_default ${PROJECT_SOURCE_DIR}/src/multi_type_vector_test_default.cpp)
add_executable(multi_type_vector_test_event ${PROJECT_SOURCE_DIR}/src/multi_type_vector_test_event.cpp)
add_executable(multi_type_vector_test_perf ${PROJECT_SOURCE_DIR}/src/multi_type_vector_test_perf.cpp)

