/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "common_types.hpp"

/**
 * A COW copy / clone / copy-assignment shares the source's element blocks
 * rather than deep-copying them.  Sharing is observed via element block
 * pointer identity right after the operation.
 */
template<typename mtv_type>
void test_cow_share()
{
    MDDS_TEST_FUNC_SCOPE;

    {
        // Copy construction and copy-assignment require copyable blocks only; a
        // non-copyable block makes the copy ctor throw (see
        // test_cow_noncopyable).
        mtv_type src;
        src.template push_back<custom_num>(1.1);
        src.template push_back<custom_num>(1.2);

        mtv_type copied(src);
        TEST_ASSERT(src == copied);
        TEST_ASSERT(same_block_pointers(src, copied)); // shared, not duplicated

        mtv_type assigned;
        assigned = src;
        TEST_ASSERT(src == assigned);
        TEST_ASSERT(same_block_pointers(src, assigned));
    }

    {
        // clone() shares too, and additionally supports non-copyable blocks.
        mtv_type src;
        src.template push_back<custom_num>(1.1);
        src.push_back(new custom_str1{"str1"});
        src.push_back(new custom_str2{"str2"});

        auto cloned = src.clone();
        TEST_ASSERT(src == cloned);
        TEST_ASSERT(same_block_pointers(src, cloned));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
