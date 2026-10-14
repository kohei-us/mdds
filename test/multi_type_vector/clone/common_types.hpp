/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
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

/**
 * Stored in managed non-copyable block; its stateful cloner stamps each
 * clone with the position within its block at which it got cloned, so that
 * tests can verify the cloning order from the cloned container alone.
 */
struct custom_ordered
{
    int id;
    int clone_seq = -1; // stamped by the cloner; -1 in an original

    bool operator==(const custom_ordered& other) const
    {
        return id == other.id;
    }
};

/**
 * Stored in managed non-copyable block; elements of the same group share one
 * payload, and its stateful clone_value preserves that sharing when cloning
 * a block.
 */
struct custom_grouped
{
    std::shared_ptr<std::string> payload;
    int group;

    bool operator==(const custom_grouped& other) const
    {
        return group == other.group && *payload == *other.payload;
    }
};

/**
 * Stored in managed non-copyable block; counts its live instances, and its
 * cloner throws on an element flagged with throw_on_clone, to exercise
 * exception safety.
 */
struct custom_counted
{
    static inline int instances = 0; // live-instance count; one per program, not per TU

    std::string value;
    bool throw_on_clone = false;

    custom_counted(std::string v, bool t = false) : value(std::move(v)), throw_on_clone(t)
    {
        ++instances;
    }

    custom_counted(const custom_counted& other) : value(other.value), throw_on_clone(other.throw_on_clone)
    {
        ++instances;
    }

    ~custom_counted()
    {
        --instances;
    }

    bool operator==(const custom_counted& other) const
    {
        return value == other.value;
    }
};

constexpr mdds::mtv::element_t num_block_id = mdds::mtv::element_type_user_start;
constexpr mdds::mtv::element_t str1_block_id = mdds::mtv::element_type_user_start + 1;
constexpr mdds::mtv::element_t str2_block_id = mdds::mtv::element_type_user_start + 2;
constexpr mdds::mtv::element_t noclone_block_id = mdds::mtv::element_type_user_start + 3;
constexpr mdds::mtv::element_t ordered_block_id = mdds::mtv::element_type_user_start + 4;
constexpr mdds::mtv::element_t grouped_block_id = mdds::mtv::element_type_user_start + 5;
constexpr mdds::mtv::element_t counted_block_id = mdds::mtv::element_type_user_start + 6;

using num_block_type = mdds::mtv::default_element_block<num_block_id, custom_num, std::vector>;
using str1_block_type = mdds::mtv::noncopyable_managed_element_block<str1_block_id, custom_str1, std::vector>;
using str2_block_type = mdds::mtv::noncopyable_managed_element_block<str2_block_id, custom_str2, std::vector>;
using noclone_block_type = mdds::mtv::noncopyable_managed_element_block<noclone_block_id, custom_noclone, std::vector>;
using ordered_block_type = mdds::mtv::noncopyable_managed_element_block<ordered_block_id, custom_ordered, std::vector>;
using grouped_block_type = mdds::mtv::noncopyable_managed_element_block<grouped_block_id, custom_grouped, std::vector>;
using counted_block_type = mdds::mtv::noncopyable_managed_element_block<counted_block_id, custom_counted, std::vector>;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(custom_num, num_block_id, custom_num{}, num_block_type)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(custom_str1, str1_block_id, nullptr, str1_block_type)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(custom_str2, str2_block_id, nullptr, str2_block_type)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(custom_noclone, noclone_block_id, nullptr, noclone_block_type)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(custom_ordered, ordered_block_id, nullptr, ordered_block_type)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(custom_grouped, grouped_block_id, nullptr, grouped_block_type)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(custom_counted, counted_block_id, nullptr, counted_block_type)

namespace mdds { namespace mtv {

template<>
struct clone_block<str1_block_type>
{
    str1_block_type* operator()(const str1_block_type& src) const
    {
        // Build incrementally into the destination block so that partially
        // cloned elements get freed if a clone throws mid-way.
        std::unique_ptr<str1_block_type> cloned_blk = std::make_unique<str1_block_type>();
        auto& cloned = cloned_blk->store();
        cloned.reserve(src.store().size());
        for (const custom_str1* p : src.store())
            cloned.push_back(new custom_str1(*p));

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

/**
 * Stateful cloner which stamps each clone with its own visit count.
 */
template<>
struct clone_value<custom_ordered*>
{
    int visits = 0;

    custom_ordered* operator()(const custom_ordered* src)
    {
        return new custom_ordered{src->id, visits++};
    }
};

/**
 * Stateful cloner which clones the shared payload only once per group, so
 * that the cloned elements of one group share one new payload.
 */
template<>
struct clone_value<custom_grouped*>
{
    std::map<int, std::shared_ptr<std::string>> group_payloads;

    custom_grouped* operator()(const custom_grouped* src)
    {
        // This inserts a payload only on the first element of each group
        auto [it, inserted] = group_payloads.try_emplace(src->group);
        if (inserted)
            // create a new instance for the cloned block
            it->second = std::make_shared<std::string>(*src->payload);

        return new custom_grouped{it->second, src->group};
    }
};

/**
 * Cloner which throws on a flagged element, to let tests observe the cleanup
 * of a partially cloned block.
 */
template<>
struct clone_value<custom_counted*>
{
    custom_counted* operator()(const custom_counted* src) const
    {
        if (src->throw_on_clone)
            throw std::runtime_error("instructed to throw on clone");

        return new custom_counted(*src);
    }
};

}} // namespace mdds::mtv

struct user_traits : mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<
        num_block_type, str1_block_type, str2_block_type, noclone_block_type, ordered_block_type, grouped_block_type,
        counted_block_type>;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
