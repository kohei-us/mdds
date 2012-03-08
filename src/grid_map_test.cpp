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

#include "mdds/grid_map.hpp"
#include "mdds/grid_map_trait.hpp"
#include "test_global.hpp"

#include <cassert>
#include <sstream>

#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])

#include <stdio.h>
#include <string>
#include <sys/time.h>

using namespace std;
using namespace mdds;

namespace {

class stack_printer
{
public:
    explicit stack_printer(const char* msg) :
        m_msg(msg)
    {
        fprintf(stdout, "%s: --begin\n", m_msg.c_str());
        m_start_time = get_time();
    }

    ~stack_printer()
    {
        double end_time = get_time();
        fprintf(stdout, "%s: --end (duration: %g sec)\n", m_msg.c_str(), (end_time-m_start_time));
    }

    void print_time(int line) const
    {
        double end_time = get_time();
        fprintf(stdout, "%s: --(%d) (duration: %g sec)\n", m_msg.c_str(), line, (end_time-m_start_time));
    }

private:
    double get_time() const
    {
        timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    std::string m_msg;
    double m_start_time;
};


struct grid_map_trait
{
    typedef long sheet_key_type;
    typedef long row_key_type;
    typedef long col_key_type;

    typedef mdds::gridmap::cell_block_func cell_block_func;
};

template<typename _ColT, typename _ValT>
bool test_cell_insertion(_ColT& col_db, typename _ColT::row_key_type row, _ValT val)
{
    _ValT test;
    col_db.set_cell(row, val);
    col_db.get_cell(row, test);
    return val == test;
}

typedef mdds::grid_map<grid_map_trait> grid_store_type;
typedef grid_store_type::sheet_type::column_type column_type;

void gridmap_test_basic()
{
    stack_printer __stack_printer__("::gridmap_test_basic");
    grid_store_type db;
    bool res;

    {
        // Single column instance with only one row.
        column_type col_db(1);

        double test = -999.0;

        // Empty cell has a numeric value of 0.0.
        col_db.get_cell(0, test);
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
        col_db.get_cell(0, test);
        assert(test == 0.0);
        test = 1.0;
        col_db.get_cell(1, test);
        assert(test == 0.0);

        res = test_cell_insertion(col_db, 0, 5.0);
        assert(res);

        col_db.get_cell(1, test);
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
        col_db.get_cell(1, test);
        assert(test == 0.0); // should be empty.

        res = test_cell_insertion(col_db, 0, 2.5);
        assert(res);

        col_db.get_cell(1, test);
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
        col_db.get_cell(3, test); // Check the cell below.
        assert(test == "foo");

        res = test_cell_insertion(col_db, 1, -2.0);
        assert(res);
        test = "hmm";
        col_db.get_cell(3, test);
        assert(test == "foo");

        res = test_cell_insertion(col_db, 0, 7.5); // overwrite.
        assert(res);

        str = "bah";
        res = test_cell_insertion(col_db, 0, str); // overwrite with different type.
        assert(res);
        double val = -999;
        col_db.get_cell(1, val); // Check the cell below.
        assert(val == -2.0);

        str = "alpha";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
        col_db.get_cell(2, val); // Check the cell below.
        assert(val == 2.0);

        col_db.get_cell(3, test);
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
        col_db.get_cell(1, test);
        assert(test == "foo");
    }

    {
        column_type col_db(3);
        string str = "alpha";
        col_db.set_cell(2, str);
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
        col_db.get_cell(0, test);
        assert(test == 1.0);
        col_db.get_cell(1, test);
        assert(test == 2.0);
        col_db.get_cell(2, test);
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
        col_db.get_cell(0, test);
        assert(test == 4.0);

        // The top 2 cells are numeric and the bottom cell is still empty.

        str = "beta";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
        col_db.get_cell(0, test);
        assert(test == 4.0);

        res = test_cell_insertion(col_db, 1, 6.5);
        assert(res);
        col_db.get_cell(0, test);
        assert(test == 4.0);

        str = "gamma";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
        col_db.get_cell(0, test);
        assert(test == 4.0);
        col_db.get_cell(1, test);
        assert(test == 6.5);

        // The top 2 cells are numeric and the bottom cell is string.

        str = "delta";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
    }

    {
        column_type col_db(4);
        col_db.set_cell(0, 1.0);
        string str = "foo";
        col_db.set_cell(1, str);
        col_db.set_cell(2, str);
        col_db.set_cell(3, 4.0);

        res = test_cell_insertion(col_db, 2, 3.0);
        assert(res);
        double test;
        col_db.get_cell(3, test);
        assert(test == 4.0);
    }

    {
        column_type col_db(4);
        col_db.set_cell(0, 1.0);
        string str = "foo";
        col_db.set_cell(1, str);
        col_db.set_cell(2, str);
        col_db.set_cell(3, str);

        res = test_cell_insertion(col_db, 3, 3.0);
        assert(res);
    }

    {
        column_type col_db(4);
        col_db.set_cell(0, 1.0);
        string str = "foo";
        col_db.set_cell(1, str);
        col_db.set_cell(2, str);

        res = test_cell_insertion(col_db, 2, 3.0);
        assert(res);

        // Next cell should still be empty.
        double test_val;
        col_db.get_cell(3, test_val);
        assert(test_val == 0.0);
        string test_str;
        col_db.get_cell(3, test_str);
        assert(test_str.empty());
    }

    {
        column_type col_db(4);
        col_db.set_cell(0, 1.0);
        col_db.set_cell(1, 1.0);
        col_db.set_cell(2, 1.0);
        col_db.set_cell(3, 1.0);
        string str = "alpha";
        res = test_cell_insertion(col_db, 2, str);
        assert(res);
    }

    {
        column_type col_db(3);
        col_db.set_cell(0, 1.0);
        col_db.set_cell(1, 1.0);
        string str = "foo";
        col_db.set_cell(2, str);
        size_t index = 5;
        test_cell_insertion(col_db, 2, index);
    }

    {
        column_type col_db(3);
        col_db.set_cell(1, 1.0);
        string str = "foo";
        col_db.set_cell(2, str);
        str = "bah";
        res = test_cell_insertion(col_db, 1, str);
        assert(res);
        res = test_cell_insertion(col_db, 1, 2.0);
        assert(res);
        size_t index = 2;
        res = test_cell_insertion(col_db, 1, index);
        assert(res);
        string test;
        col_db.get_cell(2, test);
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
        col_db.set_cell(0, 1.0);
        string str = "alpha";
        col_db.set_cell(1, str);
        str = "beta";
        col_db.set_cell(2, str);
        size_t index = 1;
        res = test_cell_insertion(col_db, 1, index);
        assert(res);
        string test;
        col_db.get_cell(2, test);
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
        size_t index = 2;
        res = test_cell_insertion(col_db, 1, index);
        assert(res);
    }
}

void gridmap_test_empty_cells()
{
    stack_printer __stack_printer__("::gridmap_test_empty");
    {
        column_type db(3);
        assert(db.is_empty(0));
        assert(db.is_empty(2));

        // These won't change the state of the container since it's already empty.
        db.set_empty(0, 0);
        db.set_empty(1, 1);
        db.set_empty(2, 2);
        db.set_empty(0, 2);

        db.set_cell(0, 1.0);
        db.set_cell(2, 5.0);
        assert(!db.is_empty(0));
        assert(!db.is_empty(2));
        assert(db.is_empty(1));

        db.set_cell(1, 2.3);
        assert(!db.is_empty(1));

        // Container contains a single block of numeric cells at this point.

        // Set the whole block empty.
        db.set_empty(0, 2);

        // Reset.
        db.set_cell(0, 1.0);
        db.set_cell(1, 2.0);
        db.set_cell(2, 4.0);

        // Set the upper part of the block empty.
        db.set_empty(0, 1);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
        assert(!db.is_empty(2));

        double test;
        db.get_cell(2, test);
        assert(test == 4.0);

        // Reset.
        db.set_cell(0, 5.0);
        db.set_cell(1, 5.1);
        db.set_cell(2, 5.2);

        // Set the lower part of the block empty.
        db.set_empty(1, 2);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));

