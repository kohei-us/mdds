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
#include "test_global.hpp"

#include <cassert>

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

enum cell_t { celltype_numeric = 0, celltype_string };

struct get_cell_type
{
    template<typename T>
    cell_t operator() (const T& t);
};

template<typename T>
cell_t get_cell_type::operator() (const T& t)
{
    return celltype_numeric;
}

template<>
cell_t get_cell_type::operator()<double> (const double& t)
{
    return celltype_numeric;
}

template<>
cell_t get_cell_type::operator()<std::string> (const std::string& t)
{
    return celltype_string;
}

struct base_cell_block
{
    cell_t type;
    base_cell_block(cell_t _t) : type(_t) {}
};

struct numeric_cell_block : public base_cell_block, public vector<double>
{
public:
    numeric_cell_block() : base_cell_block(celltype_numeric), vector<double>(1) {}
};

struct string_cell_block : public base_cell_block, public vector<string>
{
public:
    string_cell_block() : base_cell_block(celltype_string), vector<string>(1) {}
};

struct get_cell_block_type : public std::unary_function<base_cell_block, cell_t>
{
    cell_t operator() (const base_cell_block& r)
    {
        return r.type;
    }
};

struct cell_block_func
{
    static base_cell_block* create_new_block(cell_t type);

    static void delete_block(base_cell_block* p);

    template<typename T>
    static void set_value(base_cell_block* block, long pos, const T& val);

    template<typename T>
    static void prepend_value(base_cell_block* block, const T& val);

    static void prepend_value(base_cell_block* dest, base_cell_block* src);

    template<typename T>
    static void append_value(base_cell_block* block, const T& val);

    static void append_value(base_cell_block* dest, base_cell_block* src);

    template<typename T>
    static void get_value(base_cell_block* block, long pos, T& val);

    template<typename T>
    static void get_empty_value(T& val);
};

base_cell_block* cell_block_func::create_new_block(cell_t type)
{
    switch (type)
    {
        case celltype_numeric:
            return new numeric_cell_block;
        case celltype_string:
            return new string_cell_block;
        default:
            ;
    }
    return NULL;
}

void cell_block_func::delete_block(base_cell_block* p)
{
    if (!p)
        return;

    switch (p->type)
    {
        case celltype_numeric:
            delete static_cast<numeric_cell_block*>(p);
        break;
        case celltype_string:
            delete static_cast<string_cell_block*>(p);
        break;
        default:
            assert(!"attempting to delete a cell block instance of unknown type!");
    }
}

numeric_cell_block* get_numeric_block(base_cell_block* block)
{
    if (!block || block->type != celltype_numeric)
        throw general_error("block is not of numeric type!");

    return static_cast<numeric_cell_block*>(block);
}

template<typename T>
void cell_block_func::set_value(base_cell_block* block, long pos, const T& val)
{
    throw general_error("non-specialized version called.");
}

template<>
void cell_block_func::set_value<double>(base_cell_block* block, long pos, const double& val)
{
    numeric_cell_block& blk = *get_numeric_block(block);
    blk[pos] = val;
}

template<typename T>
void cell_block_func::prepend_value(base_cell_block* block, const T& val)
{
    throw general_error("non-specialized version called.");
}

template<>
void cell_block_func::prepend_value<double>(base_cell_block* block, const double& val)
{
    numeric_cell_block& blk = *get_numeric_block(block);
    blk.insert(blk.begin(), val);
}

template<typename T>
void cell_block_func::append_value(base_cell_block* block, const T& val)
{
    throw general_error("non-specialized version called.");
}

template<>
void cell_block_func::append_value<double>(base_cell_block* block, const double& val)
{
    numeric_cell_block& blk = *get_numeric_block(block);
    blk.push_back(val);
}

void cell_block_func::append_value(base_cell_block* dest, base_cell_block* src)
{
    if (!dest)
        throw general_error("destination cell block is NULL.");

    switch (dest->type)
    {
        case celltype_numeric:
        {
            numeric_cell_block& d = *get_numeric_block(dest);
            numeric_cell_block& s = *get_numeric_block(src);
            d.insert(d.end(), s.begin(), s.end());
        }
        break;
        case celltype_string:
        default:
            assert(!"unhandled cell type.");
    }
}

template<typename T>
void cell_block_func::get_value(base_cell_block* block, long pos, T& val)
{
    throw general_error("non-specialized version called.");
}

