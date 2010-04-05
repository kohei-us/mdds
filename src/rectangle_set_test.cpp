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

#include "rectangle_set.hpp"

#include <iostream>

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

template<typename _ValueType>
struct range
{
    typedef _ValueType value_type;

    value_type x1;
    value_type y1;
    value_type x2;
    value_type y2;
    string name;

    range(value_type _x1, value_type _y1, value_type _x2, value_type _y2, const string& _name) : 
        x1(_x1), y1(_y1), x2(_x2), y2(_y2), name(_name) {}
};

template<typename _Key, typename _Data>
void insert_range(rectangle_set<_Key, _Data>& db, range<_Key>& range)
{
    db.insert(range.x1, range.y1, range.x2, range.y2, &range);
}

template<typename _SetType>
bool check_rectangles(const _SetType& db, const typename _SetType::data_type** expected)
{
    typename _SetType::dataset_type test;
    size_t i = 0;
    const typename _SetType::data_type* data = expected[i++];
    while (data)
    {
        typename _SetType::rectangle rect(data->x1, data->y1, data->x2, data->y2);
        test.insert(typename _SetType::dataset_type::value_type(data, rect));
        data = expected[i++];
    }

    return db.verify_rectangles(test);
}

void rect_test_insertion()
{
    typedef uint32_t value_type;
    typedef range<value_type> range_type;
    typedef rectangle_set<value_type, range_type> set_type;

    set_type db;
    range_type A(0, 0, 1,  1, "A");
    range_type B(2, 2, 5, 10, "B");
    range_type C(0, 1, 2,  2, "C");

    insert_range(db, A);
    insert_range(db, B);
    insert_range(db, C);
    db.dump_rectangles();

    {
        const set_type::data_type* expected[] = {&A, &B, &C, 0};
        assert(check_rectangles(db, expected));
    }
}

int main(int argc, char** argv)
{
    rect_test_insertion();
    fprintf(stdout, "Test finished successfully!\n");
}
