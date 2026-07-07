/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

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
 * Assert that invoking @p call throws mdds::mtv::shared_block_error.
 */
template<typename Func>
void assert_shared_block_error(Func&& call)
{
    bool threw = false;
    try
    {
        call();
    }
    catch (const mdds::mtv::shared_block_error&)
    {
        threw = true;
    }
    TEST_ASSERT(threw);
}

/**
 * The four hint-less release() / release_range() overloads throw
 * shared_block_error while borrowing and succeed after detach().
 */
template<typename mtv_type>
void test_cow_release_requires_sole_ownership()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type src;
    src.template push_back<custom_num>(1.1);
    src.template push_back<custom_num>(1.2);
    src.template push_back<custom_num>(1.3);

    mtv_type borrower(src);
    TEST_ASSERT(same_block_pointers(src, borrower)); // both borrow the shared blocks

    assert_shared_block_error([&] { borrower.template release<custom_num>(0); });
    assert_shared_block_error([&] {
        custom_num v;
        borrower.release(0, v);
    });
    assert_shared_block_error([&] { borrower.release(); });
    assert_shared_block_error([&] { borrower.release_range(0, 1); });
    TEST_ASSERT(same_block_pointers(src, borrower)); // still borrowing, nothing released

    // After detaching, the borrower owns its blocks and the calls succeed.
    borrower.detach();
    custom_num released;
    borrower.release(0, released);
    TEST_ASSERT(released.value == 1.1);
    borrower.release_range(1, 2);
}

/**
 * The two pos_hint release() / release_range() overloads throw
 * shared_block_error just like the previous test case.
 *
 * The test case uses an invalidated non-const iterator as a position hint to
 * trigger the exception since that's the only way to reach the site.  Yes,
 * this is ugly, but since the code throws before it has the chance to use the
 * hint, this is acceptable for now.
 */
template<typename mtv_type>
void test_cow_release_pos_hint_requires_sole_ownership()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type src;
    src.template push_back<custom_num>(1.1);
    src.template push_back<custom_num>(1.2);

    // Grab a non-const iterator while sole owner, then re-borrow via a copy to
    // invalidate it.
    typename mtv_type::iterator hint = src.template set<custom_num>(0, 1.1);
    mtv_type borrower(src);
    TEST_ASSERT(same_block_pointers(src, borrower)); // both borrow; hint is now stale

    assert_shared_block_error([&] {
        custom_num v;
        src.release(hint, 0, v);
    });
    assert_shared_block_error([&] { src.release_range(hint, 0, 1); });
    TEST_ASSERT(same_block_pointers(src, borrower)); // still borrowing, nothing released
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
