
add_executable(trie-map-test-main EXCLUDE_FROM_ALL
    test_main.cpp
)

target_link_libraries(trie-map-test-main PRIVATE test-global)

add_test(trie-map-test-main trie-map-test-main)

add_dependencies(check
    trie-map-test-main
)
