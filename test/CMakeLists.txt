
add_library(test-global STATIC EXCLUDE_FROM_ALL test_global.cpp)

target_include_directories(test-global PUBLIC
    include
)

set(_TESTS
    multi-type-matrix-test
    multi-type-matrix-test-walk
    point-quad-tree-test
    segment-tree-test
    trie-map-test
)

foreach(_TEST ${_TESTS})
    set(_TEST_FILE ${_TEST}.cpp)
    string(REPLACE "-" "_" _TEST_FILE ${_TEST_FILE})
    add_executable(${_TEST} EXCLUDE_FROM_ALL
        ${_TEST_FILE}
    )
    target_link_libraries(${_TEST} PRIVATE test-global)
    add_test(${_TEST} ${_TEST} func)  # these test programs require a 'func' to be passed as an argument.
endforeach()

add_dependencies(check
    ${_TESTS}
)

add_subdirectory(flat_segment_tree)
add_subdirectory(multi_type_vector)
add_subdirectory(rtree)
add_subdirectory(sorted_string_map)
