/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "common_types.hpp"

/**
 * Return the payload of the custom_grouped element at position @p i.  Two
 * payloads compare equal only if they are the same object.
 */
template<typename mtv_type>
std::shared_ptr<std::string> payload_of(const mtv_type& con, std::size_t i)
{
    return con.template get<custom_grouped*>(i)->payload;
}

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

/**
 * Verify which types the clone(cloners...) overload accepts.
 */
template<typename mtv_type>
void test_clone_passed_cloner_constraints()
{
    MDDS_TEST_FUNC_SCOPE;

    using grouped_cloner = mdds::mtv::clone_value<custom_grouped*>;
    using ordered_cloner = mdds::mtv::clone_value<custom_ordered*>;

    // A clone_value specialization without an exec_policy is stateful.
    static_assert(mdds::mtv::stateful_clone_value<grouped_cloner>);
    static_assert(mdds::mtv::stateful_clone_value<ordered_cloner>);

    // Anything that is not a clone_value specialization is not.
    static_assert(!mdds::mtv::stateful_clone_value<int>);
    static_assert(!mdds::mtv::stateful_clone_value<custom_grouped>);

    // One or more stateful cloners of distinct types are accepted...
    static_assert(requires(const mtv_type& s, grouped_cloner& c) { s.clone(c); });
    static_assert(requires(const mtv_type& s, grouped_cloner& c, ordered_cloner& o) { s.clone(c, o); });

    // ...but only as lvalues, since the caller is to keep the instance...
    static_assert(!requires(const mtv_type& s) { s.clone(grouped_cloner{}); });

    // ...and a non-cloner is rejected.
    static_assert(!requires(const mtv_type& s, int& i) { s.clone(i); });
    static_assert(!requires(const mtv_type& s, custom_grouped& v) { s.clone(v); });
}

/**
 * Verify that a passed cloner instance preserves payload sharing across the
 * blocks of one container, which the default per-block instance does not.
 */
template<typename mtv_type>
void test_clone_passed_cloner_split_group()
{
    MDDS_TEST_FUNC_SCOPE;

    auto p1 = std::make_shared<std::string>("group 1");

    // One group split across two blocks by an unrelated block in between.
    mtv_type store;
    store.push_back(new custom_grouped{p1, 1});
    store.push_back(new custom_grouped{p1, 1});
    store.push_back(new custom_ordered{5});
    store.push_back(new custom_grouped{p1, 1});
    store.push_back(new custom_grouped{p1, 1});

    {
        // Default: one cloner instance per block, so the group gets two
        // cloned payloads.
        auto cloned = store.clone();
        TEST_ASSERT(payload_of(cloned, 0) == payload_of(cloned, 1));
        TEST_ASSERT(payload_of(cloned, 3) == payload_of(cloned, 4));
        TEST_ASSERT(payload_of(cloned, 0) != payload_of(cloned, 3));
    }

    {
        // Passed instance: one cloner for both blocks, so the group gets one
        // cloned payload.
        mdds::mtv::clone_value<custom_grouped*> cloner;
        auto cloned = store.clone(cloner);
        TEST_ASSERT(payload_of(cloned, 0) == payload_of(cloned, 1));
        TEST_ASSERT(payload_of(cloned, 0) == payload_of(cloned, 3));
        TEST_ASSERT(payload_of(cloned, 0) == payload_of(cloned, 4));
        TEST_ASSERT(payload_of(cloned, 0) != p1);
        TEST_ASSERT(*payload_of(cloned, 0) == "group 1");

        // The block in between is cloned as usual.
        TEST_ASSERT(cloned.template get<custom_ordered*>(2)->id == 5);
        TEST_ASSERT(cloned.template get<custom_ordered*>(2)->clone_seq == 0);

        // The source keeps referencing the original payload.
        TEST_ASSERT(payload_of(store, 0) == p1);
        TEST_ASSERT(payload_of(store, 3) == p1);

        // The passed instance holds the one cloned payload.
        TEST_ASSERT(cloner.group_payloads.size() == 1u);
        TEST_ASSERT(cloner.group_payloads.at(1) == payload_of(cloned, 0));
    }
}