        db.get_cell(0, test);
        assert(test == 5.0);

        // Reset.
        db.set_cell(0, 3.0);
        db.set_cell(1, 3.1);
        db.set_cell(2, 3.2);

        // Set the middle part of the block empty.
        db.set_empty(1, 1);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(!db.is_empty(2));
        db.get_cell(0, test);
        assert(test == 3.0);
        db.get_cell(2, test);
        assert(test == 3.2);

        bool res = test_cell_insertion(db, 1, 4.3);
        assert(res);
    }

    {
        // Empty multiple cells at the middle part of a block.
        column_type db(4);
        for (size_t i = 0; i < 4; ++i)
            db.set_cell(i, static_cast<double>(i+1));

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
        db.get_cell(0, test);
        assert(test == 1.0);
        db.get_cell(3, test);
        assert(test == 4.0);
    }

    {
        // Empty multiple blocks.
        column_type db(2);
        db.set_cell(0, 1.0);
        db.set_cell(1, string("foo"));
        assert(!db.is_empty(0));
        assert(!db.is_empty(1));

        db.set_empty(0, 1);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
    }

    {
        // Empty multiple blocks, part 2 - from middle block to middle block.
        column_type db(6);
        db.set_cell(0, 1.0);
        db.set_cell(1, 2.0);
        string str = "foo";
        db.set_cell(2, str);
        db.set_cell(3, str);
        size_t index = 1;
        db.set_cell(4, index);
        index = 100;
        db.set_cell(5, index);

        db.set_empty(1, 4);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));
        assert(!db.is_empty(5));
        double val;
        db.get_cell(0, val);
        assert(val == 1.0);
        size_t index_test;
        db.get_cell(5, index_test);
        assert(index_test == 100);
    }

    {
        // Empty multiple blocks, part 3 - from top block to middle block.
        column_type db(6);
        db.set_cell(0, 1.0);
        db.set_cell(1, 2.0);
        string str = "foo";
        db.set_cell(2, str);
        db.set_cell(3, str);
        size_t index = 1;
        db.set_cell(4, index);
        index = 50;
        db.set_cell(5, index);

        db.set_empty(0, 4);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));
        assert(!db.is_empty(5));
        size_t test;
        db.get_cell(5, test);
        assert(test == 50);
    }

    {
        // Empty multiple blocks, part 4 - from middle block to bottom block.
        column_type db(6);
        db.set_cell(0, 1.0);
        db.set_cell(1, 2.0);
        string str = "foo";
        db.set_cell(2, str);
        db.set_cell(3, str);
        size_t index = 1;
        db.set_cell(4, index);
        db.set_cell(5, index);

        db.set_empty(1, 5);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));
        assert(db.is_empty(5));
        double test;
        db.get_cell(0, test);
        assert(test == 1.0);
    }

    {
        // Empty multiple blocks, part 5 - from middle empty block to middle non-empty block.
        column_type db(6);
        db.set_cell(2, 1.0);
        db.set_cell(3, 2.0);
        string str = "foo";
        db.set_cell(4, str);
        str = "baa";
        db.set_cell(5, str);
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
        db.get_cell(5, test);
        assert(test == "baa");
    }

    {
        // Empty multiple blocks, part 6 - from middle non-empty block to middle empty block.
        column_type db(6);
        db.set_cell(0, 1.0);
        db.set_cell(1, 2.0);
        db.set_cell(2, string("foo"));
        db.set_cell(3, string("baa"));
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
        db.get_cell(0, test);
        assert(test == 1.0);
        assert(db.block_size() == 2);
    }

    {
        // Empty multiple blocks, part 7 - from middle empty block to middle empty block.
        column_type db(6);
        db.set_cell(2, 1.0);
        db.set_cell(3, string("foo"));
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
}

