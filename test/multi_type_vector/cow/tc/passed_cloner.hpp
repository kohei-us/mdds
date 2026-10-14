/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>
#include <utility>

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

/**
 * Two borrowers of two sources whose elements share one payload, detached
 * with one passed instance, end on one cloned payload for both; detached
 * with the plain detach() each, they end on one cloned payload per
 * borrower.
 */
template<typename cow_mtv_type>
void test_cow_detach_passed_cloner()
{
    MDDS_TEST_FUNC_SCOPE;

    auto p1 = std::make_shared<std::string>("group 1");

    cow_mtv_type src1;
    src1.push_back(new custom_grouped{p1, 1});
    src1.template push_back<custom_num>(1.1);
    src1.push_back(new custom_grouped{p1, 1});

    cow_mtv_type src2;
    src2.push_back(new custom_grouped{p1, 1});
    src2.push_back(new custom_grouped{p1, 1});

    auto payload_of = [](const cow_mtv_type& con, std::size_t i) {
        return con.template get<custom_grouped*>(i)->payload;
    };

    {
        // Plain detach() on each borrower: one cloned payload per borrower.
        auto borrower1 = src1.clone();
        auto borrower2 = src2.clone();
        borrower1.detach();
        borrower2.detach();
        TEST_ASSERT(payload_of(borrower1, 0) != p1);
        TEST_ASSERT(payload_of(borrower2, 0) != p1);
        TEST_ASSERT(payload_of(borrower1, 0) != payload_of(borrower2, 0));
    }

    {
        // Detach with one passed instance: the blocks are cloned through it
        // and both borrowers end up with one cloned payload.
        mdds::mtv::clone_value<custom_grouped*> cloner;
        auto borrower1 = src1.clone(cloner);
        auto borrower2 = src2.clone(cloner);
        TEST_ASSERT(cloner.group_payloads.empty());

        borrower1.detach(cloner);
        TEST_ASSERT(!borrower1.is_shared());
        TEST_ASSERT(!same_block_pointers(src1, borrower1));
        TEST_ASSERT(cloner.group_payloads.size() == 1u);
        TEST_ASSERT(payload_of(borrower1, 0) != p1);
        TEST_ASSERT(payload_of(borrower1, 0) == payload_of(borrower1, 2));
        TEST_ASSERT(*payload_of(borrower1, 0) == "group 1");
        TEST_ASSERT(borrower1.template get<custom_num>(1).value == 1.1);

        borrower2.detach(cloner);
        TEST_ASSERT(!borrower2.is_shared());
        TEST_ASSERT(!same_block_pointers(src2, borrower2));
        TEST_ASSERT(cloner.group_payloads.size() == 1u);
        TEST_ASSERT(payload_of(borrower2, 0) == payload_of(borrower1, 0));
        TEST_ASSERT(payload_of(borrower2, 1) == payload_of(borrower1, 0));

        // The sources keep referencing the original payload.
        TEST_ASSERT(payload_of(src1, 0) == p1);
        TEST_ASSERT(payload_of(src1, 2) == p1);
        TEST_ASSERT(payload_of(src2, 0) == p1);
        TEST_ASSERT(payload_of(src2, 1) == p1);

        // A second detach of a sole owner is a no-op and leaves the passed
        // instance untouched.
        mdds::mtv::clone_value<custom_grouped*> cloner2;
        borrower1.detach(cloner2);
        TEST_ASSERT(cloner2.group_payloads.empty());
        TEST_ASSERT(payload_of(borrower1, 0) == payload_of(borrower2, 0));
    }
}

/**
 * When the detaching container is the last sharer, detach(cloners...)
 * reclaims the blocks instead of cloning them, and leaves the passed
 * instance untouched.  Without copy-on-write, detach(cloners...) is a no-op
 * that leaves the passed instance untouched as well.
 */
template<typename cow_mtv_type, typename non_cow_mtv_type>
void test_cow_detach_passed_cloner_reclaim()
{
    MDDS_TEST_FUNC_SCOPE;

    auto p1 = std::make_shared<std::string>("group 1");

    {
        // COW on
        cow_mtv_type src;
        src.push_back(new custom_grouped{p1, 1});
        src.push_back(new custom_grouped{p1, 1});

        auto borrower = src.clone();
        borrower.detach(); // the other sharer leaves
        TEST_ASSERT(src.is_shared()); // src still references the shared store

        const mdds::mtv::base_element_block* p_block = std::as_const(src).begin()->data;
        const custom_grouped* p_elem = src.template get<custom_grouped*>(0);
        const int acquired = src.event_handler().acquired;
        const int released = src.event_handler().released;

        mdds::mtv::clone_value<custom_grouped*> cloner;
        src.detach(cloner); // last sharer detaches
        TEST_ASSERT(!src.is_shared());

        // Reclaimed, not cloned: pointer identity is preserved, no events
        // fired, and the passed instance was never invoked.
        TEST_ASSERT(std::as_const(src).begin()->data == p_block);
        TEST_ASSERT(src.template get<custom_grouped*>(0) == p_elem);
        TEST_ASSERT(src.template get<custom_grouped*>(0)->payload == p1);
        TEST_ASSERT(src.event_handler().acquired == acquired);
        TEST_ASSERT(src.event_handler().released == released);
        TEST_ASSERT(cloner.group_payloads.empty());
    }

    {
        // COW off
        non_cow_mtv_type src;
        src.push_back(new custom_grouped{p1, 1});
        src.push_back(new custom_grouped{p1, 1});

        const custom_grouped* p_elem = src.template get<custom_grouped*>(0);

        mdds::mtv::clone_value<custom_grouped*> cloner;
        src.detach(cloner); // no-op
        TEST_ASSERT(src.template get<custom_grouped*>(0) == p_elem);
        TEST_ASSERT(src.template get<custom_grouped*>(0)->payload == p1);
        TEST_ASSERT(cloner.group_payloads.empty());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
