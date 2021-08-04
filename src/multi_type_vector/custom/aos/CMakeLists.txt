
add_executable(multi-type-vector-test-custom-aos EXCLUDE_FROM_ALL
    test_basic.cpp
    test_main.cpp
    test_managed_block.cpp
    test_misc.cpp
    test_swap.cpp
    test_transfer.cpp
    ../../../test_global.cpp
)

target_include_directories(multi-type-vector-test-custom-aos PUBLIC
    ..
    ../tc
    ../../../include
)

add_test(multi-type-vector-test-custom-aos multi-type-vector-test-custom-aos)
add_dependencies(check multi-type-vector-test-custom-aos)
