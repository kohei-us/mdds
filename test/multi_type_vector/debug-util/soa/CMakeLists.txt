set(TEST_NAME multi-type-vector-debug-util-soa-trace)
set(TARGET_NAME ${TEST_NAME})

add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL
    test_trace.cpp
)

target_link_libraries(${TARGET_NAME} PUBLIC test-global)
target_include_directories(${TARGET_NAME} PUBLIC
    ../../../include
)

add_test(${TEST_NAME} ${TARGET_NAME})
add_dependencies(check ${TARGET_NAME})
