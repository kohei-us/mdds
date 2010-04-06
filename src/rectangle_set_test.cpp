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

    struct printer : public unary_function<range, void>
    {
        void operator() (const range* p) const
        {
            cout << p->name << ": (x1,y1,x2,y2) = (" << p->x1 << "," << p->y1 
                << "," << p->x2 << "," << p->y2 << ")" << endl;
        }
    };

    struct sort_by_name : public binary_function<range, range, void>
    {
        bool operator() (const range* left, const range* right) const
        {
            return left->name < right->name;
        }
    };
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

template<typename _SetType>
bool check_size(const _SetType& db, size_t size_expected)
{
    if (db.size() != size_expected)
        return false;

    if (db.empty() != (size_expected == 0))
        return false;

    return true;
}

template<typename _SetType>
void print_search_result(typename _SetType::key_type x, typename _SetType::key_type y, const typename _SetType::search_result_type& result)
{
    cout << "search result --------------------------------------------------" << endl;
    cout << "(x,y) = (" << x << "," << y << ")" << endl;
    for_each(result.begin(), result.end(), typename _SetType::data_type::printer());
}

template<typename _SetType>
bool check_search_result(_SetType& db, 
                         typename _SetType::key_type x, typename _SetType::key_type y, 
                         const typename _SetType::data_type** expected)
{
    typename _SetType::search_result_type result, test;
    bool success = db.search(x, y, result);
    if (!success)
    {
        cout << "search failed" << endl;
        return false;
    }

    sort(result.begin(), result.end(), typename _SetType::data_type::sort_by_name());
    print_search_result<_SetType>(x, y, result);

    size_t i = 0;
    const typename _SetType::data_type* data = expected[i++];
    while (data)
    {
        test.push_back(data);
        data = expected[i++];
    }
    sort(test.begin(), test.end(), typename _SetType::data_type::sort_by_name());
    return result == test;
}

void rect_test_insertion_removal()
{
    StackPrinter __stack_printer__("::rect_test_insertion_removal");

    typedef uint32_t value_type;
    typedef range<value_type> range_type;
    typedef rectangle_set<value_type, const range_type> set_type;

    set_type db;
    assert(check_size(db, 0));

    range_type A(0, 0,  1,  1, "A");
    range_type B(2, 2,  5, 10, "B");
    range_type C(0, 1,  2,  2, "C");
    range_type D(3, 3,  5,  5, "D");
    range_type E(3, 4,  5, 15, "E");
    range_type F(0, 3, 15, 15, "F");

    insert_range(db, A);
    insert_range(db, B);
    insert_range(db, C);
    db.dump_rectangles();
    {
        const set_type::data_type* expected[] = {&A, &B, &C, 0};
        assert(check_rectangles(db, expected));
        assert(check_size(db, 3));
    }

    insert_range(db, D);
    insert_range(db, E);
    insert_range(db, F);
    db.dump_rectangles();
    {
        const set_type::data_type* expected[] = {&A, &B, &C, &D, &E, &F, 0};
        assert(check_rectangles(db, expected));
        assert(check_size(db, 6));
    }

    // Duplicate insertion should *not* create new entries.
    insert_range(db, A);
    insert_range(db, C);
    insert_range(db, E);
    db.dump_rectangles();
    {
        const set_type::data_type* expected[] = {&A, &B, &C, &D, &E, &F, 0};
        assert(check_rectangles(db, expected));
        assert(check_size(db, 6));
    }

    // Start removing rectangles.

    db.remove(&E);
    db.dump_rectangles();
    {
        const set_type::data_type* expected[] = {&A, &B, &C, &D, &F, 0};
        assert(check_rectangles(db, expected));
        assert(check_size(db, 5));
    }

    db.remove(&E);
    db.dump_rectangles();
    {
        const set_type::data_type* expected[] = {&A, &B, &C, &D, &F, 0};
        assert(check_rectangles(db, expected));
        assert(check_size(db, 5));
    }
    db.remove(&A);
    db.remove(&B);
    db.remove(&C);
    db.dump_rectangles();
    {
        const set_type::data_type* expected[] = {&D, &F, 0};
        assert(check_rectangles(db, expected));
        assert(check_size(db, 2));
    }

    db.remove(&D);
    db.remove(&F);
    db.dump_rectangles();
    {
        const set_type::data_type* expected[] = {0};
        assert(check_rectangles(db, expected));
        assert(check_size(db, 0));
    }
}

