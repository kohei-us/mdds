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

/**
 * is_shared() lifecycle: false on a fresh container, true on both sharers
 * after a COW copy or clone, and back to false only when the queried
 * container itself detaches.
 */
template<typename cow_mtv_type, typename non_cow_mtv_type>
void test_cow_is_shared()
{
    MDDS_TEST_FUNC_SCOPE;

    cow_mtv_type src;
    TEST_ASSERT(!src.is_shared()); // fresh container

    src.template push_back<custom_num>(1.1);
    TEST_ASSERT(!src.is_shared()); // still the sole owner

    {
        cow_mtv_type copied(src);
        TEST_ASSERT(src.is_shared());
        TEST_ASSERT(copied.is_shared());

        auto cloned = src.clone();
        TEST_ASSERT(cloned.is_shared());

        // Detaching one sharer flips only that sharer's state.
        copied.detach();
        TEST_ASSERT(!copied.is_shared());
        TEST_ASSERT(src.is_shared());
        TEST_ASSERT(cloned.is_shared());
    }

    // Every other sharer is gone by now, yet this container keeps
    // referencing the shared store until it detaches itself.
    TEST_ASSERT(src.is_shared());
    src.detach();
    TEST_ASSERT(!src.is_shared());

    // The member is available with copy-on-write disabled, and always
    // returns false.
    non_cow_mtv_type non_cow;
    non_cow.template push_back<custom_num>(1.1);
    non_cow_mtv_type non_cow_copied(non_cow);
    TEST_ASSERT(!non_cow.is_shared());
    TEST_ASSERT(!non_cow_copied.is_shared());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
