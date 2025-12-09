/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2022 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_main.hpp"

#include <mdds/multi_type_vector/types.hpp>
#include <mdds/multi_type_vector/block_funcs.hpp>

#include <vector>
#include <deque>
#include <type_traits>

void mtv_test_element_blocks_std_vector()
{
    stack_printer __stack_printer__(__func__);

    constexpr mdds::mtv::element_t element_type_int8 = mdds::mtv::element_type_user_start + 2;
    using this_block = mdds::mtv::default_element_block<element_type_int8, std::int8_t, std::vector>;

    static_assert(this_block::block_type == element_type_int8);
    static_assert(std::is_same_v<this_block::store_type, std::vector<std::int8_t>>);

    auto* blk = this_block::create_block(10);

    TEST_ASSERT(mdds::mtv::get_block_type(*blk) == this_block::block_type);
    TEST_ASSERT(this_block::size(*blk) == 10u);

    auto cap = this_block::capacity(*blk);
    TEST_ASSERT(cap >= 10u);

    this_block::reserve(*blk, 100u);
    cap = this_block::capacity(*blk);
    TEST_ASSERT(cap >= 100u);

    this_block::shrink_to_fit(*blk);

    this_block::delete_block(blk);
}

void mtv_test_element_blocks_std_deque()
{
    stack_printer __stack_printer__(__func__);

    constexpr mdds::mtv::element_t element_type_int8 = mdds::mtv::element_type_user_start + 20;
    using this_block = mdds::mtv::default_element_block<element_type_int8, std::int8_t, std::deque>;

    static_assert(this_block::block_type == element_type_int8);
    static_assert(std::is_same_v<this_block::store_type, std::deque<std::int8_t>>);

    auto* blk = this_block::create_block(10);

    TEST_ASSERT(mdds::mtv::get_block_type(*blk) == this_block::block_type);
    TEST_ASSERT(this_block::size(*blk) == 10u);

    // std::deque does not have a capacity() method, but this should still compile.
    [[maybe_unused]] auto cap = this_block::capacity(*blk);

    // std::deque does not have a reserve() method either.
    this_block::reserve(*blk, 100u);

    // ditto with shrink_to_fit()
    this_block::shrink_to_fit(*blk);

    this_block::delete_block(blk);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
