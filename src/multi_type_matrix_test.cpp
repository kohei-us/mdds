/*************************************************************************
 *
 * Copyright (c) 2012 Kohei Yoshida
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

#include "test_global.hpp"

#include <mdds/multi_type_matrix.hpp>

#include <string>

using namespace mdds;
using namespace std;

typedef mdds::multi_type_matrix<std::string, int> mtx_type;

namespace {

template<typename _T>
void check_value(mtx_type& mtx, size_t row, size_t col, const _T& val)
{
    mtx.set(row, col, val);
    _T test = mtx.get<_T>(row, col);
    assert(test == val);
}

}

void mtm_test_construction()
{
    stack_printer __stack_printer__("::mtm_test_construction");
    {
        // default constructor.
        mtx_type mtx;
        mtx_type::size_pair_type sz = mtx.size();
        assert(sz.row == 0 && sz.column == 0);
    }

    {
        // construction to a specific size.
        mtx_type mtx(2, 5);
        mtx_type::size_pair_type sz = mtx.size();
        assert(sz.row == 2 && sz.column == 5);
    }

    {
        // construction to a specific size with default value.
        mtx_type mtx(2, 5, string("foo"));
        mtx_type::size_pair_type sz = mtx.size();
        assert(sz.row == 2 && sz.column == 5);
        assert(mtx.get_type(0,0) == mtx_type::element_string);
        assert(mtx.get_string(0,0) == "foo");
        assert(mtx.get_type(1,4) == mtx_type::element_string);
        assert(mtx.get_string(1,4) == "foo");
    }
}

void mtm_test_data_insertion()
{
    stack_printer __stack_printer__("::mtm_test_data_insertion");
    {
        // Create with empty elements.
        mtx_type mtx(3, 4);
        mtx_type::size_pair_type sz = mtx.size();
        assert(sz.row == 3 && sz.column == 4);
        assert(mtx.get_type(0,0) == mtx_type::element_empty);
        assert(mtx.get_type(2,3) == mtx_type::element_empty);
        check_value(mtx, 1, 1, 1.2);
        check_value(mtx, 2, 1, true);
        check_value(mtx, 3, 1, false);
        check_value(mtx, 0, 2, string("foo"));
        check_value(mtx, 1, 2, 23.4);

        // Overwrite
        assert(mtx.get_type(1,1) == mtx_type::element_numeric);
        check_value(mtx, 1, 1, string("baa"));

        // Setting empty.
        assert(mtx.get_type(1,1) == mtx_type::element_string);
        mtx.set_empty(1, 1);
        assert(mtx.get_type(1,1) == mtx_type::element_empty);
    }
}

void mtm_test_set_empty()
{
    stack_printer __stack_printer__("::mtm_test_set_empty");
    {
        // set whole column empty.
        mtx_type mtx(3, 5, 1.2);
        mtx.set_column_empty(2);
        assert(mtx.get_type(0, 1) != mtx_type::element_empty);
        assert(mtx.get_type(1, 1) != mtx_type::element_empty);
        assert(mtx.get_type(2, 1) != mtx_type::element_empty);
        assert(mtx.get_type(0, 2) == mtx_type::element_empty);
        assert(mtx.get_type(1, 2) == mtx_type::element_empty);
        assert(mtx.get_type(2, 2) == mtx_type::element_empty);
        assert(mtx.get_type(0, 3) != mtx_type::element_empty);
        assert(mtx.get_type(1, 3) != mtx_type::element_empty);
        assert(mtx.get_type(2, 3) != mtx_type::element_empty);
    }

    {
        // set whole row empty.
        mtx_type mtx(3, 5, 1.2);
        mtx.set_row_empty(1);
        assert(mtx.get_type(0, 0) != mtx_type::element_empty);
        assert(mtx.get_type(0, 1) != mtx_type::element_empty);
        assert(mtx.get_type(0, 2) != mtx_type::element_empty);
        assert(mtx.get_type(0, 3) != mtx_type::element_empty);
        assert(mtx.get_type(0, 4) != mtx_type::element_empty);
        assert(mtx.get_type(1, 0) == mtx_type::element_empty);
        assert(mtx.get_type(1, 1) == mtx_type::element_empty);
        assert(mtx.get_type(1, 2) == mtx_type::element_empty);
        assert(mtx.get_type(1, 3) == mtx_type::element_empty);
        assert(mtx.get_type(1, 4) == mtx_type::element_empty);
        assert(mtx.get_type(2, 0) != mtx_type::element_empty);
        assert(mtx.get_type(2, 1) != mtx_type::element_empty);
        assert(mtx.get_type(2, 2) != mtx_type::element_empty);
        assert(mtx.get_type(2, 3) != mtx_type::element_empty);
        assert(mtx.get_type(2, 4) != mtx_type::element_empty);
    }
}

/**
 * Measure the performance of object instantiation for filled storage.
 */
