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

#include "mdds/quad_type_matrix.hpp"

#include <sstream>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

#include <stdio.h>
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

typedef quad_type_matrix<string> mx_type;
typedef void (test_func_type)(matrix_density_t);

/**
 * Run specified test on all matrix density types.
 *  
 * @param func function pointer to the test function to be performed.
 */
void run_tests_on_all_density_types(test_func_type* func)
{
    func(matrix_density_filled_zero);
    func(matrix_density_filled_empty);
    func(matrix_density_sparse_zero);
    func(matrix_density_sparse_empty);
}

string get_mx_density_name(matrix_density_t dens)
{
    switch (dens)
    {
        case matrix_density_filled_zero:
            return "density filled with zero base elements";
        case matrix_density_filled_empty:
            return "density filled with empty base elements";
        case matrix_density_sparse_zero:
            return "density sparse with zero base elements";
        case matrix_density_sparse_empty:
            return "density sparse with empty base elements";
        default:
            ;
    }
    return "unknown density";
}

void print_mx_density_type(matrix_density_t dens)
{
    cout << "matrix density type: " << get_mx_density_name(dens) << endl;
}

template<typename _Mx>
bool verify_transposed_matrix(const _Mx& original, const _Mx& transposed)
{
    pair<size_t, size_t> mx_size = original.size(), mx_size_trans = transposed.size();
    if (mx_size.first != mx_size_trans.second || mx_size.second != mx_size_trans.first)
        return false;

    for (size_t row = 0; row < mx_size.first; ++row)
    {
        for (size_t col = 0; col < mx_size.second; ++col)
        {
            matrix_element_t elem_type = original.get_type(row, col);
            if (elem_type != transposed.get_type(col, row))
                return false;

            switch (elem_type)
            {
                case element_boolean:
                    if (original.get_boolean(row, col) != transposed.get_boolean(col, row))
                        return false;
                    break;
                case element_numeric:
                    if (original.get_numeric(row, col) != transposed.get_numeric(col, row))
                        return false;
                    break;
                case element_string:
                    if (original.get_string(row, col) != transposed.get_string(col, row))
                        return false;
                    break;
                case element_empty:
                default:
                    ;
            }
        }
    }
    return true;
}

template<typename _Mx>
bool verify_init_zero(const _Mx& mx)
{
    pair<size_t,size_t> mxsize = mx.size();
    for (size_t row = 0; row < mxsize.first; ++row)
    {
        for (size_t col = 0; col < mxsize.second; ++col)
        {
            if (mx.get_type(row, col) != element_numeric)
                return false;
            if (mx.get_numeric(row, col) != 0.0)
                return false;
        }
    }
    return true;
}

template<typename _Mx>
bool verify_init_empty(const _Mx& mx)
{
    pair<size_t,size_t> mxsize = mx.size();
    for (size_t row = 0; row < mxsize.first; ++row)
    {
        for (size_t col = 0; col < mxsize.second; ++col)
        {
            if (mx.get_type(row, col) != element_empty)
                return false;
        }
    }
    return true;
}

void qtm_test_resize(matrix_density_t density)
{
    StackPrinter __stack_printer__("::qtm_test_resize");
    print_mx_density_type(density);
    pair<size_t,size_t> mxsize;
    mx_type mx(3, 3, density);
    mx.dump();
    mxsize = mx.size();
    assert(mxsize.first == 3);
    assert(mxsize.second == 3);

    mx.set_string(0, 0, new string("test"));
    mx.set_numeric(0, 1, 2.3);
    mx.set_boolean(1, 1, false);
    mx.set_numeric(1, 2, 45.4);
    mx.set_empty(2, 0);
    mx.set_empty(2, 1);
    mx.set_empty(2, 2);
    mx.dump();
    mx.resize(6, 4);
    mx.dump();
    mxsize = mx.size();
    assert(mxsize.first == 6);
    assert(mxsize.second == 4);
    mx.resize(6, 6);
    mx.dump();
    mxsize = mx.size();
    assert(mxsize.first == 6);
    assert(mxsize.second == 6);
    mx.resize(3, 6);
    mx.dump();
    mxsize = mx.size();
    assert(mxsize.first == 3);
    assert(mxsize.second == 6);

    mx.resize(3, 3);
    mx.dump();
    mxsize = mx.size();
    assert(mxsize.first == 3);
    assert(mxsize.second == 3);

    mx.resize(0, 0);
    mx.dump();
    mxsize = mx.size();
    assert(mxsize.first == 0);
    assert(mxsize.second == 0);
    assert(mx.empty());
}

