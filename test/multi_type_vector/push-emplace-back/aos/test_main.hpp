/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "common_types.hpp"

#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#include <mdds/multi_type_vector/aos/main.hpp>

using mtv_type = mdds::mtv::aos::multi_type_vector<user_traits>;

void test_push_back_copy();
void test_push_back_move();
void test_emplace_back();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
