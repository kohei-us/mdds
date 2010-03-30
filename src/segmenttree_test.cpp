/*************************************************************************
 *
 * Copyright (c) 2010 Kohei Yoshida
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

#include "segmenttree.hpp"

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>

#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])

using namespace std;
using namespace mdds;

#include <sys/time.h>

namespace {

class StackPrinter
{
public:
    explicit StackPrinter(const char* msg) :
        msMsg(msg)
    {
        fprintf(stdout, "%s: --begin\n", msMsg.c_str());
        mfStartTime = getTime();
    }

    ~StackPrinter()
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --end (duration: %g sec)\n", msMsg.c_str(), (fEndTime-mfStartTime));
    }

    void printTime(int line) const
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --(%d) (duration: %g sec)\n", msMsg.c_str(), line, (fEndTime-mfStartTime));
    }

private:
    double getTime() const
    {
        timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    ::std::string msMsg;
    double mfStartTime;
};

}

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
};

template<typename key_type>
struct test_segment
{
    key_type    begin_key;
    key_type    end_key;
    test_data*  data;

    test_segment(key_type _beg, key_type _end, test_data* p) :
        begin_key(_beg), end_key(_end), data(p) {}
};

template<typename key_type, typename data_type>
bool check_leaf_nodes(
    const segment_tree<key_type, data_type>& db, 
    const key_type* keys, data_type** data_chain, size_t key_size)
{
    typedef segment_tree<key_type, data_type> st_type;
    vector<typename st_type::leaf_node_check> checks;
    checks.reserve(key_size);
    size_t dcid = 0;
    for (size_t i = 0; i < key_size; ++i)
    {    
        typename st_type::leaf_node_check c;
        c.key = keys[i];
        data_type* p = data_chain[dcid];
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

template<typename key_type, typename data_type>
bool check_search_result(
    const segment_tree<key_type, data_type>& db, 
    key_type key, data_type** expected)
{
    typedef typename segment_tree<key_type, data_type>::data_chain_type data_chain_type;

    data_chain_type data_chain;
    db.search(key, data_chain);
    data_chain.sort(test_data::sort_by_name());

    size_t i = 0;
    data_type* p = expected[i++];
    typename data_chain_type::const_iterator itr = data_chain.begin(), itr_end = data_chain.end();
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

void st_test_insert_search_removal()
{
    StackPrinter __stack_printer__("::st_test_insert_segments");

    typedef long key_type;
    typedef test_data data_type;
    typedef segment_tree<key_type, data_type> db_type;

    db_type db;
    data_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");

    build_and_dump(db);
    assert(node_base::get_instance_count() == 0);

    db.insert(0, 10, &A);
    build_and_dump(db);
    {
        key_type keys[] = {0, 10};
        data_type* data_chain[] = {&A, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(node_base::get_instance_count() == 3);
        assert(db.verify_node_lists());
    }

    db.insert(0, 5, &B);
    build_and_dump(db);
    {
        key_type keys[] = {0, 5, 10};
        data_type* data_chain[] = {&A, &B, 0, &A, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(node_base::get_instance_count() == 6);
    }

    db.insert(5, 12, &C);
    build_and_dump(db);
    {
        key_type keys[] = {0, 5, 10, 12};
        data_type* data_chain[] = {&A, &B, 0, &A, &C, 0, &C, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(node_base::get_instance_count() == 7);
        assert(db.verify_node_lists());
    }

    db.insert(10, 24, &D);
    build_and_dump(db);
    {
        key_type keys[] = {0, 5, 10, 12, 24};
        data_type* data_chain[] = {&A, &B, 0, &A, &C, 0, &C, &D, 0, &D, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(node_base::get_instance_count() == 11);
        assert(db.verify_node_lists());
    }

    db.insert(4, 24, &E);
    build_and_dump(db);
    {
        key_type keys[] = {0, 4, 5, 10, 12, 24};
        data_type* data_chain[] = {&B, 0, &B, &E, 0, &A, &C, 0, &C, &D, 0, &D, &E, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(node_base::get_instance_count() == 12);
        assert(db.verify_node_lists());
    }

    db.insert(0, 26, &F);
    build_and_dump(db);
    {
        key_type keys[] = {0, 4, 5, 10, 12, 24, 26};
        data_type* data_chain[] = {&B, 0, &B, &E, 0, &A, &C, 0, &C, &D, 0, &D, &E, 0, &F, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(node_base::get_instance_count() == 14);
        assert(db.verify_node_lists());
    }

    db.insert(12, 26, &G);
    build_and_dump(db);
    {
        key_type keys[] = {0, 4, 5, 10, 12, 24, 26};
        data_type* data_chain[] = {&B, 0, &B, &E, 0, &A, &C, 0, &C, &D, 0, &D, &E, &G, 0, &F, &G, 0, 0};
        assert(check_leaf_nodes(db, keys, data_chain, ARRAY_SIZE(keys)));
        assert(node_base::get_instance_count() == 14);
        assert(db.verify_node_lists());
    }

    // Search tests.  Test boundary cases.

    for (key_type i = -10; i <= 30; ++i)
    {
        db_type::data_chain_type data_chain;
        db.search(i, data_chain);
        data_chain.sort(test_data::sort_by_name());
        cout << "search key " << i << ": ";
        for_each(data_chain.begin(), data_chain.end(), test_data::ptr_printer());
        cout << endl;
    }

    {
        key_type key = -1;
        data_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 0;
        data_type* expected[] = {&A, &B, &F, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 4;
        data_type* expected[] = {&A, &B, &E, &F, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 5;
        data_type* expected[] = {&A, &C, &E, &F, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 10;
        data_type* expected[] = {&C, &D, &E, &F, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 12;
        data_type* expected[] = {&D, &E, &G, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 24;
        data_type* expected[] = {&F, &G, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 30;
        data_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 9999;
        data_type* expected[] = {0};
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
        db_type::data_chain_type data_chain;
        db.search(i, data_chain);
        data_chain.sort(test_data::sort_by_name());
        cout << "search key " << i << ": ";
        for_each(data_chain.begin(), data_chain.end(), test_data::ptr_printer());
        cout << endl;
    }
    
    {
        key_type key = -1;
        data_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 0;
        data_type* expected[] = {&A, &B, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 4;
        data_type* expected[] = {&A, &B, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 5;
        data_type* expected[] = {&A, &C, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 10;
        data_type* expected[] = {&C, &D, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 12;
        data_type* expected[] = {&D, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 24;
        data_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 30;
        data_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 9999;
        data_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    // Re-build the tree and check the search results once again, to make sure
    // we get the same results.

    db.build_tree();
    db.dump_tree();
    db.dump_leaf_nodes();

    {
        key_type key = -1;
        data_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 0;
        data_type* expected[] = {&A, &B, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 4;
        data_type* expected[] = {&A, &B, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 5;
        data_type* expected[] = {&A, &C, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 10;
        data_type* expected[] = {&C, &D, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 12;
        data_type* expected[] = {&D, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 24;
        data_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 30;
        data_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }
}

void st_test_copy_constructor()
{
    StackPrinter __stack_printer__("::st_test_copy_constructor");

    typedef long key_type;
    typedef test_data data_type;
    typedef segment_tree<key_type, data_type> db_type;

    db_type db;
    data_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");
    test_segment<key_type> segments[] = {
        { 0, 10, &A},
        { 0,  5, &B},
        { 5, 12, &C},
        {10, 24, &D},
        { 4, 24, &E},
        { 0, 26, &F},
        {12, 26, &G},

        {0, 0, NULL} // null terminated
    };

    for (size_t i = 0; segments[i].data; ++i)
        db.insert(segments[i].begin_key, segments[i].end_key, segments[i].data);

    db.dump_segment_data();

    db_type db_copied(db);
    db_copied.dump_segment_data();
    assert(db.is_tree_valid() == db_copied.is_tree_valid());
}

int main()
{
    st_test_insert_search_removal();
    st_test_copy_constructor();
    assert(node_base::get_instance_count() == 0);
    fprintf(stdout, "Test finished successfully!\n");
    return EXIT_SUCCESS;
}

