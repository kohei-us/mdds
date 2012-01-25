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

enum cell_t { celltype_numeric = 0, celltype_string, celltype_index };

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

template<>
cell_t get_cell_type::operator()<size_t> (const size_t& t)
{
    return celltype_index;
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

struct index_cell_block : public base_cell_block, public vector<size_t>
{
public:
    index_cell_block() : base_cell_block(celltype_index), vector<size_t>(1) {}
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

    static void resize_block(base_cell_block* p, size_t new_size);

    static void print_block(base_cell_block* p);

    static void erase(base_cell_block* block, long pos);

    template<typename T>
    static void set_value(base_cell_block* block, long pos, const T& val);

    template<typename T>
    static void prepend_value(base_cell_block* block, const T& val);

    template<typename T>
    static void append_value(base_cell_block* block, const T& val);

    static void append_values(base_cell_block* dest, const base_cell_block* src);

    static void assign_values(base_cell_block* dest, const base_cell_block* src, size_t begin_pos, size_t len);

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
        case celltype_index:
            return new index_cell_block;
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
        case celltype_index:
            delete static_cast<index_cell_block*>(p);
        break;
        default:
            assert(!"attempting to delete a cell block instance of unknown type!");
    }
}

void cell_block_func::resize_block(base_cell_block* p, size_t new_size)
{
    if (!p)
        return;

    switch (p->type)
    {
        case celltype_numeric:
            static_cast<numeric_cell_block*>(p)->resize(new_size);
        break;
        case celltype_string:
            static_cast<string_cell_block*>(p)->resize(new_size);
        break;
        case celltype_index:
            static_cast<index_cell_block*>(p)->resize(new_size);
        break;
        default:
            assert(!"attempting to delete a cell block instance of unknown type!");
    }
}

template<typename T>
struct print_block_array
{
    void operator() (const T& val) const
    {
        cout << val << " ";
    }
};

void cell_block_func::print_block(base_cell_block* p)
{
    if (!p)
        return;

    switch (p->type)
    {
        case celltype_numeric:
        {
            numeric_cell_block& blk = *static_cast<numeric_cell_block*>(p);
            for_each(blk.begin(), blk.end(), print_block_array<double>());
            cout << endl;
        }
        break;
        case celltype_string:
        {
            string_cell_block& blk = *static_cast<string_cell_block*>(p);
            for_each(blk.begin(), blk.end(), print_block_array<string>());
            cout << endl;
        }
        break;
        case celltype_index:
        {
            index_cell_block& blk = *static_cast<index_cell_block*>(p);
            for_each(blk.begin(), blk.end(), print_block_array<size_t>());
            cout << endl;
        }
        break;
        default:
            assert(!"attempting to print a cell block instance of unknown type!");
    }
}

void cell_block_func::erase(base_cell_block* block, long pos)
{
    if (!block)
        return;

    switch (block->type)
    {
        case celltype_numeric:
        {
            numeric_cell_block& blk = *static_cast<numeric_cell_block*>(block);
            blk.erase(blk.begin()+pos);
        }
        break;
        case celltype_string:
        {
            string_cell_block& blk = *static_cast<string_cell_block*>(block);
            blk.erase(blk.begin()+pos);
        }
        break;
        case celltype_index:
        {
            index_cell_block& blk = *static_cast<index_cell_block*>(block);
            blk.erase(blk.begin()+pos);
        }
        break;
        default:
            assert(!"attempting to erase a cell from a block of unknown type!");
    }
}

numeric_cell_block* get_numeric_block(base_cell_block* block)
{
    if (!block || block->type != celltype_numeric)
        throw general_error("block is not of numeric type!");

    return static_cast<numeric_cell_block*>(block);
}

const numeric_cell_block* get_numeric_block(const base_cell_block* block)
{
    if (!block || block->type != celltype_numeric)
        throw general_error("block is not of numeric type!");

    return static_cast<const numeric_cell_block*>(block);
}

string_cell_block* get_string_block(base_cell_block* block)
{
    if (!block || block->type != celltype_string)
        throw general_error("block is not of string type!");

    return static_cast<string_cell_block*>(block);
}

