/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "common_types.hpp"

/**
 * Under COW the non-const iterator accessors are gated, but a position hint
 * requires no iterator access to seed: a default-constructed hint combined
 * with the feed-back pattern makes the hinted mutators fully usable.  A
 * hinted write on a borrower detaches it like any other write.
 */
template<typename mtv_type>
void test_cow_hinted_mutation()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type src;
    src.template push_back<custom_num>(1.1);
    src.template push_back<custom_num>(1.2);
    src.template push_back<custom_num>(1.3);

    mtv_type borrower(src);
    TEST_ASSERT(same_block_pointers(src, borrower)); // both borrow the shared blocks

    mdds::mtv::position_hint hint;
    hint = borrower.template set<custom_num>(hint, 0, 2.1);
    TEST_ASSERT(!same_block_pointers(src, borrower)); // the hinted write detached the borrower

    hint = borrower.template set<custom_num>(hint, 1, 2.2);
    hint = borrower.template set<custom_num>(hint, 2, 2.3);

    TEST_ASSERT(borrower.template get<custom_num>(0).value == 2.1);
    TEST_ASSERT(borrower.template get<custom_num>(1).value == 2.2);
    TEST_ASSERT(borrower.template get<custom_num>(2).value == 2.3);

    TEST_ASSERT(src.template get<custom_num>(0).value == 1.1);
    TEST_ASSERT(src.template get<custom_num>(1).value == 1.2);
    TEST_ASSERT(src.template get<custom_num>(2).value == 1.3);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
