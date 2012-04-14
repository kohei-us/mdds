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

using namespace std;
using namespace mdds;

namespace {

/** custom cell type definition. */
const gridmap::cell_t celltype_user_block  = gridmap::celltype_user_start;
const gridmap::cell_t celltype_muser_block = gridmap::celltype_user_start+1;

/** Caller manages the life cycle of these cells. */
struct user_cell
{
    double value;

    user_cell() : value(0.0) {}
    user_cell(double _v) : value(_v) {}
};

/**
 * Managed user cell: the storing block manages the life cycle of these
 * cells.
 */
struct muser_cell
{
    double value;

    muser_cell() : value(0.0) {}
    muser_cell(double _v) : value(_v) {}
};

struct user_cell_block : public mdds::gridmap::cell_block<user_cell_block, celltype_user_block, user_cell*>
{
    typedef mdds::gridmap::cell_block<user_cell_block, celltype_user_block, user_cell*> base_type;

    using base_type::get;

    user_cell_block() : base_type() {}
    user_cell_block(size_t n) : base_type(n) {}
};

struct muser_cell_block : public mdds::gridmap::cell_block<muser_cell_block, celltype_muser_block, muser_cell*>
{
    typedef mdds::gridmap::cell_block<muser_cell_block, celltype_muser_block, muser_cell*> base_type;

    using base_type::get;

    muser_cell_block() : base_type() {}
    muser_cell_block(size_t n) : base_type(n) {}
    muser_cell_block(const muser_cell_block& r)
    {
        reserve(r.size());
        muser_cell_block::const_iterator it = r.begin(), it_end = r.end();
        for (; it != it_end; ++it)
            push_back(new muser_cell(**it));
    }

