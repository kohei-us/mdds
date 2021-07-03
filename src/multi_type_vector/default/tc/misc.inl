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

void mtv_test_misc_swap()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db1(3), db2(5);
    db1.set(0, 1.0);
    db1.set(1, 2.0);
    db1.set(2, 3.0);

    db2.set(0, 4.0);
    db2.set(1, 5.0);
    db2.set(4, std::string("foo"));
    db1.swap(db2);

    assert(db1.size() == 5 && db1.block_size() == 3);
    assert(db2.size() == 3 && db2.block_size() == 1);
}

void mtv_test_misc_equality()
{
    stack_printer __stack_printer__(__FUNCTION__);
    {
        // Two columns of equal size.
        mtv_type db1(3), db2(3);
        assert(db1 == db2);
        db1.set(0, 1.0);
        assert(db1 != db2);
        db2.set(0, 1.0);
        assert(db1 == db2);
        db2.set(0, 1.2);
        assert(db1 != db2);
        db1.set(0, 1.2);
        assert(db1 == db2);
    }

    {
        // Two columns of different sizes.  They are always non-equal no
        // matter what.
        mtv_type db1(3), db2(4);
        assert(db1 != db2);
        db1.set(0, 1.2);
        db2.set(0, 1.2);
        assert(db1 != db2);

        // Comparison to self.
        assert(db1 == db1);
        assert(db2 == db2);
    }

    {
        // Two containers both consisting of numeric - empty - numeric blocks,
        // and the last numeric blocks differ.

        mtv_type db1(5), db2(5);
        db1.set(0, 1.1);
        db1.set(3, 2.1);
        db1.set(4, 2.2);

        db2.set(0, 1.1);
        db2.set(3, 2.1);
        db2.set(4, 2.3); // different value

        assert(db1 != db2);
    }

    {
        mtv_type db1(2), db2(2);
        db1.set(1, 10.1);
        db2.set(1, 10.1);
        assert(db1 == db2);

        db2.set(0, std::string("foo"));
        assert(db1 != db2);

        db1.set(0, std::string("foo"));
        assert(db1 == db2);

        db2.set_empty(0, 0);
        assert(db1 != db2);
    }
}

void mtv_test_misc_clone()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db1(3);
    db1.set(0, 3.4);
    db1.set(1, std::string("foo"));
    db1.set(2, true);

    // copy construction

    mtv_type db2(db1);
    assert(db1.size() == db2.size());
    assert(db1.block_size() == db2.block_size());
    assert(db1 == db2);

    {
        double test1, test2;
        db1.get(0, test1);
        db2.get(0, test2);
        assert(test1 == test2);
    }
    {
        std::string test1, test2;
        db1.get(1, test1);
        db2.get(1, test2);
        assert(test1 == test2);
    }

    {
        bool test1, test2;
        db1.get(2, test1);
        db2.get(2, test2);
        assert(test1 == test2);
    }

    // assignment

    mtv_type db3 = db1;
    assert(db3 == db1);
    db3.set(0, std::string("alpha"));
    assert(db3 != db1);

    mtv_type db4, db5;
    db4 = db5 = db3;
    assert(db4 == db5);
    assert(db3 == db5);
    assert(db3 == db4);
}

void mtv_test_misc_resize()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db(0);
    assert(db.size() == 0);
    assert(db.empty());

    // Resize to create initial empty block.
    db.resize(3);
    assert(db.size() == 3);
    assert(db.block_size() == 1);

    // Resize to increase the existing empty block.
    db.resize(5);
    assert(db.size() == 5);
    assert(db.block_size() == 1);

    for (long row = 0; row < 5; ++row)
        db.set(row, static_cast<double>(row));

    assert(db.size() == 5);
    assert(db.block_size() == 1);

    // Increase its size by one.  This should append an empty cell block of size one.
    db.resize(6);
    assert(db.size() == 6);
    assert(db.block_size() == 2);
    assert(db.is_empty(5));

    // Do it again.
    db.resize(7);
    assert(db.size() == 7);
    assert(db.block_size() == 2);

    // Now, reduce its size to eliminate the last empty block.
    db.resize(5);
    assert(db.size() == 5);
    assert(db.block_size() == 1);

    // Reset.
    db.resize(7);
    assert(db.size() == 7);
    assert(db.block_size() == 2);

    // Now, resize across multiple blocks.
    db.resize(4);
    assert(db.size() == 4);
    assert(db.block_size() == 1);
    double test;
    db.get(3, test);
    assert(test == 3.0);

    // Empty it.
    db.resize(0);
    assert(db.size() == 0);
    assert(db.block_size() == 0);
    assert(db.empty());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

