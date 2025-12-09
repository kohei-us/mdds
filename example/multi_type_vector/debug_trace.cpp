/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2022 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

//!code-start: header
#define MDDS_MULTI_TYPE_VECTOR_TRACE 1
#include <mdds/multi_type_vector/soa/main.hpp>

#include <iostream>
//!code-end: header

//!code-start: types
namespace mtv = mdds::mtv;

struct mtv_traits : public mtv::standard_element_blocks_traits
{
    static void trace(const mtv::trace_method_properties_t& props)
    {
        std::cout << "function:" << std::endl
                  << "  name: " << props.function_name << std::endl
                  << "  args: " << props.function_args << std::endl;
    }
};

using mtv_type = mtv::soa::multi_type_vector<mtv_traits>;
//!code-end: types

int main() try
{
    //!code-start: main
    mtv_type db(10);
    db.set<int32_t>(0, 12);
    db.set<int8_t>(2, 34);
    db.set<int16_t>(4, 56);
    //!code-end: main

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

