
add_executable(multi-type-vector-test-collection-aos EXCLUDE_FROM_ALL
    test_main.cpp
    ../../../test_global.cpp
)

target_include_directories(multi-type-vector-test-collection-aos PUBLIC
    ../tc
    ../../../include
)

add_test(multi-type-vector-test-collection-aos multi-type-vector-test-collection-aos)
add_dependencies(check multi-type-vector-test-collection-aos)
