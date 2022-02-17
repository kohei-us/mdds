/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2021 Kohei Yoshida
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

#include "test_global.hpp" // This must be the first header to be included.
#include "test_main.hpp"

int main()
{
    try
    {
        mtv_test_construction();
        mtv_test_basic();
        mtv_test_basic_numeric();
        mtv_test_empty_cells();
        mtv_test_misc_types();
        mtv_test_misc_swap();
        mtv_test_misc_equality();
        mtv_test_misc_clone();
        mtv_test_misc_resize();
        mtv_test_misc_value_type();
        mtv_test_misc_block_identifier();
        mtv_test_misc_push_back();
        mtv_test_misc_capacity();
        mtv_test_misc_position_type_end_position();
        mtv_test_misc_block_pos_adjustments();
        mtv_test_erase();
        mtv_test_insert_empty();
        mtv_test_set_cells();
        mtv_test_insert_cells();
        mtv_test_iterators();
        mtv_test_iterators_element_block();
        mtv_test_iterators_mutable_element_block();
        mtv_test_iterators_set();
        mtv_test_iterators_set_2();
        mtv_test_iterators_insert();
        mtv_test_iterators_insert_empty();
        mtv_test_iterators_set_empty();
        mtv_test_hints_set();
        mtv_test_hints_set_cells();
        mtv_test_hints_insert_cells();
        mtv_test_hints_set_empty();
        mtv_test_hints_insert_empty();
        mtv_test_position();
        mtv_test_position_next();
        mtv_test_position_advance();
        mtv_test_swap_range();
        mtv_test_transfer();
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