    ~muser_cell_block()
    {
        std::for_each(begin(), end(), default_deleter<muser_cell>());
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

cell_t get_cell_type(const muser_cell*)
{
    return celltype_muser_block;
}

//----------------------------------------------------------------------------

void set_value(base_cell_block& block, size_t pos, user_cell* p)
{
    user_cell_block& blk = user_cell_block::get(block);
    blk[pos] = p;
}

void set_value(base_cell_block& block, size_t pos, muser_cell* p)
{
    muser_cell_block& blk = muser_cell_block::get(block);
    blk[pos] = p;
}

//----------------------------------------------------------------------------

template<typename _Iter>
void set_values(
    base_cell_block& block, size_t pos, user_cell*, const _Iter& it_begin, const _Iter& it_end)
{
    user_cell_block& d = user_cell_block::get(block);
    for (_Iter it = it_begin; it != it_end; ++it, ++pos)
        d[pos] = *it;
}

template<typename _Iter>
void set_values(
    base_cell_block& block, size_t pos, muser_cell*, const _Iter& it_begin, const _Iter& it_end)
{
    muser_cell_block& d = muser_cell_block::get(block);
    for (_Iter it = it_begin; it != it_end; ++it, ++pos)
        d[pos] = *it;
}

//----------------------------------------------------------------------------

void get_value(const base_cell_block& block, size_t pos, user_cell*& val)
{
    const user_cell_block& blk = user_cell_block::get(block);
    val = blk[pos];
}

void get_value(const base_cell_block& block, size_t pos, muser_cell*& val)
{
    const muser_cell_block& blk = muser_cell_block::get(block);
    val = blk[pos];
}

//----------------------------------------------------------------------------

void append_value(base_cell_block& block, user_cell* val)
{
    user_cell_block& blk = user_cell_block::get(block);
    blk.push_back(val);
}

void append_value(base_cell_block& block, muser_cell* val)
{
    muser_cell_block& blk = muser_cell_block::get(block);
    blk.push_back(val);
}

//----------------------------------------------------------------------------

void prepend_value(base_cell_block& block, user_cell* val)
{
    user_cell_block& blk = user_cell_block::get(block);
    blk.insert(blk.begin(), val);
}

void prepend_value(base_cell_block& block, muser_cell* val)
{
    muser_cell_block& blk = muser_cell_block::get(block);
    blk.insert(blk.begin(), val);
}

//----------------------------------------------------------------------------

template<typename _Iter>
void append_values(mdds::gridmap::base_cell_block& block, user_cell*, const _Iter& it_begin, const _Iter& it_end)
{
    user_cell_block& d = user_cell_block::get(block);
    user_cell_block::iterator it = d.end();
    d.insert(it, it_begin, it_end);
}

template<typename _Iter>
void append_values(mdds::gridmap::base_cell_block& block, muser_cell*, const _Iter& it_begin, const _Iter& it_end)
{
    muser_cell_block& d = muser_cell_block::get(block);
    muser_cell_block::iterator it = d.end();
    d.insert(it, it_begin, it_end);
}

//----------------------------------------------------------------------------

template<typename _Iter>
void prepend_values(mdds::gridmap::base_cell_block& block, user_cell*, const _Iter& it_begin, const _Iter& it_end)
{
    user_cell_block& d = user_cell_block::get(block);
    d.insert(d.begin(), it_begin, it_end);
}

template<typename _Iter>
void prepend_values(mdds::gridmap::base_cell_block& block, muser_cell*, const _Iter& it_begin, const _Iter& it_end)
{
    muser_cell_block& d = muser_cell_block::get(block);
    d.insert(d.begin(), it_begin, it_end);
}

//----------------------------------------------------------------------------

template<typename _Iter>
void assign_values(mdds::gridmap::base_cell_block& dest, user_cell*, const _Iter& it_begin, const _Iter& it_end)
{
    user_cell_block& d = user_cell_block::get(dest);
    d.assign(it_begin, it_end);
}

template<typename _Iter>
void assign_values(mdds::gridmap::base_cell_block& dest, muser_cell*, const _Iter& it_begin, const _Iter& it_end)
{
    muser_cell_block& d = muser_cell_block::get(dest);
    d.assign(it_begin, it_end);
}

//----------------------------------------------------------------------------

template<typename _Iter>
void insert_values(
    mdds::gridmap::base_cell_block& block, size_t pos, user_cell*, const _Iter& it_begin, const _Iter& it_end)
{
    user_cell_block& d = user_cell_block::get(block);
    d.insert(d.begin()+pos, it_begin, it_end);
}

template<typename _Iter>
void insert_values(
    mdds::gridmap::base_cell_block& block, size_t pos, muser_cell*, const _Iter& it_begin, const _Iter& it_end)
{
    muser_cell_block& d = muser_cell_block::get(block);
    d.insert(d.begin()+pos, it_begin, it_end);
}

//----------------------------------------------------------------------------

void get_empty_value(user_cell*& val)
{
    val = NULL;
}

void get_empty_value(muser_cell*& val)
{
    val = NULL;
}

//----------------------------------------------------------------------------

}}

struct my_cell_block_func : public mdds::gridmap::cell_block_func_base
{
    template<typename T>
    static mdds::gridmap::cell_t get_cell_type(const T& cell)
    {
        return mdds::gridmap::get_cell_type(cell);
    }

    template<typename T>
    static void set_value(mdds::gridmap::base_cell_block& block, size_t pos, const T& val)
    {
        mdds::gridmap::set_value(block, pos, val);
    }

    template<typename T>
    static void set_values(mdds::gridmap::base_cell_block& block, size_t pos, const T& it_begin, const T& it_end)
    {
        assert(it_begin != it_end);
        mdds::gridmap::set_values(block, pos, *it_begin, it_begin, it_end);
    }

    template<typename T>
    static void get_value(const mdds::gridmap::base_cell_block& block, size_t pos, T& val)
    {
        mdds::gridmap::get_value(block, pos, val);
    }

    template<typename T>
    static void append_value(mdds::gridmap::base_cell_block& block, const T& val)
    {
        mdds::gridmap::append_value(block, val);
    }

    template<typename T>
    static void insert_values(
        mdds::gridmap::base_cell_block& block, size_t pos, const T& it_begin, const T& it_end)
    {
        assert(it_begin != it_end);
        mdds::gridmap::insert_values(block, pos, *it_begin, it_begin, it_end);
    }

    template<typename T>
    static void append_values(mdds::gridmap::base_cell_block& block, const T& it_begin, const T& it_end)
    {
        assert(it_begin != it_end);
        mdds::gridmap::append_values(block, *it_begin, it_begin, it_end);
    }

    template<typename T>
    static void assign_values(mdds::gridmap::base_cell_block& dest, const T& it_begin, const T& it_end)
    {
        assert(it_begin != it_end);
        mdds::gridmap::assign_values(dest, *it_begin, it_begin, it_end);
    }

