set(TEST_NAME multi-type-vector-test-push-emplace-back-aos)
set(TARGET_NAME ${TEST_NAME})

add_executable(${TARGET_NAME} EXCLUDE_FROM_ALL
    test_main.cpp
    test_push_back.cpp
)

target_link_libraries(${TARGET_NAME} PUBLIC test-global)
target_include_directories(${TARGET_NAME} PUBLIC
    ../tc
    ../../../include
)

add_test(${TEST_NAME} ${TARGET_NAME})
add_dependencies(check ${TARGET_NAME})
