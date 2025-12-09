/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#include <mdds/multi_type_vector/aos/main.hpp>

#include <iostream>
#include <vector>

template<typename Traits>
using mtv_template_type = mdds::mtv::aos::multi_type_vector<Traits>;

void mtv_test_block_counter();
void mtv_test_block_counter_clone();
void mtv_test_block_init();
void mtv_test_swap();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
