/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>
#include <utility>

#include "common_types.hpp"

/**
 * The non-const begin() / end() / rbegin() / rend() / position() accessors
 * exist under COW and provide the same mutable types as the non-COW
 * counterpart does.
 */
template<typename cow_mtv_type, typename non_cow_mtv_type>
void test_cow_mutable_accessor_types()
{
    MDDS_TEST_FUNC_SCOPE;

    cow_mtv_type cow;
    static_assert(std::is_same_v<decltype(cow.begin()), typename cow_mtv_type::iterator>);
    static_assert(std::is_same_v<decltype(cow.end()), typename cow_mtv_type::iterator>);
    static_assert(std::is_same_v<decltype(cow.rbegin()), typename cow_mtv_type::reverse_iterator>);
    static_assert(std::is_same_v<decltype(cow.rend()), typename cow_mtv_type::reverse_iterator>);
    static_assert(std::is_same_v<decltype(cow.position(0)), typename cow_mtv_type::position_type>);

    non_cow_mtv_type non_cow;
    static_assert(std::is_same_v<decltype(non_cow.begin()), typename non_cow_mtv_type::iterator>);
    static_assert(std::is_same_v<decltype(non_cow.end()), typename non_cow_mtv_type::iterator>);
    static_assert(std::is_same_v<decltype(non_cow.rbegin()), typename non_cow_mtv_type::reverse_iterator>);
    static_assert(std::is_same_v<decltype(non_cow.rend()), typename non_cow_mtv_type::reverse_iterator>);
    static_assert(std::is_same_v<decltype(non_cow.position(0)), typename non_cow_mtv_type::position_type>);
}

/**
 * Each of the six non-const accessors throws shared_block_error while the
 * container is borrowing shared blocks, and the throw leaves the container
 * untouched: still sharing, contents intact, the other sharer unaffected.
 */
template<typename mtv_type>
void test_cow_mutable_access_throws_while_shared()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type src;
    src.template push_back<custom_num>(1.1);
    src.template push_back<custom_num>(1.2);

    mtv_type borrower(src);
    TEST_ASSERT(same_block_pointers(src, borrower)); // both borrow the shared blocks

    assert_shared_block_error([&] { borrower.position(0); });
    assert_shared_block_error([&] { borrower.position(mdds::mtv::position_hint{}, 0); });
    assert_shared_block_error([&] { borrower.begin(); });
    assert_shared_block_error([&] { borrower.end(); });
    assert_shared_block_error([&] { borrower.rbegin(); });
    assert_shared_block_error([&] { borrower.rend(); });

    // The throws left the borrower untouched: still sharing, contents intact.
    TEST_ASSERT(borrower.is_shared());
    TEST_ASSERT(same_block_pointers(src, borrower));
    TEST_ASSERT(borrower.template get<custom_num>(0).value == 1.1);
    TEST_ASSERT(borrower.template get<custom_num>(1).value == 1.2);

    // The other sharer is unaffected too.
    TEST_ASSERT(src.is_shared());
    TEST_ASSERT(src.template get<custom_num>(0).value == 1.1);
    TEST_ASSERT(src.template get<custom_num>(1).value == 1.2);
}

/**
 * After an explicit detach() the six accessors succeed, while the other
 * sharer keeps referencing the original blocks with its content intact.
 */
template<typename mtv_type>
void test_cow_mutable_access_after_detach()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type src;
    src.template push_back<custom_num>(1.1);
    src.template push_back<custom_num>(1.2);

    mtv_type borrower(src);
    const mdds::mtv::base_element_block* p_src_block = std::as_const(src).begin()->data;

    borrower.detach();
    TEST_ASSERT(!borrower.is_shared());

    auto pos = borrower.position(0);
    TEST_ASSERT(pos.first == borrower.begin());
    TEST_ASSERT(pos.second == 0);

    pos = borrower.position(mdds::mtv::position_hint{}, 1);
    TEST_ASSERT(pos.second == 1);

    auto it = borrower.begin();
    TEST_ASSERT(it != borrower.end());
    TEST_ASSERT(it->size == 2);

    auto rit = borrower.rbegin();
    TEST_ASSERT(rit != borrower.rend());
    TEST_ASSERT(rit->size == 2);

    // The other sharer still references the original blocks, content intact.
    TEST_ASSERT(src.is_shared());
    TEST_ASSERT(std::as_const(src).begin()->data == p_src_block);
    TEST_ASSERT(src == borrower);
}

/**
 * The const accessors never throw nor detach: both sharers keep sharing the
 * blocks throughout.
 */
template<typename mtv_type>
void test_cow_const_access_never_detaches()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type src;
    src.template push_back<custom_num>(1.1);
    src.template push_back<custom_num>(1.2);

    mtv_type borrower(src);
    TEST_ASSERT(same_block_pointers(src, borrower)); // both borrow the shared blocks

    auto it = borrower.cbegin();
    TEST_ASSERT(it != borrower.cend());
    TEST_ASSERT(it->size == 2);

    auto rit = borrower.crbegin();
    TEST_ASSERT(rit != borrower.crend());
    TEST_ASSERT(rit->size == 2);

    // std::as_const forces the const overloads on a non-const container.
    TEST_ASSERT(std::as_const(borrower).begin() != std::as_const(borrower).end());
    TEST_ASSERT(std::as_const(borrower).rbegin() != std::as_const(borrower).rend());

    auto pos = std::as_const(borrower).position(0);
    TEST_ASSERT(pos.second == 0);

    // No const access detached anything.
    TEST_ASSERT(borrower.is_shared());
    TEST_ASSERT(src.is_shared());
    TEST_ASSERT(same_block_pointers(src, borrower));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
