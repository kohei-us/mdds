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

void mtv_test_insert_empty()
{
    stack_printer __stack_printer__(__FUNCTION__);
    {
        mtv_type db(5);
        db.insert_empty(0, 5);
        assert(db.size() == 10);
        assert(db.block_size() == 1);

        // Insert data from row 0 to 4.
        for (long i = 0; i < 5; ++i)
            db.set(i, static_cast<double>(i+1));

        assert(db.block_size() == 2);
        assert(db.size() == 10);

        // Now, insert an empty block of size 2 at the top.
        db.insert_empty(0, 2);
        assert(db.block_size() == 3);
        assert(db.size() == 12);

        double test;
        db.get(2, test);
        assert(test == 1.0);

        // Insert an empty cell into an empty block.  This should shift the
        // data block down by one.
        db.insert_empty(1, 1);
        assert(db.block_size() == 3);
        assert(db.size() == 13);
        db.get(4, test);
        assert(test == 2.0);
    }

    {
        mtv_type db(5);
        for (long i = 0; i < 5; ++i)
            db.set(i, static_cast<double>(i+1));

        assert(db.block_size() == 1);
        assert(db.size() == 5);

        // Insert an empty block into the middle of a non-empty block.
        db.insert_empty(2, 2);

        assert(db.block_size() == 3);
        assert(db.size() == 7);
        assert(db.is_empty(2));
        assert(db.is_empty(3));

        double test;
        db.get(0, test);
        assert(test == 1.0);
        db.get(1, test);
        assert(test == 2.0);

        db.get(4, test);
        assert(test == 3.0);
        db.get(5, test);
        assert(test == 4.0);
        db.get(6, test);
        assert(test == 5.0);
    }

    {
        mtv_type db(1);
        db.set(0, 2.5);
        db.insert_empty(0, 2);
        assert(db.block_size() == 2);
        assert(db.size() == 3);
        assert(db.is_empty(1));
        assert(!db.is_empty(2));

        double test;
        db.get(2, test);
        assert(test == 2.5);
    }

    {
        mtv_type db(2);
        db.set(0, 1.2);
        db.set(1, 2.3);
        db.insert_empty(1, 1);

        assert(db.block_size() == 3);
        assert(db.size() == 3);
        double test;
        db.get(0, test);
        assert(test == 1.2);
        db.get(2, test);
        assert(test == 2.3);
    }

    {
        mtv_type db(2);
        db.push_back<double>(12.0);
        db.push_back<float>(13.0);
        db.insert_empty(2, 2);

        assert(db.block_size() == 3);
        assert(db.size() == 6);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.get<double>(4) == 12.0);
        assert(db.get<float>(5) == 13.0);
    }

    {
        mtv_type db;
        for (int32_t i = 0; i < 9; ++i)
            db.push_back(i);

        db.push_back<int16_t>(123);
        db.insert_empty(7, 3);

        assert(db.block_size() == 4);
        assert(db.size() == 13);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

