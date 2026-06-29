/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "common_types.hpp"

/**
 * Scenario 4: destroy the source while a borrower still lives, then read and
 * mutate the borrower.  The shared blocks must stay alive until the last
 * borrower releases them, with no dangling read and no double-free.
 *
 * This needs to be run under the check-memcheck target.
 */
template<typename mtv_type>
void test_cow_lifetime()
{
    MDDS_TEST_FUNC_SCOPE;

    {
        // Copyable blocks: the borrower outlives the source it was copied from.
        mtv_type borrower;
        {
            mtv_type src;
            src.template push_back<custom_num>(1.1);
            src.template push_back<custom_num>(1.2);

            borrower = src;
            TEST_ASSERT(same_block_pointers(src, borrower)); // borrowing
        } // src destroyed while borrower still borrows the shared blocks

        // Reading after the source is gone touches the still-live shared blocks.
        TEST_ASSERT(borrower.template get<custom_num>(0).value == 1.1);
        TEST_ASSERT(borrower.template get<custom_num>(1).value == 1.2);

        // The borrower's handler saw one acquire per shared block at copy time
        // and nothing has been released on it yet.
        TEST_ASSERT(borrower.event_handler().live() == static_cast<int>(borrower.block_size()));

        // Mutating now detaches from the holder (the borrower is its sole
        // referrer), which frees the originals exactly once.
        borrower.template set<custom_num>(0, 9.9);
        TEST_ASSERT(borrower.template get<custom_num>(0).value == 9.9);
        TEST_ASSERT(borrower.template get<custom_num>(1).value == 1.2);
    }

    {
        // Non-copyable managed blocks via clone(): the same lifetime guarantee
        // must hold when the shared blocks own heap pointees freed through
        // clone_value / clone_block.
        mtv_type borrower;
        {
            mtv_type src;
            src.template push_back<custom_num>(2.1);
            src.push_back(new custom_str1{"str1"});
            src.push_back(new custom_str2{"str2"});

            borrower = src.clone();
            TEST_ASSERT(same_block_pointers(src, borrower)); // borrowing
        } // src destroyed; borrower still borrows the managed blocks

        TEST_ASSERT(borrower.template get<custom_num>(0).value == 2.1);
        TEST_ASSERT(borrower.template get<custom_str1*>(1)->value == "str1");
        TEST_ASSERT(borrower.template get<custom_str2*>(2)->value == "str2");

        // First write detaches: the managed pointees are deep-cloned and the
        // borrowed originals are freed once by the holder.
        borrower.template set<custom_num>(0, 5.5);
        TEST_ASSERT(borrower.template get<custom_num>(0).value == 5.5);
        TEST_ASSERT(borrower.template get<custom_str1*>(1)->value == "str1");
        TEST_ASSERT(borrower.template get<custom_str2*>(2)->value == "str2");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