    template<typename T>
    static void prepend_value(mdds::gridmap::base_cell_block& block, const T& val)
    {
        mdds::gridmap::prepend_value(block, val);
    }

    template<typename T>
    static void prepend_values(mdds::gridmap::base_cell_block& block, const T& it_begin, const T& it_end)
    {
        assert(it_begin != it_end);
        mdds::gridmap::prepend_values(block, *it_begin, it_begin, it_end);
    }

    template<typename T>
    static void get_empty_value(T& val)
    {
        mdds::gridmap::get_empty_value(val);
    }

    static mdds::gridmap::base_cell_block* create_new_block(
        mdds::gridmap::cell_t type, size_t init_size)
    {
        switch (type)
        {
            case celltype_user_block:
                return new user_cell_block(init_size);
            case celltype_muser_block:
                return new muser_cell_block(init_size);
            default:
                ;
        }

        return cell_block_func_base::create_new_block(type, init_size);
    }

    static mdds::gridmap::base_cell_block* clone_block(const mdds::gridmap::base_cell_block& block)
    {
        switch (block.type)
        {
            case celltype_user_block:
                return new user_cell_block(user_cell_block::get(block));
            case celltype_muser_block:
                return new muser_cell_block(muser_cell_block::get(block));
            default:
                ;
        }

        return cell_block_func_base::clone_block(block);
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
            case celltype_muser_block:
                delete static_cast<muser_cell_block*>(p);
            break;
            default:
                cell_block_func_base::delete_block(p);
        }
    }

    static void resize_block(mdds::gridmap::base_cell_block& block, size_t new_size)
    {
        switch (block.type)
        {
            case celltype_user_block:
                static_cast<user_cell_block&>(block).resize(new_size);
            break;
            case celltype_muser_block:
                static_cast<muser_cell_block&>(block).resize(new_size);
            break;
            default:
                cell_block_func_base::resize_block(block, new_size);
        }
    }

    static void print_block(const mdds::gridmap::base_cell_block& block)
    {
        switch (block.type)
        {
            case celltype_user_block:
            {
                const user_cell_block& blk = user_cell_block::get(block);
                for_each(blk.begin(), blk.end(),
                         mdds::gridmap::print_block_array<user_cell*>());
                cout << endl;
            }
            break;
            case celltype_muser_block:
            {
                const muser_cell_block& blk = muser_cell_block::get(block);
                for_each(blk.begin(), blk.end(),
                         mdds::gridmap::print_block_array<muser_cell*>());
                cout << endl;
            }
            break;
            default:
                cell_block_func_base::print_block(block);
        }
    }

    static void erase(mdds::gridmap::base_cell_block& block, size_t pos)
    {
        switch (block.type)
        {
            case celltype_user_block:
            {
                user_cell_block& blk = user_cell_block::get(block);
                blk.erase(blk.begin()+pos);
            }
            break;
            case celltype_muser_block:
            {
                muser_cell_block& blk = muser_cell_block::get(block);
                blk.erase(blk.begin()+pos);
            }
            break;
            default:
                cell_block_func_base::erase(block, pos);
        }
    }

    static void erase(mdds::gridmap::base_cell_block& block, size_t pos, size_t size)
    {
        switch (block.type)
        {
            case celltype_user_block:
            {
                user_cell_block& blk = user_cell_block::get(block);
                blk.erase(blk.begin()+pos, blk.begin()+pos+size);
            }
            break;
            case celltype_muser_block:
            {
                muser_cell_block& blk = muser_cell_block::get(block);
                blk.erase(blk.begin()+pos, blk.begin()+pos+size);
            }
            break;
            default:
                cell_block_func_base::erase(block, pos, size);
        }
    }

    static void append_values_from_block(
        mdds::gridmap::base_cell_block& dest, const mdds::gridmap::base_cell_block& src)
    {
        switch (dest.type)
        {
            case celltype_user_block:
            {
                user_cell_block& d = user_cell_block::get(dest);
                const user_cell_block& s = user_cell_block::get(src);
                d.insert(d.end(), s.begin(), s.end());
            }
            break;
            case celltype_muser_block:
            {
                muser_cell_block& d = muser_cell_block::get(dest);
                const muser_cell_block& s = muser_cell_block::get(src);
                d.insert(d.end(), s.begin(), s.end());
            }
            break;
            default:
                cell_block_func_base::append_values_from_block(dest, src);
        }
    }

