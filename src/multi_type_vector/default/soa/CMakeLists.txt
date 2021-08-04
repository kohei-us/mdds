
add_executable(multi-type-vector-test-default-soa EXCLUDE_FROM_ALL
    test_basic.cpp
    test_construction.cpp
    test_empty_cells.cpp
    test_erase.cpp
    test_hints.cpp
    test_insert.cpp
    test_iterators.cpp
    test_iterators_insert.cpp
    test_iterators_set.cpp
    test_iterators_set_empty.cpp
    test_main.cpp
    test_misc.cpp
    test_position.cpp
    test_set.cpp
    test_swap_range.cpp
    test_transfer.cpp
    ../../../test_global.cpp
)

target_include_directories(multi-type-vector-test-default-soa PUBLIC
    ../tc
    ../../../include
)

add_test(multi-type-vector-test-default-soa multi-type-vector-test-default-soa)
add_dependencies(check multi-type-vector-test-default-soa)
