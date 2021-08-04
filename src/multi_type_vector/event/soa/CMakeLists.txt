
add_executable(multi-type-vector-test-event-soa EXCLUDE_FROM_ALL
    test_block_counter.cpp
    test_block_init.cpp
    test_main.cpp
    ../../../test_global.cpp
)

target_include_directories(multi-type-vector-test-event-soa PUBLIC
    ../tc
    ../../../include
)

add_test(multi-type-vector-test-event-soa multi-type-vector-test-event-soa)
add_dependencies(check multi-type-vector-test-event-soa)
