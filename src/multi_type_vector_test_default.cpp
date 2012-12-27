/*************************************************************************
 *
 * Copyright (c) 2011-2012 Kohei Yoshida
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

#include "test_global.hpp"

#include <mdds/multi_type_vector.hpp>
#include <mdds/multi_type_vector_trait.hpp>

#include <cassert>
#include <sstream>
#include <vector>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/noncopyable.hpp>

#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])

using namespace std;
using namespace mdds;

namespace {

template<typename _ColT, typename _ValT>
bool test_cell_insertion(_ColT& col_db, size_t row, _ValT val)
{
    _ValT test;
    col_db.set(row, val);
    col_db.get(row, test);
    return val == test;
}

typedef mdds::multi_type_vector<mdds::mtv::element_block_func> column_type;

enum test_mtv_type {
    _bool, _short, _ushort, _int, _uint, _long, _ulong, _double, _string
};

#define TEST_TYPE(_type_,_type_enum_) test_mtv_type test_type(_type_) { return _type_enum_; }
TEST_TYPE(bool,_bool)
TEST_TYPE(short,_short)
TEST_TYPE(unsigned short,_ushort)
TEST_TYPE(int,_int)
TEST_TYPE(unsigned int,_uint)
TEST_TYPE(long,_long)
TEST_TYPE(unsigned long,_ulong)
TEST_TYPE(double,_double)
TEST_TYPE(string,_string)

void mtv_test_types()
{
    // Test function overloading of standard types.

    stack_printer __stack_printer__("::mtv_test_types");
    {
        bool val = false;
        assert(test_type(val) == _bool);
        cout << "bool is good" << endl;
    }
    {
        short val = 0;
        assert(test_type(val) == _short);
        cout << "short is good" << endl;
    }
    {
        unsigned short val = 0;
        assert(test_type(val) == _ushort);
        cout << "unsigned short is good" << endl;
    }
    {
        int val = 0;
        assert(test_type(val) == _int);
        cout << "int is good" << endl;
    }
    {
        unsigned int val = 0;
        assert(test_type(val) == _uint);
        cout << "unsigned int is good" << endl;
    }
    {
        long val = 0;
        assert(test_type(val) == _long);
        cout << "long is good" << endl;
    }
    {
        unsigned long val = 0;
        assert(test_type(val) == _ulong);
        cout << "unsigned long is good" << endl;
    }
    {
        double val = 0;
        assert(test_type(val) == _double);
        cout << "double is good" << endl;
    }
    {
        string val;
        assert(test_type(val) == _string);
        cout << "string is good" << endl;
    }
}

void mtv_test_construction()
{
    stack_printer __stack_printer__("::mtv_test_construction");
    {
        // Create with initial value and size.
        column_type db(10, 1.0);
        assert(db.size() == 10);
        assert(db.block_size() == 1);
        assert(db.get<double>(0) == 1.0);
        assert(db.get<double>(9) == 1.0);
    }

    {
        // Create with initial value and size.
        column_type db(10, string("foo"));
        assert(db.size() == 10);
        assert(db.block_size() == 1);
        assert(db.get<string>(0) == "foo");
        assert(db.get<string>(9) == "foo");
    }
}

void mtv_test_basic()
{
    stack_printer __stack_printer__("::mtv_test_basic");
    bool res;

    {
        // Single column instance with only one row.
        column_type col_db(1);

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
        column_type col_db(2);
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
        column_type col_db(3);
        res = test_cell_insertion(col_db, 0, 4.5);
        assert(res);
        res = test_cell_insertion(col_db, 1, 5.1);
        assert(res);
        res = test_cell_insertion(col_db, 2, 34.2);
        assert(res);
    }

    {
        // Insert first value into the bottom row.
        column_type col_db(3);

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
        column_type col_db(3);
        res = test_cell_insertion(col_db, 2, 1.2);
        assert(res);
        res = test_cell_insertion(col_db, 1, 0.2);
        assert(res);
        res = test_cell_insertion(col_db, 0, 23.1);
        assert(res);
    }

    {
        column_type col_db(4);
        long order[] = { 3, 1, 2, 0 };
        double val = 1.0;
        for (size_t i = 0; i < 4; ++i, ++val)
        {
            res = test_cell_insertion(col_db, order[i], val);
            assert(res);
        }
    }

    {
        column_type col_db(4);
        long order[] = { 0, 3, 1, 2 };
        double val = 1.0;
        for (size_t i = 0; i < 4; ++i, ++val)
        {
            res = test_cell_insertion(col_db, order[i], val);
            assert(res);
        }
    }

    {
        column_type col_db(4);
        long order[] = { 0, 2, 3, 1 };
        double val = 1.0;
        for (size_t i = 0; i < 4; ++i, ++val)
        {
            res = test_cell_insertion(col_db, order[i], val);
            assert(res);
        }
    }

    {
        column_type col_db(5);
        long order[] = { 0, 4, 3, 2, 1 };
        double val = 1.0;
        for (size_t i = 0; i < 5; ++i, ++val)
        {
            res = test_cell_insertion(col_db, order[i], val);
            assert(res);
        }
    }

    {
        // Insert first value into a middle row.
        column_type col_db(10);
        res = test_cell_insertion(col_db, 5, 5.0);
        assert(res);
        string str = "test";
        res = test_cell_insertion(col_db, 4, str);
        assert(res);
    }

    {
        column_type col_db(3);
        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);
        string str = "test";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
        res = test_cell_insertion(col_db, 1, 2.0);
        assert(res);
    }

    {
        column_type col_db(2);
        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);
        string str = "test";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
    }

    {
        column_type col_db(3);
        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);
        string str = "test";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
        str = "foo";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
    }

    {
        column_type col_db(3);
        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);
        res = test_cell_insertion(col_db, 2, 2.0);
        assert(res);
        string str = "foo";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
    }

    {
        column_type col_db(3);
        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);
        string str = "foo";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
        str = "test";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
    }

    {
        column_type col_db(4);
        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);
        string str = "foo";
        res = test_cell_insertion(col_db, 3, str);
        assert(res);

        res = test_cell_insertion(col_db, 2, 2.0);
        assert(res);
        string test;
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
        column_type col_db(1);
        res = test_cell_insertion(col_db, 0, 2.0);
        assert(res);
        string str = "foo";
        res = test_cell_insertion(col_db, 0, str);
        assert(res);
        res = test_cell_insertion(col_db, 0, 3.0);
        assert(res);
    }

    {
        column_type col_db(2);
        res = test_cell_insertion(col_db, 0, 2.0);
        assert(res);
        string str = "foo";
        res = test_cell_insertion(col_db, 0, str);
        assert(res);
        res = test_cell_insertion(col_db, 0, 3.0);
        assert(res);
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
        str = "alpha";
        res = test_cell_insertion(col_db, 0, str);
        assert(res);
        string test;
        col_db.get(1, test);
        assert(test == "foo");
    }

    {
        column_type col_db(3);
        string str = "alpha";
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
        column_type col_db(3);
        res = test_cell_insertion(col_db, 1, 5.0);
        assert(res);
        string str = "alpha";
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

        // The top 2 cells are numeric and the bottom cell is string.

        str = "delta";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
    }

    {
        column_type col_db(4);
        col_db.set(0, 1.0);
        string str = "foo";
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
        column_type col_db(4);
        col_db.set(0, 1.0);
        string str = "foo";
        col_db.set(1, str);
        col_db.set(2, str);
        col_db.set(3, str);

        res = test_cell_insertion(col_db, 3, 3.0);
        assert(res);
    }

    {
        column_type col_db(4);
        col_db.set(0, 1.0);
        string str = "foo";
        col_db.set(1, str);
        col_db.set(2, str);

        res = test_cell_insertion(col_db, 2, 3.0);
        assert(res);

        // Next cell should still be empty.
        double test_val;
        col_db.get(3, test_val);
        assert(test_val == 0.0);
        string test_str;
        col_db.get(3, test_str);
        assert(test_str.empty());
    }

    {
        column_type col_db(4);
        col_db.set(0, 1.0);
        col_db.set(1, 1.0);
        col_db.set(2, 1.0);
        col_db.set(3, 1.0);
        string str = "alpha";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
    }

    {
        column_type col_db(3);
        col_db.set(0, 1.0);
        col_db.set(1, 1.0);
        string str = "foo";
        col_db.set(2, str);
        unsigned long index = 5;
        test_cell_insertion(col_db, 2, index);
    }

    {
        column_type col_db(3);
        col_db.set(1, 1.0);
        string str = "foo";
        col_db.set(2, str);
        str = "bah";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
        res = test_cell_insertion(col_db, 1, 2.0);
        assert(res);
        unsigned long index = 2;
        res = test_cell_insertion(col_db, 1, index);
        assert(res);
        string test;
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
        column_type col_db(3);
        col_db.set(0, 1.0);
        string str = "alpha";
        col_db.set(1, str);
        str = "beta";
        col_db.set(2, str);
        unsigned long index = 1;
        res = test_cell_insertion(col_db, 1, index);
        assert(res);
        string test;
        col_db.get(2, test);
        assert(test == "beta");
    }

    {
        column_type col_db(3);

        // Insert 3 cells of 3 different types.
        res = test_cell_insertion(col_db, 0, true);
        assert(res);
        res = test_cell_insertion(col_db, 1, 1.2);
        assert(res);
        string str = "foo";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);

        // Now, insert a cell of the 4th type to the middle spot.
        unsigned long index = 2;
        res = test_cell_insertion(col_db, 1, index);
        assert(res);
    }

    {
        // set_cell() to merge 3 blocks.
        column_type db(6);
        db.set(0, static_cast<unsigned long>(12));
        db.set(1, 1.0);
        db.set(2, 2.0);
        db.set(3, string("foo"));
        db.set(4, 3.0);
        db.set(5, 4.0);
        assert(db.block_size() == 4);
        assert(db.get<unsigned long>(0) == 12);
        assert(db.get<double>(1) == 1.0);
        assert(db.get<double>(2) == 2.0);
        assert(db.get<string>(3) == "foo");
        assert(db.get<double>(4) == 3.0);
        assert(db.get<double>(5) == 4.0);

        db.set(3, 5.0); // merge blocks.
        assert(db.block_size() == 2);
        assert(db.get<unsigned long>(0) == 12);
        assert(db.get<double>(1) == 1.0);
        assert(db.get<double>(2) == 2.0);
        assert(db.get<double>(3) == 5.0);
        assert(db.get<double>(4) == 3.0);
        assert(db.get<double>(5) == 4.0);
    }

    {
        column_type db(25);
        db.set(0, 1.2);
        db.set(5, string("test"));
        db.set(1, string("foo"));
        db.set(6, true);
        assert(db.get<double>(0) == 1.2);
        assert(db.get<string>(5) == "test");
        assert(db.get<string>(1) == "foo");
        assert(db.get<bool>(6) == true);
    }

    {
        // Test various integer types.
        column_type db(7);
        db.set(0, static_cast<long>(-10));
        db.set(1, static_cast<unsigned long>(10));
        db.set(2, static_cast<int>(-10));
        db.set(3, static_cast<unsigned int>(10));
        db.set(4, static_cast<short>(-10));
        db.set(5, static_cast<unsigned short>(10));
        db.set(6, true);
        assert(db.block_size() == 7);
        assert(db.get_type(0) == mtv::element_type_long);
        assert(db.get_type(1) == mtv::element_type_ulong);
        assert(db.get_type(2) == mtv::element_type_int);
        assert(db.get_type(3) == mtv::element_type_uint);
        assert(db.get_type(4) == mtv::element_type_short);
        assert(db.get_type(5) == mtv::element_type_ushort);
        assert(db.get_type(6) == mtv::element_type_boolean);
    }
}

void mtv_test_empty_cells()
{
    stack_printer __stack_printer__("::mtv_test_empty");
    {
        column_type db(3);
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
        column_type db(4);
        for (size_t i = 0; i < 4; ++i)
            db.set(i, static_cast<double>(i+1));

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
        column_type db(2);
        db.set(0, 1.0);
        db.set(1, string("foo"));
        assert(!db.is_empty(0));
        assert(!db.is_empty(1));

        db.set_empty(0, 1);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
    }

    {
        // Empty multiple blocks, part 2 - from middle block to middle block.
        column_type db(6);
        db.set(0, 1.0);
        db.set(1, 2.0);
        string str = "foo";
        db.set(2, str);
        db.set(3, str);
        unsigned long index = 1;
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
        unsigned long index_test;
        db.get(5, index_test);
        assert(index_test == 100);
    }

    {
        // Empty multiple blocks, part 3 - from top block to middle block.
        column_type db(6);
        db.set(0, 1.0);
        db.set(1, 2.0);
        string str = "foo";
        db.set(2, str);
        db.set(3, str);
        unsigned long index = 1;
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
        unsigned long test;
        db.get(5, test);
        assert(test == 50);
    }

    {
        // Empty multiple blocks, part 4 - from middle block to bottom block.
        column_type db(6);
        db.set(0, 1.0);
        db.set(1, 2.0);
        string str = "foo";
        db.set(2, str);
        db.set(3, str);
        unsigned long index = 1;
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
        column_type db(6);
        db.set(2, 1.0);
        db.set(3, 2.0);
        string str = "foo";
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
        string test;
        db.get(5, test);
        assert(test == "baa");
    }

    {
        // Empty multiple blocks, part 6 - from middle non-empty block to middle empty block.
        column_type db(6);
        db.set(0, 1.0);
        db.set(1, 2.0);
        db.set(2, string("foo"));
        db.set(3, string("baa"));
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
        column_type db(6);
        db.set(2, 1.0);
        db.set(3, string("foo"));
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
        column_type db(5);
        db.set(0, 1.0);
        db.set(1, static_cast<unsigned long>(1));
        db.set(2, static_cast<unsigned long>(2));
        db.set(3, static_cast<unsigned long>(3));
        db.set(4, static_cast<unsigned long>(4));
        db.set_empty(2, 4);
        assert(!db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));

        db.set(2, static_cast<unsigned long>(5));
        db.set(3, static_cast<unsigned long>(6));
        db.set(4, static_cast<unsigned long>(7));
        db.set_empty(1, 2);
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(!db.is_empty(3));
        assert(!db.is_empty(4));

        db.set(3, static_cast<unsigned long>(8));
        db.set(4, static_cast<unsigned long>(9));
        db.set_empty(2, 3);
    }

    {
        // Set individual single elements empty.
        cout << "Setting individual single elements empty..." << endl;
        column_type db(15, 1.2);
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
}

void mtv_test_swap()
{
    stack_printer __stack_printer__("::mtv_test_swap");
    column_type db1(3), db2(5);
    db1.set(0, 1.0);
    db1.set(1, 2.0);
    db1.set(2, 3.0);

    db2.set(0, 4.0);
    db2.set(1, 5.0);
    db2.set(4, string("foo"));
    db1.swap(db2);

    assert(db1.size() == 5 && db1.block_size() == 3);
    assert(db2.size() == 3 && db2.block_size() == 1);
}

void mtv_test_equality()
{
    stack_printer __stack_printer__("::mtv_test_equality");
    {
        // Two columns of equal size.
        column_type db1(3), db2(3);
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
        column_type db1(3), db2(4);
        assert(db1 != db2);
        db1.set(0, 1.2);
        db2.set(0, 1.2);
        assert(db1 != db2);

        // Comparison to self.
        assert(db1 == db1);
        assert(db2 == db2);
    }
}

void mtv_test_clone()
{
    stack_printer __stack_printer__("::mtv_test_clone");
    column_type db1(3);
    db1.set(0, 3.4);
    db1.set(1, string("foo"));
    db1.set(2, true);

    // copy construction

    column_type db2(db1);
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
        string test1, test2;
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

    column_type db3 = db1;
    assert(db3 == db1);
    db3.set(0, string("alpha"));
    assert(db3 != db1);

    column_type db4, db5;
    db4 = db5 = db3;
    assert(db4 == db5);
    assert(db3 == db5);
    assert(db3 == db4);
}

void mtv_test_resize()
{
    stack_printer __stack_printer__("::mtv_test_resize");
    column_type db(0);
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

void mtv_test_erase()
{
    stack_printer __stack_printer__("::mtv_test_erase");
    {
        // Single empty block.
        column_type db(5);
        db.erase(0, 2); // erase rows 0-2.
        assert(db.size() == 2);
        db.erase(0, 1);
        assert(db.size() == 0);
        assert(db.empty());
    }

    {
        // Single non-empty block.
        column_type db(5);
        for (long i = 0; i < 5; ++i)
            db.set(i, static_cast<double>(i+1));

        assert(db.block_size() == 1);
        assert(db.size() == 5);

        db.erase(0, 2); // erase rows 0-2
        assert(db.size() == 2);
        double test;
        db.get(0, test);
        assert(test == 4.0);
        db.get(1, test);
        assert(test == 5.0);

        db.erase(0, 1);
        assert(db.size() == 0);
        assert(db.empty());
    }

    {
        // Two blocks - non-empty to empty blocks.
        column_type db(8);
        for (long i = 0; i < 4; ++i)
            db.set(i, static_cast<double>(i+1));

        assert(db.block_size() == 2);
        assert(db.size() == 8);
        assert(!db.is_empty(3));
        assert(db.is_empty(4));

        // Erase across two blocks.
        db.erase(3, 6); // 4 cells
        assert(db.block_size() == 2);
        assert(db.size() == 4);

        // Check the integrity of the data.
        double test;
        db.get(2, test);
        assert(test == 3.0);
        assert(db.is_empty(3));

        // Empty it.
        db.erase(0, 3);
        assert(db.block_size() == 0);
        assert(db.size() == 0);
        assert(db.empty());
    }

    {
        // Two blocks - non-empty to non-empty blocks.
        column_type db(8);
        for (long i = 0; i < 4; ++i)
            db.set(i, static_cast<double>(i+1));

        for (long i = 4; i < 8; ++i)
            db.set(i, static_cast<unsigned long>(i+1));

        assert(db.block_size() == 2);
        assert(db.size() == 8);

        // Erase across two blocks.
        db.erase(3, 6); // 4 cells
        assert(db.block_size() == 2);
        assert(db.size() == 4);

        // Check the integrity of the data.
        double test;
        db.get(2, test);
        assert(test == 3.0);

        unsigned long test2;
        db.get(3, test2);
        assert(test2 == 8);

        // Empty it.
        db.erase(0, 3);
        assert(db.block_size() == 0);
        assert(db.size() == 0);
        assert(db.empty());
    }

    {
        // 3 blocks, all non-empty.
        column_type db(9);
        for (long i = 0; i < 3; ++i)
            db.set(i, static_cast<double>(i+1));

        for (long i = 3; i < 6; ++i)
            db.set(i, static_cast<unsigned long>(i+1));

        for (long i = 6; i < 9; ++i)
        {
            ostringstream os;
            os << i+1;
            db.set(i, os.str());
        }

        assert(db.block_size() == 3);
        assert(db.size() == 9);

        db.erase(2, 7);
        assert(db.block_size() == 2);
        assert(db.size() == 3);

        // Check the integrity of the data.
        double test1;
        db.get(1, test1);
        assert(test1 == 2.0);
        string test2;
        db.get(2, test2);
        assert(test2 == "9");

        db.erase(2, 2); // Erase only one-block.
        assert(db.block_size() == 1);
        assert(db.size() == 2);
        test1 = -1.0;
        db.get(1, test1);
        assert(test1 == 2.0);

        db.erase(0, 1);
        assert(db.size() == 0);
        assert(db.empty());
    }

    {
        // erase() to merge two blocks.
        column_type db(4);
        db.set(0, 1.1);
        db.set(1, string("foo"));
        db.set(2, static_cast<unsigned long>(2));
        db.set(3, string("baa"));
        assert(db.block_size() == 4);
        assert(db.size() == 4);

        db.erase(2, 2);
        assert(db.block_size() == 2);
        assert(db.size() == 3);

        // Try again, but this time merge two empty blocks.
        db.resize(4);
        db.set_empty(1, 3);
        db.set(2, static_cast<unsigned long>(10));
        assert(db.get<double>(0) == 1.1);
        assert(db.is_empty(1));
        assert(db.get<unsigned long>(2) == 10);
        assert(db.is_empty(3));

        db.erase(2, 2);
        assert(db.block_size() == 2);
        assert(db.size() == 3);
        assert(db.get<double>(0) == 1.1);
        assert(db.is_empty(1));
        assert(db.is_empty(2));
    }
}

void mtv_test_insert_empty()
{
    stack_printer __stack_printer__("::mtv_test_insert_empty");
    {
        column_type db(5);
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
        column_type db(5);
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
        column_type db(1);
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
        column_type db(2);
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
}

void mtv_test_set_cells()
{
    stack_printer __stack_printer__("::mtv_test_set_cells");
    {
        column_type db(5);

        // Replace the whole block.

        {
            vector<double> vals;
            vals.reserve(5);
            for (size_t i = 0; i < db.size(); ++i)
                vals.push_back(i+1);

            db.set(0, vals.begin(), vals.end());

            double test;
            db.get(0, test);
            assert(test == 1.0);
            db.get(4, test);
            assert(test == 5.0);
        }

        {
            vector<string> vals;
            vals.reserve(5);
            for (size_t i = 0; i < db.size(); ++i)
            {
                ostringstream os;
                os << (i+1);
                vals.push_back(os.str());
            }

            db.set(0, vals.begin(), vals.end());

            string test;
            db.get(0, test);
            assert(test == "1");
            db.get(4, test);
            assert(test == "5");
        }

        {
            double vals[] = { 5.0, 6.0, 7.0, 8.0, 9.0 };
            double* p = &vals[0];
            double* p_end = p + 5;
            db.set(0, p, p_end);
            double test;
            db.get(0, test);
            assert(test == 5.0);
            db.get(4, test);
            assert(test == 9.0);
        }

        {
            // Replace the whole block of the same type, which shouldn't
            // delete the old data array.
            double vals[] = { 5.1, 6.1, 7.1, 8.1, 9.1 };
            double* p = &vals[0];
            double* p_end = p + 5;
            db.set(0, p, p_end);
            double test;
            db.get(0, test);
            assert(test == 5.1);
            db.get(4, test);
            assert(test == 9.1);

            double vals2[] = { 8.2, 9.2 };
            p = &vals2[0];
            p_end = p + 2;
            db.set(3, p, p_end);
            db.get(3, test);
            assert(test == 8.2);
            db.get(4, test);
            assert(test == 9.2);
        }


        {
            // Replace the upper part of a single block.
            unsigned long vals[] = { 1, 2, 3 };
            unsigned long* p = &vals[0];
            unsigned long* p_end = p + 3;
            db.set(0, p, p_end);
            assert(db.block_size() == 2);
            assert(db.size() == 5);
            unsigned long test;
            db.get(0, test);
            assert(test == 1);
            db.get(2, test);
            assert(test == 3);
            double test2;
            db.get(3, test2);
            assert(test2 == 8.2);
        }

        {
            // Merge with the previos block and erase the whole block.
            unsigned long vals[] = { 4, 5 };
            unsigned long* p = &vals[0];
            unsigned long* p_end = p + 2;
            db.set(3, p, p_end);
            assert(db.block_size() == 1);
            assert(db.size() == 5);
            unsigned long test;
            db.get(2, test);
            assert(test == 3);
            db.get(3, test);
            assert(test == 4);
        }

        {
            // Merge with the previous block while keeping the lower part of
            // the block.
            unsigned long prev_value;
            db.get(2, prev_value);

            double val = 2.3;
            db.set(0, val);
            assert(db.block_size() == 2);
            val = 4.5;
            double* p = &val;
            double* p_end = p + 1;
            db.set(1, p, p_end);
            assert(db.block_size() == 2);
            assert(db.size() == 5);
            {
                double test;
                db.get(0, test);
                assert(test == 2.3);
                db.get(1, test);
                assert(test == 4.5);
            }

            unsigned long test;
            db.get(2, test);
            assert(test == prev_value);
        }
    }

    {
        column_type db(5);
        for (size_t i = 0; i < 5; ++i)
            db.set(i, static_cast<double>(i+1));

        assert(db.block_size() == 1);
        assert(db.size() == 5);

        {
            unsigned long vals[] = { 10, 11 };
            unsigned long* p = &vals[0];
            unsigned long* p_end = p + 2;
            db.set(3, p, p_end);

            assert(db.block_size() == 2);
            assert(db.size() == 5);

            double test;
            db.get(2, test);
            assert(test == 3.0);
            unsigned long test2;
            db.get(3, test2);
            assert(test2 == 10);
            db.get(4, test2);
            assert(test2 == 11);

            // Insertion into a single block but this time it needs to be
            // merged with the subsequent block.
            db.set(1, p, p_end);

            assert(db.block_size() == 2);
            assert(db.size() == 5);

            db.get(1, test2);
            assert(test2 == 10);
            db.get(2, test2);
            assert(test2 == 11);
            db.get(3, test2);
            assert(test2 == 10);
            db.get(4, test2);
            assert(test2 == 11);
        }
    }

    {
        column_type db(6);
        double vals_d[] = { 1.0, 1.1, 1.2, 1.3, 1.4, 1.5 };
        unsigned long vals_i[] = { 12, 13, 14, 15 };
        string vals_s[] = { "a", "b" };

        {
            double* p = &vals_d[0];
            double* p_end = p + 6;
            db.set(0, p, p_end);
            assert(db.block_size() == 1);
            assert(db.size() == 6);
            double test;
            db.get(0, test);
            assert(test == 1.0);
            db.get(5, test);
            assert(test == 1.5);
        }

        {
            unsigned long* p = &vals_i[0];
            unsigned long* p_end = p + 4;
            db.set(0, p, p_end);
            assert(db.block_size() == 2);
            unsigned long test;
            db.get(0, test);
            assert(test == 12);
            db.get(3, test);
            assert(test == 15);
        }

        {
            string* p = &vals_s[0];
            string* p_end = p + 2;
            db.set(2, p, p_end);
            assert(db.block_size() == 3);
            string test;
            db.get(2, test);
            assert(test == "a");
            db.get(3, test);
            assert(test == "b");
            double test_d;
            db.get(4, test_d);
            assert(test_d == 1.4);
            unsigned long test_i;
            db.get(1, test_i);
            assert(test_i == 13);
        }
    }

    {
        column_type db(3);
        {
            double vals[] = { 2.1, 2.2, 2.3 };
            double* p = &vals[0];
            double* p_end = p + 3;
            db.set(0, p, p_end);
            assert(db.block_size() == 1);
        }

        {
            unsigned long val_i = 23;
            unsigned long* p = &val_i;
            unsigned long* p_end = p + 1;
            db.set(1, p, p_end);
            assert(db.block_size() == 3);
            unsigned long test;
            db.get(1, test);
            assert(test == 23);
            double test_d;
            db.get(0, test_d);
            assert(test_d == 2.1);
            db.get(2, test_d);
            assert(test_d == 2.3);
        }
    }

    {
        // Set cells over multiple blocks. Very simple case.

        column_type db(2);
        db.set(0, static_cast<double>(1.1));
        db.set(1, string("foo"));
        assert(db.block_size() == 2);

        double vals[] = { 2.1, 2.2 };
        double* p = &vals[0];
        double* p_end = p + 2;
        db.set(0, p, p_end);
        assert(db.block_size() == 1);
        assert(db.size() == 2);

        double test;
        db.get(0, test);
        assert(test == 2.1);
        db.get(1, test);
        assert(test == 2.2);
    }

    {
        // Same as above, except that the last block is only partially replaced.

        column_type db(3);
        db.set(0, static_cast<double>(1.1));
        db.set(1, string("foo"));
        db.set(2, string("baa"));

        double vals[] = { 2.1, 2.2 };
        double* p = &vals[0];
        double* p_end = p + 2;
        db.set(0, p, p_end);
        assert(db.block_size() == 2);
        assert(db.size() == 3);

        double test_val;
        db.get(0, test_val);
        assert(test_val == 2.1);
        db.get(1, test_val);
        assert(test_val == 2.2);

        string test_s;
        db.get(2, test_s);
        assert(test_s == "baa");
    }

    {
        column_type db(3);
        db.set(0, static_cast<double>(3.1));
        db.set(1, static_cast<double>(3.2));
        db.set(2, string("foo"));
        assert(db.block_size() == 2);
        assert(db.size() == 3);

        double vals[] = { 2.1, 2.2 };
        double* p = &vals[0];
        double* p_end = p + 2;
        db.set(1, p, p_end);
        assert(db.block_size() == 1);
        double test;
        db.get(0, test);
        assert(test == 3.1);
        db.get(1, test);
        assert(test == 2.1);
        db.get(2, test);
        assert(test == 2.2);
    }

    {
        column_type db(5);
        db.set(0, 1.1);
        db.set(1, 1.2);
        db.set(2, string("foo"));
        db.set(3, 1.3);
        db.set(4, 1.4);
        assert(db.block_size() == 3);

        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        double* p_end = p + 3;
        db.set(1, p, p_end);
        assert(db.block_size() == 1);
        assert(db.size() == 5);
        assert(db.get<double>(0) == 1.1);
        assert(db.get<double>(1) == 2.1);
        assert(db.get<double>(2) == 2.2);
        assert(db.get<double>(3) == 2.3);
        assert(db.get<double>(4) == 1.4);
    }

    {
        column_type db(4);
        db.set(0, string("A"));
        db.set(1, string("B"));
        db.set(2, 1.1);
        db.set(3, 1.2);
        assert(db.block_size() == 2);

        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        db.set(1, p, p+3);
        assert(db.block_size() == 2);
        assert(db.get<string>(0) == string("A"));
        assert(db.get<double>(1) == 2.1);
        assert(db.get<double>(2) == 2.2);
        assert(db.get<double>(3) == 2.3);
    }

    {
        column_type db(4);
        db.set(0, string("A"));
        db.set(1, string("B"));
        db.set(2, 1.1);
        db.set(3, 1.2);
        assert(db.block_size() == 2);

        double vals[] = { 2.1, 2.2 };
        double* p = &vals[0];
        db.set(1, p, p+2);
        assert(db.block_size() == 2);
        assert(db.get<string>(0) == string("A"));
        assert(db.get<double>(1) == 2.1);
        assert(db.get<double>(2) == 2.2);
        assert(db.get<double>(3) == 1.2);
    }

    {
        column_type db(5);
        db.set(0, string("A"));
        db.set(1, string("B"));
        db.set(2, 1.1);
        db.set(3, 1.2);
        db.set(4, static_cast<unsigned long>(12));
        assert(db.block_size() == 3);

        unsigned long vals[] = { 21, 22, 23 };
        unsigned long* p = &vals[0];
        db.set(1, p, p+3);
        assert(db.block_size() == 2);
        assert(db.get<string>(0) == string("A"));
        assert(db.get<unsigned long>(1) == 21);
        assert(db.get<unsigned long>(2) == 22);
        assert(db.get<unsigned long>(3) == 23);
        assert(db.get<unsigned long>(4) == 12);
    }

    {
        column_type db(3);
        db.set(0, string("A"));
        db.set(1, 1.1);
        db.set(2, 1.2);
        assert(db.block_size() == 2);

        unsigned long vals[] = { 11, 12 };
        unsigned long* p = &vals[0];
        db.set(0, p, p+2);
        assert(db.block_size() == 2);
        assert(db.get<unsigned long>(0) == 11);
        assert(db.get<unsigned long>(1) == 12);
        assert(db.get<double>(2) == 1.2);
    }

    {
        column_type db(4);
        db.set(0, static_cast<unsigned long>(35));
        db.set(1, string("A"));
        db.set(2, 1.1);
        db.set(3, 1.2);
        assert(db.block_size() == 3);

        unsigned long vals[] = { 11, 12 };
        unsigned long* p = &vals[0];
        db.set(1, p, p+2);
        assert(db.block_size() == 2);
        assert(db.get<unsigned long>(0) == 35);
        assert(db.get<unsigned long>(1) == 11);
        assert(db.get<unsigned long>(2) == 12);
        assert(db.get<double>(3) == 1.2);
    }

    {
        // Block 1 is empty.

        column_type db(2);
        db.set(1, 1.2);
        assert(db.block_size() == 2);

        double vals[] = { 2.1, 2.2 };
        double* p = &vals[0];
        db.set(0, p, p+2);
        assert(db.block_size() == 1);
        assert(db.get<double>(0) == 2.1);
        assert(db.get<double>(1) == 2.2);
    }

    {
        column_type db(3);
        db.set(0, 1.1);
        db.set(2, 1.2);
        assert(db.block_size() == 3);

        double vals[] = { 2.1, 2.2 };
        double* p = &vals[0];
        db.set(1, p, p+2);
        assert(db.block_size() == 1);
        assert(db.get<double>(0) == 1.1);
        assert(db.get<double>(1) == 2.1);
        assert(db.get<double>(2) == 2.2);
    }

    {
        column_type db(5);
        db.set(2, string("A"));
        db.set(3, string("B"));
        db.set(4, string("C"));
        assert(db.block_size() == 2);

        double vals[] = { 1.1, 1.2, 1.3 };
        double* p = &vals[0];
        db.set(1, p, p+3);
        assert(db.block_size() == 3);

        assert(db.is_empty(0));
        assert(db.get<double>(1) == 1.1);
        assert(db.get<double>(2) == 1.2);
        assert(db.get<double>(3) == 1.3);
        assert(db.get<string>(4) == string("C"));
    }
}

void mtv_test_insert_cells()
{
    stack_printer __stack_printer__("::mtv_test_insert_cells");
    {
        // Insert into non-empty block of the same type.
        column_type db(1);
        db.set(0, 1.1);
        assert(db.block_size() == 1);
        assert(db.size() == 1);

        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        db.insert(0, p, p+3);
        assert(db.block_size() == 1);
        assert(db.size() == 4);
        assert(db.get<double>(0) == 2.1);
        assert(db.get<double>(1) == 2.2);
        assert(db.get<double>(2) == 2.3);
        assert(db.get<double>(3) == 1.1);
    }

    {
        // Insert into an existing empty block.
        column_type db(1);
        assert(db.block_size() == 1);
        assert(db.size() == 1);

        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        db.insert(0, p, p+3);
        assert(db.block_size() == 2);
        assert(db.size() == 4);
        assert(db.get<double>(0) == 2.1);
        assert(db.get<double>(1) == 2.2);
        assert(db.get<double>(2) == 2.3);
        assert(db.is_empty(3));
    }

    {
        column_type db(2);
        db.set(0, 1.1);
        assert(db.block_size() == 2);
        assert(db.size() == 2);

        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        db.insert(1, p, p+3);
        assert(db.block_size() == 2);
        assert(db.size() == 5);
        assert(db.get<double>(0) == 1.1);
        assert(db.get<double>(1) == 2.1);
        assert(db.get<double>(2) == 2.2);
        assert(db.get<double>(3) == 2.3);
        assert(db.is_empty(4));
    }

    {
        column_type db(2);
        db.set(0, static_cast<unsigned long>(23));
        assert(db.block_size() == 2);
        assert(db.size() == 2);

        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        db.insert(1, p, p+3);
        assert(db.block_size() == 3);
        assert(db.size() == 5);
        assert(db.get<unsigned long>(0) == 23);
        assert(db.get<double>(1) == 2.1);
        assert(db.get<double>(2) == 2.2);
        assert(db.get<double>(3) == 2.3);
        assert(db.is_empty(4));
    }

    {
        column_type db(2);
        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        db.insert(1, p, p+3);
        assert(db.block_size() == 3);
        assert(db.size() == 5);
        assert(db.is_empty(0));
        assert(db.get<double>(1) == 2.1);
        assert(db.get<double>(2) == 2.2);
        assert(db.get<double>(3) == 2.3);
        assert(db.is_empty(4));
    }

    {
        column_type db(2);
        db.set(0, 1.1);
        db.set(1, static_cast<unsigned long>(23));
        assert(db.block_size() == 2);
        assert(db.size() == 2);

        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        db.insert(1, p, p+3);
        assert(db.block_size() == 2);
        assert(db.size() == 5);
        assert(db.get<double>(0) == 1.1);
        assert(db.get<double>(1) == 2.1);
        assert(db.get<double>(2) == 2.2);
        assert(db.get<double>(3) == 2.3);
        assert(db.get<unsigned long>(4) == 23);
    }

    {
        column_type db(2);
        db.set(0, true);
        db.set(1, static_cast<unsigned long>(23));
        assert(db.block_size() == 2);
        assert(db.size() == 2);

        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        db.insert(1, p, p+3);
        assert(db.block_size() == 3);
        assert(db.size() == 5);
        assert(db.get<bool>(0) == true);
        assert(db.get<double>(1) == 2.1);
        assert(db.get<double>(2) == 2.2);
        assert(db.get<double>(3) == 2.3);
        assert(db.get<unsigned long>(4) == 23);
    }

    {
        column_type db(2);
        db.set(0, static_cast<unsigned long>(12));
        db.set(1, static_cast<unsigned long>(23));
        assert(db.block_size() == 1);
        assert(db.size() == 2);

        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        db.insert(1, p, p+3);
        assert(db.block_size() == 3);
        assert(db.size() == 5);
        assert(db.get<unsigned long>(0) == 12);
        assert(db.get<double>(1) == 2.1);
        assert(db.get<double>(2) == 2.2);
        assert(db.get<double>(3) == 2.3);
        assert(db.get<unsigned long>(4) == 23);
    }

    {
        column_type db(3);
        db.set(0, 1.0);
        db.set(1, string("foo"));
        db.set(2, string("baa"));
        assert(db.size() == 3);
        assert(db.block_size() == 2);
        double vals[] = { 2.1 };
        const double* p = &vals[0];
        db.insert(2, p, p+1);
        assert(db.size() == 4);
        assert(db.block_size() == 4);
    }

    {
        column_type db(2);
        db.set(0, static_cast<unsigned long>(11));
        db.set(1, static_cast<unsigned long>(12));
        double vals[] = { 1.2 };
        const double* p = &vals[0];
        db.insert(1, p, p+1);
        assert(db.block_size() == 3);

        // Append value to the top block.
        unsigned long vals2[] = { 22 };
        const unsigned long* p2 = &vals2[0];
        db.insert(1, p2, p2+1);
        assert(db.block_size() == 3);
        assert(db.get<unsigned long>(0) == 11);
        assert(db.get<unsigned long>(1) == 22);
        assert(db.get<double>(2) == 1.2);
        assert(db.get<unsigned long>(3) == 12);
    }
}

void mtv_test_iterators()
{
    stack_printer __stack_printer__("::mtv_test_iterators");
    {
        column_type db(5);
        column_type::const_iterator it;
        it = db.begin();
        column_type::const_iterator it_end = db.end();
        size_t len = std::distance(it, it_end);
        assert(len == 1);
        assert(it != it_end);
        assert(it->type == mtv::element_type_empty);
        assert(it->size == 5);
        const column_type::const_iterator::value_type& val = *it;
        assert(val.type == it->type);
        assert(val.size == it->size);

        ++it;
        assert(it == it_end);
    }

    {
        column_type db(6);
        db.set(0, 1.1);
        db.set(1, 2.2);
        db.set(4, string("boo"));
        db.set(5, string("hoo"));
        assert(db.block_size() == 3);
        {
            // Forward iterator
            column_type::const_iterator it = db.begin(), it_end = db.end();
            size_t len = std::distance(it, it_end);
            assert(len == 3);
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_numeric);
            assert(it->size == 2);

            ++it;
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_empty);
            assert(it->size == 2);

            ++it;
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_string);
            assert(it->size == 2);

            ++it;
            assert(it == it_end);
        }

        {
            // Reverse iterator
            column_type::const_reverse_iterator it = db.rbegin(), it_end = db.rend();
            size_t len = std::distance(it, it_end);
            assert(len == 3);
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_string);
            assert(it->size == 2);

            ++it;
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_empty);
            assert(it->size == 2);

            ++it;
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_numeric);
            assert(it->size == 2);

            ++it;
            assert(it == it_end);
        }
    }
}

void mtv_test_data_iterators()
{
    stack_printer __stack_printer__("::mtv_test_data_iterators");

    column_type db(10);
    db.set(0, 1.1);
    db.set(1, 1.2);
    db.set(2, 1.3);
    db.set(4, string("A"));
    db.set(5, string("B"));
    db.set(6, string("C"));
    db.set(7, string("D"));
    column_type::const_iterator it_blk = db.begin(), it_blk_end = db.end();

    // First block is a numeric block.
    assert(it_blk != it_blk_end);
    assert(it_blk->type == mdds::mtv::element_type_numeric);
    assert(it_blk->size == 3);
    assert(it_blk->data);
    {
        mtv::numeric_element_block::const_iterator it_data = mtv::numeric_element_block::begin(*it_blk->data);
        mtv::numeric_element_block::const_iterator it_data_end = mtv::numeric_element_block::end(*it_blk->data);
        assert(it_data != it_data_end);
        assert(*it_data == 1.1);
        ++it_data;
        assert(*it_data == 1.2);
        ++it_data;
        assert(*it_data == 1.3);
        ++it_data;
        assert(it_data == it_data_end);
    }

    // Next block is empty.
    ++it_blk;
    assert(it_blk->type == mdds::mtv::element_type_empty);
    assert(it_blk->size == 1);
    assert(it_blk->data == NULL);

    // Next block is a string block.
    ++it_blk;
    assert(it_blk->type == mdds::mtv::element_type_string);
    assert(it_blk->size == 4);
    assert(it_blk->data);
    {
        mtv::string_element_block::const_reverse_iterator it_data = mtv::string_element_block::rbegin(*it_blk->data);
        mtv::string_element_block::const_reverse_iterator it_data_end = mtv::string_element_block::rend(*it_blk->data);
        assert(it_data != it_data_end);
        assert(*it_data == "D");
        ++it_data;
        assert(*it_data == "C");
        ++it_data;
        assert(*it_data == "B");
        ++it_data;
        assert(*it_data == "A");
        ++it_data;
        assert(it_data == it_data_end);
    }

    // Another empty block follows.
    ++it_blk;
    assert(it_blk->type == mdds::mtv::element_type_empty);
    assert(it_blk->size == 2);
    assert(it_blk->data == NULL);

    ++it_blk;
    assert(it_blk == it_blk_end);
}

void mtv_test_non_const_data_iterators()
{
    stack_printer __stack_printer__("::mtv_test_non_const_data_iterators");

    column_type db(1);
    db.set(0, 1.2);
    column_type::iterator it_blk = db.begin(), it_blk_end = db.end();
    size_t n = std::distance(it_blk, it_blk_end);
    assert(n == 1);
    assert(it_blk->type == mtv::element_type_numeric);
    assert(it_blk->data != NULL);
    mtv::numeric_element_block::iterator it = mtv::numeric_element_block::begin(*it_blk->data);
    mtv::numeric_element_block::iterator it_end = mtv::numeric_element_block::end(*it_blk->data);
    n = std::distance(it, it_end);
    assert(n == 1);
    assert(*it == 1.2);

    *it = 2.3; // write via iterator.
    assert(db.get<double>(0) == 2.3);
}

}

int main (int argc, char **argv)
{
    cmd_options opt;
    if (!parse_cmd_options(argc, argv, opt))
        return EXIT_FAILURE;

    if (opt.test_func)
    {
        mtv_test_types();
        mtv_test_construction();
        mtv_test_basic();
        mtv_test_empty_cells();
        mtv_test_swap();
        mtv_test_equality();
        mtv_test_clone();
        mtv_test_resize();
        mtv_test_erase();
        mtv_test_insert_empty();
        mtv_test_set_cells();
        mtv_test_insert_cells();
        mtv_test_iterators();
        mtv_test_data_iterators();
        mtv_test_non_const_data_iterators();
    }

    if (opt.test_perf)
    {
    }

    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}
