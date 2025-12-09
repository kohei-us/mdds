/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#include <mdds/multi_type_vector/soa/main.hpp>

template<typename Traits>
using mtv_alias_type = mdds::mtv::soa::multi_type_vector<Traits>;

void mtv_test_loop_unrolling_0();
void mtv_test_loop_unrolling_4();
void mtv_test_loop_unrolling_8();
void mtv_test_loop_unrolling_16();
void mtv_test_loop_unrolling_32();

void mtv_test_loop_unrolling_sse2_x64(); // SoA only
void mtv_test_loop_unrolling_sse2_x64_4(); // SoA only
void mtv_test_loop_unrolling_sse2_x64_8(); // SoA only
void mtv_test_loop_unrolling_sse2_x64_16(); // SoA only

void mtv_test_loop_unrolling_avx2_x64(); // SoA only
void mtv_test_loop_unrolling_avx2_x64_4(); // SoA only
void mtv_test_loop_unrolling_avx2_x64_8(); // SoA only

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
