/*************************************************************************
 *
 * Copyright (c) 2012-2015 Kohei Yoshida
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

#include "test_global.hpp" // This must be the first header to be included.

#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#include <mdds/multi_type_vector.hpp>
#include <mdds/multi_type_vector_trait.hpp>
#include <mdds/multi_type_vector_custom_func1.hpp>
#include <mdds/multi_type_vector_custom_func2.hpp>
#include <mdds/multi_type_vector_custom_func3.hpp>

#include <cassert>
#include <memory>
#include <sstream>
#include <vector>

#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])

using namespace std;
using namespace mdds;

namespace {

/** custom cell type definition. */
const mtv::element_t element_type_user_block  = mtv::element_type_user_start;
const mtv::element_t element_type_muser_block = mtv::element_type_user_start+1;
const mtv::element_t element_type_fruit_block = mtv::element_type_user_start+2;
const mtv::element_t element_type_date_block  = mtv::element_type_user_start+3;

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
    ~muser_cell() {}
};

struct date
{
    int year;
    int month;
    int day;

    date() : year(0), month(0), day(0) {}
    date(int _year, int _month, int _day) : year(_year), month(_month), day(_day) {}
};

template<typename T>
class cell_pool
{
    std::vector<std::unique_ptr<T>> m_pool;
public:
    cell_pool() = default;
    cell_pool(const cell_pool&) = delete;
    cell_pool& operator=(const cell_pool&) = delete;