const string_cell_block* get_string_block(const base_cell_block* block)
{
    if (!block || block->type != celltype_string)
        throw general_error("block is not of string type!");

    return static_cast<const string_cell_block*>(block);
}

index_cell_block* get_index_block(base_cell_block* block)
{
    if (!block || block->type != celltype_index)
        throw general_error("block is not of index type!");

    return static_cast<index_cell_block*>(block);
}

const index_cell_block* get_index_block(const base_cell_block* block)
{
    if (!block || block->type != celltype_index)
        throw general_error("block is not of index type!");

    return static_cast<const index_cell_block*>(block);
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

template<>
void cell_block_func::set_value<string>(base_cell_block* block, long pos, const string& val)
{
    string_cell_block& blk = *get_string_block(block);
    blk[pos] = val;
}

template<>
void cell_block_func::set_value<size_t>(base_cell_block* block, long pos, const size_t& val)
{
    index_cell_block& blk = *get_index_block(block);
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

template<>
void cell_block_func::prepend_value<string>(base_cell_block* block, const string& val)
{
    string_cell_block& blk = *get_string_block(block);
    blk.insert(blk.begin(), val);
}

template<>
void cell_block_func::prepend_value<size_t>(base_cell_block* block, const size_t& val)
{
    index_cell_block& blk = *get_index_block(block);
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

template<>
void cell_block_func::append_value<string>(base_cell_block* block, const string& val)
{
    string_cell_block& blk = *get_string_block(block);
    blk.push_back(val);
}

template<>
void cell_block_func::append_value<size_t>(base_cell_block* block, const size_t& val)
{
    index_cell_block& blk = *get_index_block(block);
    blk.push_back(val);
}

void cell_block_func::append_values(base_cell_block* dest, const base_cell_block* src)
{
    if (!dest)
        throw general_error("destination cell block is NULL.");

    switch (dest->type)
    {
        case celltype_numeric:
        {
            numeric_cell_block& d = *get_numeric_block(dest);
            const numeric_cell_block& s = *get_numeric_block(src);
            d.insert(d.end(), s.begin(), s.end());
        }
        break;
        case celltype_string:
        {
            string_cell_block& d = *get_string_block(dest);
            const string_cell_block& s = *get_string_block(src);
            d.insert(d.end(), s.begin(), s.end());
        }
        break;
        case celltype_index:
        {
            index_cell_block& d = *get_index_block(dest);
            const index_cell_block& s = *get_index_block(src);
            d.insert(d.end(), s.begin(), s.end());
        }
        break;
        default:
            assert(!"unhandled cell type.");
    }
}

void cell_block_func::assign_values(base_cell_block* dest, const base_cell_block* src, size_t begin_pos, size_t len)
{
    if (!dest)
        throw general_error("destination cell block is NULL.");

    switch (dest->type)
    {
        case celltype_numeric:
        {
            numeric_cell_block& d = *get_numeric_block(dest);
            const numeric_cell_block& s = *get_numeric_block(src);
            numeric_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            numeric_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.assign(it, it_end);
        }
        break;
        case celltype_string:
        {
            string_cell_block& d = *get_string_block(dest);
            const string_cell_block& s = *get_string_block(src);
            string_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            string_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.assign(it, it_end);
        }
        break;
        case celltype_index:
        {
            index_cell_block& d = *get_index_block(dest);
            const index_cell_block& s = *get_index_block(src);
            index_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            index_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.assign(it, it_end);
        }
        break;
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

template<>
void cell_block_func::get_value<string>(base_cell_block* block, long pos, string& val)
{
    string_cell_block& blk = *get_string_block(block);
    val = blk[pos];
}

template<>
void cell_block_func::get_value<size_t>(base_cell_block* block, long pos, size_t& val)
{
    index_cell_block& blk = *get_index_block(block);
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

template<>
void cell_block_func::get_empty_value<string>(string& val)
{
    val = string();
}

template<>
void cell_block_func::get_empty_value<size_t>(size_t& val)
{
    val = 0;
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
bool test_cell_insertion(_ColT& col_db, typename _ColT::row_key_type row, _ValT val)
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
