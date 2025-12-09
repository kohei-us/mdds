# SPDX-FileCopyrightText: 2025 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

add_executable(runtime-env EXCLUDE_FROM_ALL main.cpp)
target_include_directories(runtime-env PRIVATE ${PROJECT_SOURCE_DIR}/include)
