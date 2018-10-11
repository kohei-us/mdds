cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0048 NEW)

option(BOOST_INCLUDEDIR "path to boost header include directory.")

project(mdds VERSION 1.3.99 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)

find_package(Boost)
find_package(Threads)

enable_testing()
set(CMAKE_CTEST_COMMAND ctest -V)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4275 /MP")
endif(MSVC)

add_definitions(-DMDDS_UNIT_TEST)
add_definitions(-DMDDS_DEBUG_NODE_BASE)
include_directories(${PROJECT_SOURCE_DIR}/include ${Boost_INCLUDE_DIR})

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

add_executable(flat-segment-tree-test EXCLUDE_FROM_ALL ${PROJECT_SOURCE_DIR}/src/flat_segment_tree_test.cpp)
add_executable(segment-tree-test EXCLUDE_FROM_ALL ${PROJECT_SOURCE_DIR}/src/segment_tree_test.cpp)
add_executable(rectangle-set-test EXCLUDE_FROM_ALL ${PROJECT_SOURCE_DIR}/src/rectangle_set_test.cpp)
add_executable(point-quad-tree-test EXCLUDE_FROM_ALL ${PROJECT_SOURCE_DIR}/src/point_quad_tree_test.cpp)
add_executable(sorted-string-map-test EXCLUDE_FROM_ALL ${PROJECT_SOURCE_DIR}/src/sorted_string_map_test.cpp)
add_executable(trie-map-test EXCLUDE_FROM_ALL ${PROJECT_SOURCE_DIR}/src/trie_map_test.cpp)
add_executable(multi-type-matrix-test EXCLUDE_FROM_ALL ${PROJECT_SOURCE_DIR}/src/multi_type_matrix_test.cpp)
add_executable(multi-type-matrix-test-walk EXCLUDE_FROM_ALL ${PROJECT_SOURCE_DIR}/src/multi_type_matrix_test_walk.cpp)
add_executable(multi-type-vector-test-collection EXCLUDE_FROM_ALL ${PROJECT_SOURCE_DIR}/src/multi_type_vector_test_collection.cpp)
add_executable(multi-type-vector-test-custom EXCLUDE_FROM_ALL ${PROJECT_SOURCE_DIR}/src/multi_type_vector_test_custom.cpp)
add_executable(multi-type-vector-test-default EXCLUDE_FROM_ALL ${PROJECT_SOURCE_DIR}/src/multi_type_vector_test_default.cpp)
add_executable(multi-type-vector-test-event EXCLUDE_FROM_ALL ${PROJECT_SOURCE_DIR}/src/multi_type_vector_test_event.cpp)
add_executable(multi-type-vector-test-perf EXCLUDE_FROM_ALL ${PROJECT_SOURCE_DIR}/src/multi_type_vector_test_perf.cpp)
add_executable(rtree-test EXCLUDE_FROM_ALL ${PROJECT_SOURCE_DIR}/src/rtree_test.cpp)
add_executable(rtree-test-bulkload EXCLUDE_FROM_ALL ${PROJECT_SOURCE_DIR}/src/rtree_test_bulkload.cpp)

add_test(flat-segment-tree-test flat-segment-tree-test func)
add_test(segment-tree-test segment-tree-test func)
add_test(rectangle-set-test rectangle-set-test func)
add_test(point-quad-tree-test point-quad-tree-test func)
add_test(sorted-string-map-test sorted-string-map-test func)
#TODO: this currently crashes.  Look into this.
#add_test(trie-map-test trie-map-test func)
add_test(multi-type-matrix-test multi-type-matrix-test func)
add_test(multi-type-matrix-test-walk multi-type-matrix-test-walk func)
add_test(multi-type-vector-test-collection multi-type-vector-test-collection func)
add_test(multi-type-vector-test-custom multi-type-vector-test-custom func)
add_test(multi-type-vector-test-default multi-type-vector-test-default func)
add_test(multi-type-vector-test-event multi-type-vector-test-event func)
#TODO: performance test does not need to be executed by the build.
#add_test(multi-type-vector-test-perf multi-type-vector-test-perf)
add_test(rtree-test rtree-test)
add_test(rtree-test-bulkload rtree-test-bulkload)

add_dependencies(check
    flat-segment-tree-test
    multi-type-matrix-test
    multi-type-matrix-test-walk
    multi-type-vector-test-collection
    multi-type-vector-test-custom
    multi-type-vector-test-default
    multi-type-vector-test-event
    multi-type-vector-test-perf
    point-quad-tree-test
    rectangle-set-test
    rtree-test
    rtree-test-bulkload
    segment-tree-test
    sorted-string-map-test
    trie-map-test
)
