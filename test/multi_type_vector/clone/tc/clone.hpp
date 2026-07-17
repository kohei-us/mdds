/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "common_types.hpp"

template<typename mtv_type>
void test_clone()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type store;
    store.template push_back<custom_num>(1.1);
    store.template push_back<custom_num>(1.2);
    store.template push_back<custom_num>(1.3);
    store.push_back_empty();
    store.push_back(new custom_str1{"custom str1 1"});
    store.push_back(new custom_str1{"custom str1 2"});
    store.push_back(new custom_str1{"custom str1 3"});
    store.push_back(new custom_str1{"custom str1 4"});
    store.push_back(new custom_str2{"custom str2 1"});
    store.push_back(new custom_str2{"custom str2 2"});
    store.push_back(new custom_str2{"custom str2 3"});
    store.push_back(new custom_str2{"custom str2 4"});

    auto cloned = store.clone();

    TEST_ASSERT(store == cloned);
}

/**
 * Verify that one cloner instance is used for each cloned block.
 */
template<typename mtv_type>
void test_clone_stateful()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type store;
    store.push_back(new custom_ordered{0});
    store.push_back(new custom_ordered{1});
    store.push_back(new custom_ordered{2});
    store.push_back_empty();
    store.push_back(new custom_ordered{10});
    store.push_back(new custom_ordered{11});

    auto cloned = store.clone();

    TEST_ASSERT(cloned.size() == store.size());
    TEST_ASSERT(cloned.is_empty(3));

    // The source elements are never stamped.
    TEST_ASSERT(store.template get<custom_ordered*>(0)->clone_seq == -1);
    TEST_ASSERT(store.template get<custom_ordered*>(1)->clone_seq == -1);
    TEST_ASSERT(store.template get<custom_ordered*>(2)->clone_seq == -1);
    TEST_ASSERT(store.template get<custom_ordered*>(4)->clone_seq == -1);
    TEST_ASSERT(store.template get<custom_ordered*>(5)->clone_seq == -1);

    auto check = [&cloned](std::size_t pos, int id, int seq) {
        const custom_ordered* p = cloned.template get<custom_ordered*>(pos);
        return p->id == id && p->clone_seq == seq;
    };

    // One stateful cloner instance per block, applied in store order: the
    // stamped sequence increases within a block and restarts on the next block.
    TEST_ASSERT(check(0, 0, 0));
    TEST_ASSERT(check(1, 1, 1));
    TEST_ASSERT(check(2, 2, 2));
    TEST_ASSERT(check(4, 10, 0));
    TEST_ASSERT(check(5, 11, 1));
}

/**
 * Verify that a stateful cloner preserves payload sharing among the cloned
 * elements of the same group, one cloned payload per group per block.
 */
template<typename mtv_type>
void test_clone_group_payload()
{
    MDDS_TEST_FUNC_SCOPE;

    // payloads
    auto p1 = std::make_shared<std::string>("group 1");
    auto p2 = std::make_shared<std::string>("group 2");

    mtv_type store;
    store.push_back(new custom_grouped{p1, 1});
    store.push_back(new custom_grouped{p1, 1});
    store.push_back(new custom_grouped{p2, 2});
    store.push_back(new custom_grouped{p2, 2});
    store.push_back_empty();
    store.push_back(new custom_grouped{p1, 1});

    auto cloned = store.clone();

    auto payload_of = [](const mtv_type& con, std::size_t i) { return con.template get<custom_grouped*>(i)->payload; };

    // Within a block, cloned elements of the same group share one payload...
    TEST_ASSERT(payload_of(cloned, 0) == payload_of(cloned, 1));
    TEST_ASSERT(payload_of(cloned, 2) == payload_of(cloned, 3));
    TEST_ASSERT(payload_of(cloned, 0) != payload_of(cloned, 2));

    // ...different from the source's payload but of equal content...
    TEST_ASSERT(payload_of(cloned, 0) != p1);
    TEST_ASSERT(payload_of(cloned, 2) != p2);
    TEST_ASSERT(*payload_of(cloned, 0) == *p1);
    TEST_ASSERT(*payload_of(cloned, 2) == *p2);

    // ...while the source elements keep referencing the original payloads.
    TEST_ASSERT(payload_of(store, 0) == p1);
    TEST_ASSERT(payload_of(store, 2) == p2);

    // The cloner state is scoped to a block: the group-1 element in the
    // second block gets its own cloned payload.
    TEST_ASSERT(payload_of(cloned, 5) != payload_of(cloned, 0));
    TEST_ASSERT(payload_of(cloned, 5) != p1);
    TEST_ASSERT(*payload_of(cloned, 5) == *p1);
}

template<typename mtv_type>
void test_clone_throw_mid_block()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type store;
    store.push_back(new custom_counted{"one"});
    store.push_back(new custom_counted{"two"});
    store.push_back_empty();
    store.push_back(new custom_counted{"three"});
    store.push_back(new custom_counted{"four", true}); // the cloner throws on this one
    store.push_back(new custom_counted{"five"});

    const int baseline = custom_counted::instances;
    TEST_ASSERT(baseline == 5);

    try
    {
        [[maybe_unused]] auto cloned = store.clone();
        TEST_ASSERT(!"exception was not thrown as expected");
    }
    catch (const std::runtime_error&)
    {
        // expected
    }

    // The first block was cloned in whole and the second block in part
    // before the throw; all of those clones must have been freed...
    TEST_ASSERT(custom_counted::instances == baseline);

    // ...and the source must remain intact.
    TEST_ASSERT(store.size() == 6);
    TEST_ASSERT(store.template get<custom_counted*>(0)->value == "one");
    TEST_ASSERT(store.template get<custom_counted*>(1)->value == "two");
    TEST_ASSERT(store.is_empty(2));
    TEST_ASSERT(store.template get<custom_counted*>(3)->value == "three");
    TEST_ASSERT(store.template get<custom_counted*>(4)->value == "four");
    TEST_ASSERT(store.template get<custom_counted*>(5)->value == "five");

    // Disarm the flagged element; cloning now succeeds.
    store.template get<custom_counted*>(4)->throw_on_clone = false;

    {
        auto cloned = store.clone();
        TEST_ASSERT(custom_counted::instances == baseline * 2);
        TEST_ASSERT(cloned.template get<custom_counted*>(0)->value == "one");
        TEST_ASSERT(cloned.template get<custom_counted*>(1)->value == "two");
        TEST_ASSERT(cloned.is_empty(2));
        TEST_ASSERT(cloned.template get<custom_counted*>(3)->value == "three");
        TEST_ASSERT(cloned.template get<custom_counted*>(4)->value == "four");
        TEST_ASSERT(cloned.template get<custom_counted*>(5)->value == "five");
    }

    // The clone went out of scope and released its elements.
    TEST_ASSERT(custom_counted::instances == baseline);
}

template<typename mtv_type>
void test_noclone()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type store;
    store.push_back(new custom_noclone{"no-clone string"});

    try
    {
        [[maybe_unused]] auto cloned = store.clone();
        TEST_ASSERT(!"exception was not thrown as expected");
    }
    catch (const mdds::mtv::element_block_error&)
    {
        // expected
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
