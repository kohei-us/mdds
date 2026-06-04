/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2020 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>
#include <utility>

namespace mdds {

/**
 * Pair of references to two existing values.  It is primarily used as the
 * value type of an iterator that needs to reference a key-value pair stored
 * in a container without copying it.
 *
 * It is comparable with a std::pair that holds the corresponding decayed
 * value types.
 */
template<typename T1, typename T2>
struct ref_pair
{
    using first_type = std::add_lvalue_reference_t<T1>;
    using second_type = std::add_lvalue_reference_t<T2>;

    first_type first;
    second_type second;

    ref_pair(first_type _first, second_type _second) : first(_first), second(_second)
    {}

    ref_pair(const ref_pair& other) = default;

    bool operator==(const std::pair<std::decay_t<T1>, std::decay_t<T2>>& other) const
    {
        return first == other.first && second == other.second;
    }

    bool operator==(const ref_pair& other) const
    {
        return first == other.first && second == other.second;
    }

    ref_pair* operator->()
    {
        return this;
    }
};

} // namespace mdds

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
