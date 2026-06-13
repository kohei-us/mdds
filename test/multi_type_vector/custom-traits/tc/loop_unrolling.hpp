/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <string>
#include <vector>

using mdds::mtv::lu_factor_t;

template<lu_factor_t F>
struct trait_lu : public mdds::mtv::standard_element_blocks_traits
{
    using event_func = mdds::mtv::empty_event_func;

    constexpr static lu_factor_t loop_unrolling = F;
};

template<typename mtv_type>
void mtv_test_loop_unrolling()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type db(5, std::string("test"));

    // keep inserting blocks of alternating types at position 0 to force block
    // position adjustments.

    for (int i = 0; i < 50; ++i)
    {
        db.insert_empty(0, 2);

        std::vector<int8_t> values(2, 89);
        db.insert(0, values.begin(), values.end());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
