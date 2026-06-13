/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2011 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.

#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#include <mdds/multi_type_vector/soa/main.hpp>

#include "run.hpp"

int main()
{
    using mtv_type = mdds::mtv::soa::multi_type_vector<mdds::mtv::standard_element_blocks_traits>;

    try
    {
        run_all_tests<mtv_type>();
    }
    catch (const std::exception& e)
    {
        std::cout << "Test failed: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Test finished successfully!" << std::endl;
    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
