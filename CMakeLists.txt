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
    ${Boost_INCLUDE_DIR}
)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})

install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/include/mdds
    DESTINATION include/mdds-${API_VERSION}
    FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.inl"
)

add_subdirectory(test)
