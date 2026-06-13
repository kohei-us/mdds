/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "basic.hpp"
#include "construction.hpp"
#include "empty_cells.hpp"
#include "erase.hpp"
#include "hints.hpp"
#include "insert.hpp"
#include "iterators.hpp"
#include "iterators_insert.hpp"
#include "iterators_set.hpp"
#include "iterators_set_empty.hpp"
#include "misc.hpp"
#include "position.hpp"
#include "set.hpp"
#include "swap_range.hpp"
#include "transfer.hpp"

template<typename mtv_type>
void run_all_tests()
{
    mtv_test_construction<mtv_type>();
    mtv_test_basic<mtv_type>();
    mtv_test_basic_numeric<mtv_type>();
    mtv_test_empty_cells<mtv_type>();
    mtv_test_misc_types<mtv_type>();
    mtv_test_misc_swap<mtv_type>();
    mtv_test_misc_equality<mtv_type>();
    mtv_test_misc_clone<mtv_type>();
    mtv_test_misc_resize<mtv_type>();
    mtv_test_misc_value_type<mtv_type>();
    mtv_test_misc_block_identifier<mtv_type>();
    mtv_test_misc_push_back<mtv_type>();
    mtv_test_misc_capacity<mtv_type>();
    mtv_test_misc_position_type_end_position<mtv_type>();
    mtv_test_misc_block_pos_adjustments<mtv_type>();
    mtv_test_erase<mtv_type>();
    mtv_test_insert_empty<mtv_type>();
    mtv_test_set_cells<mtv_type>();
    mtv_test_insert_cells<mtv_type>();
    mtv_test_iterators<mtv_type>();
    mtv_test_iterators_element_block<mtv_type>();
    mtv_test_iterators_mutable_element_block<mtv_type>();
    mtv_test_iterators_private_data<mtv_type>();
    mtv_test_iterators_set<mtv_type>();
    mtv_test_iterators_set_2<mtv_type>();
    mtv_test_iterators_insert<mtv_type>();
    mtv_test_iterators_insert_empty<mtv_type>();
    mtv_test_iterators_set_empty<mtv_type>();
    mtv_test_hints_set<mtv_type>();
    mtv_test_hints_set_cells<mtv_type>();
    mtv_test_hints_insert_cells<mtv_type>();
    mtv_test_hints_set_empty<mtv_type>();
    mtv_test_hints_insert_empty<mtv_type>();
    mtv_test_position<mtv_type>();
    mtv_test_position_next<mtv_type>();
    mtv_test_position_advance<mtv_type>();
    mtv_test_swap_range<mtv_type>();
    mtv_test_transfer<mtv_type>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
