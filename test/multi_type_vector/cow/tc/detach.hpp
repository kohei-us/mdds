/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include "common_types.hpp"

/**
 * The first write to a borrowing container deep-copies (detaches) its store;
 * the other sharer is left untouched; subsequent writes do not re-clone.
 */
template<typename mtv_type>
void test_cow_detach_on_write()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type src;
    src.template push_back<custom_num>(1.1);
    src.template push_back<custom_num>(1.2);
    src.template push_back<custom_num>(1.3);

    mtv_type copied(src);
    TEST_ASSERT(same_block_pointers(src, copied)); // borrowing

    // First write detaches: the two containers no longer share blocks.
    copied.template set<custom_num>(0, 9.9);
    TEST_ASSERT(!same_block_pointers(src, copied));

    // The untouched sharer is unchanged; the written one holds the new value.
    TEST_ASSERT(src.template get<custom_num>(0).value == 1.1);
    TEST_ASSERT(copied.template get<custom_num>(0).value == 9.9);

    // A subsequent write does not re-clone: the element block pointer is stable.
    const mdds::mtv::base_element_block* p_before = copied.begin()->data;
    copied.template set<custom_num>(1, 8.8);
    TEST_ASSERT(copied.begin()->data == p_before);
}

/**
 * Mutating either sharer leaves the other intact (both-side independence), in
 * both detach directions.
 */
template<typename mtv_type>
void test_cow_both_side()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type a;
    a.template push_back<custom_num>(1.1);
    a.template push_back<custom_num>(1.2);

    mtv_type b(a);
    TEST_ASSERT(same_block_pointers(a, b));

    // Mutating the source detaches it and leaves the borrower intact.
    a.template set<custom_num>(0, 7.7);
    TEST_ASSERT(a.template get<custom_num>(0).value == 7.7);
    TEST_ASSERT(b.template get<custom_num>(0).value == 1.1);

    // Mutating a fresh borrower detaches it and leaves the source intact.
    mtv_type c(b);
    c.template set<custom_num>(1, 3.3);
    TEST_ASSERT(c.template get<custom_num>(1).value == 3.3);
    TEST_ASSERT(b.template get<custom_num>(1).value == 1.2);
}

/**
 * The release() overloads that return the released element detach first like
 * any other mutating method, then succeed, leaving the other sharer
 * unaffected.
 */
template<typename mtv_type>
void test_cow_release_detaches()
{
    MDDS_TEST_FUNC_SCOPE;

    {
        // release(pos) detaches, then provides the value.
        mtv_type src;
        src.template push_back<custom_num>(1.1);
        src.template push_back<custom_num>(1.2);
        src.template push_back<custom_num>(1.3);

        mtv_type borrower(src);
        TEST_ASSERT(same_block_pointers(src, borrower)); // both borrow the shared blocks

        const mdds::mtv::base_element_block* p_shared = std::as_const(src).begin()->data;

        custom_num v = borrower.template release<custom_num>(0);
        TEST_ASSERT(v.value == 1.1);
        TEST_ASSERT(!borrower.is_shared()); // the release detached the borrower
        TEST_ASSERT(borrower.is_empty(0));

        // The detach cloned the blocks: the borrower's remaining element block
        // is a new instance while the other sharer keeps the original.
        TEST_ASSERT(std::as_const(borrower).rbegin()->data != p_shared);
        TEST_ASSERT(std::as_const(src).begin()->data == p_shared);

        // The other sharer is unaffected.
        TEST_ASSERT(src.template get<custom_num>(0).value == 1.1);
        TEST_ASSERT(src.template get<custom_num>(1).value == 1.2);
        TEST_ASSERT(src.template get<custom_num>(2).value == 1.3);
    }

    {
        // release(pos, value) detaches the same way.
        mtv_type src;
        src.template push_back<custom_num>(1.1);
        src.template push_back<custom_num>(1.2);
        src.template push_back<custom_num>(1.3);

        mtv_type borrower(src);
        TEST_ASSERT(same_block_pointers(src, borrower));

        custom_num v;
        borrower.release(0, v);
        TEST_ASSERT(v.value == 1.1);
        TEST_ASSERT(!borrower.is_shared());
        TEST_ASSERT(borrower.is_empty(0));

        TEST_ASSERT(src.template get<custom_num>(0).value == 1.1);
        TEST_ASSERT(src.template get<custom_num>(1).value == 1.2);
        TEST_ASSERT(src.template get<custom_num>(2).value == 1.3);
    }
}

/**
 * The release() and release_range() overloads that do not return the
 * released elements require sole ownership: they throw shared_block_error
 * while the container is borrowing shared blocks, and succeed after an
 * explicit detach().
 */
