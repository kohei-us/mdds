/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2022 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_soa.hpp"
#include "no_standard_blocks_defs.inl"

// Settting this to 0 should make the multi_type_vector code to NOT include
// the header for the standard element blocks.
#define MDDS_MTV_USE_STANDARD_ELEMENT_BLOCKS 0
#include <mdds/multi_type_vector/soa/main.hpp>

template<typename... Ts>
using mtv_type = mdds::mtv::soa::multi_type_vector<Ts...>;

#include "no_standard_blocks_funcs.inl"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