void rect_test_search()
{
    StackPrinter __stack_printer__("::rect_test_search");
    typedef uint32_t value_type;
    typedef range<value_type> range_type;
    typedef rectangle_set<value_type, const range_type> set_type;

    range_type A(0, 0, 1, 1, "A");
    range_type B(0, 0, 2, 2, "B");
    range_type C(0, 0, 3, 3, "C");
    range_type D(0, 0, 4, 4, "D");
    range_type E(0, 0, 5, 5, "E");
    range_type F(0, 0, 6, 6, "F");
    range_type G(0, 0, 7, 7, "G");

    set_type db;
    {
        // Search before any data is inserted.
        const set_type::data_type* expected[] = {0};
        assert(check_search_result<set_type>(db, 0, 0, expected));
    }
    insert_range(db, A);
    insert_range(db, B);
    insert_range(db, C);
    insert_range(db, D);
    insert_range(db, E);
    insert_range(db, F);
    insert_range(db, G);
    db.dump_rectangles();
    check_size(db, 7);

    {
        // Hits all rectangles.
        const set_type::data_type* expected[] = {&A, &B, &C, &D, &E, &F, &G, 0};
        assert(check_search_result<set_type>(db, 0, 0, expected));
    }
    {
        const set_type::data_type* expected[] = {&B, &C, &D, &E, &F, &G, 0};
        assert(check_search_result<set_type>(db, 0, 1, expected));
        assert(check_search_result<set_type>(db, 1, 0, expected));
        assert(check_search_result<set_type>(db, 1, 1, expected));
    }
    {
        const set_type::data_type* expected[] = {&C, &D, &E, &F, &G, 0};
        assert(check_search_result<set_type>(db, 0, 2, expected));
        assert(check_search_result<set_type>(db, 2, 0, expected));
        assert(check_search_result<set_type>(db, 2, 2, expected));
    }
    {
        const set_type::data_type* expected[] = {&D, &E, &F, &G, 0};
        assert(check_search_result<set_type>(db, 0, 3, expected));
        assert(check_search_result<set_type>(db, 3, 0, expected));
        assert(check_search_result<set_type>(db, 3, 3, expected));
    }
    {
        const set_type::data_type* expected[] = {&E, &F, &G, 0};
        assert(check_search_result<set_type>(db, 0, 4, expected));
        assert(check_search_result<set_type>(db, 4, 0, expected));
        assert(check_search_result<set_type>(db, 4, 4, expected));
    }
    {
        const set_type::data_type* expected[] = {&F, &G, 0};
        assert(check_search_result<set_type>(db, 0, 5, expected));
        assert(check_search_result<set_type>(db, 5, 0, expected));
        assert(check_search_result<set_type>(db, 5, 5, expected));
    }
    {
        const set_type::data_type* expected[] = {&G, 0};
        assert(check_search_result<set_type>(db, 0, 6, expected));
        assert(check_search_result<set_type>(db, 6, 0, expected));
        assert(check_search_result<set_type>(db, 6, 6, expected));
    }
    {
        const set_type::data_type* expected[] = {0};
        assert(check_search_result<set_type>(db, 0, 7, expected));
        assert(check_search_result<set_type>(db, 7, 0, expected));
        assert(check_search_result<set_type>(db, 7, 7, expected));
    }

    // Remove some rectangles and run the same set of searches again.
    db.remove(&B);
    db.remove(&D);
    db.remove(&F);
    check_size(db, 4);
    db.dump_rectangles();

    {
        // Hits all rectangles.
        const set_type::data_type* expected[] = {&A, &C, &E, &G, 0};
        assert(check_search_result<set_type>(db, 0, 0, expected));
    }
    {
        const set_type::data_type* expected[] = {&C, &E, &G, 0};
        assert(check_search_result<set_type>(db, 0, 1, expected));
        assert(check_search_result<set_type>(db, 1, 0, expected));
        assert(check_search_result<set_type>(db, 1, 1, expected));
    }
    {
        const set_type::data_type* expected[] = {&C, &E, &G, 0};
        assert(check_search_result<set_type>(db, 0, 2, expected));
        assert(check_search_result<set_type>(db, 2, 0, expected));
        assert(check_search_result<set_type>(db, 2, 2, expected));
    }
    {
        const set_type::data_type* expected[] = {&E, &G, 0};
        assert(check_search_result<set_type>(db, 0, 3, expected));
        assert(check_search_result<set_type>(db, 3, 0, expected));
        assert(check_search_result<set_type>(db, 3, 3, expected));
    }
    {
        const set_type::data_type* expected[] = {&E, &G, 0};
        assert(check_search_result<set_type>(db, 0, 4, expected));
        assert(check_search_result<set_type>(db, 4, 0, expected));
        assert(check_search_result<set_type>(db, 4, 4, expected));
    }
    {
        const set_type::data_type* expected[] = {&G, 0};
        assert(check_search_result<set_type>(db, 0, 5, expected));
        assert(check_search_result<set_type>(db, 5, 0, expected));
        assert(check_search_result<set_type>(db, 5, 5, expected));
    }
    {
        const set_type::data_type* expected[] = {&G, 0};
        assert(check_search_result<set_type>(db, 0, 6, expected));
        assert(check_search_result<set_type>(db, 6, 0, expected));
        assert(check_search_result<set_type>(db, 6, 6, expected));
    }
    {
        const set_type::data_type* expected[] = {0};
        assert(check_search_result<set_type>(db, 0, 7, expected));
        assert(check_search_result<set_type>(db, 7, 0, expected));
        assert(check_search_result<set_type>(db, 7, 7, expected));
    }
}