    static void append_values_from_block(
        mdds::gridmap::base_cell_block& dest, const mdds::gridmap::base_cell_block& src,
        size_t begin_pos, size_t len)
    {
        switch (dest.type)
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
            case celltype_muser_block:
            {
                muser_cell_block& d = muser_cell_block::get(dest);
                const muser_cell_block& s = muser_cell_block::get(src);
                muser_cell_block::const_iterator it = s.begin();
                std::advance(it, begin_pos);
                muser_cell_block::const_iterator it_end = it;
                std::advance(it_end, len);
                d.reserve(d.size() + len);
                std::copy(it, it_end, std::back_inserter(d));
            }
            break;
            default:
                cell_block_func_base::append_values_from_block(dest, src, begin_pos, len);
        }
    }

    static void assign_values_from_block(
        mdds::gridmap::base_cell_block& dest, const mdds::gridmap::base_cell_block& src,
        size_t begin_pos, size_t len)
    {
        switch (dest.type)
        {
            case celltype_user_block:
            {
                user_cell_block& d = user_cell_block::get(dest);
                const user_cell_block& s = user_cell_block::get(src);
                user_cell_block::const_iterator it = s.begin();
                std::advance(it, begin_pos);
                user_cell_block::const_iterator it_end = it;
                std::advance(it_end, len);
                d.assign(it, it_end);
            }
            break;
            case celltype_muser_block:
            {
                muser_cell_block& d = muser_cell_block::get(dest);
                const muser_cell_block& s = muser_cell_block::get(src);
                muser_cell_block::const_iterator it = s.begin();
                std::advance(it, begin_pos);
                muser_cell_block::const_iterator it_end = it;
                std::advance(it_end, len);
                d.assign(it, it_end);
            }
            break;
            default:
                cell_block_func_base::assign_values_from_block(dest, src, begin_pos, len);
        }
    }

    static bool equal_block(
        const mdds::gridmap::base_cell_block& left, const mdds::gridmap::base_cell_block& right)
    {
        if (left.type == celltype_user_block)
        {
            if (right.type != celltype_user_block)
                return false;

            return user_cell_block::get(left) == user_cell_block::get(right);
        }
        else if (right.type == celltype_user_block)
            return false;

        if (left.type == celltype_muser_block)
        {
            if (right.type != celltype_muser_block)
                return false;

            return muser_cell_block::get(left) == muser_cell_block::get(right);
        }
        else if (right.type == celltype_muser_block)
            return false;

        return cell_block_func_base::equal_block(left, right);
    }

    static void overwrite_cells(mdds::gridmap::base_cell_block& block, size_t pos, size_t len)
    {
        switch (block.type)
        {
            case celltype_user_block:
                // Do nothing.  The client code manages the life cycle of these cells.
            break;
            case celltype_muser_block:
            {
                // The block manages the life cycle of stored instances.
                muser_cell_block& blk = muser_cell_block::get(block);
                muser_cell_block::iterator it = blk.begin() + pos;
                muser_cell_block::iterator it_end = it + len;
                std::for_each(it, it_end, default_deleter<muser_cell>());
            }
            break;
            default:
                cell_block_func_base::overwrite_cells(block, pos, len);
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

        // set_empty(), is_empty().
        db.set_empty(2, 4);
        assert(db.block_size() == 3);
        assert(db.get_cell<user_cell*>(1)->value == 22);
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));
        assert(db.get_cell<user_cell*>(5)->value == 3.3);

        // erase()
        db.erase(3, 5);
        assert(db.size() == 3);
        assert(db.get_cell<user_cell*>(1)->value == 22);
        assert(db.is_empty(2));

        // insert_empty().
        db.insert_empty(1, 2);
        assert(db.size() == 5);
        assert(db.get_cell<user_cell*>(0)->value == 11);
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.get_cell<user_cell*>(3)->value == 22);
        assert(db.is_empty(4));

        pool.clear();
    }

    {
        // set_cells() to overwrite existing values of type user_cell*.
        column_type db(2);
        user_cell* p0 = pool.construct(1.2);
        db.set_cell(1, p0);
        db.set_cell(0, p0);

        vector<user_cell*> vals;
        vals.push_back(pool.construct(2.3));
        vals.push_back(pool.construct(2.4));
        db.set_cells(0, vals.begin(), vals.end());
        pool.clear();
    }

    {
        column_type db(4);
        user_cell* p0 = pool.construct(1.1);
        db.set_cell(3, p0);

        vector<user_cell*> vals;
        vals.push_back(pool.construct(2.3));
        vals.push_back(pool.construct(2.4));
        db.set_cells(1, vals.begin(), vals.end());
        assert(db.is_empty(0));
        assert(db.get_cell<user_cell*>(1)->value == 2.3);
        assert(db.get_cell<user_cell*>(2)->value == 2.4);
        assert(db.get_cell<user_cell*>(3)->value == 1.1);

        pool.clear();
    }

    {
        // Get empty value.
        column_type db(1);
        user_cell* p = db.get_cell<user_cell*>(0);
        assert(p == NULL);
    }
}

