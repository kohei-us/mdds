// SPDX-FileCopyrightText: 2015 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.
#include "test_main.hpp"

int main()
{
    try
    {
        mtv_test_block_counter();
        mtv_test_block_counter_clone();
        mtv_test_block_init();
        mtv_test_swap();
    }
    catch (const std::exception& e)
    {
        cout << "Test failed: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}
