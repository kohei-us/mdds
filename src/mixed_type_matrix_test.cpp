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

#include "mdds/mixed_type_matrix.hpp"

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

typedef mixed_type_matrix<string, uint8_t> mx_type;
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

template<typename _Elem>
string print_element(const _Elem& e)
{
    ostringstream os;
    switch (e.m_type)
    {
        case element_empty:
            os << "empty";
        break;
        case element_numeric:
            os << "numeric (" << e.m_numeric << ")";
        break;
        case element_boolean:
            os << "boolean";
        break;
        case element_string:
            os << "string";
        break;
        default:
            os << "<unkwown>";
    }
    return os.str();
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
                    if (*original.get_string(row, col) != *transposed.get_string(col, row))
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

template<typename _Mx>
bool verify_assign(const _Mx& mx1, const _Mx& mx2)
{
    size_t row_count = min(mx1.size().first,  mx2.size().first);
    size_t col_count = min(mx1.size().second, mx2.size().second);
    for (size_t i = 0; i < row_count; ++i)
    {
        for (size_t j = 0; j < col_count; ++j)
        {
            matrix_element_t elem_type = mx1.get_type(i, j);
            if (elem_type != mx2.get_type(i, j))
                return false;
            
            switch (elem_type)
            {
                case element_boolean:
                    if (mx1.get_boolean(i, j) != mx2.get_boolean(i, j))
                        return false;
                    break;
                case element_numeric:
                    if (mx1.get_numeric(i, j) != mx2.get_numeric(i, j))
                        return false;
                    break;
                case element_string:
                    if (*mx1.get_string(i, j) != *mx2.get_string(i, j))
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
bool verify_assignment(const _Mx& mx1, const _Mx& mx2)
{
    if (mx1.size() != mx2.size())
        return false;

    return verify_assign<_Mx>(mx1, mx2);
}

void mtm_test_resize(matrix_density_t density)
{
    StackPrinter __stack_printer__("::mtm_test_resize");
    print_mx_density_type(density);
    pair<size_t,size_t> mxsize;

    {
        // Start with an empty matrix, and resize into a non-empty one.
        mx_type mx(0, 0, density);
        mxsize = mx.size();
        assert(mxsize.first == 0);
        assert(mxsize.second == 0);
        mx.resize(1, 1);
        mxsize = mx.size();
        assert(mxsize.first == 1);
        assert(mxsize.second == 1);
        // Back to an empty matrix again.
        mx.resize(0, 0);
        mxsize = mx.size();
        assert(mxsize.first == 0);
        assert(mxsize.second == 0);
        // Resize to a non-square matrix.
        mx.resize(5, 10);
        mxsize = mx.size();
        assert(mxsize.first == 5);
        assert(mxsize.second == 10);
    }

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

void mtm_test_value_store(matrix_density_t density)
{
    StackPrinter __stack_printer__("::mtm_test_value_store");
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
            string s = *mx.get_string(i, j);
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

void mtm_test_transpose(matrix_density_t density)
{
    StackPrinter __stack_printer__("::mtm_test_transpose");
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
        mx_type mx_trans(mx);
        mx_trans.transpose();
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
        mx_type mx_trans(mx);
        mx_trans.transpose();
        print_transposed_mx(mx, mx_trans);
        bool success = verify_transposed_matrix(mx, mx_trans);
        assert(success);
    }

    {
        // Empty matrix.
        mx_type mx, mx_trans;
        mx_trans.transpose();
        print_transposed_mx(mx, mx_trans);
        bool success = verify_transposed_matrix(mx, mx_trans);
        assert(success);
    }
}

void mtm_test_initial_elements()
{
    StackPrinter __stack_printer__("::mtm_test_initial_elements");
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

void mtm_test_numeric_matrix()
{
    StackPrinter __stack_printer__("::mtm_test_numeric_matrix");
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

void mtm_test_assign(matrix_density_t dens1, matrix_density_t dens2)
{
    StackPrinter __stack_printer__("::mtm_test_assign");
    print_mx_density_type(dens1);
    print_mx_density_type(dens2);

    // Assigning from a smaller matrix to a bigger one.
    mx_type mx1(5, 5, dens1), mx2(2, 2, dens2);
    mx2.set(0, 0, 1.2);
    mx2.set(1, 1, true);
    mx2.set(0, 1, new string("test"));
    mx2.set(1, 0, new string("foo"));
    cout << "matrix 1:" << endl;
    mx1.dump();
    cout << "matrix 2:" << endl;
    mx2.dump();
    mx1.assign(mx2);
    cout << "matrix 1 after assign:" << endl;
    mx1.dump();

    bool success = verify_assign(mx1, mx2);
    assert(success);

    mx2.resize(8, 8);
    mx2.assign(mx1);
    cout << "matrix 2 after resize and assign:" << endl;
    mx2.dump();
    success = verify_assign(mx1, mx2);
    assert(success);

    // from a larger matrix to a smaller one.
    mx1.set(0, 0, new string("test1"));
    mx2.set(0, 0, new string("test2"));
    mx2.set(4, 4, true);
    mx2.set(7, 7, false);
    mx1.assign(mx2);
    cout << "matrix 1 after assign:" << endl;
    mx1.dump();
    success = verify_assign(mx1, mx2);
    assert(success);

    // self assignment (should be no-op).
    mx1.assign(mx1);
    success = verify_assign(mx1, mx1);
    assert(success);

    mx2.assign(mx2);
    success = verify_assign(mx2, mx2);
    assert(success);
}

void mtm_test_assignment(matrix_density_t density)
{
    StackPrinter __stack_printer__("::mtm_test_assignment");
    print_mx_density_type(density);
    mx_type mx1(0, 0, density), mx2(3, 3, density);
    mx2.set(0, 0, 3.4);
    mx2.set(2, 1, new string("hmm..."));
    mx2.set(1, 2, new string("help"));
    cout << "matrix 1 initial:" << endl;
    mx1.dump();
    cout << "matrix 2 initial:" << endl;
    mx2.dump();
    mx1 = mx2;
    cout << "matrix 1 after assignment:" << endl;
    mx1.dump();
    bool success = verify_assignment(mx1, mx2);
    assert(success);

    // self assignment.
    mx2 = mx2;
    cout << "matrix 2 after self-assignment:" << endl;
    mx2.dump();
    success = verify_assignment(mx1, mx2);
    assert(success);

    mx1.set(0, 1, true);
    mx1.set(1, 0, false);
    mx2 = mx1;
    cout << "matrix 2 after assignment:" << endl;
    mx2.dump();
    success = verify_assignment(mx1, mx2);
    assert(success);

    // Assigning an empty matrix.
    mx2 = mx_type();
    assert(mx2.empty());
}

void mtm_test_flag_storage(matrix_density_t density)
{
    StackPrinter __stack_printer__("::mtm_test_flag_storage");
    print_mx_density_type(density);
    mx_type mx(3, 3, density);
    mx.dump();
    mx.dump_flags();
    assert(mx.get_flag(0, 0) == 0);

    mx_type::flag_type flag = 0x01;
    mx.set_flag(0, 0, flag);
    mx.dump_flags();
    assert(mx.get_flag(0, 0) == flag);

    flag |= 0xF0;
    mx.set_flag(0, 0, flag);
    mx.set_flag(2, 1, flag);
    mx.dump_flags();
    assert(mx.get_flag(0, 0) == flag);
    assert(mx.get_flag(2, 1) == flag);
    assert(mx.get_flag(2, 2) == 0);

    mx.clear_flag(0, 0);
    mx.dump_flags();
    assert(mx.get_flag(0, 0) == 0);
    assert(mx.get_flag(2, 1) == flag);
}

template<typename _StoreType>
void traverse_itr_access(typename _StoreType::const_itr_access& itr_access)
{
    typedef _StoreType store_type;
    if (itr_access.empty())
    {
        cout << "no element stored." << endl;
        return;
    }

    cout << "increment" << endl;
    long i = 0;
    do
    {
        cout << i++ << ": " << print_element(itr_access.get()) << endl;
    }
    while (itr_access.inc());

    cout << "decrement" << endl;

    while (itr_access.dec())
    {
        cout << --i << ": " << print_element(itr_access.get()) << endl;
    }
    assert(i == 0);
}

void mtm_test_iterator_access_filled(size_t rows, size_t cols)
{
    StackPrinter __stack_printer__("::mtm_test_iterator_access_filled");
    typedef storage_filled<mx_type> store_type;

    {
        cout << "rows: " << rows << "  cols: " << cols << endl;
        store_type store(rows, cols, matrix_init_element_zero);
        store_type::const_itr_access itr_access = store.get_const_itr_access();
        traverse_itr_access<store_type>(itr_access);
    }
}

void mtm_test_iterator_access_sparse()
{
    StackPrinter __stack_printer__("::mem_test_iterator_access_sparse");
    typedef storage_sparse<mx_type> store_type;
    store_type store(5, 5, matrix_init_element_empty);
    {
        store_type::const_itr_access itr_access = store.get_const_itr_access();
        assert(itr_access.empty());
    }
    {
        store_type::element& elem = store.get_element(0, 0);
        elem.m_type = element_numeric;
        elem.m_numeric = 3.5;
        store_type::const_itr_access itr_access = store.get_const_itr_access();
        assert(!itr_access.empty());
        traverse_itr_access<store_type>(itr_access);
    }
    {
        store_type::element& elem = store.get_element(4, 4);
        elem.m_type = element_numeric;
        elem.m_numeric = 12;
        store_type::const_itr_access itr_access = store.get_const_itr_access();
        assert(!itr_access.empty());
        traverse_itr_access<store_type>(itr_access);
    }
    {
        store_type::element& elem = store.get_element(3, 2);
        elem.m_type = element_numeric;
        elem.m_numeric = 26.567;
        store_type::const_itr_access itr_access = store.get_const_itr_access();
        assert(!itr_access.empty());
        traverse_itr_access<store_type>(itr_access);
    }
}

int main()
{
    run_tests_on_all_density_types(mtm_test_resize);
    run_tests_on_all_density_types(mtm_test_value_store);
    run_tests_on_all_density_types(mtm_test_transpose);
    run_tests_on_all_density_types(mtm_test_assignment);

    mtm_test_initial_elements();
    mtm_test_numeric_matrix();
    mtm_test_assign(matrix_density_filled_zero, matrix_density_filled_zero);
    mtm_test_assign(matrix_density_filled_empty, matrix_density_filled_zero);
    mtm_test_assign(matrix_density_filled_zero, matrix_density_filled_empty);
    mtm_test_assign(matrix_density_filled_empty, matrix_density_filled_empty);

    run_tests_on_all_density_types(mtm_test_flag_storage);

    mtm_test_iterator_access_filled(1, 1);
    mtm_test_iterator_access_filled(3, 1);
    mtm_test_iterator_access_filled(1, 3);
    mtm_test_iterator_access_filled(3, 3);
    mtm_test_iterator_access_filled(0, 0);

    mtm_test_iterator_access_sparse();

    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}
