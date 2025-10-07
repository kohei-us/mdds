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

#pragma once

#include <string>
#include <vector>

#include <mdds/multi_type_vector/types.hpp>
#include <mdds/multi_type_vector/macro.hpp>
#include <mdds/multi_type_vector/util.hpp>

/**
 * Stored in default element block.
 */
struct custom_num
{
    double value;

    custom_num() : value(0.0)
    {}
    custom_num(double v) : value(v)
    {}
    operator double() const
    {
        return value;
    }

    bool operator==(const custom_num& other) const
    {
        return value == other.value;
    }

    bool operator!=(const custom_num& other) const
    {
        return !operator==(other);
    }
};

/**
 * Stored in managed non-copyable block with specialized clone_block to allow
 * cloning.
 */
struct custom_str1
{
    std::string value;

    operator const char*() const
    {
        return value.c_str();
    }

    bool operator==(const custom_str1& other) const
    {
        return value == other.value;
    }

    bool operator!=(const custom_str1& other) const
    {
        return !operator==(other);
    }
};

/**
 * Stored in managed non-copyable block with specialized clone_value to allow
 * cloning.
 */
struct custom_str2
{
    std::string value;

    operator const char*() const
    {
        return value.c_str();
    }

    bool operator==(const custom_str2& other) const
    {
        return value == other.value;
    }

    bool operator!=(const custom_str2& other) const
    {
        return !operator==(other);
    }
};

/**
 * Stored in managed non-copyable block with no specialization for cloning.
 */
struct custom_noclone
{
    std::string value;

    operator const char*() const
    {
        return value.c_str();
    }

    bool operator==(const custom_noclone& other) const
    {
        return value == other.value;
    }

    bool operator!=(const custom_noclone& other) const
    {
        return !operator==(other);
    }
};

constexpr mdds::mtv::element_t block1_id = mdds::mtv::element_type_user_start;
constexpr mdds::mtv::element_t block2_id = mdds::mtv::element_type_user_start + 1;
constexpr mdds::mtv::element_t block3_id = mdds::mtv::element_type_user_start + 2;
constexpr mdds::mtv::element_t block4_id = mdds::mtv::element_type_user_start + 3;

using block1_type = mdds::mtv::default_element_block<block1_id, custom_num, std::vector>;
using block2_type = mdds::mtv::noncopyable_managed_element_block<block2_id, custom_str1, std::vector>;
using block3_type = mdds::mtv::noncopyable_managed_element_block<block3_id, custom_str2, std::vector>;
using block4_type = mdds::mtv::noncopyable_managed_element_block<block4_id, custom_noclone, std::vector>;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(custom_num, block1_id, custom_num{}, block1_type)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(custom_str1, block2_id, nullptr, block2_type)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(custom_str2, block3_id, nullptr, block3_type)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(custom_noclone, block4_id, nullptr, block4_type)

namespace mdds { namespace mtv {

template<>
struct clone_block<block2_type>
{
    block2_type* operator()(const block2_type& src) const
    {
        std::unique_ptr<block2_type> cloned_blk = std::make_unique<block2_type>();
        auto cloned(src.store());
        std::transform(
            cloned.begin(), cloned.end(), cloned.begin(), [](const custom_str1* p) { return new custom_str1(*p); });

        cloned_blk->store().swap(cloned);
        return cloned_blk.release();
    }
};

template<>
struct clone_value<custom_str2*>
{
    custom_str2* operator()(const custom_str2* src) const
    {
        return new custom_str2(*src);
    }
};

}} // namespace mdds::mtv

struct user_traits : mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<block1_type, block2_type, block3_type, block4_type>;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
