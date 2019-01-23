/*************************************************************************
 *
 * Copyright (c) 2010, 2011 Kohei Yoshida
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
#include "mdds/segment_tree.hpp"

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])

using namespace std;
using namespace mdds;

template<typename key_type, typename value_type>
void build_and_dump(segment_tree<key_type, value_type>&db)
{
    cout << "build and dump (start) -----------------------------------------" << endl;
    db.build_tree();
    db.dump_tree();
    db.dump_leaf_nodes();
    cout << "build and dump (end) -------------------------------------------" << endl;
}

struct test_data
{
    string name; // data structure expects the data to have 'name' data member.

    test_data(const string& s) : name(s) {}

    struct ptr_printer : public unary_function<test_data*, void>
    {
        void operator() (const test_data* data) const
        {
            cout << data->name << " ";
        }
    };

    /**
     * Use this to sort instances of test_data by name, in ascending order.
     */
    struct sort_by_name : public binary_function<test_data*, test_data*, bool>
    {
        bool operator() (const test_data* left, const test_data* right) const
        {
            return left->name < right->name;
        }
    };

    struct name_printer : public unary_function<test_data*, void>
    {
        void operator() (const test_data* p) const
        {
            cout << p->name << " ";
        }
    };
};

template<typename key_type, typename value_type>
bool check_leaf_nodes(
    const segment_tree<key_type, value_type>& db,
    const key_type* keys, value_type* data_chain, size_t key_size)
{
    typedef segment_tree<key_type, value_type> st_type;
    vector<typename st_type::leaf_node_check> checks;
    checks.reserve(key_size);
    size_t dcid = 0;
    for (size_t i = 0; i < key_size; ++i)
    {
        typename st_type::leaf_node_check c;
        c.key = keys[i];
        value_type p = data_chain[dcid];
        while (p)
        {
            c.data_chain.push_back(p);
            p = data_chain[++dcid];
        }
        checks.push_back(c);
        ++dcid;
    }

    return db.verify_leaf_nodes(checks);
}

template<typename value_type>
bool check_against_expected(const list<value_type>& test, value_type* expected)
{
    size_t i = 0;
    value_type p = expected[i++];
    typename list<value_type>::const_iterator itr = test.begin(), itr_end = test.end();
    while (p)
    {
        if (itr == itr_end)
            // data chain ended prematurely.
            return false;

        if (*itr != p)
            // the value is not as expected.
            return false;

        p = expected[i++];
        ++itr;
    }
    if (itr != itr_end)
        // data chain is too long.
        return false;

    return true;
}

/**
 * Only check the search result against expected result set.  The caller
 * needs to run search and pass the result to this function.
 */
template<typename key_type, typename value_type>
bool check_search_result_only(
    const segment_tree<key_type, value_type>& db,
    const typename segment_tree<key_type, value_type>::search_result_type& result,
    key_type key, value_type* expected)
{
    cout << "search key: " << key << " ";

    list<value_type> test;
    copy(result.begin(), result.end(), back_inserter(test));
    test.sort(test_data::sort_by_name());

    cout << "search result (sorted): ";
    for_each(test.begin(), test.end(), test_data::name_printer());
    cout << endl;

    return check_against_expected(test, expected);
}

/**
 * Run the search and check the search result.
 */
template<typename key_type, typename value_type>
bool check_search_result(
    const segment_tree<key_type, value_type>& db,
    key_type key, value_type* expected)
{
    cout << "search key: " << key << " ";

    typedef typename segment_tree<key_type, value_type>::search_result_type search_result_type;
    search_result_type data_chain;
    db.search(key, data_chain);
    return check_search_result_only(db, data_chain, key, expected);
}

template<typename key_type, typename value_type>
bool check_search_result_iterator(
    const segment_tree<key_type, value_type>& db,
    key_type key, value_type* expected)
{
    cout << "search key: " << key << " ";

    typedef segment_tree<key_type, value_type> db_type;
    typename db_type::search_result result = db.search(key);
    list<value_type> test;
    copy(result.begin(), result.end(), back_inserter(test));
    test.sort(test_data::sort_by_name());

    cout << "search result (sorted): ";
    for_each(test.begin(), test.end(), test_data::name_printer());
    cout << endl;

    return check_against_expected(test, expected);
}

