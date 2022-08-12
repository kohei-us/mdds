/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2022 Kohei Yoshida
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

//!code-start: header
#include <mdds/multi_type_vector.hpp>
//!code-end: header

//!code-start: element-types
constexpr mdds::mtv::element_t custom_value1_type = mdds::mtv::element_type_user_start;
constexpr mdds::mtv::element_t custom_value2_type = mdds::mtv::element_type_user_start + 1;
constexpr mdds::mtv::element_t custom_value3_type = mdds::mtv::element_type_user_start + 2;
//!code-end: element-types

//!code-start: custom-values
struct custom_value1 {};
struct custom_value2 {};

using custom_value1_block = mdds::mtv::default_element_block<custom_value1_type, custom_value1>;
using custom_value2_block = mdds::mtv::default_element_block<custom_value2_type, custom_value2>;
//!code-end: custom-values

//!code-start: custom-values-macro
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(custom_value1, custom_value1_type, custom_value1{}, custom_value1_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(custom_value2, custom_value2_type, custom_value2{}, custom_value2_block)
//!code-end: custom-values-macro

//!code-start: custom-value-ns
namespace ns {

struct custom_value3 {};

using custom_value3_block = mdds::mtv::default_element_block<custom_value3_type, custom_value3>;

// This macro MUST be in the same namespace as that of the value type, in order for
// argument-dependent lookup to work properly during overload resolution.
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(custom_value3, custom_value2_type, custom_value3{}, custom_value3_block)

} // namespace ns
//!code-end: custom-value-ns

//!code-start: trait-and-mtv
struct my_custom_trait : public mdds::mtv::default_trait
{
    using block_funcs = mdds::mtv::element_block_funcs<
        custom_value1_block, custom_value2_block, ns::custom_value3_block>;
};

using mtv_type = mdds::multi_type_vector<my_custom_trait>;
//!code-end: trait-and-mtv

//!code-start: main
int main() try
{
    mtv_type con{}; // initialize it as empty container.

    con.push_back(custom_value1{});
    con.push_back(custom_value2{});
    con.push_back(ns::custom_value3{});

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}
//!code-end: main

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

