/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2023 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_main.hpp"

#include <mdds/multi_type_vector/types.hpp>

#include <vector>
#include <iostream>

void mtv_test_element_blocks_range()
{
    stack_printer __stack_printer__(__func__);

    constexpr mdds::mtv::element_t element_type_int32 = mdds::mtv::element_type_user_start + 2;
    using block_type = mdds::mtv::default_element_block<element_type_int32, std::int32_t, std::vector>;

    const std::vector<std::int8_t> values = {1, 2, 3, 4, 5};
    auto blk = block_type(values.begin(), values.end());
    const auto& crblk = blk;

    TEST_ASSERT(block_type::size(blk) == 5u);

    {
        auto it = block_type::begin(blk), it_end = block_type::end(blk);
        TEST_ASSERT(std::distance(it, it_end) == 5u);
        TEST_ASSERT(*it++ == 1);
        TEST_ASSERT(*it++ == 2);
        TEST_ASSERT(*it++ == 3);
        TEST_ASSERT(*it++ == 4);
        TEST_ASSERT(*it++ == 5);
        TEST_ASSERT(it == it_end);
    }

    {
        // immutable range access
        auto it = block_type::begin(crblk);
        for (auto v : block_type::range(crblk))
        {
            std::cout << "v=" << v << std::endl;
            TEST_ASSERT(v == *it++);
        }

        TEST_ASSERT(it == block_type::end(crblk));
    }

    {
        // mutable range access
        auto it = block_type::begin(blk);
        for (auto v : block_type::range(blk))
        {
            std::cout << "v=" << v << std::endl;
            TEST_ASSERT(v == *it);
            *it = v + 2;
            ++it;
        }

        TEST_ASSERT(it == block_type::end(blk));

        it = block_type::begin(blk);
        TEST_ASSERT(*it++ == 3);
        TEST_ASSERT(*it++ == 4);
        TEST_ASSERT(*it++ == 5);
        TEST_ASSERT(*it++ == 6);
        TEST_ASSERT(*it++ == 7);
        TEST_ASSERT(it == block_type::end(blk));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
