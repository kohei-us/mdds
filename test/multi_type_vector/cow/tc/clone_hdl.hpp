/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "common_types.hpp"

namespace {

/**
 * Event handler that carries an identity in addition to counting logical block
 * acquisitions / releases.  The identity lets a test tell which handler a clone
 * actually installed - a copied handler keeps its identity but starts its own
 * counts from zero (mirroring cow_event's copy semantics).
 */
struct id_event
{
    int id = 0;
    int acquired = 0;
    int released = 0;

    id_event() = default;

    explicit id_event(int id_) : id(id_)
    {}

    id_event(const id_event& r) : id(r.id)
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

template<bool Cow>
struct id_traits : mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<block1_type, block2_type, block3_type>;
    using event_func = id_event;
    static constexpr bool enable_cow = Cow;
};

} // anonymous namespace

/**
 * Test case for the variant of clone(hdl) that takes caller-supplied event
 * handler.
 */
template<typename mtv_type, bool Cow>
void test_cow_clone_hdl_one()
{
    mtv_type src(id_event{1});
    src.template push_back<custom_num>(1.1);
    src.push_back(new custom_str1{"a"});
    src.push_back(new custom_str2{"b"});

    const int n_blocks = static_cast<int>(src.block_size());

    // Building the source fired one acquire per block on its handler.
    TEST_ASSERT(src.event_handler().acquired == n_blocks);
    TEST_ASSERT(src.event_handler().released == 0);

    auto cloned = src.clone(id_event{2});

    // The clone stores the supplied handler (id 2), not a copy of the
    // source's (id 1).
    TEST_ASSERT(cloned.event_handler().id == 2);
    TEST_ASSERT(src.event_handler().id == 1);

    // The new handler should receive event callbacks on the borrowed element
    // blocks.
    TEST_ASSERT(cloned.event_handler().acquired == n_blocks);
    TEST_ASSERT(cloned.event_handler().released == 0);

    // The source's handler is unchanged by the clone.
    TEST_ASSERT(src.event_handler().acquired == n_blocks);
    TEST_ASSERT(src.event_handler().released == 0);

    // First write to the clone.
    cloned.template set<custom_num>(0, 9.9);

    if constexpr (Cow)
    {
        // The clone detached: a logical release then re-acquire fired per block
        // on the clone's own (new) handler.
        TEST_ASSERT(cloned.event_handler().released == n_blocks);
        TEST_ASSERT(cloned.event_handler().acquired == 2 * n_blocks);
    }
    else
    {
        // No detach; the in-place set fires nothing.
        TEST_ASSERT(cloned.event_handler().released == 0);
        TEST_ASSERT(cloned.event_handler().acquired == n_blocks);
    }

    // Either way the total arithmetic of the number of live blocks should be
    // the same COW or non-COW.
    TEST_ASSERT(src.event_handler().id == 1);
    TEST_ASSERT(src.event_handler().live() == n_blocks);
    TEST_ASSERT(src.event_handler().live() == cloned.event_handler().live());
    TEST_ASSERT(cloned.event_handler().id == 2);
}

inline void test_cow_clone_hdl()
{
    MDDS_TEST_FUNC_SCOPE;

    test_cow_clone_hdl_one<mdds::mtv::soa::multi_type_vector<id_traits<true>>, true>();
    test_cow_clone_hdl_one<mdds::mtv::soa::multi_type_vector<id_traits<false>>, false>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
