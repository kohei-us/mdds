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
    mtv_type db1(5), db2(5);
    db1.set(0, 1.0);
    db1.set(1, 2.0);
    mtv_type::iterator it = db1.transfer(1, 2, db2, 1);

    assert(db1.get<double>(0) == 1.0);
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.is_empty(3));
    assert(db1.is_empty(4));

    assert(db2.is_empty(0));
    assert(db2.get<double>(1) == 2.0);
    assert(db2.is_empty(2));
    assert(db2.is_empty(3));
    assert(db2.is_empty(4));

    assert(it->size == 4);
    assert(it->__private_data.block_index == 1);
    assert(it->position == 1);

    // Reset and start over.
    db1.clear();
    db1.resize(5);
    db2.clear();
    db2.resize(5);

    db1.set(2, 1.2);
    db1.set(3, 1.3);

    // Transfer 1:2 in db1 to 2:3 in db2.
    it = db1.transfer(1, 2, db2, 2);

    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.get<double>(3) == 1.3);
    assert(db1.is_empty(4));

    assert(db2.is_empty(0));
    assert(db2.is_empty(1));
    assert(db2.is_empty(2));
    assert(db2.get<double>(3) == 1.2);
    assert(db2.is_empty(4));

    assert(it->size == 3);
    assert(it->position == 0);
    assert(it->__private_data.block_index == 0);

    // Reset and start over.
    db1.clear();
    db1.resize(4);
    db2.clear();
    db2.resize(4);

    db1.set(0, std::string("A"));
    db1.set(1, std::string("B"));
    db1.set(2, 11.1);
    db1.set(3, 11.2);

    it = db1.transfer(1, 2, db2, 1);

    assert(db1.get<std::string>(0) == "A");
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.get<double>(3) == 11.2);

    assert(db2.is_empty(0));
    assert(db2.get<std::string>(1) == "B");
    assert(db2.get<double>(2) == 11.1);
    assert(db2.is_empty(3));

    assert(it->size == 2);
    assert(it->position == 1);
    assert(it->__private_data.block_index == 1);

    // Reset and start over.
    db1.clear();
    db1.resize(4);
    db2.clear();
    db2.resize(4);

    db1.set(2, 12.8);
    it = db1.transfer(1, 2, db2, 1);

    // Reset and start over.
    db1.clear();
    db1.resize(20);
    db2.clear();
    db2.resize(20);

    db1.set(9, 1.1);

    db2.set(10, 1.2);
    db2.set(11, 1.3);

    it = db1.transfer(9, 9, db2, 9);

    // The source should be entirely empty after the transfer.
    assert(db1.block_size() == 1);
    assert(it == db1.begin());
    assert(it->__private_data.block_index == 0);
    assert(it->size == 20);
    assert(it->type == mdds::mtv::element_type_empty);
    ++it;
    assert(it == db1.end());

    // Check the destination as well.
    assert(db2.block_size() == 3);
    it = db2.begin();
    assert(it->size == 9);
    assert(it->__private_data.block_index == 0);
    assert(it->type == mdds::mtv::element_type_empty);
    ++it;
    assert(it->size == 3);
    assert(it->__private_data.block_index == 1);
    assert(it->type == mdds::mtv::element_type_double);
    ++it;
    assert(it->size == 8);
    assert(it->__private_data.block_index == 2);
    assert(it->type == mdds::mtv::element_type_empty);
    ++it;
    assert(it == db2.end());
    assert(db2.get<double>(9) == 1.1);
    assert(db2.get<double>(10) == 1.2);
    assert(db2.get<double>(11) == 1.3);

    // Reset and start over.
    db1.clear();
    db1.resize(20);
    db2.clear();
    db2.resize(20);

    db1.set(8, 1.0);
    db1.set(9, 1.1);

    db2.set(10, 1.2);
    db2.set(11, 1.3);

    it = db1.transfer(9, 9, db2, 9);
    assert(it->__private_data.block_index == 2);
    assert(db1.block_size() == 3);
    assert(db1.get<double>(8) == 1.0);
    it = db1.begin();
    assert(it->size == 8);
    assert(it->type == mdds::mtv::element_type_empty);
    ++it;
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_double);
    ++it;
    assert(it->size == 11);
    assert(it->type == mdds::mtv::element_type_empty);
    ++it;
    assert(it == db1.end());

    // Reset and start over.
    db1.clear();
    db1.resize(8);
    db2 = mtv_type(8, true);

    db1.transfer(0, 4, db2, 1);

    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.is_empty(3));
    assert(db1.is_empty(4));
    assert(db1.is_empty(5));
    assert(db1.is_empty(6));
    assert(db1.is_empty(7));

    assert(db2.get<bool>(0) == true);
    assert(db2.is_empty(1));
    assert(db2.is_empty(2));
    assert(db2.is_empty(3));
    assert(db2.is_empty(4));
    assert(db2.is_empty(5));
    assert(db2.get<bool>(6) == true);
    assert(db2.get<bool>(7) == true);

    // Start over.
    db1 = mtv_type(8, int16_t(43));
    db2 = mtv_type(8);
    db1.transfer(3, 5, db2, 0);
    assert(db1.get<int16_t>(0) == 43);
    assert(db1.get<int16_t>(1) == 43);
    assert(db1.get<int16_t>(2) == 43);
    assert(db1.is_empty(3));
    assert(db1.is_empty(4));
    assert(db1.is_empty(5));
    assert(db1.get<int16_t>(6) == 43);
    assert(db1.get<int16_t>(7) == 43);

    assert(db2.get<int16_t>(0) == 43);
    assert(db2.get<int16_t>(1) == 43);
    assert(db2.get<int16_t>(2) == 43);
    assert(db2.is_empty(3));
    assert(db2.is_empty(4));
    assert(db2.is_empty(5));
    assert(db2.is_empty(6));
    assert(db2.is_empty(7));

    // Start over.
    db1 = mtv_type(5);
    db2 = mtv_type(5);
    db1.set<double>(3, 1.1);
    db1.set<double>(4, 1.2);
    db2.set<int32_t>(0, 1);
    db2.set<int32_t>(1, 1);

    db1.transfer(3, 4, db2, 0);

    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.is_empty(3));
    assert(db1.is_empty(4));

    assert(db2.get<double>(0) == 1.1);
    assert(db2.get<double>(1) == 1.2);
    assert(db2.is_empty(2));
    assert(db2.is_empty(3));
    assert(db2.is_empty(4));

    // Start over.
    db1 = mtv_type(5);
    db2 = mtv_type(5);
    db1.set<double>(3, 1.1);
    db1.set<int32_t>(4, 892);
    db2.set<int32_t>(0, 1);
    db2.set<int32_t>(1, 1);

    db1.transfer(3, 4, db2, 0);

    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.is_empty(3));
    assert(db1.is_empty(4));

    assert(db2.get<double>(0) == 1.1);
    assert(db2.get<int32_t>(1) == 892);
    assert(db2.is_empty(2));
    assert(db2.is_empty(3));
    assert(db2.is_empty(4));

    // Start over.
    db1 = mtv_type(5, false);
    db2 = mtv_type(5);
    db1.set<int32_t>(4, 234);
    db2.set<int8_t>(0, 32);
    db2.set<uint16_t>(1, 11);
    db2.set<uint16_t>(2, 22);
    db2.set<int8_t>(3, 56);

    db1.transfer(3, 4, db2, 1);

    assert(db1.get<bool>(0) == false);
    assert(db1.get<bool>(1) == false);
    assert(db1.get<bool>(2) == false);
    assert(db1.is_empty(3));
    assert(db1.is_empty(4));

    assert(db2.get<int8_t>(0) == 32);
    assert(db2.get<bool>(1) == false);
    assert(db2.get<int32_t>(2) == 234);
    assert(db2.get<int8_t>(3) == 56);
    assert(db2.is_empty(4));

    // Start over.
    db1 = mtv_type(5, false);
    db2 = mtv_type(6);
    db1.set<int32_t>(4, 234);
    db2.set<uint32_t>(0, 2345);

    db1.transfer(3, 4, db2, 4);

    assert(db1.get<bool>(0) == false);
    assert(db1.get<bool>(1) == false);
    assert(db1.get<bool>(2) == false);
    assert(db1.is_empty(3));
    assert(db1.is_empty(4));

    assert(db2.get<uint32_t>(0) == 2345);
    assert(db2.is_empty(1));
    assert(db2.is_empty(2));
    assert(db2.is_empty(3));
    assert(db2.get<bool>(4) == false);
    assert(db2.get<int32_t>(5) == 234);

    // Start over
    db1.clear();
    db2 = mtv_type(10);
    db1.push_back<int8_t>(5);
    db1.push_back<int16_t>(5);
    db1.push_back<int32_t>(5);
    db1.push_back<double>(5);

    db1.transfer(0, 3, db2, 6);

    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.is_empty(3));

    assert(db2.is_empty(0));
    assert(db2.is_empty(1));
    assert(db2.is_empty(2));
    assert(db2.is_empty(3));
    assert(db2.is_empty(4));
    assert(db2.is_empty(5));
    assert(db2.get<int8_t>(6) == 5);
    assert(db2.get<int16_t>(7) == 5);
    assert(db2.get<int32_t>(8) == 5);
    assert(db2.get<double>(9) == 5.0);

    // Start over.
    db1 = mtv_type(5, true);
    db2 = mtv_type(6);

    db1.transfer(0, 2, db2, 1);

    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.get<bool>(3) == true);
    assert(db1.get<bool>(4) == true);

    assert(db2.is_empty(0));
    assert(db2.get<bool>(1) == true);
    assert(db2.get<bool>(2) == true);
    assert(db2.get<bool>(3) == true);
    assert(db2.is_empty(4));
    assert(db2.is_empty(5));

    // Start over.
    db1 = mtv_type(5, int32_t(987));
    db1.set(3, 1.1);
    db1.set(4, 1.2);

    db2 = mtv_type(5);
    db2.set(0, true);
    db2.set(1, false);

    db1.transfer(0, 2, db2, 2);

    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.get<double>(3) == 1.1);
    assert(db1.get<double>(4) == 1.2);

    assert(db2.get<bool>(0) == true);
    assert(db2.get<bool>(1) == false);
    assert(db2.get<int32_t>(2) == 987);
    assert(db2.get<int32_t>(3) == 987);
    assert(db2.get<int32_t>(4) == 987);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
