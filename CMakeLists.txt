cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0048 NEW)

project(mdds VERSION 1.9.99 LANGUAGES CXX)
set(API_VERSION 2.0)
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
add_subdirectory(tools)
