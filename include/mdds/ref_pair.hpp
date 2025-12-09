/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2020 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

namespace mdds { namespace detail {

template<typename T1, typename T2>
struct ref_pair
{
    using first_type = typename std::add_lvalue_reference<T1>::type;
    using second_type = typename std::add_lvalue_reference<T2>::type;

    first_type first;
    second_type second;

    ref_pair(first_type _first, second_type _second) : first(_first), second(_second)
    {}

    ref_pair(const ref_pair& other) = default;

    bool operator==(const std::pair<typename std::decay<T1>::type, typename std::decay<T2>::type>& other) const
    {
        return first == other.first && second == other.second;
    }

    bool operator!=(const std::pair<typename std::decay<T1>::type, typename std::decay<T2>::type>& other) const
    {
        return !operator==(other);
    }

    bool operator==(const ref_pair& other) const
    {
        return first == other.first && second == other.second;
    }

    bool operator!=(const ref_pair& other) const
    {
        return !operator==(other);
    }

    ref_pair* operator->()
    {
        return this;
    }
};

}} // namespace mdds::detail

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
