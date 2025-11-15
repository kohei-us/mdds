/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2024 Kohei Yoshida
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