void rect_test_copy_constructor()
{
    StackPrinter __stack_printer__("::rect_test_copy_constructor");
    typedef int16_t value_type;
    typedef range<value_type> range_type;
    typedef rectangle_set<value_type, const range_type> set_type;

    range_type A(0, 0, 1, 1, "A");
    range_type B(0, 0, 2, 2, "B");
    range_type C(0, 0, 3, 3, "C");
    range_type D(0, 0, 4, 4, "D");
    range_type E(0, 0, 5, 5, "E");
    range_type F(0, 0, 6, 6, "F");
    range_type G(0, 0, 7, 7, "G");

    set_type db;
    insert_range(db, A);
    insert_range(db, B);
    insert_range(db, C);
    insert_range(db, D);
    insert_range(db, E);
    insert_range(db, F);
    insert_range(db, G);
    set_type db_copied(db);
    check_size(db, 7);
    check_size(db_copied, 7);
    db_copied.dump_rectangles();

    {
        // Hits all rectangles.
        const set_type::data_type* expected[] = {&A, &B, &C, &D, &E, &F, &G, 0};
        assert(check_search_result<set_type>(db_copied, 0, 0, expected));
    }
    {
        const set_type::data_type* expected[] = {&B, &C, &D, &E, &F, &G, 0};
        assert(check_search_result<set_type>(db_copied, 0, 1, expected));
        assert(check_search_result<set_type>(db_copied, 1, 0, expected));
        assert(check_search_result<set_type>(db_copied, 1, 1, expected));
    }
    {
        const set_type::data_type* expected[] = {&C, &D, &E, &F, &G, 0};
        assert(check_search_result<set_type>(db_copied, 0, 2, expected));
        assert(check_search_result<set_type>(db_copied, 2, 0, expected));
        assert(check_search_result<set_type>(db_copied, 2, 2, expected));
    }
    {
        const set_type::data_type* expected[] = {&D, &E, &F, &G, 0};
        assert(check_search_result<set_type>(db_copied, 0, 3, expected));
        assert(check_search_result<set_type>(db_copied, 3, 0, expected));
        assert(check_search_result<set_type>(db_copied, 3, 3, expected));
    }
    {
        const set_type::data_type* expected[] = {&E, &F, &G, 0};
        assert(check_search_result<set_type>(db_copied, 0, 4, expected));
        assert(check_search_result<set_type>(db_copied, 4, 0, expected));
        assert(check_search_result<set_type>(db_copied, 4, 4, expected));
    }
    {
        const set_type::data_type* expected[] = {&F, &G, 0};
        assert(check_search_result<set_type>(db_copied, 0, 5, expected));
        assert(check_search_result<set_type>(db_copied, 5, 0, expected));
        assert(check_search_result<set_type>(db_copied, 5, 5, expected));
    }
    {
        const set_type::data_type* expected[] = {&G, 0};
        assert(check_search_result<set_type>(db_copied, 0, 6, expected));
        assert(check_search_result<set_type>(db_copied, 6, 0, expected));
        assert(check_search_result<set_type>(db_copied, 6, 6, expected));
    }
    {
        const set_type::data_type* expected[] = {0};
        assert(check_search_result<set_type>(db_copied, 0, 7, expected));
        assert(check_search_result<set_type>(db_copied, 7, 0, expected));
        assert(check_search_result<set_type>(db_copied, 7, 7, expected));
    }
    db_copied.clear();
    check_size(db_copied, 0);
    {
        // There is no rectangle left, hence the search result should be empty.
        const set_type::data_type* expected[] = {0};
        assert(check_search_result<set_type>(db_copied, 0, 0, expected));
    }

    // Insert a new set of ranges to the copied instance.
    range_type A1(0, 0, 1, 1, "A1");
    range_type B1(0, 0, 2, 2, "B1");
    range_type C1(0, 0, 3, 3, "C1");
    range_type D1(0, 0, 4, 4, "D1");
    range_type E1(0, 0, 5, 5, "E1");
    range_type F1(0, 0, 6, 6, "F1");
    range_type G1(0, 0, 7, 7, "G1");
    insert_range(db_copied, A1);
    insert_range(db_copied, B1);
    insert_range(db_copied, C1);
    insert_range(db_copied, D1);
    insert_range(db_copied, E1);
    insert_range(db_copied, F1);
    insert_range(db_copied, G1);
    db_copied.dump_rectangles();
    {
        const set_type::data_type* expected[] = {&A1, &B1, &C1, &D1, &E1, &F1, &G1, 0};
        assert(check_search_result<set_type>(db_copied, 0, 0, expected));
    }

    {
        // Check against the origintal dataset, to ensure modification of the 
        // copy does not modify the original.
        const set_type::data_type* expected[] = {&A, &B, &C, &D, &E, &F, &G, 0};
        assert(check_search_result<set_type>(db, 0, 0, expected));
    }
}

