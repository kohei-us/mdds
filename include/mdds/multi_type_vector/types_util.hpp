/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2022 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>

namespace mdds { namespace mtv { namespace detail {

template<typename T>
struct has_capacity_method
{
    using yes_type = char;
    using no_type = int;

    template<typename U, typename T::size_type (U::*)() const>
    struct test_has_method
    {
    };

    template<typename U>
    static yes_type test(test_has_method<U, &U::capacity>*);
    template<typename U>
    static no_type test(...);

    using type = std::conditional_t<sizeof(test<T>(0)) == sizeof(yes_type), std::true_type, std::false_type>;
};

template<typename T>
std::size_t get_block_capacity(const T& blk, std::true_type)
{
    return blk.capacity();
}

template<typename T>
std::size_t get_block_capacity(const T&, std::false_type)
{
    return 0;
}

template<typename T>
std::size_t get_block_capacity(const T& blk)
{
    typename has_capacity_method<T>::type v;
    return get_block_capacity(blk, v);
}

template<typename T>
struct has_reserve_method
{
    using yes_type = char;
    using no_type = int;

    template<typename U, void (U::*)(typename T::size_type)>
    struct test_has_method
    {
    };

    template<typename U>
    static yes_type test(test_has_method<U, &U::reserve>*);
    template<typename U>
    static no_type test(...);

    using type = std::conditional_t<sizeof(test<T>(0)) == sizeof(yes_type), std::true_type, std::false_type>;
};

template<typename T>
void reserve(T& blk, typename T::size_type size, std::true_type)
{
    return blk.reserve(size);
}

template<typename T>
void reserve(T&, typename T::size_type, std::false_type)
{}

template<typename T>
void reserve(T& blk, typename T::size_type size)
{
    typename has_reserve_method<T>::type v;
    reserve(blk, size, v);
}

template<typename T>
struct has_shrink_to_fit_method
{
    using yes_type = char;
    using no_type = int;

    template<typename U, void (U::*)()>
    struct test_has_method
    {
    };

    template<typename U>
    static yes_type test(test_has_method<U, &U::shrink_to_fit>*);
    template<typename U>
    static no_type test(...);

    using type = std::conditional_t<sizeof(test<T>(0)) == sizeof(yes_type), std::true_type, std::false_type>;
};

template<typename T>
void shrink_to_fit(T& blk, std::true_type)
{
    return blk.shrink_to_fit();
}

template<typename T>
void shrink_to_fit(T&, std::false_type)
{}

template<typename T>
void shrink_to_fit(T& blk)
{
    typename has_shrink_to_fit_method<T>::type v;
    shrink_to_fit(blk, v);
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

template<typename T, typename = void>
struct has_exec_policy : std::false_type
{
};

template<typename T>
struct has_exec_policy<T, std::void_t<typename T::exec_policy>> : std::true_type
{
};

}}} // namespace mdds::mtv::detail

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
