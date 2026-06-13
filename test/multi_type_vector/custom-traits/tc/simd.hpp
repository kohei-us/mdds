/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

// SoA-only extension: the SSE2/AVX2 loop-unrolling factors are exercised only
// against the soa variant (the aos driver does not run these). This is a
// deliberate, documented exclusion - not a parity gap - and is gated on the
// build architecture exactly as before.

#include "loop_unrolling.hpp"

template<template<typename> class mtv_tmpl>
void run_simd_tests()
{
#if SIZEOF_VOID_P == 8 && defined(__SSE2__)
    mtv_test_loop_unrolling<mtv_tmpl<trait_lu<lu_factor_t::sse2_x64>>>();
    mtv_test_loop_unrolling<mtv_tmpl<trait_lu<lu_factor_t::sse2_x64_lu4>>>();
    mtv_test_loop_unrolling<mtv_tmpl<trait_lu<lu_factor_t::sse2_x64_lu8>>>();
    mtv_test_loop_unrolling<mtv_tmpl<trait_lu<lu_factor_t::sse2_x64_lu16>>>();
#else
    cout << "SSE2 loop-unrolling tests disabled for the following reasons:" << endl;
#if SIZEOF_VOID_P != 8
    cout << "  * not a 64-bit build" << endl;
#endif
#ifndef __SSE2__
    cout << "  * __SSE2__ not defined" << endl;
#endif
#endif

#if SIZEOF_VOID_P == 8 && defined(__AVX2__)
    mtv_test_loop_unrolling<mtv_tmpl<trait_lu<lu_factor_t::avx2_x64>>>();
    mtv_test_loop_unrolling<mtv_tmpl<trait_lu<lu_factor_t::avx2_x64_lu4>>>();
    mtv_test_loop_unrolling<mtv_tmpl<trait_lu<lu_factor_t::avx2_x64_lu8>>>();
#else
    cout << "AVX2 loop-unrolling tests disabled for the following reasons:" << endl;
#if SIZEOF_VOID_P != 8
    cout << "  * not a 64-bit build" << endl;
#endif
#ifndef __AVX2__
    cout << "  * __AVX2__ not defined" << endl;
#endif
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