void gridmap_test_swap()
{
    stack_printer __stack_printer__("::gridmap_test_swap");
    column_type db1(3), db2(5);
    db1.set_cell(0, 1.0);
    db1.set_cell(1, 2.0);
    db1.set_cell(2, 3.0);

    db2.set_cell(0, 4.0);
    db2.set_cell(1, 5.0);
    db2.set_cell(4, string("foo"));
    db1.swap(db2);

    assert(db1.size() == 5 && db1.block_size() == 3);
    assert(db2.size() == 3 && db2.block_size() == 1);
}

void gridmap_test_equality()
{
    stack_printer __stack_printer__("::gridmap_test_equality");
    {
        // Two columns of equal size.
        column_type db1(3), db2(3);
        assert(db1 == db2);
        db1.set_cell(0, 1.0);
        assert(db1 != db2);
        db2.set_cell(0, 1.0);
        assert(db1 == db2);
        db2.set_cell(0, 1.2);
        assert(db1 != db2);
        db1.set_cell(0, 1.2);
        assert(db1 == db2);
    }

    {
        // Two columns of different sizes.  They are always non-equal no
        // matter what.
        column_type db1(3), db2(4);
        assert(db1 != db2);
        db1.set_cell(0, 1.2);
        db2.set_cell(0, 1.2);
        assert(db1 != db2);

        // Comparison to self.
        assert(db1 == db1);
        assert(db2 == db2);
    }
}

void gridmap_test_clone()
{
    stack_printer __stack_printer__("::gridmap_test_clone");
    column_type db1(3);
    db1.set_cell(0, 3.4);
    db1.set_cell(1, string("foo"));
    db1.set_cell(2, true);

    // copy construction

    column_type db2(db1);
    assert(db1.size() == db2.size());
    assert(db1.block_size() == db2.block_size());
    assert(db1 == db2);

    {
        double test1, test2;
        db1.get_cell(0, test1);
        db2.get_cell(0, test2);
        assert(test1 == test2);
    }
    {
        string test1, test2;
        db1.get_cell(1, test1);
        db2.get_cell(1, test2);
        assert(test1 == test2);
    }

    {
        bool test1, test2;
        db1.get_cell(2, test1);
        db2.get_cell(2, test2);
        assert(test1 == test2);
    }

    // assignment

    column_type db3 = db1;
    assert(db3 == db1);
    db3.set_cell(0, string("alpha"));
    assert(db3 != db1);

    column_type db4, db5;
    db4 = db5 = db3;
    assert(db4 == db5);
    assert(db3 == db5);
    assert(db3 == db4);
}

