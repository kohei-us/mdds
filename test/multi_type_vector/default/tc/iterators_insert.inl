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

void mtv_test_iterators_insert()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type::iterator it, check;
    std::vector<double> doubles;
    std::vector<bool> bools;
    std::vector<std::string> strings;
    std::vector<int32_t> ints;

    // Insert values into empty block.  They are to be appended to the previous block.
    mtv_type db(10); // start with empty set.
    db.set(0, std::string("top"));
    db.set(3, 0.9);
    doubles.resize(4, 1.1);
    it = db.insert(4, doubles.begin(), doubles.end());
    check = db.begin();
    std::advance(check, 2);
    assert(it == check);
    assert(it->size == 5);
    assert(it->position == 3);
    ++it;
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->size == 6);
    ++it;
    assert(it == db.end());

    // Same as above, except that the values will not be appended to the previous block.
    db = mtv_type(3);
    db.set(0, std::string("top"));
    doubles.resize(5, 3.3);
    it = db.insert(1, doubles.begin(), doubles.end());
    check = db.begin();
    ++check;
    assert(it == check);
    assert(it->size == 5);
    assert(it->type == mdds::mtv::element_type_double);
    ++it;
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->size == 2);
    ++it;
    assert(it == db.end());

    // Insert into the middle of an empty block.
    db = mtv_type(2);
    doubles.resize(3, 1.2);
    it = db.insert(1, doubles.begin(), doubles.end());
    check = db.begin();
    assert(check->type == mdds::mtv::element_type_empty);
    assert(check->size == 1);
    ++check;
    assert(check == it);
    assert(it->type == mdds::mtv::element_type_double);
    assert(it->size == 3);
    ++it;
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->size == 1);
    ++it;
    assert(it == db.end());

    // Insert into a block of the same type.
    db = mtv_type(5, false);
    db.set(0, std::string("top"));
    db.set(4, std::string("bottom"));
    bools.resize(3, true);
    it = db.insert(2, bools.begin(), bools.end());
    check = db.begin();
    ++check;
    assert(it == check);
    assert(it->type == mdds::mtv::element_type_boolean);
    assert(it->size == 6);
    std::advance(it, 2);
    assert(it == db.end());

    // Insert values which will be append to the previous block.
    db = mtv_type(5, 1.1);
    strings.resize(3, std::string("test"));
    db.set(0, true);
    db.set(2, strings.begin(), strings.end()); // 2 thru 4
    doubles.resize(2, 2.2);
    it = db.insert(2, doubles.begin(), doubles.end());
    check = db.begin();
    ++check;
    assert(it == check);
    assert(it->type == mdds::mtv::element_type_double);
    assert(it->size == 3);
    ++it;
    assert(it->type == mdds::mtv::element_type_string);
    assert(it->size == 3);
    ++it;
    assert(it == db.end());

    // Insert between blocks without merge.
    db = mtv_type(3);
    db.set(0, 1.1);
    db.set(1, std::string("middle"));
    db.set(2, int32_t(50));
    bools.resize(4, true);
    it = db.insert(1, bools.begin(), bools.end());
    check = db.begin();
    assert(check->type == mdds::mtv::element_type_double);
    ++check;
    assert(it == check);
    assert(it->type == mdds::mtv::element_type_boolean);
    assert(it->size == 4);
    ++it;
    assert(it->type == mdds::mtv::element_type_string);
    assert(it->size == 1);
    ++it;
    assert(it->type == mdds::mtv::element_type_int32);
    assert(it->size == 1);
    ++it;
    assert(it == db.end());

    // Insert values of differing type into middle of a block.
    db = mtv_type(4, 0.01);
    db.set(0, std::string("top"));
    ints.resize(3, 55);
    it = db.insert(2, ints.begin(), ints.end());
    check = db.begin();
    assert(check->type == mdds::mtv::element_type_string);
    assert(check->size == 1);
    ++check;
    assert(check->type == mdds::mtv::element_type_double);
    assert(check->size == 1);
    ++check;
    assert(it == check);
    assert(it->type == mdds::mtv::element_type_int32);
    assert(it->size == 3);
    ++it;
    assert(it->type == mdds::mtv::element_type_double);
    assert(it->size == 2);
    ++it;
    assert(it == db.end());
}

void mtv_test_iterators_insert_empty()
{
    stack_printer __stack_printer__(__FUNCTION__);

    // Insert into an already empty spot.
    mtv_type db(2);
    db.set(1, 1.2);
    mtv_type::iterator it = db.insert_empty(0, 3);
    assert(it == db.begin());
    assert(it->size == 4);
    assert(it->type == mdds::mtv::element_type_empty);
    ++it;
    assert(it->type == mdds::mtv::element_type_double);
    assert(it->size == 1);
    ++it;
    assert(it == db.end());

    // Insert an empty range that will be tucked into the previous empty block.
    db = mtv_type(4);
    db.set(0, std::string("foo"));
    db.set(2, 1.1);
    db.set(3, 1.2);
    it = db.insert_empty(2, 2);
    mtv_type::iterator check = db.begin();
    ++check;
    assert(it == check);
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->size == 3);
    ++it;
    assert(it->type == mdds::mtv::element_type_double);
    assert(it->size == 2);
    ++it;
    assert(it == db.end());

    // Insert an empty range between non-empty blocks.
    db = mtv_type(2, false);
    db.set(0, 1.1);
    it = db.insert_empty(1, 2);
    check = db.begin();
    assert(check->type == mdds::mtv::element_type_double);
    assert(check->size == 1);
    ++check;
    assert(it == check);
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->size == 2);
    ++it;
    assert(it->type == mdds::mtv::element_type_boolean);
    assert(it->size == 1);
    ++it;
    assert(it == db.end());

    // Insert in the middle of a non-empty block.
    db = mtv_type(3, std::string("foo"));
    it = db.insert_empty(2, 4);
    check = db.begin();
    assert(check->type == mdds::mtv::element_type_string);
    assert(check->size == 2);
    ++check;
    assert(it == check);
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->size == 4);
    ++it;
    assert(it->type == mdds::mtv::element_type_string);
    assert(it->size == 1);
    ++it;
    assert(it == db.end());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
