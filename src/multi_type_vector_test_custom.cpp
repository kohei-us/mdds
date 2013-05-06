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

#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#include <mdds/multi_type_vector.hpp>
#include <mdds/multi_type_vector_trait.hpp>
#include <mdds/multi_type_vector_custom_func1.hpp>
#include <mdds/multi_type_vector_custom_func2.hpp>

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
const mtv::element_t element_type_user_block  = mtv::element_type_user_start;
const mtv::element_t element_type_muser_block = mtv::element_type_user_start+1;
const mtv::element_t element_type_fruit_block = mtv::element_type_user_start+2;

enum my_fruit_type { unknown_fruit = 0, apple, orange, mango, peach };

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
    muser_cell(const muser_cell& r) : value(r.value) {}
    muser_cell(double _v) : value(_v) {}
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

typedef mdds::mtv::default_element_block<element_type_user_block, user_cell*> user_cell_block;
typedef mdds::mtv::managed_element_block<element_type_muser_block, muser_cell> muser_cell_block;
typedef mdds::mtv::default_element_block<element_type_fruit_block, my_fruit_type> fruit_block;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(user_cell, element_type_user_block, NULL, user_cell_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(muser_cell, element_type_muser_block, NULL, muser_cell_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(my_fruit_type, element_type_fruit_block, unknown_fruit, fruit_block)

}

namespace {

typedef multi_type_vector<mtv::custom_block_func2<element_type_user_block, user_cell_block, element_type_muser_block, muser_cell_block> > mtv_type;
typedef multi_type_vector<mtv::custom_block_func1<element_type_fruit_block, fruit_block> > mtv_fruit_type;

template<typename _ColT, typename _ValT>
bool test_cell_insertion(_ColT& col_db, size_t row, _ValT val)
{
    _ValT test;
    col_db.set(row, val);
    col_db.get(row, test);
    return val == test;
}

void mtv_test_types()
{
    stack_printer __stack_printer__("::mtv_test_types");

    mdds::mtv::element_t ct;

    // Basic types
    ct = mtv_type::get_element_type(double(12.3));
    assert(ct == mtv::element_type_numeric);
    ct = mtv_type::get_element_type(string());
    assert(ct == mtv::element_type_string);
    ct = mtv_type::get_element_type(static_cast<unsigned long>(12));
    assert(ct == mtv::element_type_ulong);
    ct = mtv_type::get_element_type(true);
    assert(ct == mtv::element_type_boolean);
    ct = mtv_type::get_element_type(false);
    assert(ct == mtv::element_type_boolean);

    // Custom cell type
    user_cell* p = NULL;
    ct = mtv_type::get_element_type(p);
    assert(ct == element_type_user_block && ct >= mtv::element_type_user_start);
    ct = mtv_type::get_element_type(static_cast<muser_cell*>(NULL));
    assert(ct == element_type_muser_block && ct >= mtv::element_type_user_start);
    ct = mtv_fruit_type::get_element_type(unknown_fruit);
    assert(ct == element_type_fruit_block && ct >= mtv::element_type_user_start);
}

void mtv_test_basic()
{
    stack_printer __stack_printer__("::mtv_test_basic");

    // mdds::multi_type_vector does not manage the life cycle of individual cells;
    // the client code needs to manage them when storing pointers.

    user_cell_pool pool;

    {
        // set_cell()
        mtv_type db(4);
        user_cell* p = pool.construct(1.2);
        db.set(0, p);
        db.set(1, p);
        db.set(3, p);
        db.set(2, p);

        user_cell* p2 = db.get<user_cell*>(0);
        assert(p->value == p2->value);

        p = pool.construct(3.4);
        db.set(0, p);
        p2 = db.get<user_cell*>(0);
        assert(p->value == p2->value);
        pool.clear();
    }

    {
        // set_cells(), resize(), insert_cells().
        mtv_type db(3);
        user_cell* p1 = pool.construct(1.1);
        user_cell* p2 = pool.construct(2.2);
        user_cell* p3 = pool.construct(3.3);
        std::vector<user_cell*> vals;
        vals.reserve(3);
        vals.push_back(p1);
        vals.push_back(p2);
        vals.push_back(p3);
        db.set(0, vals.begin(), vals.end());

        user_cell* ptest;
        ptest = db.get<user_cell*>(0);
        assert(ptest && ptest->value == 1.1);
        ptest = db.get<user_cell*>(1);
        assert(ptest && ptest->value == 2.2);
        ptest = db.get<user_cell*>(2);
        assert(ptest && ptest->value == 3.3);

        db.resize(6);
        user_cell* p4 = pool.construct(11);
        user_cell* p5 = pool.construct(22);
        user_cell* p6 = pool.construct(33);
        vals.clear();
        vals.push_back(p4);
        vals.push_back(p5);
        vals.push_back(p6);
        db.set(3, vals.begin(), vals.end());

        ptest = db.get<user_cell*>(0);
        assert(ptest && ptest->value == 1.1);
        ptest = db.get<user_cell*>(1);
        assert(ptest && ptest->value == 2.2);
        ptest = db.get<user_cell*>(2);
        assert(ptest && ptest->value == 3.3);
        ptest = db.get<user_cell*>(3);
        assert(ptest && ptest->value == 11);
        ptest = db.get<user_cell*>(4);
        assert(ptest && ptest->value == 22);
        ptest = db.get<user_cell*>(5);
        assert(ptest && ptest->value == 33);

        // Shrink the block to erase the bottom 3 cells.
        db.resize(3);
        assert(db.size() == 3);
        ptest = db.get<user_cell*>(2);
        assert(ptest && ptest->value == 3.3);

        // Re-insert the values at the front.
        db.insert(0, vals.begin(), vals.end());
        assert(db.size() == 6);

        ptest = db.get<user_cell*>(0);
        assert(ptest && ptest->value == 11);
        ptest = db.get<user_cell*>(1);
        assert(ptest && ptest->value == 22);
        ptest = db.get<user_cell*>(2);
        assert(ptest && ptest->value == 33);
        ptest = db.get<user_cell*>(3);
        assert(ptest && ptest->value == 1.1);
        ptest = db.get<user_cell*>(4);
        assert(ptest && ptest->value == 2.2);
        ptest = db.get<user_cell*>(5);
        assert(ptest && ptest->value == 3.3);

        // set_empty(), is_empty().
        db.set_empty(2, 4);
        assert(db.block_size() == 3);
        assert(db.get<user_cell*>(1)->value == 22);
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        assert(db.is_empty(4));
        assert(db.get<user_cell*>(5)->value == 3.3);

        // erase()
        db.erase(3, 5);
        assert(db.size() == 3);
        assert(db.get<user_cell*>(1)->value == 22);
        assert(db.is_empty(2));

        // insert_empty().
        db.insert_empty(1, 2);
        assert(db.size() == 5);
        assert(db.get<user_cell*>(0)->value == 11);
        assert(db.is_empty(1));
        assert(db.is_empty(2));
        assert(db.get<user_cell*>(3)->value == 22);
        assert(db.is_empty(4));

        pool.clear();
    }

    {
        // set_cells() to overwrite existing values of type user_cell*.
        mtv_type db(2);
        user_cell* p0 = pool.construct(1.2);
        db.set(1, p0);
        db.set(0, p0);

        vector<user_cell*> vals;
        vals.push_back(pool.construct(2.3));
        vals.push_back(pool.construct(2.4));
        db.set(0, vals.begin(), vals.end());
        pool.clear();
    }

    {
        mtv_type db(4);
        user_cell* p0 = pool.construct(1.1);
        db.set(3, p0);

        vector<user_cell*> vals;
        vals.push_back(pool.construct(2.3));
        vals.push_back(pool.construct(2.4));
        db.set(1, vals.begin(), vals.end());
        assert(db.is_empty(0));
        assert(db.get<user_cell*>(1)->value == 2.3);
        assert(db.get<user_cell*>(2)->value == 2.4);
        assert(db.get<user_cell*>(3)->value == 1.1);

        pool.clear();
    }

    {
        // Get empty value.
        mtv_type db(1);
        user_cell* p = db.get<user_cell*>(0);
        assert(p == NULL);
    }
}

void mtv_test_equality()
{
    stack_printer __stack_printer__("::mtv_test_clone");

    user_cell_pool pool;

    mtv_type db1(3);
    mtv_type db2 = db1;
    assert(db2 == db1);
    user_cell* p0 = pool.construct(1.1);
    db1.set(0, p0);
    assert(db1 != db2);
    db2.set(0, p0);
    assert(db1 == db2);
    db1.set(2, string("foo"));
    db2.set(2, string("foo"));
    assert(db1 == db2);

    // same value but different memory addresses.
    user_cell* p1 = pool.construct(1.2);
    user_cell* p2 = pool.construct(1.2);
    db1.set(1, p1);
    db2.set(1, p2);
    assert(db1 != db2); // equality is by the pointer value.
}

/**
 * This test is to be run with valgrind, to ensure no memory leak occurs.
 */
void mtv_test_managed_block()
{
    stack_printer __stack_printer__("::mtv_test_managed_block");
    {
        mtv_type db(1);
        db.set(0, new muser_cell(1.0));
        const muser_cell* p = db.get<muser_cell*>(0);
        assert(p->value == 1.0);
        db.set(0, new muser_cell(2.0)); // overwrite.
        p = db.get<muser_cell*>(0);
        assert(p->value == 2.0);
    }

    {
        // Overwrite with empty cells.
        mtv_type db(3);

        // Empty the upper part.
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));
        db.set_empty(0, 0);

        // Empty the lower part.
        db.set(0, new muser_cell(4.0));
        db.set_empty(2, 2);

        // Empty the middle part.
        db.set(2, new muser_cell(5.0));
        db.set_empty(1, 1);
    }

    {
        // More overwrite with empty cells.
        mtv_type db(3);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, 3.0);
        db.set_empty(1, 2);

        db.set(0, string("foo"));
        db.set(1, new muser_cell(4.0));
        db.set(2, new muser_cell(5.0));
        db.set_empty(0, 1);

        db.set(0, new muser_cell(6.0));
        db.set(1, static_cast<unsigned long>(12));
        db.set_empty(0, 2);
    }

    {
        // Another case for set_empty().
        mtv_type db(5);
        db.set(0, 1.2);
        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));
        db.set(3, new muser_cell(4.0));
        db.set(4, new muser_cell(5.0));
        db.set_empty(2, 4);

        db.set(2, new muser_cell(3.0));
        db.set(3, new muser_cell(4.0));
        db.set(4, new muser_cell(5.0));
        db.set_empty(1, 2);

        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));
        db.set_empty(2, 3);
    }

    {
        // Test for cloning.
        mtv_type db(3);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));

        // swap
        mtv_type db2;
        db2.swap(db);
        assert(db.empty());
        assert(db2.get<muser_cell*>(0)->value == 1.0);
        assert(db2.get<muser_cell*>(1)->value == 2.0);
        assert(db2.get<muser_cell*>(2)->value == 3.0);
        db.swap(db2);
        assert(db2.empty());
        assert(db.get<muser_cell*>(0)->value == 1.0);
        assert(db.get<muser_cell*>(1)->value == 2.0);
        assert(db.get<muser_cell*>(2)->value == 3.0);

        // copy constructor
        mtv_type db_copied(db);
        assert(db_copied.size() == 3);
        assert(db_copied.get<muser_cell*>(0)->value == 1.0);
        assert(db_copied.get<muser_cell*>(1)->value == 2.0);
        assert(db_copied.get<muser_cell*>(2)->value == 3.0);

        // Assignment.
        mtv_type db_assigned = db;
        assert(db_assigned.size() == 3);
        assert(db_assigned.get<muser_cell*>(0)->value == 1.0);
        assert(db_assigned.get<muser_cell*>(1)->value == 2.0);
        assert(db_assigned.get<muser_cell*>(2)->value == 3.0);
    }

    {
        // Resize and clear
        mtv_type db(3);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));
        db.resize(1);
        assert(db.get<muser_cell*>(0)->value == 1.0);

        db.clear();
    }

    {
        // Overwrite with a cell of different type.
        mtv_type db(3);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));
        db.set(1, 4.5);
    }

    {
        // Erase (single block)
        mtv_type db(3);

        // Erase the whole thing.
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));
        db.erase(0, 2);
        assert(db.empty());

        // Erase top.
        db.resize(3);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));
        db.erase(0, 1);
        assert(db.size() == 1);

        // Erase bottom.
        db.resize(3);
        db.set(1, new muser_cell(4.0));
        db.set(2, new muser_cell(5.0));
        db.erase(1, 2);
        assert(db.size() == 1);

        // Erase middle.
        db.resize(3);
        db.set(1, new muser_cell(4.0));
        db.set(2, new muser_cell(5.0));
        db.erase(1, 1);
        assert(db.size() == 2);
    }

    {
        // Erase (single block with preceding block)
        mtv_type db(4);

        // Erase the whole thing.
        db.set(0, 1.1);
        db.set(1, new muser_cell(1.0));
        db.set(2, new muser_cell(2.0));
        db.set(3, new muser_cell(3.0));
        db.erase(1, 3);
        assert(db.size() == 1);

        // Erase top.
        db.resize(4);
        db.set(1, new muser_cell(1.0));
        db.set(2, new muser_cell(2.0));
        db.set(3, new muser_cell(3.0));
        db.erase(1, 2);
        assert(db.size() == 2);

        // Erase bottom.
        db.resize(4);
        db.set(2, new muser_cell(4.0));
        db.set(3, new muser_cell(5.0));
        db.erase(2, 3);
        assert(db.size() == 2);

        // Erase middle.
        db.resize(4);
        db.set(2, new muser_cell(4.0));
        db.set(3, new muser_cell(5.0));
        db.erase(2, 2);
        assert(db.size() == 3);
    }

    {
        // Erase (multi-block 1)
        mtv_type db(6);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));
        db.set(3, 4.1);
        db.set(4, 4.2);
        db.set(5, 4.3);
        db.erase(1, 4);
    }

    {
        // Erase (multi-block 2)
        mtv_type db(6);
        db.set(0, 4.1);
        db.set(1, 4.2);
        db.set(2, 4.3);
        db.set(3, new muser_cell(5.0));
        db.set(4, new muser_cell(6.0));
        db.set(5, new muser_cell(7.0));
        db.erase(1, 4);
    }

    {
        // Erase (multi-block 3)
        mtv_type db(6);
        db.set(0, 1.0);
        db.set(1, 2.0);
        db.set(2, new muser_cell(3.0));
        db.set(3, new muser_cell(4.0));
        db.set(4, 5.0);
        db.set(5, 6.0);
        db.erase(1, 4);
    }

    {
        // Insert into the middle of block.  This one shouldn't overwrite any
        // cells, but just to be safe...
        mtv_type db(2);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.insert_empty(1, 2);
        assert(db.size() == 4);
        assert(db.get<muser_cell*>(0)->value == 1.0);
        assert(db.get<muser_cell*>(3)->value == 2.0);
    }

    {
        // set_cells (simple overwrite)
        mtv_type db(2);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));

        std::vector<muser_cell*> vals;
        vals.push_back(new muser_cell(3.0));
        vals.push_back(new muser_cell(4.0));
        db.set(0, vals.begin(), vals.end());
        assert(db.get<muser_cell*>(0)->value == 3.0);
        assert(db.get<muser_cell*>(1)->value == 4.0);
    }

    {
        // set_cells (overwrite upper)
        mtv_type db(2);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        double vals[] = { 3.0 };
        const double* p = &vals[0];
        db.set(0, p, p+1);
        assert(db.get<double>(0) == 3.0);
        assert(db.get<muser_cell*>(1)->value == 2.0);
    }

    {
        // set_cells (overwrite lower)
        mtv_type db(2);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        double vals[] = { 3.0 };
        const double* p = &vals[0];
        db.set(1, p, p+1);
        assert(db.get<muser_cell*>(0)->value == 1.0);
        assert(db.get<double>(1) == 3.0);
    }

    {
        // set_cells (overwrite middle)
        mtv_type db(4);
        db.set(0, 1.1);
        db.set(1, new muser_cell(1.0));
        db.set(2, new muser_cell(2.0));
        db.set(3, new muser_cell(3.0));
        double vals[] = { 4.0 };
        const double* p = &vals[0];
        db.set(2, p, p+1);
        assert(db.get<muser_cell*>(1)->value == 1.0);
        assert(db.get<double>(2) == 4.0);
        assert(db.get<muser_cell*>(3)->value == 3.0);
    }

    {
        // insert_empty() to split the block into two.
        mtv_type db(3);
        db.set(0, 1.1);
        db.set(1, new muser_cell(1.0));
        db.set(2, new muser_cell(2.0));
        db.insert_empty(2, 2);
        assert(db.size() == 5);
        assert(db.get<muser_cell*>(1)->value == 1.0);
        assert(db.get<muser_cell*>(4)->value == 2.0);
    }

    {
        // erase() to merge two blocks.
        mtv_type db(4);
        db.set(0, 1.1);
        db.set(1, new muser_cell(1.0));
        db.set(2, static_cast<unsigned long>(2));
        db.set(3, new muser_cell(3.0));
        assert(db.block_size() == 4);
        assert(db.size() == 4);

        db.erase(2, 2);
        assert(db.block_size() == 2);
        assert(db.size() == 3);
        assert(db.get<double>(0) == 1.1);
        assert(db.get<muser_cell*>(1)->value == 1.0);
        assert(db.get<muser_cell*>(2)->value == 3.0);
    }

    {
        // set_cells() across multiple blocks.
        mtv_type db(5);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, 1.2);
        db.set(3, new muser_cell(3.0));
        db.set(4, new muser_cell(4.0));
        unsigned long vals[] = { 5, 6, 7 };
        const unsigned long* p = &vals[0];
        db.set(1, p, p+3);
    }

    {
        // set_cells() across multiple blocks, part 2.
        mtv_type db(6);
        db.set(0, static_cast<unsigned long>(12));
        db.set(1, new muser_cell(1.0));
        db.set(2, new muser_cell(2.0));
        db.set(3, 1.2);
        db.set(4, new muser_cell(3.0));
        db.set(5, new muser_cell(4.0));
        assert(db.block_size() == 4);

        std::vector<muser_cell*> vals;
        vals.push_back(new muser_cell(5.0));
        vals.push_back(new muser_cell(6.0));
        vals.push_back(new muser_cell(7.0));
        db.set(2, vals.begin(), vals.end());
        assert(db.block_size() == 2);
    }

    {
        // set_cell() to merge 3 blocks.
        mtv_type db(6);
        db.set(0, static_cast<unsigned long>(12));
        db.set(1, new muser_cell(1.0));
        db.set(2, new muser_cell(2.0));
        db.set(3, 1.2);
        db.set(4, new muser_cell(3.0));
        db.set(5, new muser_cell(4.0));
        assert(db.block_size() == 4);
        assert(db.get<unsigned long>(0) == 12);
        assert(db.get<muser_cell*>(1)->value == 1.0);
        assert(db.get<muser_cell*>(2)->value == 2.0);
        assert(db.get<double>(3) == 1.2);
        assert(db.get<muser_cell*>(4)->value == 3.0);
        assert(db.get<muser_cell*>(5)->value == 4.0);

        db.set(3, new muser_cell(5.0)); // merge blocks.
        assert(db.block_size() == 2);
        assert(db.get<unsigned long>(0) == 12);
        assert(db.get<muser_cell*>(1)->value == 1.0);
        assert(db.get<muser_cell*>(2)->value == 2.0);
        assert(db.get<muser_cell*>(3)->value == 5.0);
        assert(db.get<muser_cell*>(4)->value == 3.0);
        assert(db.get<muser_cell*>(5)->value == 4.0);
    }

    {
        // set_cell() to merge 2 blocks.
        mtv_type db(3);
        db.set(0, static_cast<unsigned long>(23));
        db.set(1, new muser_cell(2.1));
        db.set(2, new muser_cell(3.1));

        db.set(0, new muser_cell(4.2)); // merge
        assert(db.block_size() == 1);
        assert(db.get<muser_cell*>(0)->value == 4.2);
        assert(db.get<muser_cell*>(1)->value == 2.1);
        assert(db.get<muser_cell*>(2)->value == 3.1);
    }

    {
        // insert_cells() to split block into two.
        mtv_type db(2);
        db.set(0, new muser_cell(2.1));
        db.set(1, new muser_cell(2.2));
        double vals[] = { 3.1, 3.2 };
        const double* p = &vals[0];
        db.insert(1, p, p+2);
    }

    {
        // set_cells() - merge new data block with existing block below.
        mtv_type db(6);
        db.set(0, string("foo"));
        db.set(1, string("baa"));
        db.set(2, 1.1);
        db.set(3, 1.2);
        db.set(4, new muser_cell(2.2));
        db.set(5, new muser_cell(2.3));
        assert(db.block_size() == 3);

        vector<muser_cell*> vals;
        vals.push_back(new muser_cell(2.4));
        vals.push_back(new muser_cell(2.5));
        vals.push_back(new muser_cell(2.6));
        db.set(1, vals.begin(), vals.end());
        assert(db.block_size() == 2);

        assert(db.get<string>(0) == "foo");
        assert(db.get<muser_cell*>(1)->value == 2.4);
        assert(db.get<muser_cell*>(2)->value == 2.5);
        assert(db.get<muser_cell*>(3)->value == 2.6);
        assert(db.get<muser_cell*>(4)->value == 2.2);
        assert(db.get<muser_cell*>(5)->value == 2.3);
    }

    {
        // set_cells() - merge new data block with existing block below, but
        // it overwrites the upper cell.
        mtv_type db(6);
        db.set(0, string("foo"));
        db.set(1, string("baa"));
        db.set(2, 1.1);
        db.set(3, new muser_cell(2.1));
        db.set(4, new muser_cell(2.2));
        db.set(5, new muser_cell(2.3));
        vector<muser_cell*> vals;
        vals.push_back(new muser_cell(2.4));
        vals.push_back(new muser_cell(2.5));
        vals.push_back(new muser_cell(2.6));
        db.set(1, vals.begin(), vals.end());
        assert(db.block_size() == 2);

        assert(db.get<string>(0) == "foo");
        assert(db.get<muser_cell*>(1)->value == 2.4);
        assert(db.get<muser_cell*>(2)->value == 2.5);
        assert(db.get<muser_cell*>(3)->value == 2.6);
        assert(db.get<muser_cell*>(4)->value == 2.2);
        assert(db.get<muser_cell*>(5)->value == 2.3);
    }

    {
        mtv_type db(3);
        db.set(0, new muser_cell(1.0));
        db.set(2, new muser_cell(1.0));
        db.set(1, new muser_cell(1.0));
        assert(db.block_size() == 1);
    }

    {
        mtv_type db(10);
        for (size_t i = 0; i < 10; ++i)
            db.set(i, new muser_cell(1.1));

        vector<double> doubles(3, 2.2);
        db.set(3, doubles.begin(), doubles.end());
        assert(db.block_size() == 3);

        vector<muser_cell*> cells;
        cells.push_back(new muser_cell(2.1));
        cells.push_back(new muser_cell(2.2));
        cells.push_back(new muser_cell(2.3));
        db.set(3, cells.begin(), cells.end());
        assert(db.block_size() == 1);
        assert(db.get<muser_cell*>(0)->value == 1.1);
        assert(db.get<muser_cell*>(1)->value == 1.1);
        assert(db.get<muser_cell*>(2)->value == 1.1);
        assert(db.get<muser_cell*>(3)->value == 2.1);
        assert(db.get<muser_cell*>(4)->value == 2.2);
        assert(db.get<muser_cell*>(5)->value == 2.3);
        assert(db.get<muser_cell*>(6)->value == 1.1);
        assert(db.get<muser_cell*>(7)->value == 1.1);
        assert(db.get<muser_cell*>(8)->value == 1.1);
        assert(db.get<muser_cell*>(9)->value == 1.1);
    }

    {
        mtv_type db(3);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));
        db.set_empty(1, 1);
        assert(db.block_size() == 3);
        assert(db.get<muser_cell*>(0)->value == 1.0);
        assert(db.is_empty(1));
        assert(db.get<muser_cell*>(2)->value == 3.0);
    }

    {
        mtv_type db(3);
        db.set(1, new muser_cell(3.3));
        assert(db.block_size() == 3);
        db.set_empty(1, 1);
        assert(db.block_size() == 1);
    }

    {
        // Release an element.
        mtv_type db(1);
        muser_cell* p = new muser_cell(4.5);
        db.set(0, p);
        muser_cell* p2 = db.release<muser_cell*>(0);
        assert(p == p2);
        assert(p2->value == 4.5);
        assert(db.is_empty(0));
        delete p2;

        db = mtv_type(2);
        db.set(0, new muser_cell(23.3));
        assert(db.block_size() == 2);
        p2 = db.release<muser_cell*>(0);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.block_size() == 1);
        delete p2;

        db = mtv_type(2);
        db.set(0, new muser_cell(1.2));
        db.set(1, new muser_cell(1.3));

        p2 = db.release<muser_cell*>(0);
        assert(db.is_empty(0));
        assert(!db.is_empty(1));
        assert(p2->value == 1.2);
        delete p2;

        db.set(0, new muser_cell(1.4));
        p2 = db.release<muser_cell*>(1);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(p2->value == 1.3);
        delete p2;

        db = mtv_type(3);
        db.set(0, new muser_cell(2.1));
        db.set(1, new muser_cell(2.2));
        db.set(2, new muser_cell(2.3));

        p2 = db.release<muser_cell*>(1);
        assert(p2->value == 2.2);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(!db.is_empty(2));

        delete p2;

        db = mtv_type(3);
        db.set(0, new muser_cell(2.1));
        db.set(1, new muser_cell(2.2));
        db.set(2, new muser_cell(2.3));
        db.set_empty(0, 2); // Make sure this doesn't release anything.
    }
}

void mtv_test_custom_block_func1()
{
    stack_printer __stack_printer__("::mtv_test_custom_block_func1");
    mtv_fruit_type db(10);
    db.set(0, apple);
    db.set(1, orange);
    db.set(2, mango);
    db.set(3, peach);
    assert(db.block_size() == 2);
    assert(db.get_type(0) == element_type_fruit_block);
    assert(db.get<my_fruit_type>(0) == apple);
    assert(db.get<my_fruit_type>(1) == orange);
    assert(db.get<my_fruit_type>(2) == mango);
    assert(db.get<my_fruit_type>(3) == peach);
    db.set<int>(1, 234);
    assert(db.block_size() == 4);
    db.set(1, apple);
    assert(db.block_size() == 2);
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
        mtv_test_basic();
        mtv_test_equality();
        mtv_test_managed_block();
        mtv_test_custom_block_func1();
    }

    if (opt.test_perf)
    {
    }

    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}