template<>
void cell_block_func::get_value<double>(base_cell_block* block, long pos, double& val)
{
    numeric_cell_block& blk = *get_numeric_block(block);
    val = blk[pos];
}

template<typename T>
void cell_block_func::get_empty_value(T& val)
{
    throw general_error("non-specialized version called.");
}

template<>
void cell_block_func::get_empty_value<double>(double& val)
{
    val = 0.0;
}

struct grid_map_trait
{
    typedef base_cell_block cell_block_type;
    typedef cell_t cell_category_type;
    typedef long sheet_key_type;
    typedef long row_key_type;
    typedef long col_key_type;

    typedef get_cell_type cell_type_inspector;
    typedef get_cell_block_type cell_block_type_inspector;

    typedef cell_block_func cell_block_modifier;
};

template<typename _ColT, typename _ValT>
bool test_numeric_cell_insertion(_ColT& col_db, typename _ColT::row_key_type row, _ValT val)
{
    _ValT test;
    col_db.set_cell(row, val);
    col_db.get_cell(row, test);
    return val == test;
}

void gridmap_test_basic()
{
    stack_printer __stack_printer__("::gridmap_test_basic");
    typedef mdds::grid_map<grid_map_trait> grid_store_type;
    typedef grid_store_type::sheet_type::column_type column_type;
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
        res = test_numeric_cell_insertion(col_db, 0, 2.0);
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

        res = test_numeric_cell_insertion(col_db, 0, 5.0);
        assert(res);

        col_db.get_cell(1, test);
        assert(test == 0.0); // should be empty.

        // Insert a new value to an empty row right below a non-empty one.
        res = test_numeric_cell_insertion(col_db, 1, 7.5);
        assert(res);
    }

    {
        column_type col_db(3);
        res = test_numeric_cell_insertion(col_db, 0, 4.5);
        assert(res);
        res = test_numeric_cell_insertion(col_db, 1, 5.1);
        assert(res);
        res = test_numeric_cell_insertion(col_db, 2, 34.2);
        assert(res);
    }

    {
        // Insert first value into the bottom row.
        column_type col_db(3);

        res = test_numeric_cell_insertion(col_db, 2, 5.0); // Insert into the last row.
        assert(res);

        double test = 9;
        col_db.get_cell(1, test);
        assert(test == 0.0); // should be empty.

        res = test_numeric_cell_insertion(col_db, 0, 2.5);
        assert(res);

        col_db.get_cell(1, test);
        assert(test == 0.0); // should be empty.

        res = test_numeric_cell_insertion(col_db, 1, 1.2);
        assert(res);
    }

    {
        // This time insert from bottom up one by one.
        column_type col_db(3);
        res = test_numeric_cell_insertion(col_db, 2, 1.2);
        assert(res);
        res = test_numeric_cell_insertion(col_db, 1, 0.2);
        assert(res);
        res = test_numeric_cell_insertion(col_db, 0, 23.1);
        assert(res);
    }

    {
        column_type col_db(4);
        long order[] = { 3, 1, 2, 0 };
        double val = 1.0;
        for (size_t i = 0; i < 4; ++i, ++val)
        {
            res = test_numeric_cell_insertion(col_db, order[i], val);
            assert(res);
        }
    }

    {
        column_type col_db(4);
        long order[] = { 0, 3, 1, 2 };
        double val = 1.0;
        for (size_t i = 0; i < 4; ++i, ++val)
        {
            res = test_numeric_cell_insertion(col_db, order[i], val);
            assert(res);
        }
    }

    {
        column_type col_db(4);
        long order[] = { 0, 2, 3, 1 };
        double val = 1.0;
        for (size_t i = 0; i < 4; ++i, ++val)
        {
            res = test_numeric_cell_insertion(col_db, order[i], val);
            assert(res);
        }
    }

    {
        column_type col_db(5);
        long order[] = { 0, 4, 3, 2, 1 };
        double val = 1.0;
        for (size_t i = 0; i < 5; ++i, ++val)
        {
            res = test_numeric_cell_insertion(col_db, order[i], val);
            assert(res);
        }
    }

    {
        // Insert first value into a middle row.
        column_type col_db(10);
        res = test_numeric_cell_insertion(col_db, 5, 5.0);
        assert(res);
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
    }

    if (opt.test_perf)
    {
    }

    return EXIT_SUCCESS;
}