    T* construct()
    {
        m_pool.emplace_back(new T);
        return m_pool.back().get();
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
typedef mdds::mtv::default_element_block<element_type_date_block, date> date_block;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(user_cell, element_type_user_block, nullptr, user_cell_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(muser_cell, element_type_muser_block, nullptr, muser_cell_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(my_fruit_type, element_type_fruit_block, unknown_fruit, fruit_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(date, element_type_date_block, date(), date_block)

}

namespace {

typedef multi_type_vector<mtv::custom_block_func2<user_cell_block, muser_cell_block> > mtv_type;
typedef multi_type_vector<mtv::custom_block_func1<fruit_block> > mtv_fruit_type;
typedef multi_type_vector<
    mtv::custom_block_func3<muser_cell_block, fruit_block, date_block> > mtv3_type;

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
    assert(ct == mtv::element_type_double);
    ct = mtv_type::get_element_type(string());
    assert(ct == mtv::element_type_string);
    ct = mtv_type::get_element_type(static_cast<uint64_t>(12));
    assert(ct == mtv::element_type_uint64);
    ct = mtv_type::get_element_type(true);
    assert(ct == mtv::element_type_boolean);
    ct = mtv_type::get_element_type(false);
    assert(ct == mtv::element_type_boolean);

    // Custom cell type
    user_cell* p = nullptr;
    ct = mtv_type::get_element_type(p);
    assert(ct == element_type_user_block && ct >= mtv::element_type_user_start);
    ct = mtv_type::get_element_type(static_cast<muser_cell*>(nullptr));
    assert(ct == element_type_muser_block && ct >= mtv::element_type_user_start);
    ct = mtv_fruit_type::get_element_type(unknown_fruit);
    assert(ct == element_type_fruit_block && ct >= mtv::element_type_user_start);
}

void mtv_test_block_identifier()
{
    stack_printer __stack_printer__("::mtv_test_block_identifier");
    assert(user_cell_block::block_type == element_type_user_block);
    assert(muser_cell_block::block_type == element_type_muser_block);
    assert(fruit_block::block_type == element_type_fruit_block);
    assert(date_block::block_type == element_type_date_block);
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
        assert(p == nullptr);
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
        db.set(1, static_cast<uint64_t>(12));
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
        db.set(2, static_cast<uint64_t>(2));
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
        uint64_t vals[] = { 5, 6, 7 };
        const uint64_t* p = &vals[0];
        db.set(1, p, p+3);
    }

    {
        // set_cells() across multiple blocks, part 2.
        mtv_type db(6);
        db.set(0, static_cast<uint64_t>(12));
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
        db.set(0, static_cast<uint64_t>(12));
        db.set(1, new muser_cell(1.0));
        db.set(2, new muser_cell(2.0));
        db.set(3, 1.2);
        db.set(4, new muser_cell(3.0));
        db.set(5, new muser_cell(4.0));
        assert(db.block_size() == 4);
        assert(db.get<uint64_t>(0) == 12);
        assert(db.get<muser_cell*>(1)->value == 1.0);
        assert(db.get<muser_cell*>(2)->value == 2.0);
        assert(db.get<double>(3) == 1.2);
        assert(db.get<muser_cell*>(4)->value == 3.0);
        assert(db.get<muser_cell*>(5)->value == 4.0);

        db.set(3, new muser_cell(5.0)); // merge blocks.
        assert(db.block_size() == 2);
        assert(db.get<uint64_t>(0) == 12);
        assert(db.get<muser_cell*>(1)->value == 1.0);
        assert(db.get<muser_cell*>(2)->value == 2.0);
        assert(db.get<muser_cell*>(3)->value == 5.0);
        assert(db.get<muser_cell*>(4)->value == 3.0);
        assert(db.get<muser_cell*>(5)->value == 4.0);
    }

    {
        // set_cell() to merge 2 blocks.
        mtv_type db(3);
        db.set(0, static_cast<uint64_t>(23));
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
        muser_cell* p1 = new muser_cell(4.5);
        db.set(0, p1);
        muser_cell* p2 = db.release<muser_cell*>(0);
        assert(p1 == p2);
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

        // Release with position hint.
        db = mtv_type(4);
        db.set(0, new muser_cell(4.5));
        db.set(1, new muser_cell(4.6));
        db.set(3, new muser_cell(5.1));

        mtv_type::iterator pos = db.release(0, p1);
        assert(pos == db.begin());
        pos = db.release(pos, 3, p2);
        ++pos;
        assert(pos == db.end());
        assert(p1->value == 4.5);
        assert(p2->value == 5.1);
        assert(db.block_size() == 3);
        assert(db.is_empty(0));
        assert(db.get<muser_cell*>(1)->value == 4.6);
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        delete p1;
        delete p2;
    }

    {
        mtv_type db(5);

        db.set(1, new muser_cell(1.1));
        db.set(2, new muser_cell(1.2));
        db.set(3, new muser_cell(1.3));

        db.set(1, 2.1); // Don't leak the overwritten muser_cell instance.
        db.set(2, 2.2); // ditto
    }

    {
        mtv_type db(4);
        db.set(0, new muser_cell(1.1));
        db.set(1, new muser_cell(1.2));
        db.set(2, new muser_cell(1.3));

        db.set(2, 2.1); // Don't leak the overwritten muser_cell instance.
        db.set(1, 2.0); // ditto
    }

    {
        mtv_type db(8);
        db.set(3, new muser_cell(1.1));
        db.set(4, new muser_cell(1.2));
        db.set(5, 1.3);

        db.set(4, 2.2); // Overwrite muser_cell and don't leak.
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

void mtv_test_transfer()
{
    stack_printer __stack_printer__("::mtv_test_transfer");
    mtv_type db1(3), db2(4); // db2 is larger than db1.
    db1.set(0, new muser_cell(1.1));
    db1.set(1, new muser_cell(1.2));
    db1.set(2, new muser_cell(1.3));
    assert(db1.block_size() == 1);

    try
    {
        db1.transfer(0, 1, db1, 0);
        assert(!"Exception should have been thrown");
    }
    catch (const invalid_arg_error&)
    {
        // Good.
    }

    // Do the transfer.
    db1.transfer(0, 2, db2, 0);

    // Now db1 should be totally empty.
    assert(db1.block_size() == 1);
    mtv_type::iterator check = db1.begin();
    assert(check != db1.end());
    assert(check->type == mtv::element_type_empty);
    assert(check->size == 3);

    assert(db2.block_size() == 2);
    assert(db2.get<muser_cell*>(0)->value == 1.1);
    assert(db2.get<muser_cell*>(1)->value == 1.2);
    assert(db2.get<muser_cell*>(2)->value == 1.3);
    assert(db2.is_empty(3));

    // Transfer back to db1. This should make db2 to be totally empty again.
    db2.transfer(0, 2, db1, 0);
    assert(db2.block_size() == 1);
    check = db2.begin();
    assert(check != db2.end());
    assert(check->size == 4);
    assert(check->type == mtv::element_type_empty);

    assert(db1.block_size() == 1);
    assert(db1.get<muser_cell*>(0)->value == 1.1);
    assert(db1.get<muser_cell*>(1)->value == 1.2);
    assert(db1.get<muser_cell*>(2)->value == 1.3);

    // Now, transfer only the top 2 elements.
    db1.transfer(0, 1, db2, 0);
    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.get<muser_cell*>(2)->value == 1.3);

    assert(db2.get<muser_cell*>(0)->value == 1.1);
    assert(db2.get<muser_cell*>(1)->value == 1.2);
    assert(db2.is_empty(2));
    assert(db2.is_empty(3));

    // .. and back.
    db2.transfer(0, 1, db1, 0);
    assert(db1.block_size() == 1);
    assert(db1.get<muser_cell*>(0)->value == 1.1);
    assert(db1.get<muser_cell*>(1)->value == 1.2);
    assert(db1.get<muser_cell*>(2)->value == 1.3);

    assert(db2.block_size() == 1);
    check = db2.begin();
    assert(check != db2.end());
    assert(check->size == 4);
    assert(check->type == mtv::element_type_empty);

    db1 = mtv_type(4);
    db2 = mtv_type(4);
    db2.set(1, new muser_cell(2.1));
    db2.set(2, new muser_cell(2.2));
    assert(db2.block_size() == 3);
    db1.transfer(0, 1, db2, 1); // This causes db2's 3 blocks to merge into one.
    assert(db2.block_size() == 1);
    check = db2.begin();
    assert(check != db2.end());
    assert(check->size == 4);
    assert(check->type == mtv::element_type_empty);
    assert(db1.block_size() == 1);
    check = db1.begin();
    assert(check != db1.end());
    assert(check->size == 4);
    assert(check->type == mtv::element_type_empty);

    db2.set(0, new muser_cell(3.1));
    assert(db2.block_size() == 2);

    db1.set(1, new muser_cell(3.2));
    db1.set(2, new muser_cell(3.3));
    db1.set(3, new muser_cell(3.4));
    assert(db1.block_size() == 2);

    db1.transfer(1, 2, db2, 1);
    assert(db1.block_size() == 2);
    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.get<muser_cell*>(3)->value == 3.4);
    assert(db2.block_size() == 2);
    assert(db2.get<muser_cell*>(0)->value == 3.1);
    assert(db2.get<muser_cell*>(1)->value == 3.2);
    assert(db2.get<muser_cell*>(2)->value == 3.3);
    assert(db2.is_empty(3));

    db1 = mtv_type(3);
    db2 = mtv_type(3);
    db1.set(1, new muser_cell(4.2));
    db2.set(0, new muser_cell(4.1));
    db2.set(2, new muser_cell(4.3));

    db1.transfer(1, 1, db2, 1);
    assert(db1.block_size() == 1);
    check = db1.begin();
    assert(check != db1.end());
    assert(check->size == 3);
    assert(check->type == mtv::element_type_empty);
    assert(db2.block_size() == 1);
    assert(db2.get<muser_cell*>(0)->value == 4.1);
    assert(db2.get<muser_cell*>(1)->value == 4.2);
    assert(db2.get<muser_cell*>(2)->value == 4.3);

    // Transfer to middle of block.
    db1 = mtv_type(3);
    db2 = mtv_type(3);
    db1.set(0, new muser_cell(5.2));
    assert(db1.block_size() == 2);
    db1.transfer(0, 0, db2, 1);
    assert(db1.block_size() == 1);
    check = db1.begin();
    assert(check != db1.end());
    assert(check->size == 3);
    assert(check->type == mtv::element_type_empty);
    assert(db2.block_size() == 3);
    assert(db2.is_empty(0));
    assert(db2.get<muser_cell*>(1)->value == 5.2);
    assert(db2.is_empty(2));

    db1 = mtv_type(2);
    db2 = mtv_type(3);
    db1.set(0, new muser_cell(11.1));
    db1.set(1, new muser_cell(11.2));
    db1.transfer(1, 1, db2, 1);
    assert(db1.block_size() == 2);
    assert(db1.get<muser_cell*>(0)->value == 11.1);
    assert(db1.is_empty(1));
    assert(db2.block_size() == 3);
    assert(db2.is_empty(0));
    assert(db2.get<muser_cell*>(1)->value == 11.2);
    assert(db2.is_empty(2));

    // Transfer to bottom of block.
    db1 = mtv_type(4);
    db2 = mtv_type(5);
    db1.set(0, new muser_cell(6.1));
    db1.set(1, new muser_cell(6.2));
    db1.transfer(0, 1, db2, 3);
    assert(db1.block_size() == 1);
    check = db1.begin();
    assert(check != db1.end());
    assert(check->size == 4);
    assert(check->type == mtv::element_type_empty);
    assert(db2.block_size() == 2);
    assert(db2.is_empty(0));
    assert(db2.is_empty(1));
    assert(db2.is_empty(2));
    assert(db2.get<muser_cell*>(3)->value == 6.1);
    assert(db2.get<muser_cell*>(4)->value == 6.2);

    // Transfer multiple blocks.  Very simple use case.
    db1 = mtv_type(4);
    db2 = mtv_type(3);
    db1.set(1, new muser_cell(10.1));
    db1.set(3, new muser_cell(10.2));
    db1.transfer(1, 3, db2, 0);

    // db1 should be completely empty.
    assert(db1.block_size() == 1);
    check = db1.begin();
    assert(check != db1.end());
    assert(check->size == 4);
    assert(check->type == mtv::element_type_empty);

    assert(db2.block_size() == 3);
    assert(db2.get<muser_cell*>(0)->value == 10.1);
    assert(db2.is_empty(1));
    assert(db2.get<muser_cell*>(2)->value == 10.2);

    // Multiple-block transfer that involves merging.
    db1 = mtv_type(5);
    db2 = mtv_type(5);
    db1.set(0, new muser_cell(0.1));
    db1.set(1, new muser_cell(0.2));
    db1.set(3, new muser_cell(0.3));
    db1.set(4, new muser_cell(0.4));

    db2.set(0, new muser_cell(1.1));
    db2.set(4, new muser_cell(1.2));

    mtv_type::iterator it = db1.transfer(1, 3, db2, 1);
    assert(db1.block_size() == 3);
    assert(db1.get<muser_cell*>(0)->value == 0.1);
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.is_empty(3));
    assert(db1.get<muser_cell*>(4)->value == 0.4);

    assert(db2.block_size() == 3);
    assert(db2.get<muser_cell*>(0)->value == 1.1);
    assert(db2.get<muser_cell*>(1)->value == 0.2);
    assert(db2.is_empty(2));
    assert(db2.get<muser_cell*>(3)->value == 0.3);
    assert(db2.get<muser_cell*>(4)->value == 1.2);

    assert(it != db1.end());
    assert(it->size == 3);
    assert(it->type == mtv::element_type_empty);
    it = db1.transfer(it, 4, 4, db2, 2); // Transfer single element at 4.
    assert(db1.block_size() == 2);
    assert(db1.get<muser_cell*>(0)->value == 0.1);
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.is_empty(3));
    assert(db1.is_empty(4));

    assert(db2.block_size() == 1);
    assert(db2.get<muser_cell*>(0)->value == 1.1);
    assert(db2.get<muser_cell*>(1)->value == 0.2);
    assert(db2.get<muser_cell*>(2)->value == 0.4);
    assert(db2.get<muser_cell*>(3)->value == 0.3);
    assert(db2.get<muser_cell*>(4)->value == 1.2);

    assert(it != db1.end());
    assert(it->size == 4);
    assert(it->type == mtv::element_type_empty);
    ++it;
    assert(it == db1.end());

    // Multi-block transfer to the top part of destination block.
    db1 = mtv_type(5);
    db2 = mtv_type(5);
    db1.set(0, new muser_cell(-1.1));
    db1.set(1, new muser_cell(-2.1));
    db1.set(2, new muser_cell(-3.1));
    db1.set(3, string("foo"));
    db1.set(4, new muser_cell(-5.1));
    db2.set(1, true);
    db2.set(2, false);
    db2.set(3, true);
    it = db1.transfer(2, 3, db2, 2);
    assert(it != db1.end());
    assert(it->size == 2);
    assert(it->type == mtv::element_type_empty);
    std::advance(it, 2);
    assert(it == db1.end());
    assert(db1.block_size() == 3);
    assert(db1.get<muser_cell*>(0)->value == -1.1);
    assert(db1.get<muser_cell*>(1)->value == -2.1);
    assert(db1.is_empty(2));
    assert(db1.is_empty(3));
    assert(db1.get<muser_cell*>(4)->value == -5.1);

    assert(db2.block_size() == 5);
    assert(db2.is_empty(0));
    assert(db2.get<bool>(1) == true);
    assert(db2.get<muser_cell*>(2)->value == -3.1);
    assert(db2.get<string>(3) == "foo");
    assert(db2.is_empty(4));

    // Multi-block transfer to the bottom part of destination block.
    db1 = mtv_type(10);
    db2 = mtv_type(10);
    db1.set(0, new muser_cell(2.1));
    db1.set(1, new muser_cell(2.2));
    db1.set(2, int8_t('a'));
    db1.set(3, int8_t('b'));
    db2.set(0, true);
    db2.set(1, false);

    it = db1.transfer(0, 2, db2, 7);
    assert(it != db1.end());
    assert(it->size == 3);
    assert(it->type == mtv::element_type_empty);
    ++it;
    assert(it != db1.end());
    assert(it->size == 1);
    assert(it->type == mtv::element_type_int8);
    ++it;
    assert(it != db1.end());
    assert(it->size == 6);
    assert(it->type == mtv::element_type_empty);
    ++it;
    assert(it == db1.end());
    assert(db1.block_size() == 3);
    assert(db1.get<int8_t>(3) == 'b');

    assert(db2.block_size() == 4);
    assert(db2.get<bool>(0) == true);
    assert(db2.get<bool>(1) == false);
    assert(db2.is_empty(2));
    assert(db2.is_empty(3));
    assert(db2.is_empty(4));
    assert(db2.is_empty(5));
    assert(db2.is_empty(6));
    assert(db2.get<muser_cell*>(7)->value == 2.1);
    assert(db2.get<muser_cell*>(8)->value == 2.2);
    assert(db2.get<int8_t>(9) == 'a');

    // Multi-block transfer to the middle part of destination block.
    db1 = mtv_type(10);
    db2 = mtv_type(10);
    db2.set(0, true);
    db2.set(9, true);
    db1.set(3, new muser_cell(2.4));
    db1.set(4, new muser_cell(2.5));
    db1.set(5, string("abc"));
    db1.set(6, new muser_cell(2.6));
    db1.set(7, new muser_cell(2.7));
    db1.set(8, true);
    it = db1.transfer(3, 6, db2, 2);
    assert(it != db1.end());
    assert(it->size == 7);
    assert(it->type == mtv::element_type_empty);
    ++it;
    assert(it != db1.end());
    assert(it->size == 1);
    assert(it->type == element_type_muser_block);
    ++it;
    assert(it->size == 1);
    assert(it->type == mtv::element_type_boolean);
    ++it;
    assert(it->size == 1);
    assert(it->type == mtv::element_type_empty);
    ++it;
    assert(it == db1.end());

    assert(db1.block_size() == 4);
    assert(db1.is_empty(6));
    assert(db1.get<muser_cell*>(7)->value == 2.7);
    assert(db1.get<bool>(8) == true);
    assert(db1.is_empty(9));

    assert(db2.block_size() == 7);
    assert(db2.get<bool>(0) == true);
    assert(db2.is_empty(1));
    assert(db2.get<muser_cell*>(2)->value == 2.4);
    assert(db2.get<muser_cell*>(3)->value == 2.5);
    assert(db2.get<string>(4) == "abc");
    assert(db2.get<muser_cell*>(5)->value == 2.6);
    assert(db2.is_empty(6));
    assert(db2.is_empty(7));
    assert(db2.is_empty(8));
    assert(db2.get<bool>(9) == true);

    db1 = mtv_type(10);
    db2 = mtv_type(10);
    db1.set(3, true);
    db2.set(3, string("test"));
    db1.transfer(0, 6, db2, 0);
    assert(db1.block_size() == 1);
    check = db1.begin();
    assert(check != db1.end());
    assert(check->size == 10);
    assert(check->type == mtv::element_type_empty);
    ++check;
    assert(check == db1.end());
    assert(db2.block_size() == 3);
    assert(db2.is_empty(0));
    assert(db2.is_empty(1));
    assert(db2.is_empty(2));
    assert(db2.get<bool>(3) == true);
    assert(db2.is_empty(4));
    assert(db2.is_empty(5));
    assert(db2.is_empty(6));
    assert(db2.is_empty(7));
    assert(db2.is_empty(8));
    assert(db2.is_empty(9));

    // Make sure that transfer will overwrite cells in managed blocks.
    db1 = mtv_type(3);
    db2 = mtv_type(3);
    db1.set(0, new muser_cell(1.1));
    db1.set(1, new muser_cell(1.2));
    db1.set(2, new muser_cell(1.3));

    db2.set(1, new muser_cell(2.1)); // This element will be overwritten.
    db1.transfer(0, 2, db2, 0);
    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db2.get<muser_cell*>(0)->value == 1.1);
    assert(db2.get<muser_cell*>(1)->value == 1.2);
    assert(db2.get<muser_cell*>(2)->value == 1.3);
}

