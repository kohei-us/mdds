/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2021 Kohei Yoshida
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

#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#include <mdds/multi_type_vector.hpp>
#include <mdds/multi_type_vector_trait.hpp>

#include <cassert>
#include <sstream>
#include <vector>
#include <deque>

using mtv_type = mdds::multi_type_vector<mdds::mtv::element_block_func>;

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
void mtv_test_misc_swap();
void mtv_test_misc_equality();
void mtv_test_misc_clone();
void mtv_test_misc_resize();
void mtv_test_erase();
void mtv_test_insert_empty();
void mtv_test_set_cells();
void mtv_test_insert_cells();
void mtv_test_iterators();
void mtv_test_iterators_element_block();
void mtv_test_iterators_mutable_element_block();
void mtv_test_iterators_private_data();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

