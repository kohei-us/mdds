/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.

#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#include <mdds/multi_type_vector/aos/main.hpp>

#include "run.hpp"

template<typename Traits>
using mtv_tmpl = mdds::mtv::aos::multi_type_vector<Traits>;

int main()
{
    try
    {
        run_all_tests<mtv_tmpl>();
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
