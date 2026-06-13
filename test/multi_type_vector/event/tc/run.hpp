/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2015 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "block_counter.hpp"
#include "block_init.hpp"
#include "swap.hpp"

template<template<typename> class mtv_tmpl>
void run_all_tests()
{
    {
        using mtv_type = mtv_tmpl<eb_counter_traits>;
        mtv_test_block_counter<mtv_type>();
        mtv_test_block_counter_clone<mtv_type>();
    }

    {
        using mtv_type = mtv_tmpl<eb_init_traits>;
        mtv_test_block_init<mtv_type>();
    }

    {
        // now runs for both variants (previously aos-only)
        using mtv_type = mtv_tmpl<traits>;
        mtv_test_swap<mtv_type>();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
