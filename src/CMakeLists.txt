
add_library(test-global STATIC EXCLUDE_FROM_ALL test_global.cpp)

target_include_directories(test-global PUBLIC
    include
)

add_subdirectory(multi_type_vector)
add_subdirectory(rtree)
