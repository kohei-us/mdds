/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2022 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_main.hpp"

int main()
{
    try
    {
        mtv_test_element_blocks_buildability();
        mtv_test_element_blocks_std_vector();
        mtv_test_element_blocks_std_deque();
        mtv_test_element_blocks_std_vector_bool();
        mtv_test_element_blocks_std_deque_bool();
        mtv_test_element_blocks_delayed_delete_vector_bool();
        mtv_test_element_blocks_range();
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
