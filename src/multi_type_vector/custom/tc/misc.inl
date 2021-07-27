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

void mtv_test_misc_types()
{
    stack_printer __stack_printer__(__FUNCTION__);

    mdds::mtv::element_t ct;

    // Basic types
    ct = mtv_type::get_element_type(double(12.3));
    assert(ct == mdds::mtv::element_type_double);
    ct = mtv_type::get_element_type(std::string());
    assert(ct == mdds::mtv::element_type_string);
    ct = mtv_type::get_element_type(static_cast<uint64_t>(12));
    assert(ct == mdds::mtv::element_type_uint64);
    ct = mtv_type::get_element_type(true);
    assert(ct == mdds::mtv::element_type_boolean);
    ct = mtv_type::get_element_type(false);
    assert(ct == mdds::mtv::element_type_boolean);

    // Custom cell type
    user_cell* p = nullptr;
    ct = mtv_type::get_element_type(p);
    assert(ct == element_type_user_block && ct >= mdds::mtv::element_type_user_start);
    ct = mtv_type::get_element_type(static_cast<muser_cell*>(nullptr));
    assert(ct == element_type_muser_block && ct >= mdds::mtv::element_type_user_start);
    ct = mtv_fruit_type::get_element_type(unknown_fruit);
    assert(ct == element_type_fruit_block && ct >= mdds::mtv::element_type_user_start);
}

void mtv_test_misc_block_identifier()
{
    stack_printer __stack_printer__(__FUNCTION__);
    assert(user_cell_block::block_type == element_type_user_block);
    assert(muser_cell_block::block_type == element_type_muser_block);
    assert(fruit_block::block_type == element_type_fruit_block);
    assert(date_block::block_type == element_type_date_block);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

