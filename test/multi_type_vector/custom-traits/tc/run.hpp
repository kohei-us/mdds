/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "loop_unrolling.hpp"

template<template<typename> class mtv_tmpl>
void run_all_tests()
{
    {
        using mtv_type = mtv_tmpl<trait_lu<lu_factor_t::none>>;
        mtv_test_loop_unrolling<mtv_type>();
    }

    {
        using mtv_type = mtv_tmpl<trait_lu<lu_factor_t::lu4>>;
        mtv_test_loop_unrolling<mtv_type>();
    }

    {
        using mtv_type = mtv_tmpl<trait_lu<lu_factor_t::lu8>>;
        mtv_test_loop_unrolling<mtv_type>();
    }

    {
        using mtv_type = mtv_tmpl<trait_lu<lu_factor_t::lu16>>;
        mtv_test_loop_unrolling<mtv_type>();
    }

    {
        using mtv_type = mtv_tmpl<trait_lu<lu_factor_t::lu32>>;
        mtv_test_loop_unrolling<mtv_type>();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
