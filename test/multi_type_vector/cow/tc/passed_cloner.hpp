/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include "common_types.hpp"

/**
 * With copy-on-write enabled, clone(cloners...) only borrows the blocks and
 * leaves the passed cloners untouched, since nothing is cloned until the
 * clone detaches.  Without copy-on-write, clone(cloners...) clones through
 * the passed instance right away.
 */
template<typename cow_mtv_type, typename non_cow_mtv_type>
void test_cow_clone_passed_cloner()
{
    MDDS_TEST_FUNC_SCOPE;

    auto p1 = std::make_shared<std::string>("group 1");

    {
        // COW on
        cow_mtv_type src;
        src.push_back(new custom_grouped{p1, 1});
        src.template push_back<custom_num>(1.1);
        src.push_back(new custom_grouped{p1, 1});

        mdds::mtv::clone_value<custom_grouped*> cloner;

        auto borrower = src.clone(cloner);
        TEST_ASSERT(borrower.is_shared());
        TEST_ASSERT(same_block_pointers(src, borrower));
        TEST_ASSERT(borrower.template get<custom_grouped*>(0)->payload == p1);
        TEST_ASSERT(borrower.template get<custom_grouped*>(2)->payload == p1);
        TEST_ASSERT(cloner.group_payloads.empty());

        // Same with an event handler passed.
        auto borrower2 = src.clone(cow_event{}, cloner);
        TEST_ASSERT(borrower2.is_shared());
        TEST_ASSERT(same_block_pointers(src, borrower2));
        TEST_ASSERT(cloner.group_payloads.empty()); // nothing is cloned
    }

    {
        // COW off
        non_cow_mtv_type src;
        src.push_back(new custom_grouped{p1, 1});
        src.template push_back<custom_num>(1.1);
        src.push_back(new custom_grouped{p1, 1});

        mdds::mtv::clone_value<custom_grouped*> cloner;

        // The group split across two blocks ends on one cloned payload.
        auto cloned = src.clone(cloner);
        TEST_ASSERT(!cloned.is_shared());
        TEST_ASSERT(!same_block_pointers(src, cloned));
        TEST_ASSERT(cloner.group_payloads.size() == 1u);
        TEST_ASSERT(cloned.template get<custom_grouped*>(0)->payload != p1);
        TEST_ASSERT(
            cloned.template get<custom_grouped*>(0)->payload == cloned.template get<custom_grouped*>(2)->payload);
        TEST_ASSERT(*cloned.template get<custom_grouped*>(0)->payload == "group 1");

        // Same with an event handler passed; the same instance continues, so
        // the second clone shares the payload cloned for the first.
        auto cloned2 = src.clone(cow_event{}, cloner);
        TEST_ASSERT(!cloned2.is_shared());
        TEST_ASSERT(cloner.group_payloads.size() == 1u);
        TEST_ASSERT(
            cloned2.template get<custom_grouped*>(0)->payload == cloned.template get<custom_grouped*>(0)->payload);
        TEST_ASSERT(
            cloned2.template get<custom_grouped*>(2)->payload == cloned.template get<custom_grouped*>(0)->payload);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
