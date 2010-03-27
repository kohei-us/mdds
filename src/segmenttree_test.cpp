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

template<typename key_type, typename value_type>
bool check_leaf_nodes(
    const segment_tree<key_type, value_type>& db, 
    const key_type* keys, size_t key_size)
{
    vector<key_type> keys_array;
    keys_array.reserve(key_size);
    for (size_t i = 0; i < key_size; ++i)
        keys_array.push_back(keys[i]);

    return db.verify_keys(keys_array);
}

struct test_data
{
    string name; // data structure expects the data to have 'name' data member.

    test_data(const string& s) : name(s) {}
};

void st_test_insert_segments()
{
    typedef long key_type;

    StackPrinter __stack_printer__("::st_test_insert_segments");
    segment_tree<key_type, test_data> db;
    test_data A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");
    build_and_dump(db);
    assert(node_base::get_instance_count() == 0);

    db.insert(0, 10, &A);
    build_and_dump(db);
    assert(node_base::get_instance_count() == 3);

    {
        key_type keys[] = {0, 10}; 
        assert(check_leaf_nodes(db, keys, ARRAY_SIZE(keys)));
    }
    return;
    db.insert(0, 5, &B);
    db.insert(5, 12, &C);
    db.insert(10, 24, &D);
    db.insert(4, 24, &E);
    db.insert(0, 26, &F);
    db.insert(12, 26, &G);
    build_and_dump(db);
}

int main()
{
    st_test_insert_segments();
    fprintf(stdout, "Test finished successfully!\n");
    return EXIT_SUCCESS;
}
