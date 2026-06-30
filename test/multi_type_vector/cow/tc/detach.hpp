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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
