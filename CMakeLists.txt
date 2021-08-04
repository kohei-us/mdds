cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0048 NEW)

project(mdds VERSION 1.9.99 LANGUAGES CXX)
set(API_VERSION 2.0)
set(CMAKE_CXX_STANDARD 17)

find_package(Boost)
find_package(Threads)

enable_testing()
set(CMAKE_CTEST_COMMAND ctest -V)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4251 /wd4275 /wd4244 /MP")
endif(MSVC)

add_definitions(-DMDDS_UNIT_TEST)
add_definitions(-DMDDS_DEBUG_NODE_BASE)
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/include
    ${Boost_INCLUDE_DIR}
)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

# TODO : move the test executable definitions to their respective subdirectories.

set(_TESTS
    flat-segment-tree-test
    multi-type-matrix-test
    multi-type-matrix-test-walk
    point-quad-tree-test
    segment-tree-test
    sorted-string-map-test
    trie-map-test
)

foreach(_TEST ${_TESTS})
    set(_TEST_FILE ${_TEST}.cpp)
    string(REPLACE "-" "_" _TEST_FILE ${_TEST_FILE})
    add_executable(${_TEST} EXCLUDE_FROM_ALL
        ${PROJECT_SOURCE_DIR}/src/test_global.cpp
        ${PROJECT_SOURCE_DIR}/src/${_TEST_FILE}
    )
    add_test(${_TEST} ${_TEST} func)  # not all test programs require a 'func' parameter.
endforeach()

add_executable(rtree-test EXCLUDE_FROM_ALL
    ${PROJECT_SOURCE_DIR}/src/test_global.cpp
    ${PROJECT_SOURCE_DIR}/src/rtree/test_main.cpp
    ${PROJECT_SOURCE_DIR}/src/rtree/test_basic.cpp
    ${PROJECT_SOURCE_DIR}/src/rtree/test_copy.cpp
    ${PROJECT_SOURCE_DIR}/src/rtree/test_directory_node_split.cpp
    ${PROJECT_SOURCE_DIR}/src/rtree/test_erase_directories.cpp
    ${PROJECT_SOURCE_DIR}/src/rtree/test_exact_search_by_extent.cpp
    ${PROJECT_SOURCE_DIR}/src/rtree/test_exact_search_by_point.cpp
    ${PROJECT_SOURCE_DIR}/src/rtree/test_forced_reinsertion.cpp
    ${PROJECT_SOURCE_DIR}/src/rtree/test_geometry.cpp
    ${PROJECT_SOURCE_DIR}/src/rtree/test_intersection.cpp
    ${PROJECT_SOURCE_DIR}/src/rtree/test_main.cpp
    ${PROJECT_SOURCE_DIR}/src/rtree/test_move.cpp
    ${PROJECT_SOURCE_DIR}/src/rtree/test_node_split.cpp
    ${PROJECT_SOURCE_DIR}/src/rtree/test_point_objects.cpp
    ${PROJECT_SOURCE_DIR}/src/rtree/test_square_distance.cpp
)

add_executable(rtree-test-bulkload EXCLUDE_FROM_ALL
    ${PROJECT_SOURCE_DIR}/src/test_global.cpp
    ${PROJECT_SOURCE_DIR}/src/rtree/test_bulkload_main.cpp
)

set(_CUSTOM_TESTS
    rtree-test
    rtree-test-bulkload
)

foreach(_TEST ${_CUSTOM_TESTS})
    add_test(${_TEST} ${_TEST})
endforeach()

add_dependencies(check
    ${_TESTS}
    ${_CUSTOM_TESTS}
)

install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/include/mdds
    DESTINATION include/mdds-${API_VERSION}
    FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.inl"
)

add_subdirectory(src)
