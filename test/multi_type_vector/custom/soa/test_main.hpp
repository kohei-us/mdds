/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "common_types.hpp"

#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#include <mdds/multi_type_vector/soa/main.hpp>

template<typename T>
using mtv_template_type = mdds::mtv::soa::multi_type_vector<T>;

using mtv_type = mtv_template_type<user_muser_trait>;
using mtv_fruit_type = mtv_template_type<fruit_trait>;
using mtv3_type = mtv_template_type<muser_fruit_date_trait>;

void mtv_test_misc_types();
void mtv_test_misc_block_identifier();
void mtv_test_misc_custom_block_func1();
void mtv_test_misc_custom_block_func3();
void mtv_test_misc_release();
void mtv_test_misc_construction_with_array();
void mtv_test_basic();
void mtv_test_basic_equality();
void mtv_test_managed_block();
void mtv_test_transfer();
void mtv_test_swap();
void mtv_test_swap_2();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
