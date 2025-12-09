/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#include <mdds/multi_type_vector/aos/main.hpp>

#include <cassert>
#include <sstream>
#include <vector>
#include <deque>

using mtv_type = mdds::mtv::aos::multi_type_vector<mdds::mtv::standard_element_blocks_traits>;

template<typename _ValT>
bool test_cell_insertion(mtv_type& col_db, std::size_t row, _ValT val)
{
    _ValT test;
    col_db.set(row, val);
    col_db.get(row, test);

    if (val == test)
        return true;

    std::cout << "row " << row << ": value stored = " << val << "; value retrieved = " << test << std::endl;
    return false;
}

void mtv_test_construction();
void mtv_test_basic();
void mtv_test_basic_numeric();
void mtv_test_empty_cells();
void mtv_test_misc_types();
void mtv_test_misc_swap();
void mtv_test_misc_equality();
void mtv_test_misc_clone();
void mtv_test_misc_resize();
void mtv_test_misc_value_type();
void mtv_test_misc_block_identifier();
void mtv_test_misc_push_back();
void mtv_test_misc_capacity();
void mtv_test_misc_position_type_end_position();
void mtv_test_misc_block_pos_adjustments();
void mtv_test_erase();
void mtv_test_insert_empty();
void mtv_test_set_cells();
void mtv_test_insert_cells();
void mtv_test_iterators();
void mtv_test_iterators_element_block();
void mtv_test_iterators_mutable_element_block();
void mtv_test_iterators_private_data();
void mtv_test_iterators_set();
void mtv_test_iterators_set_2();
void mtv_test_iterators_insert();
void mtv_test_iterators_insert_empty();
void mtv_test_iterators_set_empty();
void mtv_test_hints_set();
void mtv_test_hints_set_cells();
void mtv_test_hints_insert_cells();
void mtv_test_hints_set_empty();
void mtv_test_hints_insert_empty();
void mtv_test_position();
void mtv_test_position_next();
void mtv_test_position_advance();
void mtv_test_swap_range();
void mtv_test_transfer();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
