
add_executable(runtime-env EXCLUDE_FROM_ALL main.cpp)
target_include_directories(runtime-env PRIVATE ${PROJECT_SOURCE_DIR}/include)
