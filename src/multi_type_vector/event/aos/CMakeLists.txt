
add_executable(multi-type-vector-test-event-aos EXCLUDE_FROM_ALL
    test_block_counter.cpp
    test_block_init.cpp
    test_main.cpp
    ../../../test_global.cpp
)

target_include_directories(multi-type-vector-test-event-aos PUBLIC
    ../tc
    ../../../include
)

add_test(multi-type-vector-test-event-aos multi-type-vector-test-event-aos)
add_dependencies(check multi-type-vector-test-event-aos)
