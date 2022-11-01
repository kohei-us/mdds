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

#include <mdds/multi_type_vector.hpp>

#include <iostream>
#include <vector>
#include <deque>

constexpr mdds::mtv::element_t custom_value1_type = mdds::mtv::element_type_user_start;
constexpr mdds::mtv::element_t custom_value2_type = mdds::mtv::element_type_user_start + 1;
constexpr mdds::mtv::element_t custom_value3_type = mdds::mtv::element_type_user_start + 2;

struct custom_value1 {};
struct custom_value2 {};

//!code-start: blocks-1-2
using custom_value1_block = mdds::mtv::default_element_block<
    custom_value1_type, custom_value1, std::vector>;

using custom_value2_block = mdds::mtv::default_element_block<
    custom_value2_type, custom_value2, std::deque>;
//!code-end: blocks-1-2

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(custom_value1, custom_value1_type, custom_value1{}, custom_value1_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(custom_value2, custom_value2_type, custom_value2{}, custom_value2_block)

namespace ns {

struct custom_value3 {};

//!code-start: block-3
// This implicitly uses mdds::mtv::delayed_delete_vector.
using custom_value3_block = mdds::mtv::default_element_block<
    custom_value3_type, custom_value3>;
//!code-end: block-3

// This macro MUST be in the same namespace as that of the value type, in order for
// argument-dependent lookup to work properly during overload resolution.
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(custom_value3, custom_value2_type, custom_value3{}, custom_value3_block)

} // namespace ns

struct my_custom_traits : public mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<
        custom_value1_block, custom_value2_block, ns::custom_value3_block>;
};

using mtv_type = mdds::multi_type_vector<my_custom_traits>;

int main() try
{
    //!code-start: main
    std::cout << "custom_value1 stored in std::vector? "
        << std::is_same_v<custom_value1_block::store_type, std::vector<custom_value1>>
        << std::endl;

    std::cout << "custom_value2 stored in std::deque? "
        << std::is_same_v<custom_value2_block::store_type, std::deque<custom_value2>>
        << std::endl;

    std::cout << "ns::custom_value3 stored in delayed_delete_vector? "
        << std::is_same_v<ns::custom_value3_block::store_type, mdds::mtv::delayed_delete_vector<ns::custom_value3>>
        << std::endl;
    //!code-end: main

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

