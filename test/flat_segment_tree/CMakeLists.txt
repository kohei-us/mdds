
add_executable(flat-segment-tree-test-main EXCLUDE_FROM_ALL
    test_main.cpp
)

target_link_libraries(flat-segment-tree-test-main PRIVATE test-global)

add_test(flat-segment-tree-test-main flat-segment-tree-test-main)

add_dependencies(check
    flat-segment-tree-test-main
)
