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

#include <mdds/multi_type_vector/standard_element_blocks.hpp>
#include <mdds/multi_type_vector/types.hpp>

#include <execution> // inclusion of this header requires linking to libtbb on linux if present
#include <string>

struct seq_policy_traits : mdds::mtv::standard_element_blocks_traits
{
    using exec_policy = std::execution::sequenced_policy;
};

constexpr mdds::mtv::element_t block1_id = mdds::mtv::element_type_user_start;

struct custom_str
{
    std::string value;

    bool operator==(const custom_str& other) const
    {
        return value == other.value;
    }

    bool operator!=(const custom_str& other) const
    {
        return !operator==(other);
    }
};

using block1_type = mdds::mtv::noncopyable_managed_element_block<block1_id, custom_str>;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(custom_str, block1_id, nullptr, block1_type)

struct noncopyable_traits : mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<block1_type>;
    using exec_policy = std::execution::sequenced_policy;
};

namespace mdds { namespace mtv {

template<>
struct clone_value<custom_str*>
{
    custom_str* operator()(const custom_str* src) const
    {
        return new custom_str{src->value};
    }
};

}} // namespace mdds::mtv

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
