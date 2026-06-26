/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

#include <mdds/multi_type_vector/types.hpp>
#include <mdds/multi_type_vector/macro.hpp>
#include <mdds/multi_type_vector/util.hpp>

/**
 * Stored in default (copyable) element block.
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
};

constexpr mdds::mtv::element_t block1_id = mdds::mtv::element_type_user_start;
constexpr mdds::mtv::element_t block2_id = mdds::mtv::element_type_user_start + 1;
constexpr mdds::mtv::element_t block3_id = mdds::mtv::element_type_user_start + 2;

using block1_type = mdds::mtv::default_element_block<block1_id, custom_num, std::vector>;
using block2_type = mdds::mtv::noncopyable_managed_element_block<block2_id, custom_str1, std::vector>;
using block3_type = mdds::mtv::noncopyable_managed_element_block<block3_id, custom_str2, std::vector>;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(custom_num, block1_id, custom_num{}, block1_type)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(custom_str1, block2_id, nullptr, block2_type)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(custom_str2, block3_id, nullptr, block3_type)

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

/**
 * Event handler that counts logical block acquisitions and releases.  Mirrors
 * the convention of test/multi_type_vector/event/tc/block_counter.hpp: a copied
 * (or moved-into) handler starts its own count from zero, so each container
 * instance tracks only the events fired on its own handler.
 */
struct cow_event
{
    int acquired = 0;
    int released = 0;

    cow_event() = default;

    cow_event(const cow_event&)
    {}

    int live() const
    {
        return acquired - released;
    }

    void element_block_acquired(const mdds::mtv::base_element_block* /*block*/)
    {
        ++acquired;
    }

    void element_block_released(const mdds::mtv::base_element_block* /*block*/)
    {
        ++released;
    }
};

struct cow_traits : mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<block1_type, block2_type, block3_type>;
    using event_func = cow_event;
    static constexpr bool enable_cow = true;
};

struct non_cow_traits : mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<block1_type, block2_type, block3_type>;
    using event_func = cow_event;
    static constexpr bool enable_cow = false;
};

/**
 * Compare the element-block pointers of two containers slot by slot.  Under COW
 * a borrowing container shares the source's pointers, so this returns true right
 * after a copy/clone and false once one side has detached.
 */
template<typename mtv_type>
bool same_block_pointers(const mtv_type& a, const mtv_type& b)
{
    if (a.block_size() != b.block_size())
        return false;

    auto ia = a.begin();
    auto ib = b.begin();
    for (; ia != a.end(); ++ia, ++ib)
        if (ia->data != ib->data)
            return false;

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
