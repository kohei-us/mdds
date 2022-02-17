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

void mtv_test_swap()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv3_type db1(5), db2(2);
    db1.set(0, new muser_cell(1.1));
    db1.set(1, new muser_cell(1.2));
    db1.set(2, new muser_cell(1.3));
    db1.set(3, new muser_cell(1.4));
    db1.set(4, new muser_cell(1.5));

    db2.set(0, std::string("A"));
    db2.set(1, std::string("B"));

    db1.swap(2, 3, db2, 0);

    // swap blocks of equal size, one managed, and one default.

    db1.clear();
    db1.resize(10);
    db2.clear();
    db2.resize(10);

    db1.set(3, 2.1);
    db1.set(4, 2.2);
    db1.set(5, 2.3);

    db2.set(3, new muser_cell(3.1));
    db2.set(4, new muser_cell(3.2));
    db2.set(5, new muser_cell(3.3));

    db2.swap(3, 5, db1, 3);

    assert(db1.size() == 10);
    assert(db1.block_size() == 3);
    assert(db2.size() == 10);
    assert(db2.block_size() == 3);

    assert(db1.get<muser_cell*>(3)->value == 3.1);
    assert(db1.get<muser_cell*>(4)->value == 3.2);
    assert(db1.get<muser_cell*>(5)->value == 3.3);
    assert(db2.get<double>(3) == 2.1);
    assert(db2.get<double>(4) == 2.2);
    assert(db2.get<double>(5) == 2.3);

    db2.swap(3, 5, db1, 3);

    assert(db1.get<double>(3) == 2.1);
    assert(db1.get<double>(4) == 2.2);
    assert(db1.get<double>(5) == 2.3);
    assert(db2.get<muser_cell*>(3)->value == 3.1);
    assert(db2.get<muser_cell*>(4)->value == 3.2);
    assert(db2.get<muser_cell*>(5)->value == 3.3);

    // Same as above, except that the source segment splits the block into 2.

    db1.clear();
    db1.resize(10);
    db2.clear();
    db2.resize(10);

    db1.set(3, 2.1);
    db1.set(4, 2.2);

    db2.set(3, new muser_cell(3.1));
    db2.set(4, new muser_cell(3.2));
    db2.set(5, new muser_cell(3.3));

    db2.swap(3, 4, db1, 3);

    // Another scenario that used to crash on double delete.

    db1.clear();
    db1.resize(10);
    db2.clear();
    db2.resize(10);

    db1.set(2, new muser_cell(4.1));
    db1.set(3, 4.2);
    db1.set(4, new muser_cell(4.3));

    db2.set(3, new muser_cell(6.1));
    db2.set(4, 6.2);
    db2.set(5, 6.3);

    assert(db1.get<muser_cell*>(2)->value == 4.1);
    assert(db1.get<double>(3) == 4.2);
    assert(db1.get<muser_cell*>(4)->value == 4.3);

    assert(db2.get<muser_cell*>(3)->value == 6.1);
    assert(db2.get<double>(4) == 6.2);
    assert(db2.get<double>(5) == 6.3);

    db2.swap(4, 4, db1, 4);

    assert(db1.get<muser_cell*>(2)->value == 4.1);
    assert(db1.get<double>(3) == 4.2);
    assert(db1.get<double>(4) == 6.2);

    assert(db2.get<muser_cell*>(3)->value == 6.1);
    assert(db2.get<muser_cell*>(4)->value == 4.3);
    assert(db2.get<double>(5) == 6.3);

    // One more on double deletion...

    db1.clear();
    db1.resize(10);
    db2.clear();
    db2.resize(10);

    db1.set(0, 2.1);
    db1.set(1, 2.2);
    db1.set(2, 2.3);
    db1.set(3, new muser_cell(4.5));

    db2.set(2, new muser_cell(3.1));
    db2.set(3, new muser_cell(3.2));
    db2.set(4, new muser_cell(3.3));

    db1.swap(2, 2, db2, 3);

    assert(db1.get<double>(0) == 2.1);
    assert(db1.get<double>(1) == 2.2);
    assert(db1.get<muser_cell*>(2)->value == 3.2);
    assert(db1.get<muser_cell*>(3)->value == 4.5);

    assert(db2.get<muser_cell*>(2)->value == 3.1);
    assert(db2.get<double>(3) == 2.3);
    assert(db2.get<muser_cell*>(4)->value == 3.3);
}

void mtv_test_swap_2()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv3_type db1(3), db2(3);

    db1.set(0, new muser_cell(1.1));
    db1.set(1, new muser_cell(1.2));

    db2.set(0, 1.2);
    db2.set(1, std::string("foo"));

    // Repeat the same swap twice.
    db1.swap(0, 1, db2, 0);
    assert(db2.get<muser_cell*>(0)->value == 1.1);
    assert(db2.get<muser_cell*>(1)->value == 1.2);
    assert(db1.get<double>(0) == 1.2);
    assert(db1.get<std::string>(1) == "foo");

    db1.swap(0, 1, db2, 0);
    assert(db1.get<muser_cell*>(0)->value == 1.1);
    assert(db1.get<muser_cell*>(1)->value == 1.2);
    assert(db2.get<double>(0) == 1.2);
    assert(db2.get<std::string>(1) == "foo");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