template<typename mtv_type>
void test_cow_nonreturning_release_requires_sole_ownership()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type src;
    src.template push_back<custom_num>(1.1);
    src.template push_back<custom_num>(1.2);
    src.template push_back<custom_num>(1.3);

    mtv_type borrower(src);
    TEST_ASSERT(same_block_pointers(src, borrower)); // both borrow the shared blocks

    assert_shared_block_error([&] { borrower.release(); });
    assert_shared_block_error([&] { borrower.release_range(0, 1); });
    assert_shared_block_error([&] { borrower.release_range(mdds::mtv::position_hint{}, 0, 1); });

    // The throws left the borrower untouched: still sharing, contents intact.
    TEST_ASSERT(borrower.is_shared());
    TEST_ASSERT(same_block_pointers(src, borrower));
    TEST_ASSERT(borrower.template get<custom_num>(0).value == 1.1);
    TEST_ASSERT(borrower.template get<custom_num>(1).value == 1.2);
    TEST_ASSERT(borrower.template get<custom_num>(2).value == 1.3);

    // After an explicit detach the calls succeed.
    borrower.detach();
    borrower.release_range(0, 1);
    TEST_ASSERT(borrower.is_empty(0));
    TEST_ASSERT(borrower.is_empty(1));
    borrower.release();
    TEST_ASSERT(borrower.empty());

    // The other sharer is unaffected.
    TEST_ASSERT(src.template get<custom_num>(0).value == 1.1);
    TEST_ASSERT(src.template get<custom_num>(1).value == 1.2);
    TEST_ASSERT(src.template get<custom_num>(2).value == 1.3);
}

/**
 * The pos_hint release(pos, value) overload detaches just like the hint-less
 * ones; the hint value remains valid across the detach, which the hinted
 * release_range() call after the detach demonstrates.
 */
template<typename mtv_type>
void test_cow_release_pos_hint_detaches()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type src;
    src.template push_back<custom_num>(1.1);
    src.template push_back<custom_num>(1.2);

    mdds::mtv::position_hint hint = src.template set<custom_num>(0, 1.1);
    mtv_type borrower(src);
    TEST_ASSERT(same_block_pointers(src, borrower)); // both borrow the shared blocks

    const mdds::mtv::base_element_block* p_shared = std::as_const(borrower).begin()->data;

    custom_num v;
    hint = src.release(hint, 0, v);
    TEST_ASSERT(v.value == 1.1);
    TEST_ASSERT(!src.is_shared()); // the hinted release detached the container
    TEST_ASSERT(src.is_empty(0));

    // The detach cloned the blocks: src's remaining element block is a new
    // instance while the borrower keeps referencing the shared original.
    TEST_ASSERT(std::as_const(src).rbegin()->data != p_shared);
    TEST_ASSERT(std::as_const(borrower).begin()->data == p_shared);

    // The hint fed back from the pre-detach set() is still usable post-detach.
    src.release_range(hint, 1, 1);
    TEST_ASSERT(src.is_empty(1));

    // The other sharer is unaffected.
    TEST_ASSERT(borrower.template get<custom_num>(0).value == 1.1);
    TEST_ASSERT(borrower.template get<custom_num>(1).value == 1.2);
}

/**
 * A release on a sharing container provides the post-detach clone of a
 * managed element, not the originally stored object; the original remains
 * co-owned by the other sharer.
 */
template<typename mtv_type>
void test_cow_release_clone_identity()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type src;
    src.push_back(new custom_str1{"a"});
    src.push_back(new custom_str1{"b"});

    auto borrower = src.clone();
    TEST_ASSERT(same_block_pointers(src, borrower));

    const custom_str1* p_original = src.template get<custom_str1*>(0);

    custom_str1* released = nullptr;
    borrower.release(0, released);
    TEST_ASSERT(released != nullptr);
    TEST_ASSERT(released->value == "a");
    TEST_ASSERT(released != p_original); // the post-detach clone, not the original
    TEST_ASSERT(!borrower.is_shared());

    // The original is still stored in the other sharer.
    TEST_ASSERT(src.template get<custom_str1*>(0) == p_original);
    TEST_ASSERT(p_original->value == "a");

    delete released;
}

/**
 * detach() on a non-sharing container is a no-op: no blocks get cloned, hence
 * no element block events fire.
 */
template<typename mtv_type>
void test_cow_detach_noop_when_sole_owner()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type db;
    db.template push_back<custom_num>(1.1);
    db.push_back(new custom_str1{"a"});

    const int acquired = db.event_handler().acquired;
    const int released = db.event_handler().released;

    db.detach();

    TEST_ASSERT(!db.is_shared());
    TEST_ASSERT(db.event_handler().acquired == acquired);
    TEST_ASSERT(db.event_handler().released == released);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