/**
 * Verify that one cloner instance passed to the clone() calls of two
 * containers preserves payload sharing across the two clones.
 */
template<typename mtv_type>
void test_clone_passed_cloner_two_containers()
{
    MDDS_TEST_FUNC_SCOPE;

    auto p1 = std::make_shared<std::string>("group 1");

    mtv_type store1;
    store1.push_back(new custom_grouped{p1, 1});
    store1.push_back(new custom_grouped{p1, 1});

    mtv_type store2;
    store2.template push_back<custom_num>(2.5);
    store2.push_back(new custom_grouped{p1, 1});
    store2.push_back(new custom_grouped{p1, 1});

    {
        // Default: each clone() call clones the payload for itself.
        auto cloned1 = store1.clone();
        auto cloned2 = store2.clone();
        TEST_ASSERT(payload_of(cloned1, 0) == payload_of(cloned1, 1));
        TEST_ASSERT(payload_of(cloned2, 1) == payload_of(cloned2, 2));
        TEST_ASSERT(payload_of(cloned1, 0) != payload_of(cloned2, 1));
    }

    {
        // Passed instance: the two clones share one cloned payload.
        mdds::mtv::clone_value<custom_grouped*> cloner;
        auto cloned1 = store1.clone(cloner);
        auto cloned2 = store2.clone(cloner);
        TEST_ASSERT(payload_of(cloned1, 0) == payload_of(cloned1, 1));
        TEST_ASSERT(payload_of(cloned2, 1) == payload_of(cloned2, 2));
        TEST_ASSERT(payload_of(cloned1, 0) == payload_of(cloned2, 1));
        TEST_ASSERT(payload_of(cloned1, 0) != p1);
        TEST_ASSERT(*payload_of(cloned1, 0) == "group 1");
        TEST_ASSERT(cloned2.template get<custom_num>(0).value == 2.5);

        // The sources keep referencing the original payload.
        TEST_ASSERT(payload_of(store1, 0) == p1);
        TEST_ASSERT(payload_of(store2, 1) == p1);
    }
}

/**
 * Verify that a passed cloner visits the elements in stored order across the
 * blocks of one container and across containers, and that blocks it does not
 * interact with are cloned as usual: a copyable block, a block with a user
 * clone_block specialization that takes the block only, and a block with a
 * clone_value specialization of another type.
 */
