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

void mtv_test_hints_set()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db(3);
    db.set(db.begin(), 0, 23.4);
    assert(db.get<double>(0) == 23.4);
    db.set(db.end(), 0, std::string("test")); // passing end position should have no impact.
    assert(db.get<std::string>(0) == "test");

    mtv_type::iterator pos_hint = db.set(0, 1.2);
    pos_hint = db.set(pos_hint, 1, 1.3);
    pos_hint = db.set(pos_hint, 2, 1.4);
    assert(db.get<double>(0) == 1.2);
    assert(db.get<double>(1) == 1.3);
    assert(db.get<double>(2) == 1.4);

    pos_hint = db.begin();
    pos_hint = db.set(pos_hint, 0, false);
    pos_hint = db.set(pos_hint, 1, std::string("foo"));
    pos_hint = db.set(pos_hint, 2, 34.5);
    assert(db.get<bool>(0) == false);
    assert(db.get<std::string>(1) == "foo");
    assert(db.get<double>(2) == 34.5);

    db.set(pos_hint, 0, int32_t(444)); // position hint does not precede the insertion position.
    assert(db.get<int32_t>(0) == 444); // it should still work.
}

void mtv_test_hints_set_cells()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db(9);

    std::vector<int32_t> ints;
    ints.push_back(1);
    ints.push_back(2);
    ints.push_back(3);

    std::vector<double> doubles;
    doubles.push_back(1.1);
    doubles.push_back(1.2);
    doubles.push_back(1.3);

    std::vector<std::string> strings;
    strings.push_back(std::string("A"));
    strings.push_back(std::string("B"));
    strings.push_back(std::string("C"));

    mtv_type::iterator pos_hint = db.begin();
    pos_hint = db.set(pos_hint, 0, ints.begin(), ints.end());
    pos_hint = db.set(pos_hint, 3, doubles.begin(), doubles.end());
    pos_hint = db.set(pos_hint, 6, strings.begin(), strings.end());

    assert(db.get<int32_t>(0) == 1);
    assert(db.get<int32_t>(1) == 2);
    assert(db.get<int32_t>(2) == 3);

    assert(db.get<double>(3) == 1.1);
    assert(db.get<double>(4) == 1.2);
    assert(db.get<double>(5) == 1.3);

    assert(db.get<std::string>(6) == "A");
    assert(db.get<std::string>(7) == "B");
    assert(db.get<std::string>(8) == "C");
}

void mtv_test_hints_insert_cells()
{
    stack_printer __stack_printer__(__FUNCTION__);

    mtv_type db(1, true); // We need to have at least one element to be able to insert.

    std::vector<int32_t> ints;
    ints.push_back(11);
    ints.push_back(22);

    std::vector<double> doubles;
    doubles.push_back(2.1);
    doubles.push_back(3.2);
    doubles.push_back(4.3);

    std::vector<std::string> strings;
    strings.push_back(std::string("Andy"));
    strings.push_back(std::string("Bruce"));
    strings.push_back(std::string("Charlie"));
    strings.push_back(std::string("David"));

    mtv_type::iterator pos_hint = db.insert(0, ints.begin(), ints.end());
    assert(db.get<int32_t>(0) == 11);
    assert(db.get<int32_t>(1) == 22);
    assert(db.get<bool>(2) == true);

    pos_hint = db.insert(pos_hint, 2, doubles.begin(), doubles.end());
    assert(db.get<int32_t>(0) == 11);
    assert(db.get<int32_t>(1) == 22);
    assert(db.get<double>(2) == 2.1);
    assert(db.get<double>(3) == 3.2);
    assert(db.get<double>(4) == 4.3);
    assert(db.get<bool>(5) == true);

    pos_hint = db.insert(pos_hint, 4, strings.begin(), strings.end());
    assert(db.get<int32_t>(0) == 11);
    assert(db.get<int32_t>(1) == 22);
    assert(db.get<double>(2) == 2.1);
    assert(db.get<double>(3) == 3.2);
    assert(db.get<std::string>(4) == "Andy");
    assert(db.get<std::string>(5) == "Bruce");
    assert(db.get<std::string>(6) == "Charlie");
    assert(db.get<std::string>(7) == "David");
    assert(db.get<double>(8) == 4.3);
    assert(db.get<bool>(9) == true);
}

void mtv_test_hints_set_empty()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db(20, true);
    mtv_type::iterator pos_hint = db.set_empty(db.begin(), 2, 3);
    pos_hint = db.set_empty(pos_hint, 5, 7);
    pos_hint = db.set_empty(pos_hint, 9, 12);
    pos_hint = db.set_empty(pos_hint, 14, 17);

    // Check the boundaries.
    assert(!db.is_empty(0));
    assert(!db.is_empty(4));
    assert(db.is_empty(5));
    assert(db.is_empty(7));
    assert(!db.is_empty(8));
    assert(db.is_empty(9));
    assert(db.is_empty(12));
    assert(!db.is_empty(13));
    assert(db.is_empty(14));
    assert(db.is_empty(17));
    assert(!db.is_empty(18));
}

void mtv_test_hints_insert_empty()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db(2, true);
    mtv_type::iterator pos_hint = db.begin();
    pos_hint = db.insert_empty(pos_hint, 1, 3); // the size becomes 5.
    pos_hint = db.insert_empty(pos_hint, 4, 2); // the size now becomes 7.

    mtv_type::iterator check = db.begin();
    assert(check->type == mdds::mtv::element_type_boolean);
    assert(check->size == 1);
    ++check;
    assert(check->type == mdds::mtv::element_type_empty);
    assert(check->size == 5);
    ++check;
    assert(check->type == mdds::mtv::element_type_boolean);
    assert(check->size == 1);
    ++check;
    assert(check == db.end());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
