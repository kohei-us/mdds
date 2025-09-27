/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2025 Kohei Yoshida
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

#include "test_global.hpp" // This must be the first header to be included.
#include <iostream>
#include <vector>
#include <deque>
#include <type_traits>

#include <mdds/multi_type_vector/types_util.hpp>

struct capacity_size_t
{
    using size_type = std::size_t;

    std::size_t capacity() const
    {
        return 1;
    }
};

struct has_reserve
{
    using size_type = std::size_t;

    void reserve(size_type)
    {}
};

struct has_shrink_to_fit
{
    using size_type = std::size_t;

    void shrink_to_fit()
    {}
};

void test_has_method()
{
    MDDS_TEST_FUNC_SCOPE;

    using mdds::mtv::detail::has_capacity_method;
    static_assert(std::is_same_v<has_capacity_method<std::vector<int>>::type, std::true_type>);
    static_assert(std::is_same_v<has_capacity_method<std::deque<int>>::type, std::false_type>);
    static_assert(std::is_same_v<has_capacity_method<capacity_size_t>::type, std::true_type>);
    static_assert(std::is_same_v<has_capacity_method<has_reserve>::type, std::false_type>);
    static_assert(std::is_same_v<has_capacity_method<has_shrink_to_fit>::type, std::false_type>);

    using mdds::mtv::detail::has_reserve_method;
    static_assert(std::is_same_v<has_reserve_method<std::vector<int>>::type, std::true_type>);
    static_assert(std::is_same_v<has_reserve_method<std::deque<int>>::type, std::false_type>);
    static_assert(std::is_same_v<has_reserve_method<capacity_size_t>::type, std::false_type>);
    static_assert(std::is_same_v<has_reserve_method<has_reserve>::type, std::true_type>);
    static_assert(std::is_same_v<has_reserve_method<has_shrink_to_fit>::type, std::false_type>);

    using mdds::mtv::detail::has_shrink_to_fit_method;
    static_assert(std::is_same_v<has_shrink_to_fit_method<std::vector<int>>::type, std::true_type>);
    static_assert(std::is_same_v<has_shrink_to_fit_method<std::deque<int>>::type, std::true_type>);
    static_assert(std::is_same_v<has_shrink_to_fit_method<capacity_size_t>::type, std::false_type>);
    static_assert(std::is_same_v<has_shrink_to_fit_method<has_reserve>::type, std::false_type>);
    static_assert(std::is_same_v<has_shrink_to_fit_method<has_shrink_to_fit>::type, std::true_type>);
}

int main()
{
    test_has_method();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