void rect_test_assignment()
{
    typedef int16_t value_type;
    typedef range<value_type> range_type;
    typedef rectangle_set<value_type, const range_type> set_type;

    range_type A(0, 0, 1, 1, "A");
    range_type B(0, 0, 2, 2, "B");
    range_type C(0, 0, 3, 3, "C");
    range_type D(0, 0, 4, 4, "D");
    range_type E(0, 0, 5, 5, "E");
    range_type F(0, 0, 6, 6, "F");
    range_type G(0, 0, 7, 7, "G");

    set_type db;
    insert_range(db, A);
    insert_range(db, B);
    insert_range(db, C);
    insert_range(db, D);
    insert_range(db, E);
    insert_range(db, F);
    insert_range(db, G);
    set_type db_assigned = db;
    check_size(db, 7);
    check_size(db_assigned, 7);
    db_assigned.dump_rectangles();

    {
        // Hits all rectangles.
        const set_type::data_type* expected[] = {&A, &B, &C, &D, &E, &F, &G, 0};
        assert(check_search_result<set_type>(db_assigned, 0, 0, expected));
    }
    {
        const set_type::data_type* expected[] = {&B, &C, &D, &E, &F, &G, 0};
        assert(check_search_result<set_type>(db_assigned, 0, 1, expected));
        assert(check_search_result<set_type>(db_assigned, 1, 0, expected));
        assert(check_search_result<set_type>(db_assigned, 1, 1, expected));
    }
    {
        const set_type::data_type* expected[] = {&C, &D, &E, &F, &G, 0};
        assert(check_search_result<set_type>(db_assigned, 0, 2, expected));
        assert(check_search_result<set_type>(db_assigned, 2, 0, expected));
        assert(check_search_result<set_type>(db_assigned, 2, 2, expected));
    }
    {
        const set_type::data_type* expected[] = {&D, &E, &F, &G, 0};
        assert(check_search_result<set_type>(db_assigned, 0, 3, expected));
        assert(check_search_result<set_type>(db_assigned, 3, 0, expected));
        assert(check_search_result<set_type>(db_assigned, 3, 3, expected));
    }
    {
        const set_type::data_type* expected[] = {&E, &F, &G, 0};
        assert(check_search_result<set_type>(db_assigned, 0, 4, expected));
        assert(check_search_result<set_type>(db_assigned, 4, 0, expected));
        assert(check_search_result<set_type>(db_assigned, 4, 4, expected));
    }
    {
        const set_type::data_type* expected[] = {&F, &G, 0};
        assert(check_search_result<set_type>(db_assigned, 0, 5, expected));
        assert(check_search_result<set_type>(db_assigned, 5, 0, expected));
        assert(check_search_result<set_type>(db_assigned, 5, 5, expected));
    }
    {
        const set_type::data_type* expected[] = {&G, 0};
        assert(check_search_result<set_type>(db_assigned, 0, 6, expected));
        assert(check_search_result<set_type>(db_assigned, 6, 0, expected));
        assert(check_search_result<set_type>(db_assigned, 6, 6, expected));
    }
    {
        const set_type::data_type* expected[] = {0};
        assert(check_search_result<set_type>(db_assigned, 0, 7, expected));
        assert(check_search_result<set_type>(db_assigned, 7, 0, expected));
        assert(check_search_result<set_type>(db_assigned, 7, 7, expected));
    }
    db_assigned.clear();
    check_size(db_assigned, 0);
    {
        // There is no rectangle left, hence the search result should be empty.
        const set_type::data_type* expected[] = {0};
        assert(check_search_result<set_type>(db_assigned, 0, 0, expected));
    }

    // Insert a new set of ranges to the assigned instance.
    range_type A1(0, 0, 1, 1, "A1");
    range_type B1(0, 0, 2, 2, "B1");
    range_type C1(0, 0, 3, 3, "C1");
    range_type D1(0, 0, 4, 4, "D1");
    range_type E1(0, 0, 5, 5, "E1");
    range_type F1(0, 0, 6, 6, "F1");
    range_type G1(0, 0, 7, 7, "G1");
    insert_range(db_assigned, A1);
    insert_range(db_assigned, B1);
    insert_range(db_assigned, C1);
    insert_range(db_assigned, D1);
    insert_range(db_assigned, E1);
    insert_range(db_assigned, F1);
    insert_range(db_assigned, G1);
    db_assigned.dump_rectangles();
    {
        const set_type::data_type* expected[] = {&A1, &B1, &C1, &D1, &E1, &F1, &G1, 0};
        assert(check_search_result<set_type>(db_assigned, 0, 0, expected));
    }

    {
        // Check against the origintal dataset, to ensure modification of the 
        // copy does not modify the original.
        const set_type::data_type* expected[] = {&A, &B, &C, &D, &E, &F, &G, 0};
        assert(check_search_result<set_type>(db, 0, 0, expected));
    }

    db_assigned = db;
    {
        // Hits all rectangles once again after reverting to the original data set.
        const set_type::data_type* expected[] = {&A, &B, &C, &D, &E, &F, &G, 0};
        assert(check_search_result<set_type>(db_assigned, 0, 0, expected));
    }
}

