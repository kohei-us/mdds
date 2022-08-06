/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2022 Kohei Yoshida
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************/

#pragma once

#include <vector>

namespace mdds { namespace mtv { namespace detail {

template<typename T>
struct has_capacity_method
{
    using yes_type = char;
    using no_type = int;

    template<typename U, std::size_t (U::*)() const>
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

}}} // namespace mdds::mtv::detail

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