void mtv_test_swap()
{
    stack_printer __stack_printer__("::mtv_test_swap");
    mtv3_type db1(5), db2(2);
    db1.set(0, new muser_cell(1.1));
    db1.set(1, new muser_cell(1.2));
    db1.set(2, new muser_cell(1.3));
    db1.set(3, new muser_cell(1.4));
    db1.set(4, new muser_cell(1.5));

    db2.set(0, string("A"));
    db2.set(1, string("B"));

    db1.swap(2, 3, db2, 0);

    // swap blocks of equal size, one managed, and one default.

    db1.clear();
    db1.resize(10);
    db2.clear();
    db2.resize(10);

    db1.set(3, 2.1);
    db1.set(4, 2.2);
    db1.set(5, 2.3);

    db2.set(3, new muser_cell(3.1));
    db2.set(4, new muser_cell(3.2));
    db2.set(5, new muser_cell(3.3));

    db2.swap(3, 5, db1, 3);

    assert(db1.size() == 10);
    assert(db1.block_size() == 3);
    assert(db2.size() == 10);
    assert(db2.block_size() == 3);

    assert(db1.get<muser_cell*>(3)->value == 3.1);
    assert(db1.get<muser_cell*>(4)->value == 3.2);
    assert(db1.get<muser_cell*>(5)->value == 3.3);
    assert(db2.get<double>(3) == 2.1);
    assert(db2.get<double>(4) == 2.2);
    assert(db2.get<double>(5) == 2.3);

    db2.swap(3, 5, db1, 3);

    assert(db1.get<double>(3) == 2.1);
    assert(db1.get<double>(4) == 2.2);
    assert(db1.get<double>(5) == 2.3);
    assert(db2.get<muser_cell*>(3)->value == 3.1);
    assert(db2.get<muser_cell*>(4)->value == 3.2);
    assert(db2.get<muser_cell*>(5)->value == 3.3);

    // Same as above, except that the source segment splits the block into 2.

    db1.clear();
    db1.resize(10);
    db2.clear();
    db2.resize(10);

    db1.set(3, 2.1);
    db1.set(4, 2.2);

    db2.set(3, new muser_cell(3.1));
    db2.set(4, new muser_cell(3.2));
    db2.set(5, new muser_cell(3.3));

    db2.swap(3, 4, db1, 3);

    // Another scenario that used to crash on double delete.

    db1.clear();
    db1.resize(10);
    db2.clear();
    db2.resize(10);

    db1.set(2, new muser_cell(4.1));
    db1.set(3, 4.2);
    db1.set(4, new muser_cell(4.3));

    db2.set(3, new muser_cell(6.1));
    db2.set(4, 6.2);
    db2.set(5, 6.3);

    assert(db1.get<muser_cell*>(2)->value == 4.1);
    assert(db1.get<double>(3) == 4.2);
    assert(db1.get<muser_cell*>(4)->value == 4.3);

    assert(db2.get<muser_cell*>(3)->value == 6.1);
    assert(db2.get<double>(4) == 6.2);
    assert(db2.get<double>(5) == 6.3);

    db2.swap(4, 4, db1, 4);

    assert(db1.get<muser_cell*>(2)->value == 4.1);
    assert(db1.get<double>(3) == 4.2);
    assert(db1.get<double>(4) == 6.2);

    assert(db2.get<muser_cell*>(3)->value == 6.1);
    assert(db2.get<muser_cell*>(4)->value == 4.3);
    assert(db2.get<double>(5) == 6.3);

    // One more on double deletion...

    db1.clear();
    db1.resize(10);
    db2.clear();
    db2.resize(10);

    db1.set(0, 2.1);
    db1.set(1, 2.2);
    db1.set(2, 2.3);
    db1.set(3, new muser_cell(4.5));

    db2.set(2, new muser_cell(3.1));
    db2.set(3, new muser_cell(3.2));
    db2.set(4, new muser_cell(3.3));

    db1.swap(2, 2, db2, 3);

    assert(db1.get<double>(0) == 2.1);
    assert(db1.get<double>(1) == 2.2);
    assert(db1.get<muser_cell*>(2)->value == 3.2);
    assert(db1.get<muser_cell*>(3)->value == 4.5);

    assert(db2.get<muser_cell*>(2)->value == 3.1);
    assert(db2.get<double>(3) == 2.3);
    assert(db2.get<muser_cell*>(4)->value == 3.3);

    assert(db1.check_block_integrity());
    assert(db2.check_block_integrity());
}

