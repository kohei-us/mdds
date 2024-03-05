
add_executable(segment-tree-test-main EXCLUDE_FROM_ALL
    test_main.cpp
    test_move_value.cpp
)

target_link_libraries(segment-tree-test-main PRIVATE test-global)

add_test(segment-tree-test-main segment-tree-test-main)

add_dependencies(check
    segment-tree-test-main
)
