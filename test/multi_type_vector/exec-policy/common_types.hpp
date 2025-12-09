/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

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
constexpr mdds::mtv::element_t block2_id = mdds::mtv::element_type_user_start + 1;

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

struct custom_int
{
    int64_t value;

    bool operator==(const custom_int& other) const
    {
        return value == other.value;
    }

    bool operator!=(const custom_int& other) const
    {
        return !operator==(other);
    }
};

using block1_type = mdds::mtv::noncopyable_managed_element_block<block1_id, custom_str>;
using block2_type = mdds::mtv::noncopyable_managed_element_block<block2_id, custom_int>;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(custom_str, block1_id, nullptr, block1_type)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(custom_int, block2_id, nullptr, block2_type)

struct noncopyable_traits : mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<block1_type, block2_type>;
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

template<>
struct clone_value<custom_int*>
{
    using exec_policy = std::execution::sequenced_policy;

    custom_int* operator()(const custom_int* src) const
    {
        return new custom_int{src->value};
    }
};

}} // namespace mdds::mtv

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