void gridmap_test_resize()
{
    stack_printer __stack_printer__("::gridmap_test_resize");
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
        db.set_cell(row, static_cast<double>(row));

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
    db.get_cell(3, test);
    assert(test == 3.0);

    // Empty it.
    db.resize(0);
    assert(db.size() == 0);
    assert(db.block_size() == 0);
    assert(db.empty());
}

void gridmap_test_erase()
{
    stack_printer __stack_printer__("::gridmap_test_erase");
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
            db.set_cell(i, static_cast<double>(i+1));

        assert(db.block_size() == 1);
        assert(db.size() == 5);

        db.erase(0, 2); // erase rows 0-2
        assert(db.size() == 2);
        double test;
        db.get_cell(0, test);
        assert(test == 4.0);
        db.get_cell(1, test);
        assert(test == 5.0);

        db.erase(0, 1);
        assert(db.size() == 0);
        assert(db.empty());
    }

    {
        // Two blocks - non-empty to empty blocks.
        column_type db(8);
        for (long i = 0; i < 4; ++i)
            db.set_cell(i, static_cast<double>(i+1));

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
        db.get_cell(2, test);
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
            db.set_cell(i, static_cast<double>(i+1));

        for (long i = 4; i < 8; ++i)
            db.set_cell(i, static_cast<size_t>(i+1));

        assert(db.block_size() == 2);
        assert(db.size() == 8);

        // Erase across two blocks.
        db.erase(3, 6); // 4 cells
        assert(db.block_size() == 2);
        assert(db.size() == 4);

        // Check the integrity of the data.
        double test;
        db.get_cell(2, test);
        assert(test == 3.0);

        size_t test2;
        db.get_cell(3, test2);
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
            db.set_cell(i, static_cast<double>(i+1));

        for (long i = 3; i < 6; ++i)
            db.set_cell(i, static_cast<size_t>(i+1));

        for (long i = 6; i < 9; ++i)
        {
            ostringstream os;
            os << i+1;
            db.set_cell(i, os.str());
        }

        assert(db.block_size() == 3);
        assert(db.size() == 9);

        db.erase(2, 7);
        assert(db.block_size() == 2);
        assert(db.size() == 3);

        // Check the integrity of the data.
        double test1;
        db.get_cell(1, test1);
        assert(test1 == 2.0);
        string test2;
        db.get_cell(2, test2);
        assert(test2 == "9");

        db.erase(2, 2); // Erase only one-block.
        assert(db.block_size() == 1);
        assert(db.size() == 2);
        test1 = -1.0;
        db.get_cell(1, test1);
        assert(test1 == 2.0);

        db.erase(0, 1);
        assert(db.size() == 0);
        assert(db.empty());
    }
}

void gridmap_test_insert_empty()
{
    stack_printer __stack_printer__("::gridmap_test_insert_empty");
    {
        column_type db(5);
        db.insert_empty(0, 5);
        assert(db.size() == 10);
        assert(db.block_size() == 1);

        // Insert data from row 0 to 4.
        for (long i = 0; i < 5; ++i)
            db.set_cell(i, static_cast<double>(i+1));

        assert(db.block_size() == 2);
        assert(db.size() == 10);

        // Now, insert an empty block of size 2 at the top.
        db.insert_empty(0, 2);
        assert(db.block_size() == 3);
        assert(db.size() == 12);

        double test;
        db.get_cell(2, test);
        assert(test == 1.0);

        // Insert an empty cell into an empty block.  This should shift the
        // data block down by one.
        db.insert_empty(1, 1);
        assert(db.block_size() == 3);
        assert(db.size() == 13);
        db.get_cell(4, test);
        assert(test == 2.0);
    }

    {
        column_type db(5);
        for (long i = 0; i < 5; ++i)
            db.set_cell(i, static_cast<double>(i+1));

        assert(db.block_size() == 1);
        assert(db.size() == 5);

        // Insert an empty block into the middle of a non-empty block.
        db.insert_empty(2, 2);

        assert(db.block_size() == 3);
        assert(db.size() == 7);
        assert(db.is_empty(2));
        assert(db.is_empty(3));

        double test;
        db.get_cell(0, test);
        assert(test == 1.0);
        db.get_cell(1, test);
        assert(test == 2.0);

        db.get_cell(4, test);
        assert(test == 3.0);
        db.get_cell(5, test);
        assert(test == 4.0);
        db.get_cell(6, test);
        assert(test == 5.0);
    }

    {
        column_type db(1);
        db.set_cell(0, 2.5);
        db.insert_empty(0, 2);
        assert(db.block_size() == 2);
        assert(db.size() == 3);
        assert(db.is_empty(1));
        assert(!db.is_empty(2));

        double test;
        db.get_cell(2, test);
        assert(test == 2.5);
    }

    {
        column_type db(2);
        db.set_cell(0, 1.2);
        db.set_cell(1, 2.3);
        db.insert_empty(1, 1);

        assert(db.block_size() == 3);
        assert(db.size() == 3);
        double test;
        db.get_cell(0, test);
        assert(test == 1.2);
        db.get_cell(2, test);
        assert(test == 2.3);
    }
}