template<typename mtv_type>
void test_clone_passed_cloner_ordered()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type store1;
    store1.push_back(new custom_ordered{0});
    store1.push_back(new custom_ordered{1});
    store1.template push_back<custom_num>(1.5);
    store1.push_back(new custom_str1{"str1"}); // user clone_block
    store1.push_back(new custom_str2{"str2"}); // stateless clone_value
    store1.push_back_empty();
    store1.push_back(new custom_ordered{10});
    store1.push_back(new custom_ordered{11});
    TEST_ASSERT(store1.block_size() == 6);

    mtv_type store2;
    store2.push_back(new custom_ordered{20});
    store2.push_back(new custom_ordered{21});
    TEST_ASSERT(store2.block_size() == 1);

    // the cloner keeps track of the total number of cloned values
    mdds::mtv::clone_value<custom_ordered*> cloner;
    auto cloned1 = store1.clone(cloner);
    auto cloned2 = store2.clone(cloner);

    TEST_ASSERT(cloner.visits == 6); // 6 custom_ordered values were cloned
    TEST_ASSERT(cloned1 == store1);
    TEST_ASSERT(cloned2 == store2);

    // check the custom_ordered value at specified pos vs specified id and
    // sequence index (seq)
    auto check = [](const mtv_type& con, std::size_t pos, int id, int seq) {
        const custom_ordered* p = con.template get<custom_ordered*>(pos);
        return p->id == id && p->clone_seq == seq;
    };

    // The stamped sequence continues from the first block into the second...
    TEST_ASSERT(check(cloned1, 0, 0, 0));
    TEST_ASSERT(check(cloned1, 1, 1, 1));
    TEST_ASSERT(check(cloned1, 6, 10, 2));
    TEST_ASSERT(check(cloned1, 7, 11, 3));

    // ...and from the first container into the second.
    TEST_ASSERT(check(cloned2, 0, 20, 4));
    TEST_ASSERT(check(cloned2, 1, 21, 5));

    // The other blocks are cloned as usual, into new elements.
    TEST_ASSERT(cloned1.template get<custom_num>(2).value == 1.5);
    TEST_ASSERT(cloned1.template get<custom_str1*>(3)->value == "str1");
    TEST_ASSERT(cloned1.template get<custom_str1*>(3) != store1.template get<custom_str1*>(3));
    TEST_ASSERT(cloned1.template get<custom_str2*>(4)->value == "str2");
    TEST_ASSERT(cloned1.template get<custom_str2*>(4) != store1.template get<custom_str2*>(4));
    TEST_ASSERT(cloned1.is_empty(5));

    // The source elements are never stamped.
    TEST_ASSERT(store1.template get<custom_ordered*>(0)->clone_seq == -1);
    TEST_ASSERT(store1.template get<custom_ordered*>(1)->clone_seq == -1);
    TEST_ASSERT(store1.template get<custom_ordered*>(6)->clone_seq == -1);
    TEST_ASSERT(store1.template get<custom_ordered*>(7)->clone_seq == -1);
    TEST_ASSERT(store2.template get<custom_ordered*>(0)->clone_seq == -1);
    TEST_ASSERT(store2.template get<custom_ordered*>(1)->clone_seq == -1);
}

/**
 * Verify that a block whose clone_value type is not among the passed
 * instances is cloned through a fresh per-block instance, leaving the passed
 * instances untouched, and that with several instances passed each block
 * picks the one of its own type.
 */
template<typename mtv_type>
void test_clone_passed_cloner_other_type()
{
    MDDS_TEST_FUNC_SCOPE;

    auto p1 = std::make_shared<std::string>("group 1");

    mtv_type store;
    store.push_back(new custom_grouped{p1, 1});
    store.push_back(new custom_grouped{p1, 1});
    store.push_back_empty();
    store.push_back(new custom_grouped{p1, 1});
    store.push_back(new custom_ordered{7});
    TEST_ASSERT(store.block_size() == 4);

    {
        // Only an instance of another type passed: the grouped blocks behave
        // as with clone(), one cloned payload per block.
        mdds::mtv::clone_value<custom_ordered*> ordered_cloner;
        auto cloned = store.clone(ordered_cloner);
        TEST_ASSERT(payload_of(cloned, 0) == payload_of(cloned, 1));
        TEST_ASSERT(payload_of(cloned, 0) != payload_of(cloned, 3));
        TEST_ASSERT(payload_of(cloned, 0) != p1);
        TEST_ASSERT(*payload_of(cloned, 0) == "group 1");
        TEST_ASSERT(cloned.template get<custom_ordered*>(4)->clone_seq == 0);
        TEST_ASSERT(ordered_cloner.visits == 1);
    }

    {
        // Instances of both types passed: each block picks its own.
        mdds::mtv::clone_value<custom_grouped*> grouped_cloner;
        mdds::mtv::clone_value<custom_ordered*> ordered_cloner;
        auto cloned = store.clone(grouped_cloner, ordered_cloner);
        TEST_ASSERT(payload_of(cloned, 0) == payload_of(cloned, 1));
        TEST_ASSERT(payload_of(cloned, 0) == payload_of(cloned, 3));
        TEST_ASSERT(cloned.template get<custom_ordered*>(4)->clone_seq == 0);
        TEST_ASSERT(ordered_cloner.visits == 1);
        TEST_ASSERT(grouped_cloner.group_payloads.size() == 1u);
    }
}

