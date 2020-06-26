/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2020 Kohei Yoshida
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

#ifndef INCLUDED_MDDS_REF_PAIR_HPP
#define INCLUDED_MDDS_REF_PAIR_HPP

#include <type_traits>

namespace mdds { namespace detail {

template<typename T1, typename T2>
struct ref_pair
{
    using first_type = typename std::add_lvalue_reference<T1>::type;
    using second_type = typename std::add_lvalue_reference<T2>::type;

    first_type first;
    second_type second;

    ref_pair(first_type _first, second_type _second) :
        first(_first), second(_second) {}

    ref_pair(const ref_pair& other) = default;

    bool operator== (const std::pair<typename std::decay<T1>::type, typename std::decay<T2>::type>& other) const
    {
        return first == other.first && second == other.second;
    }

    bool operator!= (const std::pair<typename std::decay<T1>::type, typename std::decay<T2>::type>& other) const
    {
        return !operator==(other);
    }

    bool operator== (const ref_pair& other) const
    {
        return first == other.first && second == other.second;
    }

    bool operator!= (const ref_pair& other) const
    {
        return !operator==(other);
    }

    ref_pair* operator->()
    {
        return this;
    }
};

}}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