void mtv_test_swap_2()
{
    stack_printer __stack_printer__("::mtv_test_swap_2");
    mtv3_type db1(3), db2(3);

    db1.set(0, new muser_cell(1.1));
    db1.set(1, new muser_cell(1.2));

    db2.set(0, 1.2);
    db2.set(1, std::string("foo"));

    // Repeat the same swap twice.
    db1.swap(0, 1, db2, 0);
    assert(db1.check_block_integrity());
    assert(db2.check_block_integrity());
    assert(db2.get<muser_cell*>(0)->value == 1.1);
    assert(db2.get<muser_cell*>(1)->value == 1.2);
    assert(db1.get<double>(0) == 1.2);
    assert(db1.get<std::string>(1) == "foo");

    db1.swap(0, 1, db2, 0);
    assert(db1.check_block_integrity());
    assert(db2.check_block_integrity());
    assert(db1.get<muser_cell*>(0)->value == 1.1);
    assert(db1.get<muser_cell*>(1)->value == 1.2);
    assert(db2.get<double>(0) == 1.2);
    assert(db2.get<std::string>(1) == "foo");
}

void mtv_test_custom_block_func3()
{
    stack_printer __stack_printer__("::mtv_test_custom_block_func3");
    mtv3_type db(10);

    // Insert custom elements.
    db.set(0, new muser_cell(12.3));
    db.set(1, apple);
    db.set(2, date(1989,12,13));
    db.set(3, date(2011,8,7));
    assert(db.get_type(0) == element_type_muser_block);
    assert(db.get_type(1) == element_type_fruit_block);
    assert(db.get_type(2) == element_type_date_block);
    assert(db.get_type(3) == element_type_date_block);
    assert(db.get<muser_cell*>(0)->value == 12.3);
    assert(db.get<my_fruit_type>(1) == apple);
    assert(db.get<date>(2).year == 1989);
    assert(db.get<date>(2).month == 12);
    assert(db.get<date>(2).day == 13);
    assert(db.get<date>(3).year == 2011);
    assert(db.get<date>(3).month == 8);
    assert(db.get<date>(3).day == 7);
    assert(db.block_size() == 4);

    // We should still support the primitive types.
    db.set(8, 34.56);
    assert(db.get<double>(8) == 34.56);
}

