/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2012 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "common_types.hpp"

#include "basic.hpp"
#include "managed_block.hpp"
#include "misc.hpp"
#include "swap.hpp"
#include "transfer.hpp"

template<template<typename> class mtv_tmpl>
void run_all_tests()
{
    using mtv_type = mtv_tmpl<user_muser_traits>;
    using mtv_fruit_type = mtv_tmpl<fruit_traits>;
    using mtv3_type = mtv_tmpl<muser_fruit_date_traits>;

    mtv_test_misc_types<mtv_type, mtv_fruit_type>();
    mtv_test_misc_block_identifier<mtv_type>();
    mtv_test_misc_custom_block_func1<mtv_fruit_type>();
    mtv_test_misc_custom_block_func3<mtv3_type>();
    mtv_test_misc_release<mtv_type>();
    mtv_test_misc_construction_with_array<mtv_type>();
    mtv_test_basic<mtv_type>();
    mtv_test_basic_equality<mtv_type>();
    mtv_test_managed_block<mtv_type>();
    mtv_test_transfer<mtv_type>();
    mtv_test_swap<mtv3_type>();
    mtv_test_swap_2<mtv3_type>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
