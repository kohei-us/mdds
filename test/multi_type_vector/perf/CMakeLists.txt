set(TEST_NAME multi-type-vector-test-perf)
set(TARGET_NAME ${TEST_NAME})

add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL
    test_main.cpp
)

target_link_libraries(${TARGET_NAME} PUBLIC test-global)
