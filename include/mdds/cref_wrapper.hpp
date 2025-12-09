/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2024 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <functional>
#include <type_traits>

namespace mdds { namespace detail {

/**
 * Custom const reference wrapper with an equality operator and a hash
 * adaptor.
 */
template<typename T>
class cref_wrapper
{
    std::reference_wrapper<std::add_const_t<T>> m_value;

public:
    cref_wrapper(const T& v) noexcept : m_value(std::cref(v))
    {}

    const T& get() const noexcept
    {
        return m_value;
    }

    bool operator==(const cref_wrapper& r) const
    {
        return get() == r.get();
    }

    bool operator!=(const cref_wrapper& r) const
    {
        return !operator==(r);
    }

    struct hash
    {
        std::size_t operator()(const cref_wrapper& v) const noexcept(noexcept(std::hash<T>{}(v.get())))
        {
            // NB: hash value must be based on the wrapped value, else two
            // identical values located in different memory locations may end up
            // in different buckets, and the lookup may fail.
            return std::hash<T>{}(v.get());
        }
    };
};

}} // namespace mdds::detail

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
