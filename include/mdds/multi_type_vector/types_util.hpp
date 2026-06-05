/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2022 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <concepts>
#include <vector>

namespace mdds { namespace mtv { namespace detail {

template<typename T>
concept has_capacity_method = requires(const T& blk) {
    { blk.capacity() } -> std::same_as<typename T::size_type>;
};

template<typename T>
std::size_t get_block_capacity(const T& blk)
{
    if constexpr (has_capacity_method<T>)
        return blk.capacity();
    else
        return 0;
}

template<typename T>
concept has_reserve_method = requires(T& blk, typename T::size_type size) {
    { blk.reserve(size) } -> std::same_as<void>;
};

template<typename T>
void reserve(T& blk, typename T::size_type size)
{
    if constexpr (has_reserve_method<T>)
        blk.reserve(size);
}

template<typename T>
concept has_shrink_to_fit_method = requires(T& blk) {
    { blk.shrink_to_fit() } -> std::same_as<void>;
};

template<typename T>
void shrink_to_fit(T& blk)
{
    if constexpr (has_shrink_to_fit_method<T>)
        blk.shrink_to_fit();
}

template<typename T>
struct is_std_vector_bool_store
{
    using type = std::false_type;
};

template<>
struct is_std_vector_bool_store<std::vector<bool>>
{
    using type = std::true_type;
};

template<typename Blk>
struct has_std_vector_bool_store
{
    using type = typename is_std_vector_bool_store<typename Blk::store_type>::type;
};

template<typename T>
concept has_exec_policy = requires { typename T::exec_policy; };

}}} // namespace mdds::mtv::detail

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