void mtm_perf_test_storage_creation()
{
    cout << "measuring performance on matrix object creation." << endl;
    size_t rowsize = 5000;
    size_t obj_count = 30000;
    cout << "row size: " << rowsize << "  object count: " << obj_count << endl;
    cout << "--- filled zero" << endl;
    for (size_t colsize = 1; colsize <= 5; ++colsize)
    {
        stack_watch sw;
        for (size_t i = 0; i < obj_count; ++i)
            mtx_type mx(rowsize, colsize, 0.0);

        cout << "column size: " << colsize << "  duration: " << sw.get_duration() << " sec" << endl;
    }
    cout << endl;
}

void mtm_perf_test_storage_set_numeric()
{
    cout << "measuring performance on matrix object creation and populating it with numeric data." << endl;
    size_t rowsize = 3000;
    size_t obj_count = 30000;
    cout << "row size: " << rowsize << "  object count: " << obj_count << endl;
    cout << "--- filled zero (individual insertion)" << endl;
    for (size_t colsize = 1; colsize <= 5; ++colsize)
    {
        stack_watch sw;
        for (size_t i = 0; i < obj_count; ++i)
        {
            mtx_type mx(rowsize, colsize, 0.0);
            for (size_t row = 0; row < rowsize; ++row)
            {
                for (size_t col = 0; col < colsize; ++col)
                    mx.set(row, col, 1.0);
            }
        }
        cout << "column size: " << colsize << "  duration: " << sw.get_duration() << " sec" << endl;
    }

    cout << "--- filled zero (per column)" << endl;
    for (size_t colsize = 1; colsize <= 5; ++colsize)
    {
        stack_watch sw;
        for (size_t i = 0; i < obj_count; ++i)
        {
            mtx_type mx(rowsize, colsize, 0.0);
            for (size_t col = 0; col < colsize; ++col)
            {
                vector<double> vals;
                vals.reserve(rowsize);
                for (size_t row = 0; row < rowsize; ++row)
                    vals.push_back(1.0);
                mx.set(0, col, vals.begin(), vals.end());
            }
        }
        cout << "column size: " << colsize << "  duration: " << sw.get_duration() << " sec" << endl;
    }

    cout << "--- filled zero (per column, pre-filled array)" << endl;
    for (size_t colsize = 1; colsize <= 5; ++colsize)
    {
        // Fill the data array before insertion.
        vector<double> vals;
        vals.reserve(rowsize);
        for (size_t row = 0; row < rowsize; ++row)
            vals.push_back(1.0);

        stack_watch sw;
        for (size_t i = 0; i < obj_count; ++i)
        {
            mtx_type mx(rowsize, colsize, 0.0);
            for (size_t col = 0; col < colsize; ++col)
                mx.set(0, col, vals.begin(), vals.end());
        }
        cout << "column size: " << colsize << "  duration: " << sw.get_duration() << " sec" << endl;
    }

    cout << "--- empty on creation (per column)" << endl;
    for (size_t colsize = 1; colsize <= 5; ++colsize)
    {
        stack_watch sw;
        for (size_t i = 0; i < obj_count; ++i)
        {
            mtx_type mx(rowsize, colsize);
            for (size_t col = 0; col < colsize; ++col)
            {
                vector<double> vals;
                vals.reserve(rowsize);
                for (size_t row = 0; row < rowsize; ++row)
                    vals.push_back(1.0);
                mx.set(0, col, vals.begin(), vals.end());
            }
        }
        cout << "column size: " << colsize << "  duration: " << sw.get_duration() << " sec" << endl;
    }

    cout << "--- empty on creation (per column, pre-filled array)" << endl;
    for (size_t colsize = 1; colsize <= 5; ++colsize)
    {
        // Fill the data array before insertion.
        vector<double> vals;
        vals.reserve(rowsize);
        for (size_t row = 0; row < rowsize; ++row)
            vals.push_back(1.0);

        stack_watch sw;
        for (size_t i = 0; i < obj_count; ++i)
        {
            mtx_type mx(rowsize, colsize);
            for (size_t col = 0; col < colsize; ++col)
                mx.set(0, col, vals.begin(), vals.end());
        }
        cout << "column size: " << colsize << "  duration: " << sw.get_duration() << " sec" << endl;
    }

    cout << endl;
}

int main (int argc, char **argv)
{
    cmd_options opt;
    if (!parse_cmd_options(argc, argv, opt))
        return EXIT_FAILURE;

    if (opt.test_func)
    {
        mtm_test_construction();
        mtm_test_data_insertion();
        mtm_test_set_empty();
    }

    if (opt.test_perf)
    {
        mtm_perf_test_storage_creation();
        mtm_perf_test_storage_set_numeric();
    }

    return EXIT_SUCCESS;
}
