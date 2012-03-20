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

    static user_cell_block& get(gridmap::base_cell_block* p)
    {
        if (!p || p->type != celltype_user_block)
            throw general_error("block is not of type user_cell_block.");

        return *static_cast<user_cell_block*>(p);
    }

    static const user_cell_block& get(const gridmap::base_cell_block* p)
    {
        if (!p || p->type != celltype_user_block)
            throw general_error("block is not of type user_cell_block.");

        return *static_cast<const user_cell_block*>(p);
    }
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

    void clear() { m_pool.clear(); }
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
    user_cell_block& blk = user_cell_block::get(block);
    blk[pos] = p;
}

void get_value(base_cell_block* block, size_t pos, user_cell*& val)
{
    user_cell_block& blk = user_cell_block::get(block);
    val = blk[pos];
}

void append_value(base_cell_block* block, user_cell* val)
{
    user_cell_block& blk = user_cell_block::get(block);
    blk.push_back(val);
}

template<typename _Iter>
void append_values(mdds::gridmap::base_cell_block* block, user_cell*, const _Iter& it_begin, const _Iter& it_end)
{
    user_cell_block& d = user_cell_block::get(block);
    user_cell_block::iterator it = d.end();
    d.insert(it, it_begin, it_end);
}

template<typename _Iter>
void assign_values(mdds::gridmap::base_cell_block* dest, user_cell*, const _Iter& it_begin, const _Iter& it_end)
{
    user_cell_block& d = user_cell_block::get(dest);
    d.assign(it_begin, it_end);
}

