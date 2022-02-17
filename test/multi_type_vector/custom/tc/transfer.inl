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

void mtv_test_transfer()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db1(3), db2(4); // db2 is larger than db1.
    db1.set(0, new muser_cell(1.1));
    db1.set(1, new muser_cell(1.2));
    db1.set(2, new muser_cell(1.3));
    assert(db1.block_size() == 1);

    try
    {
        db1.transfer(0, 1, db1, 0);
        assert(!"Exception should have been thrown");
    }
    catch (const mdds::invalid_arg_error&)
    {
        // Good.
    }

    // Do the transfer.
    db1.transfer(0, 2, db2, 0);

    // Now db1 should be totally empty.
    assert(db1.block_size() == 1);
    mtv_type::iterator check = db1.begin();
    assert(check != db1.end());
    assert(check->type == mdds::mtv::element_type_empty);
    assert(check->size == 3);

    assert(db2.block_size() == 2);
    assert(db2.get<muser_cell*>(0)->value == 1.1);
    assert(db2.get<muser_cell*>(1)->value == 1.2);
    assert(db2.get<muser_cell*>(2)->value == 1.3);
    assert(db2.is_empty(3));

    // Transfer back to db1. This should make db2 to be totally empty again.
    db2.transfer(0, 2, db1, 0);
    assert(db2.block_size() == 1);
    check = db2.begin();
    assert(check != db2.end());
    assert(check->size == 4);
    assert(check->type == mdds::mtv::element_type_empty);

    assert(db1.block_size() == 1);
    assert(db1.get<muser_cell*>(0)->value == 1.1);
    assert(db1.get<muser_cell*>(1)->value == 1.2);
    assert(db1.get<muser_cell*>(2)->value == 1.3);

    // Now, transfer only the top 2 elements.
    db1.transfer(0, 1, db2, 0);
    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.get<muser_cell*>(2)->value == 1.3);

    assert(db2.get<muser_cell*>(0)->value == 1.1);
    assert(db2.get<muser_cell*>(1)->value == 1.2);
    assert(db2.is_empty(2));
    assert(db2.is_empty(3));

    // .. and back.
    db2.transfer(0, 1, db1, 0);
    assert(db1.block_size() == 1);
    assert(db1.get<muser_cell*>(0)->value == 1.1);
    assert(db1.get<muser_cell*>(1)->value == 1.2);
    assert(db1.get<muser_cell*>(2)->value == 1.3);

    assert(db2.block_size() == 1);
    check = db2.begin();
    assert(check != db2.end());
    assert(check->size == 4);
    assert(check->type == mdds::mtv::element_type_empty);

    db1 = mtv_type(4);
    db2 = mtv_type(4);
    db2.set(1, new muser_cell(2.1));
    db2.set(2, new muser_cell(2.2));
    assert(db2.block_size() == 3);
    db1.transfer(0, 1, db2, 1); // This causes db2's 3 blocks to merge into one.
    assert(db2.block_size() == 1);
    check = db2.begin();
    assert(check != db2.end());
    assert(check->size == 4);
    assert(check->type == mdds::mtv::element_type_empty);
    assert(db1.block_size() == 1);
    check = db1.begin();
    assert(check != db1.end());
    assert(check->size == 4);
    assert(check->type == mdds::mtv::element_type_empty);

    db2.set(0, new muser_cell(3.1));
    assert(db2.block_size() == 2);

    db1.set(1, new muser_cell(3.2));
    db1.set(2, new muser_cell(3.3));
    db1.set(3, new muser_cell(3.4));
    assert(db1.block_size() == 2);

    db1.transfer(1, 2, db2, 1);
    assert(db1.block_size() == 2);
    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.get<muser_cell*>(3)->value == 3.4);
    assert(db2.block_size() == 2);
    assert(db2.get<muser_cell*>(0)->value == 3.1);
    assert(db2.get<muser_cell*>(1)->value == 3.2);
    assert(db2.get<muser_cell*>(2)->value == 3.3);
    assert(db2.is_empty(3));

    db1 = mtv_type(3);
    db2 = mtv_type(3);
    db1.set(1, new muser_cell(4.2));
    db2.set(0, new muser_cell(4.1));
    db2.set(2, new muser_cell(4.3));

    db1.transfer(1, 1, db2, 1);
    assert(db1.block_size() == 1);
    check = db1.begin();
    assert(check != db1.end());
    assert(check->size == 3);
    assert(check->type == mdds::mtv::element_type_empty);
    assert(db2.block_size() == 1);
    assert(db2.get<muser_cell*>(0)->value == 4.1);
    assert(db2.get<muser_cell*>(1)->value == 4.2);
    assert(db2.get<muser_cell*>(2)->value == 4.3);

    // Transfer to middle of block.
    db1 = mtv_type(3);
    db2 = mtv_type(3);
    db1.set(0, new muser_cell(5.2));
    assert(db1.block_size() == 2);
    db1.transfer(0, 0, db2, 1);
    assert(db1.block_size() == 1);
    check = db1.begin();
    assert(check != db1.end());
    assert(check->size == 3);
    assert(check->type == mdds::mtv::element_type_empty);
    assert(db2.block_size() == 3);
    assert(db2.is_empty(0));
    assert(db2.get<muser_cell*>(1)->value == 5.2);
    assert(db2.is_empty(2));

    db1 = mtv_type(2);
    db2 = mtv_type(3);
    db1.set(0, new muser_cell(11.1));
    db1.set(1, new muser_cell(11.2));
    db1.transfer(1, 1, db2, 1);
    assert(db1.block_size() == 2);
    assert(db1.get<muser_cell*>(0)->value == 11.1);
    assert(db1.is_empty(1));
    assert(db2.block_size() == 3);
    assert(db2.is_empty(0));
    assert(db2.get<muser_cell*>(1)->value == 11.2);
    assert(db2.is_empty(2));

    // Transfer to bottom of block.
    db1 = mtv_type(4);
    db2 = mtv_type(5);
    db1.set(0, new muser_cell(6.1));
    db1.set(1, new muser_cell(6.2));
    db1.transfer(0, 1, db2, 3);
    assert(db1.block_size() == 1);
    check = db1.begin();
    assert(check != db1.end());
    assert(check->size == 4);
    assert(check->type == mdds::mtv::element_type_empty);
    assert(db2.block_size() == 2);
    assert(db2.is_empty(0));
    assert(db2.is_empty(1));
    assert(db2.is_empty(2));
    assert(db2.get<muser_cell*>(3)->value == 6.1);
    assert(db2.get<muser_cell*>(4)->value == 6.2);

    // Transfer multiple blocks.  Very simple use case.
    db1 = mtv_type(4);
    db2 = mtv_type(3);
    db1.set(1, new muser_cell(10.1));
    db1.set(3, new muser_cell(10.2));
    db1.transfer(1, 3, db2, 0);

    // db1 should be completely empty.
    assert(db1.block_size() == 1);
    check = db1.begin();
    assert(check != db1.end());
    assert(check->size == 4);
    assert(check->type == mdds::mtv::element_type_empty);

    assert(db2.block_size() == 3);
    assert(db2.get<muser_cell*>(0)->value == 10.1);
    assert(db2.is_empty(1));
    assert(db2.get<muser_cell*>(2)->value == 10.2);

    // Multiple-block transfer that involves merging.
    db1 = mtv_type(5);
    db2 = mtv_type(5);
    db1.set(0, new muser_cell(0.1));
    db1.set(1, new muser_cell(0.2));
    db1.set(3, new muser_cell(0.3));
    db1.set(4, new muser_cell(0.4));

    db2.set(0, new muser_cell(1.1));
    db2.set(4, new muser_cell(1.2));

    mtv_type::iterator it = db1.transfer(1, 3, db2, 1);
    assert(db1.block_size() == 3);
    assert(db1.get<muser_cell*>(0)->value == 0.1);
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.is_empty(3));
    assert(db1.get<muser_cell*>(4)->value == 0.4);

    assert(db2.block_size() == 3);
    assert(db2.get<muser_cell*>(0)->value == 1.1);
    assert(db2.get<muser_cell*>(1)->value == 0.2);
    assert(db2.is_empty(2));
    assert(db2.get<muser_cell*>(3)->value == 0.3);
    assert(db2.get<muser_cell*>(4)->value == 1.2);

    assert(it != db1.end());
    assert(it->size == 3);
    assert(it->type == mdds::mtv::element_type_empty);
    it = db1.transfer(it, 4, 4, db2, 2); // Transfer single element at 4.
    assert(db1.block_size() == 2);
    assert(db1.get<muser_cell*>(0)->value == 0.1);
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.is_empty(3));
    assert(db1.is_empty(4));

    assert(db2.block_size() == 1);
    assert(db2.get<muser_cell*>(0)->value == 1.1);
    assert(db2.get<muser_cell*>(1)->value == 0.2);
    assert(db2.get<muser_cell*>(2)->value == 0.4);
    assert(db2.get<muser_cell*>(3)->value == 0.3);
    assert(db2.get<muser_cell*>(4)->value == 1.2);

    assert(it != db1.end());
    assert(it->size == 4);
    assert(it->type == mdds::mtv::element_type_empty);
    ++it;
    assert(it == db1.end());

    // Multi-block transfer to the top part of destination block.
    db1 = mtv_type(5);
    db2 = mtv_type(5);
    db1.set(0, new muser_cell(-1.1));
    db1.set(1, new muser_cell(-2.1));
    db1.set(2, new muser_cell(-3.1));
    db1.set(3, std::string("foo"));
    db1.set(4, new muser_cell(-5.1));
    db2.set(1, true);
    db2.set(2, false);
    db2.set(3, true);
    it = db1.transfer(2, 3, db2, 2);
    assert(it != db1.end());
    assert(it->size == 2);
    assert(it->type == mdds::mtv::element_type_empty);
    std::advance(it, 2);
    assert(it == db1.end());
    assert(db1.block_size() == 3);
    assert(db1.get<muser_cell*>(0)->value == -1.1);
    assert(db1.get<muser_cell*>(1)->value == -2.1);
    assert(db1.is_empty(2));
    assert(db1.is_empty(3));
    assert(db1.get<muser_cell*>(4)->value == -5.1);

    assert(db2.block_size() == 5);
    assert(db2.is_empty(0));
    assert(db2.get<bool>(1) == true);
    assert(db2.get<muser_cell*>(2)->value == -3.1);
    assert(db2.get<std::string>(3) == "foo");
    assert(db2.is_empty(4));

    // Multi-block transfer to the bottom part of destination block.
    db1 = mtv_type(10);
    db2 = mtv_type(10);
    db1.set(0, new muser_cell(2.1));
    db1.set(1, new muser_cell(2.2));
    db1.set(2, int8_t('a'));
    db1.set(3, int8_t('b'));
    db2.set(0, true);
    db2.set(1, false);

    it = db1.transfer(0, 2, db2, 7);
    assert(it != db1.end());
    assert(it->size == 3);
    assert(it->type == mdds::mtv::element_type_empty);
    ++it;
    assert(it != db1.end());
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_int8);
    ++it;
    assert(it != db1.end());
    assert(it->size == 6);
    assert(it->type == mdds::mtv::element_type_empty);
    ++it;
    assert(it == db1.end());
    assert(db1.block_size() == 3);
    assert(db1.get<int8_t>(3) == 'b');

    assert(db2.block_size() == 4);
    assert(db2.get<bool>(0) == true);
    assert(db2.get<bool>(1) == false);
    assert(db2.is_empty(2));
    assert(db2.is_empty(3));
    assert(db2.is_empty(4));
    assert(db2.is_empty(5));
    assert(db2.is_empty(6));
    assert(db2.get<muser_cell*>(7)->value == 2.1);
    assert(db2.get<muser_cell*>(8)->value == 2.2);
    assert(db2.get<int8_t>(9) == 'a');

    // Multi-block transfer to the middle part of destination block.
    db1 = mtv_type(10);
    db2 = mtv_type(10);
    db2.set(0, true);
    db2.set(9, true);
    db1.set(3, new muser_cell(2.4));
    db1.set(4, new muser_cell(2.5));
    db1.set(5, std::string("abc"));
    db1.set(6, new muser_cell(2.6));
    db1.set(7, new muser_cell(2.7));
    db1.set(8, true);
    it = db1.transfer(3, 6, db2, 2);
    assert(it != db1.end());
    assert(it->size == 7);
    assert(it->type == mdds::mtv::element_type_empty);
    ++it;
    assert(it != db1.end());
    assert(it->size == 1);
    assert(it->type == element_type_muser_block);
    ++it;
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_boolean);
    ++it;
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_empty);
    ++it;
    assert(it == db1.end());

    assert(db1.block_size() == 4);
    assert(db1.is_empty(6));
    assert(db1.get<muser_cell*>(7)->value == 2.7);
    assert(db1.get<bool>(8) == true);
    assert(db1.is_empty(9));

    assert(db2.block_size() == 7);
    assert(db2.get<bool>(0) == true);
    assert(db2.is_empty(1));
    assert(db2.get<muser_cell*>(2)->value == 2.4);
    assert(db2.get<muser_cell*>(3)->value == 2.5);
    assert(db2.get<std::string>(4) == "abc");
    assert(db2.get<muser_cell*>(5)->value == 2.6);
    assert(db2.is_empty(6));
    assert(db2.is_empty(7));
    assert(db2.is_empty(8));
    assert(db2.get<bool>(9) == true);

    db1 = mtv_type(10);
    db2 = mtv_type(10);
    db1.set(3, true);
    db2.set(3, std::string("test"));
    db1.transfer(0, 6, db2, 0);
    assert(db1.block_size() == 1);
    check = db1.begin();
    assert(check != db1.end());
    assert(check->size == 10);
    assert(check->type == mdds::mtv::element_type_empty);
    ++check;
    assert(check == db1.end());
    assert(db2.block_size() == 3);
    assert(db2.is_empty(0));
    assert(db2.is_empty(1));
    assert(db2.is_empty(2));
    assert(db2.get<bool>(3) == true);
    assert(db2.is_empty(4));
    assert(db2.is_empty(5));
    assert(db2.is_empty(6));
    assert(db2.is_empty(7));
    assert(db2.is_empty(8));
    assert(db2.is_empty(9));

    // Make sure that transfer will overwrite cells in managed blocks.
    db1 = mtv_type(3);
    db2 = mtv_type(3);
    db1.set(0, new muser_cell(1.1));
    db1.set(1, new muser_cell(1.2));
    db1.set(2, new muser_cell(1.3));

    db2.set(1, new muser_cell(2.1)); // This element will be overwritten.
    db1.transfer(0, 2, db2, 0);
    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db2.get<muser_cell*>(0)->value == 1.1);
    assert(db2.get<muser_cell*>(1)->value == 1.2);
    assert(db2.get<muser_cell*>(2)->value == 1.3);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
