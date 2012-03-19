/*************************************************************************
 *
 * Copyright (c) 2012 Kohei Yoshida
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

#include <mdds/grid_map.hpp>
#include <mdds/grid_map_trait.hpp>


#include <cassert>
#include <sstream>
#include <vector>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/noncopyable.hpp>

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

/** custom cell type definition. */
const gridmap::cell_t celltype_user_block = gridmap::celltype_user_start;

struct user_cell
{
    double value;

    user_cell() : value(0.0) {}
    user_cell(double _v) : value(_v) {}
};

struct user_cell_block : public gridmap::base_cell_block, public std::vector<user_cell*>
{
    user_cell_block() : gridmap::base_cell_block(celltype_user_block) {}
    user_cell_block(size_t n) : gridmap::base_cell_block(celltype_user_block), std::vector<user_cell*>(n) {}
};

template<typename T>
class cell_pool : boost::noncopyable
{
    boost::ptr_vector<T> m_pool;
public:
    T* construct()
    {
        m_pool.push_back(new T);
        return &m_pool.back();
    }
};

class user_cell_pool : public cell_pool<user_cell>
{
public:
    user_cell* construct(double val)
    {
        user_cell* p = cell_pool<user_cell>::construct();
        p->value = val;
        return p;
    }
};

}

namespace mdds { namespace gridmap {

cell_t get_cell_type(const user_cell*)
{
    return celltype_user_block;
}

void set_value(base_cell_block* block, size_t pos, user_cell* p)
{
    if (block->type != celltype_user_block)
        throw mdds::general_error("not a user block");

    user_cell_block& blk = *static_cast<user_cell_block*>(block);
    blk[pos] = p;
}

void get_value(base_cell_block* block, size_t pos, user_cell*& val)
{
    user_cell_block& blk = *static_cast<user_cell_block*>(block);
    val = blk[pos];
}

}}

struct my_cell_block_func : public mdds::gridmap::cell_block_func_base
{
    template<typename T>
    static mdds::gridmap::cell_t get_cell_type(const T& cell)
    {
        return mdds::gridmap::get_cell_type(cell);
    }

    template<typename T>
    static void set_value(mdds::gridmap::base_cell_block* block, size_t pos, const T& val)
    {
        mdds::gridmap::set_value(block, pos, val);
    }

    template<typename T>
    static void get_value(mdds::gridmap::base_cell_block* block, size_t pos, T& val)
    {
        mdds::gridmap::get_value(block, pos, val);
    }

    static mdds::gridmap::base_cell_block* create_new_block(
        mdds::gridmap::cell_t type, size_t init_size)
    {
        switch (type)
        {
            case celltype_user_block:
                return new user_cell_block(init_size);
            default:
                ;
        }

        return cell_block_func_base::create_new_block(type, init_size);
    }

    static mdds::gridmap::base_cell_block* clone_block(mdds::gridmap::base_cell_block* p)
    {
        if (!p)
            return NULL;

        switch (p->type)
        {
            case celltype_user_block:
                return new user_cell_block(*static_cast<user_cell_block*>(p));
            default:
                ;
        }

        return cell_block_func_base::clone_block(p);
    }

    static void delete_block(mdds::gridmap::base_cell_block* p)
    {
        if (!p)
            return;

        switch (p->type)
        {
            case celltype_user_block:
                delete static_cast<user_cell_block*>(p);
            break;
            default:
                cell_block_func_base::delete_block(p);
        }
    }
};

struct grid_map_trait
{
    typedef long sheet_key_type;
    typedef long row_key_type;
    typedef long col_key_type;

    typedef my_cell_block_func cell_block_func;
};

namespace {

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

void gridmap_test_custom_celltype()
{
    stack_printer __stack_printer__("::gridmap_test_custom_celltype");
    mdds::gridmap::cell_t ct;

    // Basic types
    ct = column_type::get_cell_type(double(12.3));
    assert(ct == gridmap::celltype_numeric);
    ct = column_type::get_cell_type(string());
    assert(ct == gridmap::celltype_string);
    ct = column_type::get_cell_type(size_t(12));
    assert(ct == gridmap::celltype_index);
    ct = column_type::get_cell_type(true);
    assert(ct == gridmap::celltype_boolean);
    ct = column_type::get_cell_type(false);
    assert(ct == gridmap::celltype_boolean);

    // Custom cell type
    user_cell* p = NULL;
    ct = column_type::get_cell_type(p);
    assert(ct == celltype_user_block && ct >= gridmap::celltype_user_start);

    // mdds::grid_map does not manage the life cycle of individual cells; the
    // client code needs to manage them when storing pointers.

    user_cell_pool pool;

    // set_cell()

    column_type db(4);
    p = pool.construct(1.2);
    db.set_cell(0, p);

    user_cell* p2 = db.get_cell<user_cell*>(0);
    assert(p->value == p2->value);

    p = pool.construct(3.4);
    db.set_cell(0, p);
    p2 = db.get_cell<user_cell*>(0);
    assert(p->value == p2->value);

    // set_cells()

}

}

int main (int argc, char **argv)
{
    cmd_options opt;
    if (!parse_cmd_options(argc, argv, opt))
        return EXIT_FAILURE;

    if (opt.test_func)
    {
        gridmap_test_custom_celltype();
    }

    if (opt.test_perf)
    {
    }

    return EXIT_SUCCESS;
}
