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

void qtm_test_resize(matrix_density_t density)
{
    StackPrinter __stack_printer__("::qtm_test_resize");
    typedef quad_type_matrix<string> mx_type;
    mx_type mx(3, 3, density);
    mx.dump();
    assert(mx.size_rows() == 3);
    assert(mx.size_cols() == 3);

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
    assert(mx.size_rows() == 6);
    assert(mx.size_cols() == 4);
    mx.resize(6, 6);
    mx.dump();
    assert(mx.size_rows() == 6);
    assert(mx.size_cols() == 6);
    mx.resize(3, 6);
    mx.dump();
    assert(mx.size_rows() == 3);
    assert(mx.size_cols() == 6);

    mx.resize(3, 3);
    mx.dump();
    assert(mx.size_rows() == 3);
    assert(mx.size_cols() == 3);

    mx.resize(0, 0);
    mx.dump();
    assert(mx.size_rows() == 0);
    assert(mx.size_cols() == 0);
    assert(mx.empty());
}

void qtm_test_value_store(matrix_density_t density)
{
    StackPrinter __stack_printer__("::qtm_test_value_store");
    typedef quad_type_matrix<string> mx_type;
    mx_type mx(5, 5, density);
    mx.dump();
    assert(mx.size_rows() == 5);
    assert(mx.size_cols() == 5);
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
                case matrix_density_filled:
                {
                    // filled matrices are initialized to numeric elements 
                    // having a value of 0.
                    assert(elem_type == element_numeric);
                    double val = mx.get_numeric(i, j);
                    assert(val == 0.0);
                }
                break;
                case matrix_density_sparse:
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

void qtm_test_transpose(matrix_density_t density)
{
    StackPrinter __stack_printer__("::qtm_test_transpose");
    typedef quad_type_matrix<string> mx_type;

    mx_type mx(3, 3, density);
    mx.set_numeric(0, 1, 1);
    mx.set_numeric(0, 2, 1);
    mx.set_numeric(1, 2, 1);
    mx.set_numeric(1, 0, 2);
    mx.set_numeric(2, 0, 2);
    mx.set_numeric(2, 1, 2);
    cout << "original matrix:" << endl;
    mx.dump();
    mx_type mx_trans(0, 0, density);
    mx.transpose(mx_trans);
    cout << "transposed matrix:" << endl;
    mx_trans.dump();

}

int main()
{
    qtm_test_resize(matrix_density_filled);
    qtm_test_resize(matrix_density_sparse);
    qtm_test_value_store(matrix_density_filled);
    qtm_test_value_store(matrix_density_sparse);
    qtm_test_transpose(matrix_density_filled);
    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}
