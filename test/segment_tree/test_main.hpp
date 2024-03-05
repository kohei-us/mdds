/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2024 Kohei Yoshida
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************/

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

