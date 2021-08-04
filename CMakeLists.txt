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

add_dependencies(check
    ${_TESTS}
)

install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/include/mdds
    DESTINATION include/mdds-${API_VERSION}
    FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.inl"
)

add_subdirectory(src)
