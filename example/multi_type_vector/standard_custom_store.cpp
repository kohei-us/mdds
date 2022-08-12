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

//!code-start
#include <mdds/multi_type_vector/types.hpp>
#include <mdds/multi_type_vector/macro.hpp>

#include <string>
#include <deque>

// Define element ID's for the standard element types.
constexpr mdds::mtv::element_t my_doube_type_id = mdds::mtv::element_type_user_start;
constexpr mdds::mtv::element_t my_string_type_id = mdds::mtv::element_type_user_start + 1;

// Define the block types.
using my_double_block = mdds::mtv::default_element_block<my_doube_type_id, double, std::deque>;
using my_string_block = mdds::mtv::default_element_block<my_string_type_id, std::string, std::deque>;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(double, my_doube_type_id, 0.0, my_double_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(std::string, my_string_type_id, std::string(), my_string_block)

// Provide the above before including the multi_type_vector's main header.
#define MDDS_MTV_USE_STANDARD_ELEMENT_BLOCKS 0
#include <mdds/multi_type_vector/soa/main.hpp>

struct my_custom_trait : public mdds::mtv::default_trait
{
    using block_funcs = mdds::mtv::element_block_funcs<my_double_block, my_string_block>;
};

using mtv_type = mdds::mtv::soa::multi_type_vector<my_custom_trait>;

int main() try
{
    mtv_type con(20); // Initialized with 20 empty elements.

    con.set<std::string>(0, "string value");
    con.set<double>(1, 425.1);

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}
//!code-end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

