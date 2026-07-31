/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include "common_types.hpp"

/**
 * When every other sharer is gone, a detach reclaims the shared blocks
 * instead of cloning them: the block and element pointers stay identical and
 * no element block events fire.
 */
template<typename mtv_type>
void test_cow_last_sharer_reclaim()
{
    MDDS_TEST_FUNC_SCOPE;

    {
        // The other sharer leaves by detaching.
        mtv_type src;
        src.template push_back<custom_num>(1.1);
        src.push_back(new custom_str1{"a"});

        auto borrower = src.clone();
        TEST_ASSERT(same_block_pointers(src, borrower));

        borrower.detach();
        TEST_ASSERT(!borrower.is_shared());
        TEST_ASSERT(src.is_shared()); // src still references the shared store

        const mdds::mtv::base_element_block* p_block = std::as_const(src).begin()->data;
        const custom_str1* p_elem = src.template get<custom_str1*>(1);
        const int acquired = src.event_handler().acquired;
        const int released = src.event_handler().released;

        src.detach(); // last sharer detaches
        TEST_ASSERT(!src.is_shared());

        // Reclaimed, not cloned: pointer identity is preserved and no events
        // fired.
        TEST_ASSERT(std::as_const(src).begin()->data == p_block);
        TEST_ASSERT(src.template get<custom_str1*>(1) == p_elem);
        TEST_ASSERT(src.event_handler().acquired == acquired);
        TEST_ASSERT(src.event_handler().released == released);

        TEST_ASSERT(src.template get<custom_num>(0).value == 1.1);
        TEST_ASSERT(src.template get<custom_str1*>(1)->value == "a");
    }

    {
        // The other sharer leaves by being destroyed.
        mtv_type src;
        src.template push_back<custom_num>(1.1);
        src.push_back(new custom_str1{"a"});

        {
            auto borrower = src.clone();
            TEST_ASSERT(same_block_pointers(src, borrower));
        } // borrower destroyed; src is now the last sharer

        TEST_ASSERT(src.is_shared());

        const mdds::mtv::base_element_block* p_block = std::as_const(src).begin()->data;
        const custom_str1* p_elem = src.template get<custom_str1*>(1);
        const int acquired = src.event_handler().acquired;
        const int released = src.event_handler().released;

        src.detach(); // last sharer detaches
        TEST_ASSERT(!src.is_shared());

        TEST_ASSERT(std::as_const(src).begin()->data == p_block);
        TEST_ASSERT(src.template get<custom_str1*>(1) == p_elem);
        TEST_ASSERT(src.event_handler().acquired == acquired);
        TEST_ASSERT(src.event_handler().released == released);

        TEST_ASSERT(src.template get<custom_num>(0).value == 1.1);
        TEST_ASSERT(src.template get<custom_str1*>(1)->value == "a");
    }
}

/**
 * The reclaim also applies to the automatic detach a mutating method
 * performs: a write by the last sharer mutates the reclaimed blocks in
 * place.
 */
template<typename mtv_type>
void test_cow_last_sharer_reclaim_on_write()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type src;
    src.template push_back<custom_num>(1.1);
    src.template push_back<custom_num>(1.2);

    {
        mtv_type borrower(src);
        TEST_ASSERT(same_block_pointers(src, borrower));
    } // borrower destroyed; src is now the last sharer

    TEST_ASSERT(src.is_shared());

    const mdds::mtv::base_element_block* p_block = std::as_const(src).begin()->data;
    const int acquired = src.event_handler().acquired;
    const int released = src.event_handler().released;

    src.template set<custom_num>(0, 9.9); // auto detach
    TEST_ASSERT(!src.is_shared());

    TEST_ASSERT(std::as_const(src).begin()->data == p_block);
    TEST_ASSERT(src.event_handler().acquired == acquired);
    TEST_ASSERT(src.event_handler().released == released);

    TEST_ASSERT(src.template get<custom_num>(0).value == 9.9);
    TEST_ASSERT(src.template get<custom_num>(1).value == 1.2);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
