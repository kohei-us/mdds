/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include <mdds/multi_type_matrix.hpp>

#include <iostream>
#include <string>
#include <tuple>

//!code-start: value-type
// user-defined value type
struct error_value
{
    int code = 0;
};
//!code-end: value-type

//!code-start: block-def
// static block ID for error_value
constexpr mdds::mtv::element_t error_type_id = mdds::mtv::element_type_user_start;

// block type for error_value
using error_block = mdds::mtv::default_element_block<error_type_id, error_value>;

// callbacks for error_value
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(error_value, error_type_id, error_value{}, error_block)
//!code-end: block-def

//!code-start: traits
struct matrix_traits
{
    // block that stores the numeric elements
    using numeric_element_block = mdds::mtv::double_element_block;

    // blocks that store all the other element types
    using element_blocks = std::tuple<
        mdds::mtv::boolean_element_block,
        mdds::mtv::string_element_block,
        error_block>;
};

using matrix_type = mdds::multi_type_matrix<matrix_traits>;
//!code-end: traits

int main() try
{
    //!code-start: set
    matrix_type mx(3, 2);

    mx.set(0, 0, 1.5);
    mx.set(1, 0, true);
    mx.set(2, 0, std::string("text"));
    mx.set(0, 1, error_value{7});
    mx.set(1, 1, error_value{9});
    //!code-end: set

    //!code-start: get
    // look up the position of (0, 1) once, and use it for both calls
    matrix_type::position_type pos = mx.position(0, 1);

    if (mx.get_type(pos) == error_block::block_type)
    {
        error_value err = mx.get<error_value>(pos);
        std::cout << "error code at (0, 1): " << err.code << std::endl;
    }
    //!code-end: get

    //!code-start: walk
    mx.walk([](const matrix_type::element_block_node_type& node) {
        switch (node.type)
        {
            case matrix_type::element_numeric:
            {
                auto it = node.begin<matrix_type::numeric_block_type>();
                auto ite = node.end<matrix_type::numeric_block_type>();
                for (; it != ite; ++it)
                    std::cout << "numeric: " << *it << std::endl;
                break;
            }
            case mdds::mtv::boolean_element_block::block_type:
            {
                auto it = node.begin<mdds::mtv::boolean_element_block>();
                auto ite = node.end<mdds::mtv::boolean_element_block>();
                for (; it != ite; ++it)
                    std::cout << "boolean: " << (*it ? "true" : "false") << std::endl;
                break;
            }
            case mdds::mtv::string_element_block::block_type:
            {
                auto it = node.begin<mdds::mtv::string_element_block>();
                auto ite = node.end<mdds::mtv::string_element_block>();
                for (; it != ite; ++it)
                    std::cout << "string: " << *it << std::endl;
                break;
            }
            case error_block::block_type:
            {
                auto it = node.begin<error_block>();
                auto ite = node.end<error_block>();
                for (; it != ite; ++it)
                    std::cout << "error: " << it->code << std::endl;
                break;
            }
            case matrix_type::element_empty:
            {
                std::cout << "empty: " << node.size << " element(s)" << std::endl;
                break;
            }
        }
    });
    //!code-end: walk

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