void gridmap_test_set_cells()
{
    stack_printer __stack_printer__("::gridmap_test_set_cells");
    {
        column_type db(5);

        // Replace the whole block.

        {
            vector<double> vals;
            vals.reserve(5);
            for (size_t i = 0; i < db.size(); ++i)
                vals.push_back(i+1);

            db.set_cells(0, vals.begin(), vals.end());

            double test;
            db.get_cell(0, test);
            assert(test == 1.0);
            db.get_cell(4, test);
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

            db.set_cells(0, vals.begin(), vals.end());

            string test;
            db.get_cell(0, test);
            assert(test == "1");
            db.get_cell(4, test);
            assert(test == "5");
        }

        {
            double vals[] = { 5.0, 6.0, 7.0, 8.0, 9.0 };
            double* p = &vals[0];
            double* p_end = p + 5;
            db.set_cells(0, p, p_end);
            double test;
            db.get_cell(0, test);
            assert(test == 5.0);
            db.get_cell(4, test);
            assert(test == 9.0);
        }

        {
            // Replace the whole block of the same type, which shouldn't
            // delete the old data array.
            double vals[] = { 5.1, 6.1, 7.1, 8.1, 9.1 };
            double* p = &vals[0];
            double* p_end = p + 5;
            db.set_cells(0, p, p_end);
            double test;
            db.get_cell(0, test);
            assert(test == 5.1);
            db.get_cell(4, test);
            assert(test == 9.1);

            double vals2[] = { 8.2, 9.2 };
            p = &vals2[0];
            p_end = p + 2;
            db.set_cells(3, p, p_end);
            db.get_cell(3, test);
            assert(test == 8.2);
            db.get_cell(4, test);
            assert(test == 9.2);
        }


        {
            // Replace the upper part of a single block.
            size_t vals[] = { 1, 2, 3 };
            size_t* p = &vals[0];
            size_t* p_end = p + 3;
            db.set_cells(0, p, p_end);
            assert(db.block_size() == 2);
            assert(db.size() == 5);
            size_t test;
            db.get_cell(0, test);
            assert(test == 1);
            db.get_cell(2, test);
            assert(test == 3);
            double test2;
            db.get_cell(3, test2);
            assert(test2 == 8.2);
        }

        {
            // Merge with the previos block and erase the whole block.
            size_t vals[] = { 4, 5 };
            size_t* p = &vals[0];
            size_t* p_end = p + 2;
            db.set_cells(3, p, p_end);
            assert(db.block_size() == 1);
            assert(db.size() == 5);
            size_t test;
            db.get_cell(2, test);
            assert(test == 3);
            db.get_cell(3, test);
            assert(test == 4);
        }

        {
            // Merge with the previous block while keeping the lower part of
            // the block.
            size_t prev_value;
            db.get_cell(2, prev_value);

            double val = 2.3;
            db.set_cell(0, val);
            assert(db.block_size() == 2);
            val = 4.5;
            double* p = &val;
            double* p_end = p + 1;
            db.set_cells(1, p, p_end);
            assert(db.block_size() == 2);
            assert(db.size() == 5);
            {
                double test;
                db.get_cell(0, test);
                assert(test == 2.3);
                db.get_cell(1, test);
                assert(test == 4.5);
            }

            size_t test;
            db.get_cell(2, test);
            assert(test == prev_value);
        }
    }

    {
        column_type db(5);
        for (size_t i = 0; i < 5; ++i)
            db.set_cell(i, static_cast<double>(i+1));

        assert(db.block_size() == 1);
        assert(db.size() == 5);

        {
            size_t vals[] = { 10, 11 };
            size_t* p = &vals[0];
            size_t* p_end = p + 2;
            db.set_cells(3, p, p_end);

            assert(db.block_size() == 2);
            assert(db.size() == 5);

            double test;
            db.get_cell(2, test);
            assert(test == 3.0);
            size_t test2;
            db.get_cell(3, test2);
            assert(test2 == 10);
            db.get_cell(4, test2);
            assert(test2 == 11);

            // Insertion into a single block but this time it needs to be
            // merged with the subsequent block.
            db.set_cells(1, p, p_end);

            assert(db.block_size() == 2);
            assert(db.size() == 5);

            db.get_cell(1, test2);
            assert(test2 == 10);
            db.get_cell(2, test2);
            assert(test2 == 11);
            db.get_cell(3, test2);
            assert(test2 == 10);
            db.get_cell(4, test2);
            assert(test2 == 11);
        }
    }

    {
        column_type db(6);
        double vals_d[] = { 1.0, 1.1, 1.2, 1.3, 1.4, 1.5 };
        size_t vals_i[] = { 12, 13, 14, 15 };
        string vals_s[] = { "a", "b" };

        {
            double* p = &vals_d[0];
            double* p_end = p + 6;
            db.set_cells(0, p, p_end);
            assert(db.block_size() == 1);
            assert(db.size() == 6);
            double test;
            db.get_cell(0, test);
            assert(test == 1.0);
            db.get_cell(5, test);
            assert(test == 1.5);
        }

        {
            size_t* p = &vals_i[0];
            size_t* p_end = p + 4;
            db.set_cells(0, p, p_end);
            assert(db.block_size() == 2);
            size_t test;
            db.get_cell(0, test);
            assert(test == 12);
            db.get_cell(3, test);
            assert(test == 15);
        }

        {
            string* p = &vals_s[0];
            string* p_end = p + 2;
            db.set_cells(2, p, p_end);
            assert(db.block_size() == 3);
            string test;
            db.get_cell(2, test);
            assert(test == "a");
            db.get_cell(3, test);
            assert(test == "b");
            double test_d;
            db.get_cell(4, test_d);
            assert(test_d == 1.4);
            size_t test_i;
            db.get_cell(1, test_i);
            assert(test_i == 13);
        }
    }

    {
        column_type db(3);
        {
            double vals[] = { 2.1, 2.2, 2.3 };
            double* p = &vals[0];
            double* p_end = p + 3;
            db.set_cells(0, p, p_end);
            assert(db.block_size() == 1);
        }

        {
            size_t val_i = 23;
            size_t* p = &val_i;
            size_t* p_end = p + 1;
            db.set_cells(1, p, p_end);
            assert(db.block_size() == 3);
            size_t test;
            db.get_cell(1, test);
            assert(test == 23);
            double test_d;
            db.get_cell(0, test_d);
            assert(test_d == 2.1);
            db.get_cell(2, test_d);
            assert(test_d == 2.3);
        }
    }

    {
        // Set cells over multiple blocks. Very simple case.

        column_type db(2);
        db.set_cell(0, static_cast<double>(1.1));
        db.set_cell(1, string("foo"));
        assert(db.block_size() == 2);

        double vals[] = { 2.1, 2.2 };
        double* p = &vals[0];
        double* p_end = p + 2;
        db.set_cells(0, p, p_end);
        assert(db.block_size() == 1);
        assert(db.size() == 2);

        double test;
        db.get_cell(0, test);
        assert(test == 2.1);
        db.get_cell(1, test);
        assert(test == 2.2);
    }

    {
        // Same as above, except that the last block is only partially replaced.

        column_type db(3);
        db.set_cell(0, static_cast<double>(1.1));
        db.set_cell(1, string("foo"));
        db.set_cell(2, string("baa"));

        double vals[] = { 2.1, 2.2 };
        double* p = &vals[0];
        double* p_end = p + 2;
        db.set_cells(0, p, p_end);
        assert(db.block_size() == 2);
        assert(db.size() == 3);

        double test_val;
        db.get_cell(0, test_val);
        assert(test_val == 2.1);
        db.get_cell(1, test_val);
        assert(test_val == 2.2);

        string test_s;
        db.get_cell(2, test_s);
        assert(test_s == "baa");
    }

    {
        column_type db(3);
        db.set_cell(0, static_cast<double>(3.1));
        db.set_cell(1, static_cast<double>(3.2));
        db.set_cell(2, string("foo"));
        assert(db.block_size() == 2);
        assert(db.size() == 3);

        double vals[] = { 2.1, 2.2 };
        double* p = &vals[0];
        double* p_end = p + 2;
        db.set_cells(1, p, p_end);
        assert(db.block_size() == 1);
        double test;
        db.get_cell(0, test);
        assert(test == 3.1);
        db.get_cell(1, test);
        assert(test == 2.1);
        db.get_cell(2, test);
        assert(test == 2.2);
    }

    {
        column_type db(5);
        db.set_cell(0, 1.1);
        db.set_cell(1, 1.2);
        db.set_cell(2, string("foo"));
        db.set_cell(3, 1.3);
        db.set_cell(4, 1.4);
        assert(db.block_size() == 3);

        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        double* p_end = p + 3;
        db.set_cells(1, p, p_end);
        assert(db.block_size() == 1);
        assert(db.size() == 5);
        assert(db.get_cell<double>(0) == 1.1);
        assert(db.get_cell<double>(1) == 2.1);
        assert(db.get_cell<double>(2) == 2.2);
        assert(db.get_cell<double>(3) == 2.3);
        assert(db.get_cell<double>(4) == 1.4);
    }

    {
        column_type db(4);
        db.set_cell(0, string("A"));
        db.set_cell(1, string("B"));
        db.set_cell(2, 1.1);
        db.set_cell(3, 1.2);
        assert(db.block_size() == 2);

        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        db.set_cells(1, p, p+3);
        assert(db.block_size() == 2);
        assert(db.get_cell<string>(0) == string("A"));
        assert(db.get_cell<double>(1) == 2.1);
        assert(db.get_cell<double>(2) == 2.2);
        assert(db.get_cell<double>(3) == 2.3);
    }

    {
        column_type db(4);
        db.set_cell(0, string("A"));
        db.set_cell(1, string("B"));
        db.set_cell(2, 1.1);
        db.set_cell(3, 1.2);
        assert(db.block_size() == 2);

        double vals[] = { 2.1, 2.2 };
        double* p = &vals[0];
        db.set_cells(1, p, p+2);
        assert(db.block_size() == 2);
        assert(db.get_cell<string>(0) == string("A"));
        assert(db.get_cell<double>(1) == 2.1);
        assert(db.get_cell<double>(2) == 2.2);
        assert(db.get_cell<double>(3) == 1.2);
    }

    {
        column_type db(5);
        db.set_cell(0, string("A"));
        db.set_cell(1, string("B"));
        db.set_cell(2, 1.1);
        db.set_cell(3, 1.2);
        db.set_cell(4, size_t(12));
        assert(db.block_size() == 3);

        size_t vals[] = { 21, 22, 23 };
        size_t* p = &vals[0];
        db.set_cells(1, p, p+3);
        assert(db.block_size() == 2);
        assert(db.get_cell<string>(0) == string("A"));
        assert(db.get_cell<size_t>(1) == 21);
        assert(db.get_cell<size_t>(2) == 22);
        assert(db.get_cell<size_t>(3) == 23);
        assert(db.get_cell<size_t>(4) == 12);
    }

    {
        column_type db(3);
        db.set_cell(0, string("A"));
        db.set_cell(1, 1.1);
        db.set_cell(2, 1.2);
        assert(db.block_size() == 2);

        size_t vals[] = { 11, 12 };
        size_t* p = &vals[0];
        db.set_cells(0, p, p+2);
        assert(db.block_size() == 2);
        assert(db.get_cell<size_t>(0) == 11);
        assert(db.get_cell<size_t>(1) == 12);
        assert(db.get_cell<double>(2) == 1.2);
    }

    {
        column_type db(4);
        db.set_cell(0, size_t(35));
        db.set_cell(1, string("A"));
        db.set_cell(2, 1.1);
        db.set_cell(3, 1.2);
        assert(db.block_size() == 3);

        size_t vals[] = { 11, 12 };
        size_t* p = &vals[0];
        db.set_cells(1, p, p+2);
        assert(db.block_size() == 2);
        assert(db.get_cell<size_t>(0) == 35);
        assert(db.get_cell<size_t>(1) == 11);
        assert(db.get_cell<size_t>(2) == 12);
        assert(db.get_cell<double>(3) == 1.2);
    }

    {
        // Block 1 is empty.

        column_type db(2);
        db.set_cell(1, 1.2);
        assert(db.block_size() == 2);

        double vals[] = { 2.1, 2.2 };
        double* p = &vals[0];
        db.set_cells(0, p, p+2);
        assert(db.block_size() == 1);
        assert(db.get_cell<double>(0) == 2.1);
        assert(db.get_cell<double>(1) == 2.2);
    }

    {
        column_type db(3);
        db.set_cell(0, 1.1);
        db.set_cell(2, 1.2);
        assert(db.block_size() == 3);

        double vals[] = { 2.1, 2.2 };
        double* p = &vals[0];
        db.set_cells(1, p, p+2);
        assert(db.block_size() == 1);
        assert(db.get_cell<double>(0) == 1.1);
        assert(db.get_cell<double>(1) == 2.1);
        assert(db.get_cell<double>(2) == 2.2);
    }

    {
        column_type db(5);
        db.set_cell(2, string("A"));
        db.set_cell(3, string("B"));
        db.set_cell(4, string("C"));
        assert(db.block_size() == 2);

        double vals[] = { 1.1, 1.2, 1.3 };
        double* p = &vals[0];
        db.set_cells(1, p, p+3);
        assert(db.block_size() == 3);

        assert(db.is_empty(0));
        assert(db.get_cell<double>(1) == 1.1);
        assert(db.get_cell<double>(2) == 1.2);
        assert(db.get_cell<double>(3) == 1.3);
        assert(db.get_cell<string>(4) == string("C"));
    }
}