void qtm_test_value_store(matrix_density_t density)
{
    StackPrinter __stack_printer__("::qtm_test_value_store");
    print_mx_density_type(density);
    mx_type mx(5, 5, density);
    mx.dump();
    pair<size_t,size_t> mxsize = mx.size();
    assert(mxsize.first == 5);
    assert(mxsize.second == 5);
    assert(!mx.empty());

    // Make sure all elements have been initialized properly according to the 
    // matrix type.
    for (size_t i = 0; i < 5; ++i)
    {
        for (size_t j = 0; j < 5; ++j)
        {
            matrix_element_t elem_type = mx.get_type(i, j);
            switch (density)
            {
                case matrix_density_filled_zero:
                case matrix_density_sparse_zero:
                {
                    // filled matrices are initialized to numeric elements 
                    // having a value of 0.
                    assert(elem_type == element_numeric);
                    double val = mx.get_numeric(i, j);
                    assert(val == 0.0);
                }
                break;
                case matrix_density_filled_empty:
                case matrix_density_sparse_empty:
                    // sparse matrices are initialized to empty elements.
                    assert(elem_type == element_empty);
                break;
            }
        }
    }

    // Insert strings into all elements.
    for (size_t i = 0; i < 5; ++i)
    {
        for (size_t j = 0; j < 5; ++j)
        {
            ostringstream os;
            os << "(" << i << "," << j << ")";
            mx.set_string(i, j, new string(os.str()));
        }
    }
    mx.dump();

    for (size_t i = 0; i < 5; ++i)
    {
        for (size_t j = 0; j < 5; ++j)
        {
            matrix_element_t elem_type = mx.get_type(i, j);
            assert(elem_type == element_string);
            string s = mx.get_string(i, j);
            cout << s << " ";
        }
        cout << endl;
    }

    // Now, boolean values.  Note that these operations should de-alloocate
    // all previously stored strings.
    for (size_t i = 0; i < 5; ++i)
    {
        for (size_t j = 0; j < 5; ++j)
        {
            bool b = (i+j)%2 ? true : false;
            mx.set_boolean(i, j, b);
        }
    }
    mx.dump();

    for (size_t i = 0; i < 5; ++i)
    {
        for (size_t j = 0; j < 5; ++j)
        {
            matrix_element_t elem_type = mx.get_type(i, j);
            assert(elem_type == element_boolean);
            bool stored = mx.get_boolean(i, j);
            bool expected = (i+j)%2 ? true : false;
            assert(stored == expected);
        }
    }

    // Make all elements empty.
    for (size_t i = 0; i < 5; ++i)
    {
        for (size_t j = 0; j < 5; ++j)
        {
            mx.set_empty(i, j);
        }
    }
    mx.dump();

    for (size_t i = 0; i < 5; ++i)
    {
        for (size_t j = 0; j < 5; ++j)
        {
            matrix_element_t elem_type = mx.get_type(i, j);
            assert(elem_type == element_empty);
        }
    }
}

template<typename _Mx>
void print_transposed_mx(const _Mx& original, const _Mx& transposed)
{
    cout << "original matrix:" << endl;
    original.dump();
    cout << "transposed matrix:" << endl;
    transposed.dump();
}

