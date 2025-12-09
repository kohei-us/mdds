/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2024 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "test_global.hpp" // This must be the first header to be included.
#define MDDS_SEGMENT_TREE_DEBUG 1
#include "mdds/segment_tree.hpp"

void st_test_insert_search_removal();
void st_test_invalid_insertion();
void st_test_copy_constructor();
void st_test_equality();
void st_test_clear();
void st_test_duplicate_insertion();
void st_test_search_on_uneven_tree();
void st_test_aggregated_search_results();
void st_test_dense_tree_search();
void st_test_search_on_empty_set();
void st_test_search_iterator_basic();
void st_test_search_iterator_result_check();
void st_test_empty_result_set();
void st_test_non_pointer_data();
void st_test_erase_on_invalid_tree();
void st_test_boundary_keys();

void st_test_move_constructor();
void st_test_move_equality();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