/**
 * Verify that a clone throwing mid-way with passed cloners frees the blocks
 * cloned so far, and leaves the passed cloners in the state they reached.
 */
template<typename mtv_type>
void test_clone_passed_cloner_throw_mid_block()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type store;
    store.push_back(new custom_ordered{0});
    store.push_back(new custom_ordered{1});
    store.push_back(new custom_counted{"one"});
    store.push_back(new custom_counted{"two"});
    store.push_back_empty();
    store.push_back(new custom_ordered{10});
    store.push_back(new custom_counted{"three"});
    store.push_back(new custom_counted{"four", true}); // the cloner throws on this one
    store.push_back(new custom_counted{"five"});

    const int baseline = custom_counted::instances;
    TEST_ASSERT(baseline == 5);

    mdds::mtv::clone_value<custom_ordered*> ordered_cloner;
    mdds::mtv::clone_value<custom_counted*> counted_cloner;

    try
    {
        [[maybe_unused]] auto cloned = store.clone(ordered_cloner, counted_cloner);
        TEST_ASSERT(!"exception was not thrown as expected");
    }
    catch (const std::runtime_error&)
    {
        // expected
    }

    // Every clone made before the throw must have been freed...
    TEST_ASSERT(custom_counted::instances == baseline);

    // ...the source must remain intact...
    TEST_ASSERT(store.size() == 9);
    TEST_ASSERT(store.template get<custom_ordered*>(0)->clone_seq == -1);
    TEST_ASSERT(store.template get<custom_ordered*>(1)->clone_seq == -1);
    TEST_ASSERT(store.template get<custom_counted*>(2)->value == "one");
    TEST_ASSERT(store.template get<custom_counted*>(3)->value == "two");
    TEST_ASSERT(store.is_empty(4));
    TEST_ASSERT(store.template get<custom_ordered*>(5)->clone_seq == -1);
    TEST_ASSERT(store.template get<custom_counted*>(6)->value == "three");
    TEST_ASSERT(store.template get<custom_counted*>(7)->value == "four");
    TEST_ASSERT(store.template get<custom_counted*>(8)->value == "five");

    // ...and the blocks were cloned in stored order up to the throw: the
    // passed cloner has visited the three elements of the two blocks before
    // the throwing one.
    TEST_ASSERT(ordered_cloner.visits == 3);

    // Disarm the flagged element; cloning with fresh cloners now succeeds.
    store.template get<custom_counted*>(7)->throw_on_clone = false;

    {
        mdds::mtv::clone_value<custom_ordered*> ordered_cloner2;
        mdds::mtv::clone_value<custom_counted*> counted_cloner2;
        auto cloned = store.clone(ordered_cloner2, counted_cloner2);
        TEST_ASSERT(custom_counted::instances == baseline * 2);
        TEST_ASSERT(ordered_cloner2.visits == 3);
        TEST_ASSERT(cloned.template get<custom_ordered*>(0)->clone_seq == 0);
        TEST_ASSERT(cloned.template get<custom_ordered*>(1)->clone_seq == 1);
        TEST_ASSERT(cloned.template get<custom_counted*>(2)->value == "one");
        TEST_ASSERT(cloned.template get<custom_counted*>(3)->value == "two");
        TEST_ASSERT(cloned.is_empty(4));
        TEST_ASSERT(cloned.template get<custom_ordered*>(5)->clone_seq == 2);
        TEST_ASSERT(cloned.template get<custom_counted*>(6)->value == "three");
        TEST_ASSERT(cloned.template get<custom_counted*>(7)->value == "four");
        TEST_ASSERT(cloned.template get<custom_counted*>(8)->value == "five");
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
