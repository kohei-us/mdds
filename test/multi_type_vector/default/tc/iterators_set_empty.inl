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

void mtv_test_iterators_set_empty()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type::iterator it, check;

    // Block is already empty. Calling the method does not do anything.
    mtv_type db(10);
    db.set(0, 1.1);
    it = db.set_empty(6, 8);
    check = db.begin();
    ++check;
    assert(it == check);

    // Empty a whole block.
    db = mtv_type(10, false);
    db.set(0, 1.1);
    db.set(1, std::string("A"));
    it = db.set_empty(2, 9);
    check = db.begin();
    std::advance(check, 2);
    assert(it == check);
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->size == 8);
    ++it;
    assert(it == db.end());

    // Empty the upper part of a block.
    std::vector<int16_t> shorts(8, 23);
    db.set(2, shorts.begin(), shorts.end());
    it = db.set_empty(2, 6);
    check = db.begin();
    std::advance(check, 2);
    assert(it == check);
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->size == 5);
    ++it;
    assert(it->type == mdds::mtv::element_type_int16);
    assert(it->size == 3);
    ++it;
    assert(it == db.end());

    // Empty the lower part of a block.
    db = mtv_type(10, std::string("foo"));
    it = db.set_empty(3, 9);
    check = db.begin();
    ++check;
    assert(it == check);
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->size == 7);
    ++it;
    assert(it == db.end());

    // Empty the middle part of a block.
    db = mtv_type(10, std::string("baa"));
    it = db.set_empty(3, 6);
    check = db.begin();
    assert(check->type == mdds::mtv::element_type_string);
    assert(check->size == 3);
    ++check;
    assert(it == check);
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->size == 4);
    ++it;
    assert(it->type == mdds::mtv::element_type_string);
    assert(it->size == 3);
    ++it;
    assert(it == db.end());

    // Empty multiple blocks. The first block is partially emptied.
    db = mtv_type(10, false);
    db.set(0, 1.1);
    shorts.resize(3, 22);
    db.set(4, shorts.begin(), shorts.end()); // 4 thru 6
    it = db.set_empty(5, 8);
    check = db.begin();
    assert(check->type == mdds::mtv::element_type_double);
    assert(check->size == 1);
    ++check;
    assert(check->type == mdds::mtv::element_type_boolean);
    assert(check->size == 3);
    ++check;
    assert(check->type == mdds::mtv::element_type_int16);
    assert(check->size == 1);
    ++check;
    assert(it == check);
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->size == 4);
    ++it;
    assert(it->type == mdds::mtv::element_type_boolean);
    assert(it->size == 1);
    ++it;
    assert(it == db.end());

    // This time, the first block is completely emptied.
    db = mtv_type(10, false);
    db.set(0, 1.2);
    shorts.resize(3, 42);
    db.set(4, shorts.begin(), shorts.end()); // 4 thru 6
    it = db.set_empty(4, 7);
    check = db.begin();
    assert(check->type == mdds::mtv::element_type_double);
    assert(check->size == 1);
    ++check;
    assert(check->type == mdds::mtv::element_type_boolean);
    assert(check->size == 3);
    ++check;
    assert(it == check);
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->size == 4);
    ++it;
    assert(it->type == mdds::mtv::element_type_boolean);
    assert(it->size == 2);
    ++it;
    assert(it == db.end());

    // And this time, the first block is partially emptied but it's already an
    // empty block to begin with.
    db = mtv_type(10);
    db.set(0, std::string("top"));
    std::vector<double> doubles(5, 1.2);
    db.set(5, doubles.begin(), doubles.end()); // 5 thru 9
    it = db.set_empty(3, 7);
    check = db.begin();
    ++check;
    assert(it == check);
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->size == 7);
    ++it;
    assert(it->type == mdds::mtv::element_type_double);
    assert(it->size == 2);
    ++it;
    assert(it == db.end());

    db = mtv_type(10);
    db.set(0, true);
    db.set(5, 1.1);
    it = db.set_empty(5, 5); // Merge with previous and next blocks.
    check = db.begin();
    ++check;
    assert(it == check);
    assert(it->size == 9);
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->position == 1);
    assert(it->__private_data.block_index == 1);
    ++it;
    assert(it == db.end());

    doubles.resize(3, 32.3);
    db.set(4, doubles.begin(), doubles.end()); // 4 thru 6
    assert(db.block_size() == 4);
    it = db.set_empty(4, 5); // Merge with the previous block.
    check = db.begin();
    ++check;
    assert(it == check);
    assert(it->size == 5);
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->position == 1);
    assert(it->__private_data.block_index == 1);
    ++it;
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_double);
    ++it;
    assert(it->size == 3);
    assert(it->type == mdds::mtv::element_type_empty);
    ++it;
    assert(it == db.end());

    db = mtv_type(10);
    db.set(9, false);
    db.set(3, 1.1);
    db.set(4, 1.2);
    db.set(5, 1.3);
    assert(db.block_size() == 4);
    it = db.set_empty(5, 5); // Merge with the next empty block.
    assert(db.block_size() == 4);
    check = db.begin();
    std::advance(check, 2);
    assert(it == check);
    assert(it->size == 4);
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->__private_data.block_index == 2);
    assert(it->position == 5);
    std::advance(it, 2);
    assert(it == db.end());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
