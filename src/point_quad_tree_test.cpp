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

#include "mdds/point_quad_tree.hpp"

#include <cstdint>
#include <algorithm>
#include <vector>
#include <sstream>
#include <boost/ptr_container/ptr_vector.hpp>

#include <stdio.h>
#include <string>
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

using namespace std;
using namespace mdds;
using ::boost::ptr_vector;

struct data_printer : public unary_function<string*, void>
{
    void operator() (const string* p)
    {
        cout << *p << " ";
    }
};

template<typename _DbType>
struct search_result_printer : public unary_function<pair<typename _DbType::point, typename _DbType::data_type*>, void>
{
    void operator() (const pair<const typename _DbType::point, const typename _DbType::data_type*>& r) const
    {
        cout << "  (x=" << r.first.x << ", y=" << r.first.y << ", value='" << *r.second << "')" << endl;
    }
};

void pqt_test_basic()
{
    StackPrinter __stack_printer__("::pqt_test");
    typedef point_quad_tree<uint16_t, string> db_type;
    db_type db;

    string A("A");
    string B("B");
    string C("C");
    string D("D");
    string E("E");
    string F("F");
    string G("G");
    string H("H");
    string I("I");
    string J("J");
    string K("K");
    string L("L");
    string M("M");
    string N("N");

    db.insert(25, 32, &A);
    db.insert( 5, 45, &B);
    db.insert(52, 10, &C);
    db.insert(80,  5, &D);
    db.insert(40, 50, &E);
    db.insert(10, 10, &F);
    db.insert(20, 20, &G);
    db.insert(80, 80, &H);
    db.insert(58, 46, &I);
    db.insert(36, 55, &J);
    db.insert(26, 52, &K);
    db.insert(38, 68, &L);
    db.insert(39, 78, &M);
    db.insert(72, 52, &N);

    assert(db.size() == 14);

    cout << "node count = " << get_node_instance_count() << endl;
    db.dump_tree_svg("./obj/test.svg");

    {
        db_type::data_array_type result;
        db.search_region(10, 10, 60, 20, result);
        cout << "search region: (10, 10, 60, 20)" << endl;
        cout << "result: ";
        for_each(result.begin(), result.end(), data_printer());
        cout << endl;
    
        result.clear();
        db.search_region(10, 10, 61, 61, result);
        cout << "search region: (10, 10, 61, 61)" << endl;
        cout << "result: ";
        for_each(result.begin(), result.end(), data_printer());
        cout << endl;
    }

    db_type::search_result result = db.search_region(10, 10, 60, 20);
    db_type::search_result::const_iterator itr = result.begin(), itr_end = result.end();
    cout << "result: " << endl;
    for_each(result.begin(), result.end(), search_result_printer<db_type>());

    result = db.search_region(10, 10, 61, 61);
    itr = result.begin(), itr_end = result.end();
    cout << "result: " << endl;
    for_each(result.begin(), result.end(), search_result_printer<db_type>());

    db.remove(20, 20);
    db.remove(40, 50);
    assert(db.size() == 12);
    db.dump_tree_svg("./obj/test-remove.svg");

    db.clear();
    assert(db.empty());
    assert(db.size() == 0);
}

void pqt_test_insertion()
{
    StackPrinter __stack_printer__("::pqt_test_insertion");
    typedef point_quad_tree<int32_t, string> db_type;
    db_type db;

    // Check its empty-ness...
    assert(db.empty());
    assert(db.size() == 0);

    // Create all data instances.
    ptr_vector<string> data_store;
    data_store.reserve(100);
    for (size_t i = 0; i < 100; ++i)
    {
        ostringstream os;
        os << "0x" << hex << i;
        data_store.push_back(new string(os.str()));
    }

    vector<db_type::node_data> expected;

    // Insert data one by one, and verify each insertion.
    for (int32_t i = 0; i < 10; ++i)
    {
        for (int32_t j = 0; j < 10; ++j)
        {
            int32_t x = i*10 + 1, y = j*10 + 1;
            size_t index = i*10 + j;
            string* data_ptr = &data_store[index];
            cout << "inserting '" << *data_ptr << "' at (" << x << "," << y << ")" << endl;
            db.insert(x, y, data_ptr);
            expected.push_back(db_type::node_data(x, y, data_ptr));

            vector<db_type::node_data> stored_data;
            db.get_all_stored_data(stored_data);
            assert(stored_data.size() == (index+1));
            assert(db.size() == (index+1));
            assert(!db.empty());
            bool success = db.verify_data(expected);
            assert(success);
        }
    }
    db.dump_tree_svg("./obj/pqt_test_insertion.svg");

    // Remove data one by one, and check the size after each removal.
    size_t node_count = 100;
    for (int32_t i = 0; i < 10; ++i)
    {
        for (int32_t j = 0; j < 10; ++j)
        {
            int32_t x = i*10 + 1, y = j*10 + 1;
            cout << "removing node at (" << x << "," << y << ")" << endl;
            db.remove(x, y);
            size_t n = db.size();
            cout << "size after removal: " << n << endl;
            assert(--node_count == n);
        }
    }
}

