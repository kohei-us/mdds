
add_executable(multi-type-vector-test-collection-soa EXCLUDE_FROM_ALL
    test_main.cpp
    ../../../test_global.cpp
)

target_include_directories(multi-type-vector-test-collection-soa PUBLIC
    ../tc
    ../../../include
)

add_test(multi-type-vector-test-collection-soa multi-type-vector-test-collection-soa)
add_dependencies(check multi-type-vector-test-collection-soa)
