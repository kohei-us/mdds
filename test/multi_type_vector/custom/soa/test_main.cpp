/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.
#include "test_main.hpp"

int main()
{
    try
    {
        mtv_test_misc_types();
        mtv_test_misc_block_identifier();
        mtv_test_misc_custom_block_func1();
        mtv_test_misc_custom_block_func3();
        mtv_test_misc_release();
        mtv_test_misc_construction_with_array();
        mtv_test_basic();
        mtv_test_basic_equality();
        mtv_test_managed_block();
        mtv_test_transfer();
        mtv_test_swap();
        mtv_test_swap_2();
    }
    catch (const std::exception& e)
    {
        cout << "Test failed: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