void gridmap_test_insert_cells()
{
    stack_printer __stack_printer__("::gridmap_test_insert_cells");
    {
        // Insert into non-empty block of the same type.
        column_type db(1);
        db.set_cell(0, 1.1);
        assert(db.block_size() == 1);
        assert(db.size() == 1);

        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        db.insert_cells(0, p, p+3);
        assert(db.block_size() == 1);
        assert(db.size() == 4);
        assert(db.get_cell<double>(0) == 2.1);
        assert(db.get_cell<double>(1) == 2.2);
        assert(db.get_cell<double>(2) == 2.3);
        assert(db.get_cell<double>(3) == 1.1);
    }

    {
        // Insert into an existing empty block.
        column_type db(1);
        assert(db.block_size() == 1);
        assert(db.size() == 1);

        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        db.insert_cells(0, p, p+3);
        assert(db.block_size() == 2);
        assert(db.size() == 4);
        assert(db.get_cell<double>(0) == 2.1);
        assert(db.get_cell<double>(1) == 2.2);
        assert(db.get_cell<double>(2) == 2.3);
        assert(db.is_empty(3));
    }

    {
        column_type db(2);
        db.set_cell(0, 1.1);
        assert(db.block_size() == 2);
        assert(db.size() == 2);

        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        db.insert_cells(1, p, p+3);
        assert(db.block_size() == 2);
        assert(db.size() == 5);
        assert(db.get_cell<double>(0) == 1.1);
        assert(db.get_cell<double>(1) == 2.1);
        assert(db.get_cell<double>(2) == 2.2);
        assert(db.get_cell<double>(3) == 2.3);
        assert(db.is_empty(4));
    }

    {
        column_type db(2);
        db.set_cell(0, size_t(23));
        assert(db.block_size() == 2);
        assert(db.size() == 2);

        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        db.insert_cells(1, p, p+3);
        assert(db.block_size() == 3);
        assert(db.size() == 5);
        assert(db.get_cell<size_t>(0) == 23);
        assert(db.get_cell<double>(1) == 2.1);
        assert(db.get_cell<double>(2) == 2.2);
        assert(db.get_cell<double>(3) == 2.3);
        assert(db.is_empty(4));
    }

    {
        column_type db(2);
        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        db.insert_cells(1, p, p+3);
        assert(db.block_size() == 3);
        assert(db.size() == 5);
        assert(db.is_empty(0));
        assert(db.get_cell<double>(1) == 2.1);
        assert(db.get_cell<double>(2) == 2.2);
        assert(db.get_cell<double>(3) == 2.3);
        assert(db.is_empty(4));
    }

    {
        column_type db(2);
        db.set_cell(0, 1.1);
        db.set_cell(1, size_t(23));
        assert(db.block_size() == 2);
        assert(db.size() == 2);

        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        db.insert_cells(1, p, p+3);
        assert(db.block_size() == 2);
        assert(db.size() == 5);
        assert(db.get_cell<double>(0) == 1.1);
        assert(db.get_cell<double>(1) == 2.1);
        assert(db.get_cell<double>(2) == 2.2);
        assert(db.get_cell<double>(3) == 2.3);
        assert(db.get_cell<size_t>(4) == 23);
    }

    {
        column_type db(2);
        db.set_cell(0, true);
        db.set_cell(1, size_t(23));
        assert(db.block_size() == 2);
        assert(db.size() == 2);

        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        db.insert_cells(1, p, p+3);
        assert(db.block_size() == 3);
        assert(db.size() == 5);
        assert(db.get_cell<bool>(0) == true);
        assert(db.get_cell<double>(1) == 2.1);
        assert(db.get_cell<double>(2) == 2.2);
        assert(db.get_cell<double>(3) == 2.3);
        assert(db.get_cell<size_t>(4) == 23);
    }

    {
        column_type db(2);
        db.set_cell(0, size_t(12));
        db.set_cell(1, size_t(23));
        assert(db.block_size() == 1);
        assert(db.size() == 2);

        double vals[] = { 2.1, 2.2, 2.3 };
        double* p = &vals[0];
        db.insert_cells(1, p, p+3);
        assert(db.block_size() == 3);
        assert(db.size() == 5);
        assert(db.get_cell<size_t>(0) == 12);
        assert(db.get_cell<double>(1) == 2.1);
        assert(db.get_cell<double>(2) == 2.2);
        assert(db.get_cell<double>(3) == 2.3);
        assert(db.get_cell<size_t>(4) == 23);
    }
}

}

int main (int argc, char **argv)
{
    cmd_options opt;
    if (!parse_cmd_options(argc, argv, opt))
        return EXIT_FAILURE;

    if (opt.test_func)
    {
        gridmap_test_basic();
        gridmap_test_empty_cells();
        gridmap_test_swap();
        gridmap_test_equality();
        gridmap_test_clone();
        gridmap_test_resize();
        gridmap_test_erase();
        gridmap_test_insert_empty();
        gridmap_test_set_cells();
        gridmap_test_insert_cells();
    }

    if (opt.test_perf)
    {
    }

    return EXIT_SUCCESS;
}