void mtv_test_release()
{
    stack_printer __stack_printer__("::mtv_test_release");
    mtv_type db(3);
    muser_cell c1(1.2), c2(1.3); // allocated on the stack.
    db.set(0, &c1);
    db.set(2, &c2);
    db.release(); // Prevent invalid free when db goes out of scope.

    // Variant with no argument should make the container empty after the call.
    assert(db.empty());

    db.push_back(new muser_cell(10.0));
    db.push_back(&c1);
    db.push_back(&c2);
    db.push_back(new muser_cell(10.1));

    assert(db.size() == 4);
    assert(db.block_size() == 1);

    // Release those allocated on the stack to avoid double deletion.
    mtv_type::iterator it = db.release_range(1, 2);

    // Check the integrity of the returned iterator.
    assert(it->type == mtv::element_type_empty);
    assert(it->size == 2);
    mtv_type::iterator check = it;
    --check;
    assert(check == db.begin());
    check = it;
    ++check;
    ++check;
    assert(check == db.end());

    db.push_back(new muser_cell(10.2));
    assert(db.size() == 5);

    muser_cell* p1 = db.get<muser_cell*>(3);
    muser_cell* p2 = db.get<muser_cell*>(4);
    assert(p1->value == 10.1);
    assert(p2->value == 10.2);

    // Pass iterator as a position hint.
    it = db.release_range(it, 3, 4);
    assert(db.block_size() == 2);

    // Check the returned iterator.
    check = it;
    --check;
    assert(check == db.begin());
    assert(check->type == element_type_muser_block);
    assert(check->size == 1);
    assert(it->type == mtv::element_type_empty);
    assert(it->size == 4);
    ++it;
    assert(it == db.end());

    // It should be safe to delete these instances now that they've been released.
    delete p1;
    delete p2;
}

