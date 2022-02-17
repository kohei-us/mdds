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

#include "common_types.hpp"

#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#include <mdds/multi_type_vector/soa/main.hpp>
#include <mdds/multi_type_vector_custom_func1.hpp>
#include <mdds/multi_type_vector_custom_func2.hpp>
#include <mdds/multi_type_vector_custom_func3.hpp>

template<typename T>
using mtv_template_type = mdds::mtv::soa::multi_type_vector<T>;

using mtv_type = mtv_template_type<mdds::mtv::custom_block_func2<user_cell_block, muser_cell_block>>;
using mtv_fruit_type = mtv_template_type<mdds::mtv::custom_block_func1<fruit_block>>;
using mtv3_type = mtv_template_type<mdds::mtv::custom_block_func3<muser_cell_block, fruit_block, date_block>>;

void mtv_test_misc_types();
void mtv_test_misc_block_identifier();
void mtv_test_misc_custom_block_func1();
void mtv_test_misc_custom_block_func3();
void mtv_test_misc_release();
void mtv_test_misc_construction_with_array();
void mtv_test_basic();
void mtv_test_basic_equality();
void mtv_test_managed_block();
void mtv_test_transfer();
void mtv_test_swap();
void mtv_test_swap_2();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
