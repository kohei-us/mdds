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
        mtv_test_loop_unrolling_0();
        mtv_test_loop_unrolling_4();
        mtv_test_loop_unrolling_8();
        mtv_test_loop_unrolling_16();
        mtv_test_loop_unrolling_32();
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
