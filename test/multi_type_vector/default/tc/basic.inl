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

void mtv_test_basic()
{
    stack_printer __stack_printer__(__FUNCTION__);

    bool res;
    {
        // Single column instance with only one row.
        mtv_type col_db(1);

        double test = -999.0;

        // Empty cell has a numeric value of 0.0.
        col_db.get(0, test);
        assert(test == 0.0);

        // Basic value setting and retrieval.
        res = test_cell_insertion(col_db, 0, 2.0);
        assert(res);
    }

    {
        // Insert first value into the top row.
        mtv_type col_db(2);
        double test = -999.0;

        // Test empty cell values.
        col_db.get(0, test);
        assert(test == 0.0);
        test = 1.0;
        col_db.get(1, test);
        assert(test == 0.0);

        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);

        col_db.get(1, test);
        assert(test == 0.0); // should be empty.

        // Insert a new value to an empty row right below a non-empty one.
        res = test_cell_insertion(col_db, 1, 7.5);
        assert(res);
    }

    {
        mtv_type col_db(3);
        res = test_cell_insertion(col_db, 0, 4.5);
        assert(res);
        res = test_cell_insertion(col_db, 1, 5.1);
        assert(res);
        res = test_cell_insertion(col_db, 2, 34.2);
        assert(res);
    }

    {
        // Insert first value into the bottom row.
        mtv_type col_db(3);

        res = test_cell_insertion(col_db, 2, 5.0); // Insert into the last row.
        assert(res);

        double test = 9;
        col_db.get(1, test);
        assert(test == 0.0); // should be empty.

        res = test_cell_insertion(col_db, 0, 2.5);
        assert(res);

        col_db.get(1, test);
        assert(test == 0.0); // should be empty.

        res = test_cell_insertion(col_db, 1, 1.2);
        assert(res);
    }

    {
        // This time insert from bottom up one by one.
        mtv_type col_db(3);
        res = test_cell_insertion(col_db, 2, 1.2);
        assert(res);
        res = test_cell_insertion(col_db, 1, 0.2);
        assert(res);
        res = test_cell_insertion(col_db, 0, 23.1);
        assert(res);
    }

    {
        mtv_type col_db(4);
        long order[] = {3, 1, 2, 0};
        double val = 1.0;
        for (size_t i = 0; i < 4; ++i, ++val)
        {
            res = test_cell_insertion(col_db, order[i], val);
            assert(res);
        }
    }

    {
        mtv_type col_db(4);
        long order[] = {0, 3, 1, 2};
        double val = 1.0;
        for (size_t i = 0; i < 4; ++i, ++val)
        {
            res = test_cell_insertion(col_db, order[i], val);
            assert(res);
        }
    }

    {
        mtv_type col_db(4);
        long order[] = {0, 2, 3, 1};
        double val = 1.0;
        for (size_t i = 0; i < 4; ++i, ++val)
        {
            res = test_cell_insertion(col_db, order[i], val);
            assert(res);
        }
    }

    {
        mtv_type col_db(5);
        long order[] = {0, 4, 3, 2, 1};
        double val = 1.0;
        for (size_t i = 0; i < 5; ++i, ++val)
        {
            res = test_cell_insertion(col_db, order[i], val);
            assert(res);
        }
    }

    {
        // Insert first value into a middle row.
        mtv_type col_db(10);
        res = test_cell_insertion(col_db, 5, 5.0);
        assert(res);
        std::string str = "test";
        res = test_cell_insertion(col_db, 4, str);
        assert(res);
    }

    {
        mtv_type col_db(3);
        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);
        std::string str = "test";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
        res = test_cell_insertion(col_db, 1, 2.0);
        assert(res);
    }

    {
        mtv_type col_db(2);
        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);
        std::string str = "test";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
    }

    {
        mtv_type col_db(3);
        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);
        std::string str = "test";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
        str = "foo";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
    }

    {
        mtv_type col_db(3);
        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);
        res = test_cell_insertion(col_db, 2, 2.0);
        assert(res);
        std::string str = "foo";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
    }

    {
        mtv_type col_db(3);
        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);
        std::string str = "foo";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
        str = "test";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
    }

    {
        mtv_type col_db(4);
        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);
        std::string str = "foo";
        res = test_cell_insertion(col_db, 3, str);
        assert(res);

        res = test_cell_insertion(col_db, 2, 2.0);
        assert(res);
        std::string test;
        col_db.get(3, test); // Check the cell below.
        assert(test == "foo");

        res = test_cell_insertion(col_db, 1, -2.0);
        assert(res);
        test = "hmm";
        col_db.get(3, test);
        assert(test == "foo");

        res = test_cell_insertion(col_db, 0, 7.5); // overwrite.
        assert(res);

        str = "bah";
        res = test_cell_insertion(col_db, 0, str); // overwrite with different type.
        assert(res);
        double val = -999;
        col_db.get(1, val); // Check the cell below.
        assert(val == -2.0);

        str = "alpha";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
        col_db.get(2, val); // Check the cell below.
        assert(val == 2.0);

        col_db.get(3, test);
        assert(test == "foo");

        str = "beta";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
    }

    {
        mtv_type col_db(1);
        res = test_cell_insertion(col_db, 0, 2.0);
        assert(res);
        std::string str = "foo";
        res = test_cell_insertion(col_db, 0, str);
        assert(res);
        res = test_cell_insertion(col_db, 0, 3.0);
        assert(res);
    }

    {
        mtv_type col_db(2);
        res = test_cell_insertion(col_db, 0, 2.0);
        assert(res);
        std::string str = "foo";
        res = test_cell_insertion(col_db, 0, str);
        assert(res);
        res = test_cell_insertion(col_db, 0, 3.0);
        assert(res);
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
        str = "alpha";
        res = test_cell_insertion(col_db, 0, str);
        assert(res);
        std::string test;
        col_db.get(1, test);
        assert(test == "foo");
    }

    {
        mtv_type col_db(3);
        std::string str = "alpha";
        col_db.set(2, str);
        res = test_cell_insertion(col_db, 2, 5.0);
        assert(res);

        res = test_cell_insertion(col_db, 0, 1.0);
        assert(res);
        res = test_cell_insertion(col_db, 1, 2.0);
        assert(res);

        // At this point it contains one numeric block with 3 values.

        str = "beta";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
        res = test_cell_insertion(col_db, 2, 3.0);
        assert(res);
        double test;
        col_db.get(0, test);
        assert(test == 1.0);
        col_db.get(1, test);
        assert(test == 2.0);
        col_db.get(2, test);
        assert(test == 3.0);
    }

    {
        mtv_type col_db(3);
        res = test_cell_insertion(col_db, 1, 5.0);
        assert(res);
        std::string str = "alpha";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);

        res = test_cell_insertion(col_db, 0, 4.0);
        assert(res);
        res = test_cell_insertion(col_db, 1, 3.0);
        assert(res);
        double test;
        col_db.get(0, test);
        assert(test == 4.0);

        // The top 2 cells are numeric and the bottom cell is still empty.

        str = "beta";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
        col_db.get(0, test);
        assert(test == 4.0);

        res = test_cell_insertion(col_db, 1, 6.5);
        assert(res);
        col_db.get(0, test);
        assert(test == 4.0);

        str = "gamma";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
        col_db.get(0, test);
        assert(test == 4.0);
        col_db.get(1, test);
        assert(test == 6.5);

        // The top 2 cells are numeric and the bottom cell is std::string.

        str = "delta";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
    }

    {
        mtv_type col_db(4);
        col_db.set(0, 1.0);
        std::string str = "foo";
        col_db.set(1, str);
        col_db.set(2, str);
        col_db.set(3, 4.0);

        res = test_cell_insertion(col_db, 2, 3.0);
        assert(res);
        double test;
        col_db.get(3, test);
        assert(test == 4.0);
    }

    {
        mtv_type col_db(4);
        col_db.set(0, 1.0);
        std::string str = "foo";
        col_db.set(1, str);
        col_db.set(2, str);
        col_db.set(3, str);

        res = test_cell_insertion(col_db, 3, 3.0);
        assert(res);
    }

    {
        mtv_type col_db(4);
        col_db.set(0, 1.0);
        std::string str = "foo";
        col_db.set(1, str);
        col_db.set(2, str);

        res = test_cell_insertion(col_db, 2, 3.0);
        assert(res);

        // Next cell should still be empty.
        double test_val;
        col_db.get(3, test_val);
        assert(test_val == 0.0);
        std::string test_str;
        col_db.get(3, test_str);
        assert(test_str.empty());
    }

    {
        mtv_type col_db(4);
        col_db.set(0, 1.0);
        col_db.set(1, 1.0);
        col_db.set(2, 1.0);
        col_db.set(3, 1.0);
        std::string str = "alpha";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
    }

    {
        mtv_type col_db(3);
        col_db.set(0, 1.0);
        col_db.set(1, 1.0);
        std::string str = "foo";
        col_db.set(2, str);
        uint64_t index = 5;
        test_cell_insertion(col_db, 2, index);
    }

    {
        mtv_type col_db(3);
        col_db.set(1, 1.0);
        std::string str = "foo";
        col_db.set(2, str);
        str = "bah";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
        res = test_cell_insertion(col_db, 1, 2.0);
        assert(res);
        uint64_t index = 2;
        res = test_cell_insertion(col_db, 1, index);
        assert(res);
        std::string test;
        col_db.get(2, test);
        assert(test == "foo");
        str = "alpha";
        res = test_cell_insertion(col_db, 0, str);
        assert(res);
        double val = 3.5;
        res = test_cell_insertion(col_db, 1, val);
        assert(res);
        index = 3;
        res = test_cell_insertion(col_db, 2, index);
        assert(res);

        // At this point cells 1, 2, 3 all contain different data types.

        str = "beta";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);

        // Reset.
        val = 4.5;
        res = test_cell_insertion(col_db, 1, val);
        assert(res);

        index = 4;
        res = test_cell_insertion(col_db, 1, index);
        assert(res);
    }

    {
        mtv_type col_db(3);
        col_db.set(0, 1.0);
        std::string str = "alpha";
        col_db.set(1, str);
        str = "beta";
        col_db.set(2, str);
        uint64_t index = 1;
        res = test_cell_insertion(col_db, 1, index);
        assert(res);
        std::string test;
        col_db.get(2, test);
        assert(test == "beta");
    }

    {
        mtv_type col_db(3);

        // Insert 3 cells of 3 different types.
        res = test_cell_insertion(col_db, 0, true);
        assert(res);
        res = test_cell_insertion(col_db, 1, 1.2);
        assert(res);
        std::string str = "foo";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);

        // Now, insert a cell of the 4th type to the middle spot.
        uint64_t index = 2;
        res = test_cell_insertion(col_db, 1, index);
        assert(res);
    }

    {
        // set_cell() to merge 3 blocks.
        mtv_type db(6);
        db.set(0, static_cast<uint64_t>(12));
        db.set(1, 1.0);
        db.set(2, 2.0);
        db.set(3, std::string("foo"));
        db.set(4, 3.0);
        db.set(5, 4.0);
        assert(db.block_size() == 4);
        assert(db.get<uint64_t>(0) == 12);
        assert(db.get<double>(1) == 1.0);
        assert(db.get<double>(2) == 2.0);
        assert(db.get<std::string>(3) == "foo");
        assert(db.get<double>(4) == 3.0);
        assert(db.get<double>(5) == 4.0);

        db.set(3, 5.0); // merge blocks.
        assert(db.block_size() == 2);
        assert(db.get<uint64_t>(0) == 12);
        assert(db.get<double>(1) == 1.0);
        assert(db.get<double>(2) == 2.0);
        assert(db.get<double>(3) == 5.0);
        assert(db.get<double>(4) == 3.0);
        assert(db.get<double>(5) == 4.0);
    }

    {
        mtv_type db(25);
        db.set(0, 1.2);
        db.set(5, std::string("test"));
        db.set(1, std::string("foo"));
        db.set(6, true);
        assert(db.get<double>(0) == 1.2);
        assert(db.get<std::string>(5) == "test");
        assert(db.get<std::string>(1) == "foo");
        assert(db.get<bool>(6) == true);
    }

    {
        // Test various integer types.
        mtv_type db(7);
        db.set(0, static_cast<int64_t>(-10));
        db.set(1, static_cast<uint64_t>(10));
        db.set(2, static_cast<int32_t>(-10));
        db.set(3, static_cast<uint32_t>(10));
        db.set(4, static_cast<int16_t>(-10));
        db.set(5, static_cast<uint16_t>(10));
        db.set(6, true);
        assert(db.block_size() == 7);
        assert(db.get_type(0) == mdds::mtv::element_type_int64);
        assert(db.get_type(1) == mdds::mtv::element_type_uint64);
        assert(db.get_type(2) == mdds::mtv::element_type_int32);
        assert(db.get_type(3) == mdds::mtv::element_type_uint32);
        assert(db.get_type(4) == mdds::mtv::element_type_int16);
        assert(db.get_type(5) == mdds::mtv::element_type_uint16);
        assert(db.get_type(6) == mdds::mtv::element_type_boolean);
    }

    {
        mtv_type db(10);
        db.set(0, 1.1);
        db.set(1, 1.2);
        db.set(2, true);
        db.set(3, false);
        db.set(8, std::string("A"));
        db.set(9, std::string("B"));
        db.set(7, 2.1);
        assert(db.block_size() == 5);
        assert(db.get_type(7) == mdds::mtv::element_type_double);
        assert(db.get<double>(7) == 2.1);
    }

    {
        mtv_type db(8, true);
        std::vector<double> vals(3, 1.2);
        db.set(4, vals.begin(), vals.end());
        db.set(3, 4.1);
        assert(db.get<bool>(0) == true);
        assert(db.get<bool>(1) == true);
        assert(db.get<bool>(2) == true);
        assert(db.get<double>(3) == 4.1);
        assert(db.get<double>(4) == 1.2);
        assert(db.get<double>(5) == 1.2);
        assert(db.get<double>(6) == 1.2);
        assert(db.get<bool>(7) == true);
    }

    {
        mtv_type db(10, false);
        db.set<int8_t>(0, 'a');
        db.set<int8_t>(1, 'b');
        db.set<int8_t>(2, 'c');

        db.set<uint8_t>(3, 'd');
        db.set<uint8_t>(4, 'e');
        db.set<uint8_t>(5, 'f');

        assert(db.block_size() == 3);
        db.set<int8_t>(0, 'r'); // overwrite.
        db.set<uint8_t>(5, 'z'); // overwrite

        assert(db.block_size() == 3);
        mtv_type::const_iterator it = db.begin();
        assert(it != db.end());
        assert(it->type == mdds::mtv::element_type_int8);
        {
            const int8_t* p = &mdds::mtv::int8_element_block::at(*it->data, 0);
            assert(*p == 'r');
            ++p;
            assert(*p == 'b');
            ++p;
            assert(*p == 'c');
        }

        ++it;
        assert(it != db.end());
        assert(it->type == mdds::mtv::element_type_uint8);
        {
            const uint8_t* p = mdds::mtv::uint8_element_block::data(*it->data);
            assert(*p == 'd');
            ++p;
            assert(*p == 'e');
            ++p;
            assert(*p == 'z');
        }
    }
}

/**
 * Ensure that float and double types are treated as different types.
 */
void mtv_test_basic_numeric()
{
    stack_printer __stack_printer__(__FUNCTION__);

    mtv_type db;

    db.push_back<double>(0.0);
    db.push_back<double>(1.0);
    db.push_back<double>(2.0);

    assert(db.size() == 3);
    assert(db.block_size() == 1);

    db.set<float>(1, 4.0f);
    assert(db.size() == 3);
    assert(db.block_size() == 3);

    db.set<float>(0, 3.5f);
    assert(db.size() == 3);
    assert(db.block_size() == 2);

    db.set<float>(2, 4.5f);
    assert(db.size() == 3);
    assert(db.block_size() == 1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