void rect_test_equality()
{
    StackPrinter __stack_printer__("::rect_test_equality");

    typedef int16_t value_type;
    typedef range<value_type> range_type;
    typedef rectangle_set<value_type, const range_type> set_type;

    range_type A(0, 0, 1, 1, "A");
    range_type B(0, 0, 2, 2, "B");
    range_type C(0, 0, 3, 3, "C");
    range_type D(0, 0, 4, 4, "D");
    range_type E(0, 0, 5, 5, "E");
    range_type F(0, 0, 6, 6, "F");
    range_type G(0, 0, 7, 7, "G");

    set_type db1, db2;
    assert(db1 == db2);
    insert_range(db1, A);
    assert(db1 != db2);
    insert_range(db2, B);
    assert(db1 != db2);
    insert_range(db1, B);
    assert(db1 != db2);
    insert_range(db2, A);
    assert(db1 == db2);

    db1.clear();
    db2.clear();
    assert(db1 == db2);

    // Store the same object address but with different geometries.
    db1.insert(0, 0, 2, 2, &A);
    db2.insert(3, 3, 6, 6, &A);
    assert(db1 != db2);

    db2.remove(&A);
    db2.insert(0, 0, 2, 2, &A);
    assert(db1 == db2);
}

int main(int argc, char** argv)
{
    rect_test_insertion_removal();
    rect_test_search();
    rect_test_copy_constructor();
    rect_test_assignment();
    rect_test_equality();
    fprintf(stdout, "Test finished successfully!\n");
}
