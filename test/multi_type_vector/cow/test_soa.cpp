/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.

#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#include <mdds/multi_type_vector/soa/main.hpp>

#include "common_types.hpp"
#include "run.hpp"

int main()
{
    using cow_mtv_type = mdds::mtv::soa::multi_type_vector<cow_traits>;
    using non_cow_mtv_type = mdds::mtv::soa::multi_type_vector<non_cow_traits>;

    try
    {
        run_all_tests<cow_mtv_type, non_cow_mtv_type>();
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
