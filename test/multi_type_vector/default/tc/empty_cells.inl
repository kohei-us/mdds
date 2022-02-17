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

void mtv_test_empty_cells()
{
    stack_printer __stack_printer__(__FUNCTION__);

    {
        mtv_type db(3);

        assert(db.is_empty(0));
        assert(db.is_empty(2));

        // These won't change the state of the container since it's already empty.
        db.set_empty(0, 0);
        db.set_empty(1, 1);
        db.set_empty(2, 2);
        db.set_empty(0, 2);

        db.set(0, 1.0);
        db.set(2, 5.0);
        assert(!db.is_empty(0));
        assert(!db.is_empty(2));
        assert(db.is_empty(1));

        db.set(1, 2.3);
        assert(!db.is_empty(1));

        // Container contains a single block of numeric cells at this point.

        // Set the whole block empty.
        db.set_empty(0, 2);

        // Reset.
        db.set(0, 1.0);
        db.set(1, 2.0);
        db.set(2, 4.0);

        // Set the upper part of the block empty.
        db.set_empty(0, 1);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
        assert(!db.is_empty(2));

        double test;
        db.get(2, test);
        assert(test == 4.0);

        // Reset.
        db.set(0, 5.0);
        db.set(1, 5.1);
        db.set(2, 5.2);

        // Set the lower part of the block empty.
        db.set_empty(1, 2);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));

        db.get(0, test);
        assert(test == 5.0);

        // Reset.
        db.set(0, 3.0);
        db.set(1, 3.1);
        db.set(2, 3.2);

        // Set the middle part of the block empty.
        db.set_empty(1, 1);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(!db.is_empty(2));
        db.get(0, test);
        assert(test == 3.0);
        db.get(2, test);
        assert(test == 3.2);

        bool res = test_cell_insertion(db, 1, 4.3);
        assert(res);
    }

    {
        // Empty multiple cells at the middle part of a block.
        mtv_type db(4);
        for (size_t i = 0; i < 4; ++i)
            db.set(i, static_cast<double>(i + 1));

        for (size_t i = 0; i < 4; ++i)
        {
            assert(!db.is_empty(i));
        }

        db.set_empty(1, 2);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(!db.is_empty(3));

        double test;
        db.get(0, test);
        assert(test == 1.0);
        db.get(3, test);
        assert(test == 4.0);
    }

    {
        // Empty multiple blocks.
        mtv_type db(2);
        db.set(0, 1.0);
        db.set(1, std::string("foo"));
        assert(!db.is_empty(0));
        assert(!db.is_empty(1));

        db.set_empty(0, 1);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
    }

    {
        // Empty multiple blocks, part 2 - from middle block to middle block.
        mtv_type db(6);
        db.set(0, 1.0);
        db.set(1, 2.0);
        std::string str = "foo";
        db.set(2, str);
        db.set(3, str);
        uint64_t index = 1;
        db.set(4, index);
        index = 100;
        db.set(5, index);

        db.set_empty(1, 4);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));
        assert(!db.is_empty(5));
        double val;
        db.get(0, val);
        assert(val == 1.0);
        uint64_t index_test;
        db.get(5, index_test);
        assert(index_test == 100);
    }

    {
        // Empty multiple blocks, part 3 - from top block to middle block.
        mtv_type db(6);
        db.set(0, 1.0);
        db.set(1, 2.0);
        std::string str = "foo";
        db.set(2, str);
        db.set(3, str);
        uint64_t index = 1;
        db.set(4, index);
        index = 50;
        db.set(5, index);

        db.set_empty(0, 4);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));
        assert(!db.is_empty(5));
        uint64_t test;
        db.get(5, test);
        assert(test == 50);
    }

    {
        // Empty multiple blocks, part 4 - from middle block to bottom block.
        mtv_type db(6);
        db.set(0, 1.0);
        db.set(1, 2.0);
        std::string str = "foo";
        db.set(2, str);
        db.set(3, str);
        uint64_t index = 1;
        db.set(4, index);
        db.set(5, index);

        db.set_empty(1, 5);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));
        assert(db.is_empty(5));
        double test;
        db.get(0, test);
        assert(test == 1.0);
    }

    {
        // Empty multiple blocks, part 5 - from middle empty block to middle non-empty block.
        mtv_type db(6);
        db.set(2, 1.0);
        db.set(3, 2.0);
        std::string str = "foo";
        db.set(4, str);
        str = "baa";
        db.set(5, str);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
        assert(!db.is_empty(2));
        assert(!db.is_empty(3));
        assert(!db.is_empty(4));
        assert(!db.is_empty(5));
        assert(db.block_size() == 3);

        db.set_empty(1, 4);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));
        assert(!db.is_empty(5));
        assert(db.block_size() == 2);
        std::string test;
        db.get(5, test);
        assert(test == "baa");
    }

    {
        // Empty multiple blocks, part 6 - from middle non-empty block to middle empty block.
        mtv_type db(6);
        db.set(0, 1.0);
        db.set(1, 2.0);
        db.set(2, std::string("foo"));
        db.set(3, std::string("baa"));
        assert(!db.is_empty(0));
        assert(!db.is_empty(1));
        assert(!db.is_empty(2));
        assert(!db.is_empty(3));
        assert(db.is_empty(4));
        assert(db.is_empty(5));

        db.set_empty(1, 4);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));
        assert(db.is_empty(5));
        double test;
        db.get(0, test);
        assert(test == 1.0);
        assert(db.block_size() == 2);
    }

    {
        // Empty multiple blocks, part 7 - from middle empty block to middle empty block.
        mtv_type db(6);
        db.set(2, 1.0);
        db.set(3, std::string("foo"));
        assert(db.block_size() == 4);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
        assert(!db.is_empty(2));
        assert(!db.is_empty(3));
        assert(db.is_empty(4));
        assert(db.is_empty(5));

        // This should set the whole range empty.
        db.set_empty(1, 4);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));
        assert(db.is_empty(5));
        assert(db.block_size() == 1);
    }

    {
        // Set empty on 2nd block.  Presence of first block causes an offset
        // on index in the 2nd block.
        mtv_type db(5);
        db.set(0, 1.0);
        db.set(1, static_cast<uint64_t>(1));
        db.set(2, static_cast<uint64_t>(2));
        db.set(3, static_cast<uint64_t>(3));
        db.set(4, static_cast<uint64_t>(4));

        db.set_empty(2, 4);
        assert(!db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));

        db.set(2, static_cast<uint64_t>(5));
        db.set(3, static_cast<uint64_t>(6));
        db.set(4, static_cast<uint64_t>(7));
        db.set_empty(1, 2);
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(!db.is_empty(3));
        assert(!db.is_empty(4));

        db.set(3, static_cast<uint64_t>(8));
        db.set(4, static_cast<uint64_t>(9));
        db.set_empty(2, 3);
    }

    {
        // Set individual single elements empty.
        cout << "Setting individual single elements empty..." << endl;
        mtv_type db(15, 1.2);
        cout << "setting 1 empty..." << endl;
        db.set_empty(1, 1);
        cout << "setting 4 empty..." << endl;
        db.set_empty(4, 4);
        cout << "setting 7 empty..." << endl;
        db.set_empty(7, 7);
        cout << "setting 10 empty..." << endl;
        db.set_empty(10, 10);
        cout << "setting 12 empty..." << endl;
        db.set_empty(12, 12);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(!db.is_empty(2));
        assert(!db.is_empty(3));
        assert(db.is_empty(4));
        assert(!db.is_empty(5));
        assert(!db.is_empty(6));
        assert(db.is_empty(7));
        assert(!db.is_empty(8));
        assert(!db.is_empty(9));
        assert(db.is_empty(10));
        assert(!db.is_empty(11));
        assert(db.is_empty(12));
        assert(!db.is_empty(13));
        assert(!db.is_empty(14));
    }

    {
        mtv_type db(3, true);
        assert(db.block_size() == 1);
        db.set_empty(1, 1);
        assert(db.get<bool>(0) == true);
        assert(db.is_empty(1));
        assert(db.get<bool>(2) == true);
        assert(db.block_size() == 3);
    }

    {
        mtv_type db(10);
        assert(db.block_size() == 1);

        int16_t val = 12;
        db.set(3, val);
        assert(db.block_size() == 3);
        assert(db.is_empty(2));
        assert(!db.is_empty(3));
        assert(db.is_empty(4));

        db.set_empty(3, 3); // This should merge the top, middle and bottom blocks into one.
        assert(db.block_size() == 1);

        db.set(9, val);
        assert(db.block_size() == 2);
        db.set_empty(9, 9); // Merge the block with the top one.
        assert(db.block_size() == 1);

        db = mtv_type(10, true);
        db.set(3, 1.1);
        db.set(4, 1.2);
        db.set(5, 1.3);
        assert(db.block_size() == 3);
        db.set_empty(3, 5); // No merging.
        assert(db.block_size() == 3);
    }

    {
        mtv_type db(10);
        db.set(0, 1.1);
        assert(db.block_size() == 2);
        db.set(1, 1.2);
        assert(db.block_size() == 2);
        db.set_empty(1, 1); // Merge with the next block.
        assert(db.block_size() == 2);
        db.set(1, 1.3);

        db = mtv_type(5);
        db.set(3, 2.1);
        db.set(4, 2.2);
        assert(db.block_size() == 2);
        db.set_empty(3, 3); // Merge with the previous block.
        assert(db.block_size() == 2);
    }

    {
        mtv_type db(7);
        db.set(0, 1.2);
        db.set(2, true);
        db.set(4, true);
        db.set(5, static_cast<int32_t>(22));
        db.set(6, std::string("foo"));
        assert(db.block_size() == 7);
        db.set_empty(2, 4); // Merge with the previous block.
        assert(db.block_size() == 4);
        assert(db.get<double>(0) == 1.2);
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));
        assert(db.get<int32_t>(5) == 22);
        assert(db.get<std::string>(6) == "foo");
    }

    {
        mtv_type db(4);
        db.set(0, true);
        db.set(2, true);
        assert(db.block_size() == 4);
        db.set_empty(0, 2); // Merge with the next block.
        cout << "block size: " << db.block_size() << endl;
        assert(db.block_size() == 1);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