void qtm_test_transpose(matrix_density_t density)
{
    StackPrinter __stack_printer__("::qtm_test_transpose");
    print_mx_density_type(density);

    {
        // Transposition of square matrix.
        mx_type mx(3, 3, density);
        mx.set_numeric(0, 1, 1);
        mx.set_numeric(0, 2, 1);
        mx.set_numeric(1, 2, 1);
        mx.set_numeric(1, 0, 2);
        mx.set_numeric(2, 0, 2);
        mx.set_numeric(2, 1, 2);
        mx_type mx_trans;
        mx.transpose(mx_trans);
        print_transposed_mx(mx, mx_trans);
        bool success = verify_transposed_matrix(mx, mx_trans);
        assert(success);
    }

    {
        // Non-square matrix.
        mx_type mx(5, 3, density);
        mx.set_numeric(0, 0, 10);
        mx.set_boolean(1, 0, true);
        mx.set_boolean(2, 0, false);
        mx.set_numeric(3, 0, 23);
        mx.set_string(3, 2, new string("test"));
        mx.set_empty(4, 0);
        mx_type mx_trans;
        mx.transpose(mx_trans);
        print_transposed_mx(mx, mx_trans);
        bool success = verify_transposed_matrix(mx, mx_trans);
        assert(success);
    }

    {
        // Empty matrix.
        mx_type mx, mx_trans;
        mx.transpose(mx_trans);
        print_transposed_mx(mx, mx_trans);
        bool success = verify_transposed_matrix(mx, mx_trans);
        assert(success);
    }
}

void qtm_test_initial_elements()
{
    StackPrinter __stack_printer__("::qtm_test_initial_elements");
    {
        mx_type mx(3, 3, matrix_density_filled_zero);
        mx.dump();
        bool success = verify_init_zero(mx);
        assert(success);
        assert(mx.numeric());
        mx.resize(15, 14);
        assert(mx.numeric());
    }

    {
        mx_type mx(3, 3, matrix_density_filled_empty);
        mx.dump();
        bool success = verify_init_empty(mx);
        assert(success);
        assert(!mx.numeric());
    }

    {
        mx_type mx(3, 3, matrix_density_sparse_zero);
        mx.dump();
        bool success = verify_init_zero(mx);
        assert(success);
        assert(mx.numeric());
        mx.resize(10, 32);
        assert(mx.numeric());
    }

    {
        mx_type mx(3, 3, matrix_density_sparse_empty);
        mx.dump();
        bool success = verify_init_empty(mx);
        assert(success);
        assert(!mx.numeric());
    }
}

void qtm_test_numeric_matrix()
{
    StackPrinter __stack_printer__("::qtm_test_numeric_matrix");
    {
        print_mx_density_type(matrix_density_filled_zero);
        mx_type mx(3, 3, matrix_density_filled_zero);
        mx.dump();
        assert(mx.numeric());
        mx.set_empty(0, 0);
        mx.dump();
        assert(!mx.numeric());
        mx.resize(5, 5);
        mx.dump();
        assert(!mx.numeric());
        mx.resize(2, 2);
        mx.dump();
        assert(!mx.numeric());
        mx.set_numeric(0, 0, 50);
        mx.dump();
        assert(mx.numeric());
        mx.set_boolean(1, 1, true);
        mx.dump();
        assert(mx.numeric());
        assert(mx.get_numeric(1, 1) == 1.0);
        mx.set_string(1, 0, new string("test"));
        mx.dump();
        assert(!mx.numeric());
    }

    {
        print_mx_density_type(matrix_density_sparse_zero);
        mx_type mx(3, 3, matrix_density_sparse_zero);
        mx.dump();
        assert(mx.numeric());
        mx.set_empty(0, 0);
        mx.dump();
        assert(!mx.numeric());
        mx.resize(5, 5);
        mx.dump();
        assert(!mx.numeric());
        mx.resize(2, 2);
        mx.dump();
        assert(!mx.numeric());
        mx.set_numeric(0, 0, 50);
        mx.dump();
        assert(mx.numeric());
        mx.set_boolean(1, 1, true);
        mx.dump();
        assert(mx.numeric());
        assert(mx.get_numeric(1, 1) == 1.0);
        mx.set_string(1, 0, new string("test"));
        mx.dump();
        assert(!mx.numeric());
    }
}

int main()
{
    run_tests_on_all_density_types(qtm_test_resize);
    run_tests_on_all_density_types(qtm_test_value_store);
    run_tests_on_all_density_types(qtm_test_transpose);
    qtm_test_initial_elements();
    qtm_test_numeric_matrix();
    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}