void gridmap_test_equality()
{
    stack_printer __stack_printer__("::gridmap_test_clone");

    user_cell_pool pool;

    column_type db1(3);
    column_type db2 = db1;
    assert(db2 == db1);
    user_cell* p0 = pool.construct(1.1);
    db1.set_cell(0, p0);
    assert(db1 != db2);
    db2.set_cell(0, p0);
    assert(db1 == db2);
    db1.set_cell(2, string("foo"));
    db2.set_cell(2, string("foo"));
    assert(db1 == db2);

    // same value but different memory addresses.
    user_cell* p1 = pool.construct(1.2);
    user_cell* p2 = pool.construct(1.2);
    db1.set_cell(1, p1);
    db2.set_cell(1, p2);
    assert(db1 != db2); // equality is by the pointer value.
}

/**
 * This test is to be run with valgrind, to ensure no memory leak occurs.
 */
void gridmap_test_managed_block()
{
    stack_printer __stack_printer__("::gridmap_test_managed_block");
    {
        column_type db(1);
        db.set_cell(0, new muser_cell(1.0));
        const muser_cell* p = db.get_cell<muser_cell*>(0);
        assert(p->value == 1.0);
        db.set_cell(0, new muser_cell(2.0)); // overwrite.
        p = db.get_cell<muser_cell*>(0);
        assert(p->value == 2.0);
    }

    {
        column_type db(3);

        // Empty the upper part.
        db.set_cell(0, new muser_cell(1.0));
        db.set_cell(1, new muser_cell(2.0));
        db.set_cell(2, new muser_cell(3.0));
        db.set_empty(0, 0);

        // Empty the lower part.
        db.set_cell(0, new muser_cell(4.0));
        db.set_empty(2, 2);

        // Empty the middle part.
        db.set_cell(2, new muser_cell(5.0));
        db.set_empty(1, 1);
    }

    {
        column_type db(3);
        db.set_cell(0, new muser_cell(1.0));
        db.set_cell(1, new muser_cell(2.0));
        db.set_cell(2, 3.0);
        db.set_empty(1, 2);

        db.set_cell(0, string("foo"));
        db.set_cell(1, new muser_cell(4.0));
        db.set_cell(2, new muser_cell(5.0));
        db.set_empty(0, 1);

        db.set_cell(0, new muser_cell(6.0));
        db.set_cell(1, size_t(12));
        db.set_empty(0, 2);
    }

    {
        // Cloning.
        column_type db(3);
        db.set_cell(0, new muser_cell(1.0));
        db.set_cell(1, new muser_cell(2.0));
        db.set_cell(2, new muser_cell(3.0));

        column_type db2;
        db2.swap(db);
        assert(db.empty());
        assert(db2.get_cell<muser_cell*>(0)->value == 1.0);
        assert(db2.get_cell<muser_cell*>(1)->value == 2.0);
        assert(db2.get_cell<muser_cell*>(2)->value == 3.0);
        db.swap(db2);
        assert(db2.empty());
        assert(db.get_cell<muser_cell*>(0)->value == 1.0);
        assert(db.get_cell<muser_cell*>(1)->value == 2.0);
        assert(db.get_cell<muser_cell*>(2)->value == 3.0);
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
        gridmap_test_equality();
        gridmap_test_managed_block();
    }

    if (opt.test_perf)
    {
    }

    return EXIT_SUCCESS;
}
