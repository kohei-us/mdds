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

template<typename _DbType>
bool verify_stored_data(
    const vector<typename _DbType::node_data>& stored_data, const vector<typename _DbType::node_data>& verify_data)
{
    typedef vector<typename _DbType::node_data> node_array_type;

    node_array_type v1 = stored_data, v2 = verify_data;
    sort(v1.begin(), v1.end(), typename _DbType::node_data::sorter());
    sort(v2.begin(), v2.end(), typename _DbType::node_data::sorter());

    typename node_array_type::const_iterator 
        itr1 = v1.begin(), itr1_end = v1.end(), itr2 = v2.begin(), itr2_end = v2.end();

    for (; itr1 != itr1_end; ++itr1, ++itr2)
    {
        if (itr2 == itr2_end)
            return false;

        if (*itr1 != *itr2)
            return false;
    }
    if (itr2 != itr2_end)
        return false;

    return true;
}

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
    db.dump_tree_svg("./obj/test-remove.svg");
}

void pqt_test_insertion()
{
    StackPrinter __stack_printer__("::pqt_test_insertion");
    typedef point_quad_tree<int32_t, string> db_type;
    db_type db;
    assert(db.empty());
    assert(db.size() == 0);
    ptr_vector<string> data_store;
    data_store.reserve(100);
    for (size_t i = 0; i < 100; ++i)
    {
        ostringstream os;
        os << hex << i;
        data_store.push_back(new string(os.str()));
    }

    vector<db_type::node_data> verify_data;

    for (size_t i = 0; i < 10; ++i)
    {
        for (size_t j = 0; j < 10; ++j)
        {
            size_t index = i*10 + j;
            string* data_ptr = &data_store[index];
            cout << "inserting '" << *data_ptr << "' at (" << i << "," << j << ")" << endl;
            db.insert(i, j, data_ptr);
            verify_data.push_back(db_type::node_data(i, j, data_ptr));

            vector<db_type::node_data> stored_data;
            db.get_all_stored_data(stored_data);
            assert(stored_data.size() == (index+1));
            assert(db.size() == (index+1));
            assert(!db.empty());
            bool success = verify_stored_data<db_type>(stored_data, verify_data);
            assert(success);
        }
    }
}

int main()
{
    pqt_test_basic();
    pqt_test_insertion();
    assert(get_node_instance_count() == 0);
}
