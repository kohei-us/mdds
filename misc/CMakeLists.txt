# SPDX-FileCopyrightText: 2025 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

add_executable(misc-mtv-copy-blocks EXCLUDE_FROM_ALL
    mtv_copy_blocks.cpp
)

add_executable(misc-mtv-clone-noncopyable EXCLUDE_FROM_ALL
    mtv_clone_noncopyable.cpp
)

target_link_libraries(misc-mtv-copy-blocks PUBLIC test-global)
target_link_libraries(misc-mtv-clone-noncopyable PUBLIC test-global)