template<typename _Iter>
void insert_values(
    mdds::gridmap::base_cell_block* block, size_t pos, user_cell*, const _Iter& it_begin, const _Iter& it_end)
{
    user_cell_block& d = user_cell_block::get(block);
    d.insert(d.begin()+pos, it_begin, it_end);
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

    template<typename T>
    static void append_value(mdds::gridmap::base_cell_block* block, const T& val)
    {
        mdds::gridmap::append_value(block, val);
    }

    template<typename T>
    static void insert_values(
        mdds::gridmap::base_cell_block* block, size_t pos, const T& it_begin, const T& it_end)
    {
        assert(it_begin != it_end);
        mdds::gridmap::insert_values(block, pos, *it_begin, it_begin, it_end);
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

    static void resize_block(mdds::gridmap::base_cell_block* p, size_t new_size)
    {
        if (!p)
            return;

        switch (p->type)
        {
            case celltype_user_block:
                static_cast<user_cell_block*>(p)->resize(new_size);
            break;
            default:
                cell_block_func_base::resize_block(p, new_size);
        }
    }

    static void append_values_from_block(
        mdds::gridmap::base_cell_block* dest, const mdds::gridmap::base_cell_block* src)
    {
        if (!dest)
            throw mdds::general_error("empty destination block.");

        switch (dest->type)
        {
            case celltype_user_block:
            {
                user_cell_block& d = user_cell_block::get(dest);
                const user_cell_block& s = user_cell_block::get(src);
                d.insert(d.end(), s.begin(), s.end());
            }
            break;
            default:
                cell_block_func_base::append_values_from_block(dest, src);
        }
    }

    static void append_values_from_block(
        mdds::gridmap::base_cell_block* dest, const mdds::gridmap::base_cell_block* src,
        size_t begin_pos, size_t len)
    {
        if (!dest)
            throw mdds::general_error("empty destination block.");

        switch (dest->type)
        {
            case celltype_user_block:
            {
                user_cell_block& d = user_cell_block::get(dest);
                const user_cell_block& s = user_cell_block::get(src);
                user_cell_block::const_iterator it = s.begin();
                std::advance(it, begin_pos);
                user_cell_block::const_iterator it_end = it;
                std::advance(it_end, len);
                d.reserve(d.size() + len);
                std::copy(it, it_end, std::back_inserter(d));
            }
            break;
            default:
                cell_block_func_base::append_values_from_block(dest, src, begin_pos, len);
        }
    }

    template<typename T>
    static void append_values(mdds::gridmap::base_cell_block* block, const T& it_begin, const T& it_end)
    {
        assert(it_begin != it_end);
        mdds::gridmap::append_values(block, *it_begin, it_begin, it_end);
    }

    template<typename T>
    static void assign_values(mdds::gridmap::base_cell_block* dest, const T& it_begin, const T& it_end)
    {
        assert(it_begin != it_end);
        mdds::gridmap::assign_values(dest, *it_begin, it_begin, it_end);
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

void gridmap_test_types()
{
    stack_printer __stack_printer__("::gridmap_test_types");

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
}

void gridmap_test_basic()
{
    stack_printer __stack_printer__("::gridmap_test_basic");

    // mdds::grid_map does not manage the life cycle of individual cells; the
    // client code needs to manage them when storing pointers.

    user_cell_pool pool;

    {
        // set_cell()
        column_type db(4);
        user_cell* p = pool.construct(1.2);
        db.set_cell(0, p);
        db.set_cell(1, p);
        db.set_cell(3, p);
        db.set_cell(2, p);

        user_cell* p2 = db.get_cell<user_cell*>(0);
        assert(p->value == p2->value);

        p = pool.construct(3.4);
        db.set_cell(0, p);
        p2 = db.get_cell<user_cell*>(0);
        assert(p->value == p2->value);
        pool.clear();
    }

    {
        // set_cells(), resize(), insert_cells().
        column_type db(3);
        user_cell* p1 = pool.construct(1.1);
        user_cell* p2 = pool.construct(2.2);
        user_cell* p3 = pool.construct(3.3);
        std::vector<user_cell*> vals;
        vals.reserve(3);
        vals.push_back(p1);
        vals.push_back(p2);
        vals.push_back(p3);
        db.set_cells(0, vals.begin(), vals.end());

        user_cell* ptest;
        ptest = db.get_cell<user_cell*>(0);
        assert(ptest && ptest->value == 1.1);
        ptest = db.get_cell<user_cell*>(1);
        assert(ptest && ptest->value == 2.2);
        ptest = db.get_cell<user_cell*>(2);
        assert(ptest && ptest->value == 3.3);

        db.resize(6);
        user_cell* p4 = pool.construct(11);
        user_cell* p5 = pool.construct(22);
        user_cell* p6 = pool.construct(33);
        vals.clear();
        vals.push_back(p4);
        vals.push_back(p5);
        vals.push_back(p6);
        db.set_cells(3, vals.begin(), vals.end());

        ptest = db.get_cell<user_cell*>(0);
        assert(ptest && ptest->value == 1.1);
        ptest = db.get_cell<user_cell*>(1);
        assert(ptest && ptest->value == 2.2);
        ptest = db.get_cell<user_cell*>(2);
        assert(ptest && ptest->value == 3.3);
        ptest = db.get_cell<user_cell*>(3);
        assert(ptest && ptest->value == 11);
        ptest = db.get_cell<user_cell*>(4);
        assert(ptest && ptest->value == 22);
        ptest = db.get_cell<user_cell*>(5);
        assert(ptest && ptest->value == 33);

        // Shrink the block to erase the bottom 3 cells.
        db.resize(3);
        assert(db.size() == 3);
        ptest = db.get_cell<user_cell*>(2);
        assert(ptest && ptest->value == 3.3);

        // Re-insert the values at the front.
        db.insert_cells(0, vals.begin(), vals.end());
        assert(db.size() == 6);

        ptest = db.get_cell<user_cell*>(0);
        assert(ptest && ptest->value == 11);
        ptest = db.get_cell<user_cell*>(1);
        assert(ptest && ptest->value == 22);
        ptest = db.get_cell<user_cell*>(2);
        assert(ptest && ptest->value == 33);
        ptest = db.get_cell<user_cell*>(3);
        assert(ptest && ptest->value == 1.1);
        ptest = db.get_cell<user_cell*>(4);
        assert(ptest && ptest->value == 2.2);
        ptest = db.get_cell<user_cell*>(5);
        assert(ptest && ptest->value == 3.3);
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
        gridmap_test_types();
        gridmap_test_basic();
    }

    if (opt.test_perf)
    {
    }

    return EXIT_SUCCESS;
}