void pqt_test_remove_root()
{
    StackPrinter __stack_printer__("::pqt_test_remove_root");    
    typedef point_quad_tree<int32_t, string> db_type;
    string O("O");
    string NW("NW");
    string NE("NE");
    string SW("SW");
    string SE("SE");
    db_type db;

    // Insert all data and verify their storage.
    db.insert(10, 10, &O);
    db.insert(20, 0, &NE);
    db.insert(0, 0, &NW);
    db.insert(20, 20, &SE);
    db.insert(0, 20, &SW);
    db.dump_tree_svg("./obj/pqt_test_remove_root-1.svg");

    vector<db_type::node_data> expected;
    expected.push_back(db_type::node_data(10, 10, &O));
    expected.push_back(db_type::node_data(20, 0, &NE));
    expected.push_back(db_type::node_data(0, 0, &NW));
    expected.push_back(db_type::node_data(20, 20, &SE));
    expected.push_back(db_type::node_data(0, 20, &SW));
    bool success = db.verify_data(expected);
    assert(success);
    assert(db.size() == 5);

    // Now, remove the root node.
    db.remove(10, 10);
    db.dump_tree_svg("./obj/pqt_test_remove_root-2.svg");
    expected.clear();
    expected.push_back(db_type::node_data(20, 0, &NE));
    expected.push_back(db_type::node_data(0, 0, &NW));
    expected.push_back(db_type::node_data(20, 20, &SE));
    expected.push_back(db_type::node_data(0, 20, &SW));
    success = db.verify_data(expected);
    assert(success);
    assert(db.size() == 4);
}

void pqt_test_equality()
{
    StackPrinter __stack_printer__("::pqt_test_equality");

    typedef point_quad_tree<int32_t, string> db_type;
    db_type db1, db2;

    string A("A");
    string B("B");
    string C("C");
    string D("D");
    string E("E");
    string F("F");

    assert(db1 == db2); // both are empty.
    
    db1.insert(0, 0, &A);
    db2.insert(0, 0, &A);
    assert(db1 == db2);
    db1.remove(0, 0);
    assert(db1 != db2);
    db1.insert(0, 0, &B);
    assert(db1 != db2);
    db2.insert(0, 0, &B); // B overwrites A.
    assert(db1 == db2); // Both should have B at (0,0).
    db1.insert(1, 1, &C);
    db2.insert(2, 2, &C);
    assert(db1 != db2);
    db1.insert(2, 2, &C);
    db2.insert(1, 1, &C);
    assert(db1 == db2);

    // Inserting data in different orders should make no difference in equality.
    db1.insert(1, 3, &D);
    db1.insert(1, 4, &E);
    db1.insert(1, 5, &F);

    db2.insert(1, 5, &F);
    db2.insert(1, 4, &E);
    db2.insert(1, 3, &D);
    assert(db1 == db2);
    db1.remove(1, 4);
    db2.remove(1, 4);
    assert(db1 == db2);

    // Make them empty again.
    db1.clear();
    db2.clear();
    assert(db1 == db2);
}

void pqt_test_assignment()
{
    StackPrinter __stack_printer__("::pqt_test_assignment");
    typedef point_quad_tree<int32_t, string> db_type;
    db_type db1, db2;
    string A("A");
    string B("B");
    string C("C");
    string D("D");
    string E("E");
    string F("F");

    db1.insert(0, 10, &A);
    db1.insert(2, 5, &B);
    db1.insert(-10, 2, &C);
    db1.insert(5, 7, &D);
    vector<db_type::node_data> expected;
    expected.push_back(db_type::node_data(0, 10, &A));
    expected.push_back(db_type::node_data(2, 5, &B));
    expected.push_back(db_type::node_data(-10, 2, &C));
    expected.push_back(db_type::node_data(5, 7, &D));
    bool success = db1.verify_data(expected);
    assert(success);

    db2 = db1;
    success = db2.verify_data(expected);
    assert(success);
    success = db1.verify_data(expected);
    assert(success);

    db2.insert(12, 45, &E);
    db2.insert(20, 42, &F);
    success = db2.verify_data(expected); // This should fail.
    assert(!success);
    db2 = db1; // Assign once again.
    success = db2.verify_data(expected); // This now should succeed.
    assert(success);
}

int main()
{
    pqt_test_basic();
    pqt_test_insertion();
    pqt_test_remove_root();
    pqt_test_equality();
    pqt_test_assignment();
    assert(get_node_instance_count() == 0);
    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}
