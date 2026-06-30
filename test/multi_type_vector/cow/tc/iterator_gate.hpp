/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

#include "common_types.hpp"

/**
 * Under COW the non-const begin() / end() / rbegin() / rend() / position()
 * accessors are not accessible, so calling them on a non-const container
 * should hand out a const iterator where the non-COW counterpart would have
 * handed out a non-const one.
 */
template<typename cow_mtv_type, typename non_cow_mtv_type>
void test_cow_iterator_gate()
{
    MDDS_TEST_FUNC_SCOPE;

    // COW-enabled variant should return const iterators.
    cow_mtv_type cow;
    static_assert(std::is_same_v<decltype(cow.begin()), typename cow_mtv_type::const_iterator>);
    static_assert(std::is_same_v<decltype(cow.end()), typename cow_mtv_type::const_iterator>);
    static_assert(std::is_same_v<decltype(cow.rbegin()), typename cow_mtv_type::const_reverse_iterator>);
    static_assert(std::is_same_v<decltype(cow.rend()), typename cow_mtv_type::const_reverse_iterator>);
    static_assert(std::is_same_v<decltype(cow.position(0)), typename cow_mtv_type::const_position_type>);

    // COW-disabled variant should return mutable iterators.
    non_cow_mtv_type non_cow;
    static_assert(std::is_same_v<decltype(non_cow.begin()), typename non_cow_mtv_type::iterator>);
    static_assert(std::is_same_v<decltype(non_cow.rbegin()), typename non_cow_mtv_type::reverse_iterator>);
    static_assert(std::is_same_v<decltype(non_cow.position(0)), typename non_cow_mtv_type::position_type>);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