void st_test_insert_search_removal()
{
    stack_printer __stack_printer__("::st_test_insert_segments");

    typedef long key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    db_type db;
    value_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");

    build_and_dump(db);
    assert(db_type::node::get_instance_count() == 0);

    db.insert(0, 10, &A);
    build_and_dump(db);
    {
        key_type keys[] = {0, 10};
        value_type* data_chain[] = {&A, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(db_type::node::get_instance_count() == db.leaf_size());
        assert(db.verify_node_lists());
    }

    db.insert(0, 5, &B);
    build_and_dump(db);
    {
        key_type keys[] = {0, 5, 10};
        value_type* data_chain[] = {&A, &B, 0, &A, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(db_type::node::get_instance_count() == db.leaf_size());
    }

    db.insert(5, 12, &C);
    build_and_dump(db);
    {
        key_type keys[] = {0, 5, 10, 12};
        value_type* data_chain[] = {&A, &B, 0, &A, &C, 0, &C, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(db_type::node::get_instance_count() == db.leaf_size());
        assert(db.verify_node_lists());
    }

    db.insert(10, 24, &D);
    build_and_dump(db);
    {
        key_type keys[] = {0, 5, 10, 12, 24};
        value_type* data_chain[] = {&A, &B, 0, &A, &C, 0, &C, &D, 0, &D, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(db_type::node::get_instance_count() == db.leaf_size());
        assert(db.verify_node_lists());
    }

    db.insert(4, 24, &E);
    build_and_dump(db);
    {
        key_type keys[] = {0, 4, 5, 10, 12, 24};
        value_type* data_chain[] = {&B, 0, &B, &E, 0, &A, &C, 0, &C, &D, 0, &D, &E, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(db_type::node::get_instance_count() == db.leaf_size());
        assert(db.verify_node_lists());
    }

    db.insert(0, 26, &F);
    build_and_dump(db);
    {
        key_type keys[] = {0, 4, 5, 10, 12, 24, 26};
        value_type* data_chain[] = {&B, 0, &B, &E, 0, &A, &C, 0, &C, &D, 0, &D, &E, &F, 0, &F, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(db_type::node::get_instance_count() == db.leaf_size());
        assert(db.verify_node_lists());
    }

    db.insert(12, 26, &G);
    build_and_dump(db);
    {
        key_type keys[] = {0, 4, 5, 10, 12, 24, 26};
        value_type* data_chain[] = {&B, 0, &B, &E, 0, &A, &C, 0, &C, &D, 0, &D, &E, &F, &G, 0, &F, &G, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(db_type::node::get_instance_count() == db.leaf_size());
        assert(db.verify_node_lists());
    }

    // Search tests.  Test boundary cases.

    for (key_type i = -10; i <= 30; ++i)
    {
        db_type::search_result_type data_chain;
        db.search(i, data_chain);
        cout << "search key " << i << ": ";
        for_each(data_chain.begin(), data_chain.end(), test_data::ptr_printer());
        cout << endl;
    }

    {
        key_type key = -1;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 0;
        value_type* expected[] = {&A, &B, &F, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 4;
        value_type* expected[] = {&A, &B, &E, &F, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 5;
        value_type* expected[] = {&A, &C, &E, &F, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 10;
        value_type* expected[] = {&C, &D, &E, &F, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 12;
        value_type* expected[] = {&D, &E, &F, &G, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 24;
        value_type* expected[] = {&F, &G, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 30;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 9999;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    // Remove E, F and G and check search results.

    db.remove(&E);
    db.remove(&F);
    db.remove(&G);
    cout << "removed: E F G" << endl;
    db.dump_tree();
    db.dump_leaf_nodes();

    for (key_type i = -10; i <= 30; ++i)
    {
        db_type::search_result_type data_chain;
        db.search(i, data_chain);
        cout << "search key " << i << ": ";
        for_each(data_chain.begin(), data_chain.end(), test_data::ptr_printer());
        cout << endl;
    }

    {
        key_type key = -1;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 0;
        value_type* expected[] = {&A, &B, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 4;
        value_type* expected[] = {&A, &B, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 5;
        value_type* expected[] = {&A, &C, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 10;
        value_type* expected[] = {&C, &D, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 12;
        value_type* expected[] = {&D, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 24;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 30;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 9999;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    // Re-build the tree and check the search results once again, to make sure
    // we get the same results.

    db.build_tree();
    db.dump_tree();
    db.dump_leaf_nodes();

    {
        key_type key = -1;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 0;
        value_type* expected[] = {&A, &B, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 4;
        value_type* expected[] = {&A, &B, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 5;
        value_type* expected[] = {&A, &C, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 10;
        value_type* expected[] = {&C, &D, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 12;
        value_type* expected[] = {&D, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 24;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 30;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }
}

void st_test_copy_constructor()
{
    stack_printer __stack_printer__("::st_test_copy_constructor");

    typedef long key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    db_type db;
    value_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");
    vector<db_type::segment_data> segments;
    segments.push_back(db_type::segment_data( 0, 10, &A));
    segments.push_back(db_type::segment_data( 0,  5, &B));
    segments.push_back(db_type::segment_data( 5, 12, &C));
    segments.push_back(db_type::segment_data(10, 24, &D));
    segments.push_back(db_type::segment_data( 4, 24, &E));
    segments.push_back(db_type::segment_data( 0, 26, &F));
    segments.push_back(db_type::segment_data(12, 26, &G));
    segments.push_back(db_type::segment_data(0, 0, nullptr)); // null-terminated

    db_type::segment_map_type checks;
    for (size_t i = 0; segments[i].pdata; ++i)
    {
        db.insert(segments[i].begin_key, segments[i].end_key, segments[i].pdata);
        pair<key_type, key_type> range;
        range.first = segments[i].begin_key;
        range.second = segments[i].end_key;
        checks.insert(db_type::segment_map_type::value_type(segments[i].pdata, range));
    }

    // Copy before the tree is built.

    db.dump_segment_data();
    assert(db.verify_segment_data(checks));

    db_type db_copied(db);
    db_copied.dump_segment_data();
    assert(db_copied.verify_segment_data(checks));
    assert(db.is_tree_valid() == db_copied.is_tree_valid());
    assert(db == db_copied);

    // Copy after the tree is built.
    db.build_tree();
    db_type db_copied_tree(db);
    db_copied_tree.dump_segment_data();
    db_copied_tree.dump_tree();
    assert(db_copied_tree.verify_segment_data(checks));
    assert(db.is_tree_valid() == db_copied_tree.is_tree_valid());
    assert(db == db_copied_tree);
}

void st_test_equality()
{
    stack_printer __stack_printer__("::st_test_equality");

    typedef uint32_t key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    value_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");
    {
        db_type db1, db2;
        db1.insert(0, 10, &A);
        db2.insert(0, 10, &A);
        assert(db1 == db2);
        db2.insert(5, 12, &B);
        assert(db1 != db2);
        db1.insert(5, 12, &C);
        assert(db1 != db2);
        db1.remove(&C);
        db2.remove(&B);
        assert(db1 == db2);
        db1.insert(4, 20, &D);
        db2.insert(4, 20, &D);
        assert(db1 == db2);
        db1.insert(3, 12, &E);
        db2.insert(3, 15, &E);
        assert(db1 != db2);
    }
}

void st_test_clear()
{
    stack_printer __stack_printer__("::st_test_clear");

    typedef uint8_t key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    value_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");

    vector<db_type::segment_data> segments;
    segments.push_back(db_type::segment_data( 0, 10, &A));
    segments.push_back(db_type::segment_data( 0,  5, &B));
    segments.push_back(db_type::segment_data( 5, 12, &C));
    segments.push_back(db_type::segment_data(10, 24, &D));
    segments.push_back(db_type::segment_data( 4, 24, &E));
    segments.push_back(db_type::segment_data( 0, 26, &F));
    segments.push_back(db_type::segment_data(12, 26, &G));
    segments.push_back(db_type::segment_data(0, 0, nullptr)); // null-terminated

    db_type db;
    for (size_t i = 0; segments[i].pdata; ++i)
        db.insert(segments[i].begin_key, segments[i].end_key, segments[i].pdata);

    assert(!db.empty());
    assert(db.size() == 7);
    cout << "size of db is " << db.size() << endl;

    db.clear();
    assert(db.empty());
    assert(db.size() == 0);

    // Insert the same data set once again, but this time build tree afterwards.
    for (size_t i = 0; segments[i].pdata; ++i)
        db.insert(segments[i].begin_key, segments[i].end_key, segments[i].pdata);

    db.build_tree();
    assert(!db.empty());
    assert(db.size() == 7);

    db.clear();
    assert(db.empty());
    assert(db.size() == 0);
}

void st_test_duplicate_insertion()
{
    stack_printer __stack_printer__("::st_test_duplicate_insertion");

    typedef short key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    value_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");

    db_type db;
    assert( db.insert(0, 10, &A));
    assert(!db.insert(0, 10, &A));
    assert(!db.insert(2, 30, &A));
    assert( db.insert(0, 10, &B));
    db.remove(&A);
    assert( db.insert(2, 30, &A));
    build_and_dump(db);
}

/**
 * When the number of segments is not a multiple of 2, it creates a tree
 * where the right side becomes "cut off".  Make sure the search works
 * correctly under those conditions.
 */
void st_test_search_on_uneven_tree()
{
    stack_printer __stack_printer__("::st_test_search_on_uneven_tree");

    typedef int16_t key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    for (key_type data_count = 10; data_count < 20; ++data_count)
    {
        vector<unique_ptr<test_data>> data_store;
        data_store.reserve(data_count);
        for (key_type i = 0; i < data_count; ++i)
        {
            ostringstream os;
            os << hex << showbase << i;
            data_store.emplace_back(new test_data(os.str()));
        }
        assert(data_store.size() == static_cast<size_t>(data_count));

        db_type db;
        for (key_type i = 0; i < data_count; ++i)
        {
            test_data* p = data_store[i].get();
            db.insert(0, i+1, p);
        }
        assert(db.size() == static_cast<size_t>(data_count));

        db.build_tree();

        for (key_type i = -1; i < data_count+1; ++i)
        {
            db_type::search_result_type result;
            bool success = db.search(i, result);
            assert(success);
            cout << "search key: " << i << "  result: ";
            for_each(result.begin(), result.end(), test_data::name_printer());
            cout << endl;
        }
    }
}

void st_test_perf_insertion()
{
    stack_printer __stack_printer__("::st_test_perf_insertion");

    typedef uint32_t key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    key_type data_count = 1000000;

    // First, create test data instances and store them into a vector.
    vector<unique_ptr<test_data>> data_store;
    {
        stack_printer __stack_printer2__("::st_test_perf_insertion:: data array creation");
        data_store.reserve(data_count);
        for (key_type i = 0; i < data_count; ++i)
        {
            ostringstream os;
            os << hex << i;
            data_store.emplace_back(new test_data(os.str()));
        }
    }
    assert(data_store.size() == data_count);

    db_type db;
    {
        stack_printer __stack_printer2__("::st_test_perf_insertion:: data array insertion into segment tree");
        for (key_type i = 0; i < data_count; ++i)
        {
            test_data* p = data_store[i].get();
            db.insert(0, i+1, p);
        }
    }
    assert(db.size() == data_count);

    {
        stack_printer __stack_printer2__("::st_test_perf_insertion:: build tree");
        db.build_tree();
    }
    assert(db.is_tree_valid());

    const test_data* test = nullptr;
    {
        stack_printer __stack_printer2__("::st_test_perf_insertion:: 200 searches with max results");
        for (key_type i = 0; i < 200; ++i)
        {
            db_type::search_result_type result;
            db.search(0, result);
            db_type::search_result_type::const_iterator itr = result.begin(), itr_end = result.end();
            for (; itr != itr_end; ++itr)
            {
                test = *itr;
                assert(test);
            }
        }
    }

    {
        stack_printer __stack_printer2__("::st_test_perf_insertion:: 200 searches with max results (iterator)");
        for (key_type i = 0; i < 200; ++i)
        {
            db_type::search_result result = db.search(0);
            db_type::search_result::iterator itr = result.begin(), itr_end = result.end();
            for (; itr != itr_end; ++itr)
            {
                test = *itr;
                assert(test);
            }
        }
    }

    {
        stack_printer __stack_printer2__("::st_test_perf_insertion:: 200 searches with median results");
        for (key_type i = 0; i < 200; ++i)
        {
            db_type::search_result_type result;
            db.search(data_count/2, result);
            db_type::search_result_type::const_iterator itr = result.begin(), itr_end = result.end();
            for (; itr != itr_end; ++itr)
            {
                test = *itr;
                assert(test);
            }
        }
    }

    {
        stack_printer __stack_printer2__("::st_test_perf_insertion:: 200 searches with median results (iterator)");
        for (key_type i = 0; i < 200; ++i)
        {
            db_type::search_result result = db.search(data_count/2);
            db_type::search_result::iterator itr = result.begin(), itr_end = result.end();
            for (; itr != itr_end; ++itr)
            {
                test = *itr;
                assert(test);
            }
        }
    }

    {
        stack_printer __stack_printer2__("::st_test_perf_insertion:: 200 searches with empty results");
        for (key_type i = 0; i < 200; ++i)
        {
            db_type::search_result_type result;
            db.search(data_count, result);
            db_type::search_result_type::const_iterator itr = result.begin(), itr_end = result.end();
            for (; itr != itr_end; ++itr)
            {
                test = *itr;
                assert(test);
            }
        }
    }

    {
        stack_printer __stack_printer2__("::st_test_perf_insertion:: 200 searches with empty results (iterator)");
        for (key_type i = 0; i < 200; ++i)
        {
            db_type::search_result result = db.search(data_count);
            db_type::search_result::iterator itr = result.begin(), itr_end = result.end();
            for (; itr != itr_end; ++itr)
            {
                test = *itr;
                assert(test);
            }
        }
    }

    {
        stack_printer __stack_printer2__("::st_test_perf_insertion:: 10000 segment removals");
        for (key_type i = 0; i < 10000; ++i)
        {
            test_data* p = data_store[i].get();
            db.remove(p);
        }
    }
    assert(db.size() == data_count - 10000);

    {
        stack_printer __stack_printer2__("::st_test_perf_insertion:: clear");
        db.clear();
    }
}

void st_test_aggregated_search_results()
{
    stack_printer __stack_printer__("::st_test_aggregated_search_results");

    typedef uint16_t key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    value_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");

    vector<db_type::segment_data> segments;
    segments.push_back(db_type::segment_data( 0, 10, &A));
    segments.push_back(db_type::segment_data( 0,  5, &B));
    segments.push_back(db_type::segment_data( 5, 12, &C));
    segments.push_back(db_type::segment_data(10, 24, &D));
    segments.push_back(db_type::segment_data( 4, 24, &E));
    segments.push_back(db_type::segment_data( 0, 26, &F));
    segments.push_back(db_type::segment_data(12, 26, &G));
    segments.push_back(db_type::segment_data(0, 0, nullptr)); // null-terminated

    db_type db;
    for (size_t i = 0; segments[i].pdata; ++i)
        db.insert(segments[i].begin_key, segments[i].end_key, segments[i].pdata);

    db.dump_segment_data();
    db.build_tree();

    db_type::search_result_type result;
    {
        key_type key = 0;
        db.search(key, result);
        value_type* expected[] = {&A, &B, &F, 0};
        assert(check_search_result_only(db, result, key, expected));
    }

    {
        key_type key = 10;
        db.search(key, result);
        // Note the duplicated F's in the search result.
        value_type* expected[] = {&A, &B, &C, &D, &E, &F, &F, 0};
        assert(check_search_result_only(db, result, key, expected));
    }

    {
        key_type key = 5;
        db.search(key, result);
        value_type* expected[] = {&A, &A, &B, &C, &C, &D, &E, &E, &F, &F, &F, 0};
        assert(check_search_result_only(db, result, key, expected));
    }

    {
        result.clear(); // clear the accumulated result set.
        key_type key = 5;
        db.search(key, result);
        value_type* expected[] = {&A, &C, &E, &F, 0};
        assert(check_search_result_only(db, result, key, expected));
    }
}

void st_test_dense_tree_search()
{
    stack_printer __stack_printer__("::st_test_dense_tree_search");

    typedef uint16_t key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    value_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");
    db_type db;
    db.insert(0, 1, &A);
    db.insert(0, 2, &B);
    db.insert(0, 3, &C);
    db.insert(0, 4, &D);
    db.insert(0, 5, &E);
    db.insert(0, 6, &F);
    db.insert(0, 7, &G);
    db.build_tree();
    db.dump_tree();
    db.dump_leaf_nodes();

    {
        db_type::value_type expected[] = {&A, &B, &C, &D, &E, &F, &G, 0};
        bool success = check_search_result<key_type, value_type*>(db, 0, expected);
        assert(success);
    }
    {
        db_type::value_type expected[] = {&B, &C, &D, &E, &F, &G, 0};
        bool success = check_search_result<key_type, value_type*>(db, 1, expected);
        assert(success);
    }
    {
        db_type::value_type expected[] = {&C, &D, &E, &F, &G, 0};
        bool success = check_search_result<key_type, value_type*>(db, 2, expected);
        assert(success);
    }
    {
        db_type::value_type expected[] = {&D, &E, &F, &G, 0};
        bool success = check_search_result<key_type, value_type*>(db, 3, expected);
        assert(success);
    }
    {
        db_type::value_type expected[] = {&E, &F, &G, 0};
        bool success = check_search_result<key_type, value_type*>(db, 4, expected);
        assert(success);
    }
    {
        db_type::value_type expected[] = {&F, &G, 0};
        bool success = check_search_result<key_type, value_type*>(db, 5, expected);
        assert(success);
    }
    {
        db_type::value_type expected[] = {&G, 0};
        bool success = check_search_result<key_type, value_type*>(db, 6, expected);
        assert(success);
    }
    {
        db_type::value_type expected[] = {0};
        bool success = check_search_result<key_type, value_type*>(db, 7, expected);
        assert(success);
    }
}

void st_test_search_on_empty_set()
{
    stack_printer __stack_printer__("::st_test_search_on_empty_set");

    typedef uint16_t key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    db_type db;
    db.build_tree();

    // Search on an empty set should still be considered a success as long as
    // the tree is built beforehand.
    db_type::search_result_type result;
    bool success = db.search(0, result);
    assert(success);
    assert(result.empty());
}

void st_test_search_iterator_basic()
{
    stack_printer __stack_printer__("::st_test_search_iterator");
    typedef uint16_t key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    value_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");
    db_type db;
    db.insert(0, 1, &A);
    db.insert(0, 2, &B);
    db.insert(0, 3, &C);
    db.insert(0, 4, &D);
    db.insert(0, 5, &E);
    db.insert(0, 6, &F);
    db.insert(0, 7, &G);
    db.build_tree();
    db.dump_tree();
    db.dump_leaf_nodes();

    db_type::search_result result = db.search(0);
    db_type::search_result::iterator itr;
    db_type::search_result::iterator itr_beg = result.begin();
    db_type::search_result::iterator itr_end = result.end();
    cout << "Iterate through the search results." << endl;
    for (itr = itr_beg; itr != itr_end; ++itr)
        cout << (*itr)->name << " ";
    cout << endl;

    cout << "Do it again." << endl;
    for (itr = itr_beg; itr != itr_end; ++itr)
        cout << (*itr)->name << " ";
    cout << endl;

    cout << "Iterate backwards" << endl;
    do
    {
        --itr;
        cout << (*itr)->name << " ";
    }
    while (itr != itr_beg);
    cout << endl;

    cout << "Get the last item from the end position." << endl;
    itr = itr_end;
    --itr;
    cout << (*itr)->name << endl;

    cout << "Use for_each to print names." << endl;
    for_each(itr_beg, itr_end, test_data::ptr_printer());
    cout << endl;
}

void st_test_search_iterator_result_check()
{
    stack_printer __stack_printer__("::st_test_search_iterator_result_check");

    typedef uint16_t key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    value_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");
    db_type db;
    db.insert(0, 1, &A);
    db.insert(0, 2, &B);
    db.insert(0, 3, &C);
    db.insert(0, 4, &D);
    db.insert(0, 5, &E);
    db.insert(0, 6, &F);
    db.insert(0, 7, &G);
    db.build_tree();

    {
        value_type* expected[] = {&A, &B, &C, &D, &E, &F, &G, 0};
        bool success = check_search_result_iterator<key_type, value_type*>(db, 0, expected);
        assert(success);
    }
    {
        value_type* expected[] = {&B, &C, &D, &E, &F, &G, 0};
        bool success = check_search_result_iterator<key_type, value_type*>(db, 1, expected);
        assert(success);
    }
    {
        value_type* expected[] = {&C, &D, &E, &F, &G, 0};
        bool success = check_search_result_iterator<key_type, value_type*>(db, 2, expected);
        assert(success);
    }
    {
        value_type* expected[] = {&D, &E, &F, &G, 0};
        bool success = check_search_result_iterator<key_type, value_type*>(db, 3, expected);
        assert(success);
    }
    {
        value_type* expected[] = {&E, &F, &G, 0};
        bool success = check_search_result_iterator<key_type, value_type*>(db, 4, expected);
        assert(success);
    }
    {
        value_type* expected[] = {&F, &G, 0};
        bool success = check_search_result_iterator<key_type, value_type*>(db, 5, expected);
        assert(success);
    }
    {
        value_type* expected[] = {&G, 0};
        bool success = check_search_result_iterator<key_type, value_type*>(db, 6, expected);
        assert(success);
    }
    {
        value_type* expected[] = {0};
        bool success = check_search_result_iterator<key_type, value_type*>(db, 7, expected);
        assert(success);
    }
}

/**
 * When calling search() on empty tree, even without calling build_tree()
 * should still return a valid search_result instance with a size of 0.
 */
void st_test_empty_result_set()
{
    stack_printer __stack_printer__("::st_test_empty_result_set");
    typedef segment_tree<long, string*> db_type;
    db_type db;
    db_type::search_result result = db.search(0);
    cout << "size of empty result set: " << result.size() << endl;
    assert(result.size() == 0);
}

void st_test_non_pointer_data()
{
    stack_printer __stack_printer__("::st_test_non_pointer_data");

    typedef uint16_t key_type;
    typedef size_t value_type;
    typedef segment_tree<key_type, value_type> db_type;

    db_type db;
    db.insert(0, 1, 10);
    db.build_tree();

    db_type::search_result result = db.search(0);
    assert(result.size() == 1);
    assert(*result.begin() == 10);
}

int main(int argc, char** argv)
{
    try
    {
        cmd_options opt;
        if (!parse_cmd_options(argc, argv, opt))
            return EXIT_FAILURE;

        if (opt.test_func)
        {
            st_test_insert_search_removal();
            st_test_copy_constructor();
            st_test_equality();
            st_test_clear();
            st_test_duplicate_insertion();
            st_test_search_on_uneven_tree();
            st_test_aggregated_search_results();
            st_test_dense_tree_search();
            st_test_search_on_empty_set();
            st_test_search_iterator_basic();
            st_test_search_iterator_result_check();
            st_test_empty_result_set();
            st_test_non_pointer_data();
        }

        if (opt.test_perf)
        {
            st_test_perf_insertion();
        }

        // At this point, all of the nodes created during the test run should have
        // been destroyed.  If not, we are leaking memory.
        typedef segment_tree<uint32_t, void*> db_type;
        assert(db_type::node::get_instance_count() == 0);
    }
    catch (const std::exception& e)
    {
        fprintf(stdout, "Test failed: %s\n", e.what());
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Test finished successfully!\n");
    return EXIT_SUCCESS;
}