void mtv_test_construction_with_array()
{
    stack_printer __stack_printer__("::mtv_test_construction_with_array");
    {
        std::vector<muser_cell*> vals;
        vals.push_back(new muser_cell(2.1));
        vals.push_back(new muser_cell(2.2));
        vals.push_back(new muser_cell(2.3));
        mtv_type db(vals.size(), vals.begin(), vals.end());

        db.set(1, 10.2); // overwrite.
        assert(db.size() == 3);
        assert(db.block_size() == 3);
        assert(db.get<muser_cell*>(0)->value == 2.1);
        assert(db.get<double>(1) == 10.2);
        assert(db.get<muser_cell*>(2)->value == 2.3);

        // Now those heap objects are owned by the container.  Clearing the
        // array shouldn't leak.
        vals.clear();
    }
}

}

int main (int argc, char **argv)
{
    try
    {
        mtv_test_types();
        mtv_test_block_identifier();
        mtv_test_basic();
        mtv_test_equality();
        mtv_test_managed_block();
        mtv_test_custom_block_func1();
        mtv_test_transfer();
        mtv_test_swap();
        mtv_test_swap_2();
        mtv_test_custom_block_func3();
        mtv_test_release();
        mtv_test_construction_with_array();
    }
    catch (const std::exception& e)
    {
        cout << "Test failed: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}
