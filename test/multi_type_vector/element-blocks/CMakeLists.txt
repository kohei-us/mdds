set(TEST_NAME multi-type-vector-test-element-blocks)
set(TARGET_NAME ${TEST_NAME})

add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL
    test_main.cpp
    test_std_vector_bool.cpp
    test_store_types.cpp
)

target_link_libraries(${TARGET_NAME} PUBLIC test-global)

add_test(${TEST_NAME} ${TARGET_NAME})
add_dependencies(check ${TARGET_NAME})
