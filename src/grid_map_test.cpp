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

struct cell_block_deleter : public std::unary_function<base_cell_block*, void>
{
    void operator() (base_cell_block* p)
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

    template<typename T>
    static void set_value(base_cell_block* block, long pos, const T& val);

    template<typename T>
    static void get_value(base_cell_block* block, long pos, T& val);
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

template<typename T>
void cell_block_func::set_value(base_cell_block* block, long pos, const T& val)
{
    throw general_error("non-specialized version called.");
}

template<>
void cell_block_func::set_value<double>(base_cell_block* block, long pos, const double& val)
{
    if (block->type != celltype_numeric)
        throw general_error("block is not of numeric type!");

    numeric_cell_block& blk = static_cast<numeric_cell_block&>(*block);
    blk[pos] = val;
}

template<typename T>
void cell_block_func::get_value(base_cell_block* block, long pos, T& val)
{
    throw general_error("non-specialized version called.");
}

template<>
void cell_block_func::get_value<double>(base_cell_block* block, long pos, double& val)
{
    if (block->type != celltype_numeric)
        throw general_error("block is not of numeric type!");

    numeric_cell_block& blk = static_cast<numeric_cell_block&>(*block);
    val = blk[pos];
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

    typedef cell_block_deleter cell_block_delete_handler;
    typedef cell_block_func cell_block_modifier;
};

}

void gridmap_test_basic()
{
    stack_printer __stack_printer__("::gridmap_test_basic");
    typedef mdds::grid_map<grid_map_trait> grid_store_type;
    typedef grid_store_type::sheet_type::column_type column_type;
    grid_store_type db;

    // Single column instance with only one row.
    column_type col_db(1);

    double val = 2.0, test = 0.0;
    col_db.set_cell(0, val);
    col_db.get_cell(0, test);
    assert(val == test);
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
