/*************************************************************************
 *
 * Copyright (c) 2012-2018 Kohei Yoshida
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

#include "test_global.hpp" // This must be the first header to be included.

#include <mdds/multi_type_matrix.hpp>
#include <mdds/multi_type_vector_custom_func1.hpp>

#include <string>
#include <ostream>
#include <functional>

using namespace mdds;
using namespace std;

// Standard matrix that uses std::string as its string type.
typedef multi_type_matrix<mtm::std_string_trait> mtx_type;

// Custom string code --------------------------------------------------------

class custom_string
{
    string m_val;

public:
    custom_string()
    {}
    custom_string(const string& val) : m_val(val)
    {}
    custom_string(const custom_string& r) : m_val(r.m_val)
    {}
    const string& get() const
    {
        return m_val;
    }
    bool operator==(const custom_string& r) const
    {
        return m_val == r.m_val;
    }
    bool operator!=(const custom_string& r) const
    {
        return !operator==(r);
    }
};

ostream& operator<<(ostream& os, const custom_string& str)
{
    os << str.get();
    return os;
}

const mtv::element_t element_type_custom_string = mdds::mtv::element_type_user_start;
typedef mtv::default_element_block<element_type_custom_string, custom_string> custom_string_block;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(custom_string, element_type_custom_string, custom_string(), custom_string_block)

struct custom_string_trait
{
    typedef mdds::mtv::int32_element_block integer_element_block;
    typedef custom_string_block string_element_block;

    typedef mtv::custom_block_func1<string_element_block> element_block_func;
};

typedef mdds::multi_type_matrix<custom_string_trait> mtx_custom_type;

namespace {

template<typename _T>
void check_value(mtx_type& mtx, size_t row, size_t col, const _T& val)
{
    mtx.set(row, col, val);
    _T test = mtx.get<_T>(row, col);
    assert(test == val);
}

bool check_copy(const mtx_type& mx1, const mtx_type& mx2)
{
    size_t row_count = min(mx1.size().row, mx2.size().row);
    size_t col_count = min(mx1.size().column, mx2.size().column);
    for (size_t i = 0; i < row_count; ++i)
    {
        for (size_t j = 0; j < col_count; ++j)
        {
            mtm::element_t elem_type = mx1.get_type(i, j);
            if (elem_type != mx2.get_type(i, j))
            {
                cout << "check_copy: (row=" << i << ",column=" << j << ") element types differ." << endl;
                return false;
            }

            switch (elem_type)
            {
                case mtm::element_boolean:
                    if (mx1.get<bool>(i, j) != mx2.get<bool>(i, j))
                    {
                        cout << "check_copy: (row=" << i << ",column=" << j << ") different boolean values." << endl;
                        return false;
                    }
                    break;
                case mtm::element_numeric:
                    if (mx1.get<double>(i, j) != mx2.get<double>(i, j))
                    {
                        cout << "check_copy: (row=" << i << ",column=" << j << ") different numeric values." << endl;
                        return false;
                    }
                    break;
                case mtm::element_string:
                    if (mx1.get<mtx_type::string_type>(i, j) != mx2.get<mtx_type::string_type>(i, j))
                    {
                        cout << "check_copy: (row=" << i << ",column=" << j << ") different string values." << endl;
                        return false;
                    }
                    break;
                case mtm::element_empty:
                default:;
            }
        }
    }
    return true;
}

} // namespace

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
        assert(mtx.get_type(0, 0) == mtm::element_string);
        assert(mtx.get_string(0, 0) == "foo");
        assert(mtx.get_type(1, 4) == mtm::element_string);
        assert(mtx.get_string(1, 4) == "foo");
    }

    {
        // construct with an array of data.
        vector<double> vals;
        vals.push_back(1.1);
        vals.push_back(1.2);
        vals.push_back(1.3);
        vals.push_back(1.4);
        mtx_type mtx(2, 2, vals.begin(), vals.end());
        mtx_type::size_pair_type sz = mtx.size();
        assert(sz.row == 2 && sz.column == 2);
        assert(mtx.get_numeric(0, 0) == 1.1);
        assert(mtx.get_numeric(1, 0) == 1.2);
        assert(mtx.get_numeric(0, 1) == 1.3);
        assert(mtx.get_numeric(1, 1) == 1.4);

        try
        {
            mtx_type mtx2(3, 2, vals.begin(), vals.end());
            assert(!"Construction of this matrix should have failed!");
        }
        catch (const invalid_arg_error& e)
        {
            // Good.
            cout << "exception caught (as expected) which says: " << e.what() << endl;
        }

        try
        {
            // Trying to initialize a matrix with array of unsupported data
            // type should end with an exception thrown.
            vector<int8_t> vals_ptr(4, 22);
            mtx_type mtx3(2, 2, vals_ptr.begin(), vals_ptr.end());
            assert(!"Construction of this matrix should have failed!");
        }
        catch (const exception& e)
        {
            cout << "exception caught (as expected) which says: " << e.what() << endl;
        }
    }

    {
        // Construct with an array of custom string type.
        vector<custom_string> vals;
        vals.push_back(custom_string("A"));
        vals.push_back(custom_string("B"));
        vals.push_back(custom_string("C"));
        vals.push_back(custom_string("D"));
        mtx_custom_type mtx(1, 4, vals.begin(), vals.end());
        assert(mtx.get_string(0, 0).get() == "A");
        assert(mtx.get_string(0, 1).get() == "B");
        assert(mtx.get_string(0, 2).get() == "C");
        assert(mtx.get_string(0, 3).get() == "D");
        assert(mtx.get_type(0, 0) == mtm::element_string);
        assert(mtx.get_type(0, 1) == mtm::element_string);
        assert(mtx.get_type(0, 2) == mtm::element_string);
        assert(mtx.get_type(0, 3) == mtm::element_string);
    }

    {
        // Construct size_pair_type from initializer list.
        mtx_type::size_pair_type sz({3, 4});
        assert(sz.row == 3);
        assert(sz.column == 4);
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
        assert(!mtx.empty());
        assert(mtx.get_type(0, 0) == mtm::element_empty);
        assert(mtx.get_type(2, 3) == mtm::element_empty);
        check_value(mtx, 1, 1, 1.2);
        check_value(mtx, 2, 1, true);
        check_value(mtx, 3, 1, false);
        check_value(mtx, 0, 2, string("foo"));
        check_value(mtx, 1, 2, 23.4);

        // Overwrite
        assert(mtx.get_type(1, 1) == mtm::element_numeric);
        check_value(mtx, 1, 1, string("baa"));

        // Setting empty.
        assert(mtx.get_type(1, 1) == mtm::element_string);
        mtx.set_empty(1, 1);
        assert(mtx.get_type(1, 1) == mtm::element_empty);

        mtx.clear();
        assert(mtx.size().row == 0);
        assert(mtx.size().column == 0);
        assert(mtx.empty());
    }
}

void mtm_test_data_insertion_multiple()
{
    stack_printer __stack_printer__("::mtm_test_data_insertion_multiple");
    {
        mtx_type mtx(3, 5);

        // data shorter than column length
        vector<double> vals;
        vals.push_back(1.1);
        vals.push_back(1.2);
        mtx.set_column(2, vals.begin(), vals.end());
        assert(mtx.get_numeric(0, 2) == 1.1);
        assert(mtx.get_numeric(1, 2) == 1.2);
        assert(mtx.get_type(2, 2) == mtm::element_empty);

        // data exatly at column length
        vals.clear();
        vals.push_back(2.1);
        vals.push_back(2.2);
        vals.push_back(2.3);
        mtx.set_column(2, vals.begin(), vals.end());
        assert(mtx.get_numeric(0, 2) == 2.1);
        assert(mtx.get_numeric(1, 2) == 2.2);
        assert(mtx.get_numeric(2, 2) == 2.3);
        assert(mtx.get_type(0, 3) == mtm::element_empty);

        // data longer than column length.  The excess data should be ignored.
        vals.clear();
        vals.push_back(3.1);
        vals.push_back(3.2);
        vals.push_back(3.3);
        vals.push_back(3.4);
        mtx.set_column(2, vals.begin(), vals.end());
        assert(mtx.get_numeric(0, 2) == 3.1);
        assert(mtx.get_numeric(1, 2) == 3.2);
        assert(mtx.get_numeric(2, 2) == 3.3);
        assert(mtx.get_type(0, 3) == mtm::element_empty);
    }
}

void mtm_test_data_insertion_integer()
{
    stack_printer __stack_printer__("::mtm_test_data_insertion_integer");

    // +--------+--------+
    // |  int   | double |
    // +--------+--------+
    // | double |  int   |
    // +--------+--------+

    mtx_type mtx(2, 2);
    mtx.set(0, 0, int(0));
    mtx.set(1, 1, int(22));
    mtx.set(0, 1, double(10));
    mtx.set(1, 0, double(22));

    assert(mtx.get_type(0, 0) == mtm::element_integer);
    assert(mtx.get_type(1, 1) == mtm::element_integer);
    assert(mtx.get_type(0, 1) == mtm::element_numeric);
    assert(mtx.get_type(1, 0) == mtm::element_numeric);

    assert(mtx.get_integer(0, 0) == 0);
    assert(mtx.get_integer(1, 1) == 22);
    assert(mtx.get_integer(0, 1) == 10);
    assert(mtx.get_integer(1, 0) == 22);

    assert(mtx.get_numeric(0, 0) == 0.0);
    assert(mtx.get_numeric(1, 1) == 22.0);
    assert(mtx.get_numeric(0, 1) == 10.0);
    assert(mtx.get_numeric(1, 0) == 22.0);

    assert(mtx.get_boolean(0, 0) == false);
    assert(mtx.get_boolean(1, 1) == true);
    assert(mtx.get_boolean(0, 1) == true);
    assert(mtx.get_boolean(1, 0) == true);

    assert(mtx.numeric()); // integers are considered numeric.

    assert(mtx.get<int>(0, 0) == 0);
    assert(mtx.get<int>(1, 1) == 22);

    mtx_type::position_type pos = mtx.position(0, 1);
    mtx.set(pos, int(987));
    assert(mtx.get<int>(0, 1) == 987);

    // +--------+--------+
    // |  int   |  int   |
    // +--------+--------+
    // | double |  int   |
    // +--------+--------+

    vector<mtx_type::element_block_node_type> nodes;

    std::function<void(const mtx_type::element_block_node_type&)> f =
        [&nodes](const mtx_type::element_block_node_type& node) { nodes.push_back(node); };

    mtx.walk(f);

    assert(nodes.size() == 3);
    assert(nodes[0].type == mtm::element_integer);
    assert(nodes[0].size == 1);
    assert(nodes[1].type == mtm::element_numeric);
    assert(nodes[1].size == 1);
    assert(nodes[2].type == mtm::element_integer);
    assert(nodes[2].size == 2);

    {
        auto it = nodes[2].begin<mtx_type::integer_block_type>();
        auto ite = nodes[2].end<mtx_type::integer_block_type>();
        assert(*it == 987);
        ++it;
        assert(*it == 22);
        ++it;
        assert(it == ite);
    }
}

void mtm_test_set_empty()
{
    stack_printer __stack_printer__("::mtm_test_set_empty");
    {
        // set whole column empty.
        mtx_type mtx(3, 5, 1.2);
        cout << "setting whole column 2 empty..." << endl;
        mtx.set_column_empty(2);
        assert(mtx.get_type(0, 1) != mtm::element_empty);
        assert(mtx.get_type(1, 1) != mtm::element_empty);
        assert(mtx.get_type(2, 1) != mtm::element_empty);
        assert(mtx.get_type(0, 2) == mtm::element_empty);
        assert(mtx.get_type(1, 2) == mtm::element_empty);
        assert(mtx.get_type(2, 2) == mtm::element_empty);
        assert(mtx.get_type(0, 3) != mtm::element_empty);
        assert(mtx.get_type(1, 3) != mtm::element_empty);
        assert(mtx.get_type(2, 3) != mtm::element_empty);
    }

    {
        // set whole row empty.
        mtx_type mtx(3, 5, 1.2);
        cout << "setting whole row 1 empty..." << endl;
        mtx.set_row_empty(1);
        assert(mtx.get_type(0, 0) != mtm::element_empty);
        assert(mtx.get_type(0, 1) != mtm::element_empty);
        assert(mtx.get_type(0, 2) != mtm::element_empty);
        assert(mtx.get_type(0, 3) != mtm::element_empty);
        assert(mtx.get_type(0, 4) != mtm::element_empty);
        assert(mtx.get_type(1, 0) == mtm::element_empty);
        assert(mtx.get_type(1, 1) == mtm::element_empty);
        assert(mtx.get_type(1, 2) == mtm::element_empty);
        assert(mtx.get_type(1, 3) == mtm::element_empty);
        assert(mtx.get_type(1, 4) == mtm::element_empty);
        assert(mtx.get_type(2, 0) != mtm::element_empty);
        assert(mtx.get_type(2, 1) != mtm::element_empty);
        assert(mtx.get_type(2, 2) != mtm::element_empty);
        assert(mtx.get_type(2, 3) != mtm::element_empty);
        assert(mtx.get_type(2, 4) != mtm::element_empty);
    }

    {
        // Set a range of elements empty.
        mtx_type mtx(5, 3, string("A"));
        cout << "setting element (0,1) to (1,2) empty..." << endl;
        mtx.set_empty(1, 0, 6); // rows 1-4 in column 0 and rows 0-1 in column 1.
        assert(mtx.get_type(0, 0) == mtm::element_string);
        assert(mtx.get_type(1, 0) == mtm::element_empty);
        assert(mtx.get_type(2, 0) == mtm::element_empty);
        assert(mtx.get_type(3, 0) == mtm::element_empty);
        assert(mtx.get_type(4, 0) == mtm::element_empty);
        assert(mtx.get_type(0, 1) == mtm::element_empty);
        assert(mtx.get_type(1, 1) == mtm::element_empty);
        assert(mtx.get_type(2, 1) == mtm::element_string);
        assert(mtx.get_type(3, 1) == mtm::element_string);
        assert(mtx.get_type(4, 1) == mtm::element_string);
        assert(mtx.get_type(0, 2) == mtm::element_string);
        assert(mtx.get_type(1, 2) == mtm::element_string);
        assert(mtx.get_type(2, 2) == mtm::element_string);
        assert(mtx.get_type(3, 2) == mtm::element_string);
        assert(mtx.get_type(4, 2) == mtm::element_string);

        try
        {
            mtx.set_empty(2, 2, 0);
            assert(false);
        }
        catch (const std::exception&)
        {
            cout << "exception thrown on length of zero as expected" << endl;
        }
    }
}

void mtm_test_swap()
{
    stack_printer __stack_printer__("::mtm_test_swap");
    mtx_type mtx1(3, 6), mtx2(7, 2);
    mtx1.set(0, 0, 1.1);
    mtx1.set(2, 5, 1.9);
    mtx2.set(0, 0, 2.1);
    mtx2.set(6, 1, 2.9);
    mtx1.swap(mtx2);

    assert(mtx1.size().row == 7);
    assert(mtx1.size().column == 2);
    assert(mtx1.get<double>(0, 0) == 2.1);
    assert(mtx1.get<double>(6, 1) == 2.9);

    assert(mtx2.size().row == 3);
    assert(mtx2.size().column == 6);
    assert(mtx2.get<double>(0, 0) == 1.1);
    assert(mtx2.get<double>(2, 5) == 1.9);
}

void mtm_test_transpose()
{
    stack_printer __stack_printer__("::mtm_test_transpose");
    mtx_type mtx(3, 6);
    mtx.set(0, 0, 1.1);
    mtx.set(1, 0, 1.2);
    mtx.set(2, 0, 1.3);
    mtx.set(1, 5, string("foo"));
    mtx.set(2, 3, true);
    mtx.transpose();
    assert(mtx.size().row == 6);
    assert(mtx.size().column == 3);
    assert(mtx.get<double>(0, 0) == 1.1);
    assert(mtx.get<double>(0, 1) == 1.2);
    assert(mtx.get<double>(0, 2) == 1.3);
    assert(mtx.get<string>(5, 1) == "foo");
    assert(mtx.get<bool>(3, 2) == true);
}

void mtm_test_resize()
{
    stack_printer __stack_printer__("::mtm_test_resize");
    mtx_type mtx(0, 0);
    assert(mtx.size().row == 0);
    assert(mtx.size().column == 0);
    assert(mtx.empty());

    mtx.resize(1, 3);
    assert(mtx.size().row == 1);
    assert(mtx.size().column == 3);
    assert(!mtx.empty());
    assert(mtx.get_type(0, 0) == mtm::element_empty);
    assert(mtx.get_type(0, 1) == mtm::element_empty);
    assert(mtx.get_type(0, 2) == mtm::element_empty);

    mtx.set(0, 0, 1.1);
    mtx.set(0, 1, string("foo"));
    mtx.set(0, 2, true);
    assert(mtx.get<double>(0, 0) == 1.1);
    assert(mtx.get<string>(0, 1) == "foo");
    assert(mtx.get<bool>(0, 2) == true);

    // This shouldn't alter the original content.
    mtx.resize(2, 4);
    assert(mtx.size().row == 2);
    assert(mtx.size().column == 4);
    assert(mtx.get<double>(0, 0) == 1.1);
    assert(mtx.get<string>(0, 1) == "foo");
    assert(mtx.get<bool>(0, 2) == true);
    assert(mtx.get_type(1, 3) == mtm::element_empty);

    mtx.resize(2, 2);
    assert(mtx.size().row == 2);
    assert(mtx.size().column == 2);
    assert(mtx.get<double>(0, 0) == 1.1);
    assert(mtx.get<string>(0, 1) == "foo");
    assert(mtx.get_type(1, 0) == mtm::element_empty);
    assert(mtx.get_type(1, 1) == mtm::element_empty);

    // Three ways to resize to empty matrix.
    mtx.resize(2, 0);
    assert(mtx.size().row == 0);
    assert(mtx.size().column == 0);

    mtx.resize(2, 2);
    mtx.resize(0, 2);
    assert(mtx.size().row == 0);
    assert(mtx.size().column == 0);

    mtx.resize(2, 2);
    mtx.resize(0, 0);
    assert(mtx.size().row == 0);
    assert(mtx.size().column == 0);

    // Resize with initial value when the matrix becomes larger.
    mtx.resize(3, 2, 12.5);
    assert(mtx.size().row == 3);
    assert(mtx.size().column == 2);
    assert(mtx.get<double>(2, 1) == 12.5);
    assert(mtx.get<double>(2, 0) == 12.5);
    assert(mtx.get<double>(0, 0) == 12.5);
    assert(mtx.get<double>(0, 1) == 12.5);

    // The initial value should be ignored when shrinking.
    mtx.resize(2, 1, true);
    assert(mtx.size().row == 2);
    assert(mtx.size().column == 1);
    assert(mtx.get<double>(1, 0) == 12.5);

    // Resize again with initial value of different type.
    mtx.resize(3, 2, string("extra"));
    assert(mtx.size().row == 3);
    assert(mtx.size().column == 2);
    assert(mtx.get<double>(0, 0) == 12.5);
    assert(mtx.get<double>(1, 0) == 12.5);
    assert(mtx.get<string>(2, 1) == "extra");
    assert(mtx.get<string>(2, 0) == "extra");
    assert(mtx.get<string>(1, 1) == "extra");
    assert(mtx.get<string>(0, 1) == "extra");
}

void mtm_test_copy()
{
    stack_printer __stack_printer__("::mtm_test_copy");

    // Assigning from a smaller matrix to a bigger one.
    mtx_type mx1(5, 5), mx2(2, 2);
    mx2.set(0, 0, 1.2);
    mx2.set(1, 1, true);
    mx2.set(0, 1, string("test"));
    mx2.set(1, 0, string("foo"));
    mx1.copy(mx2);

    bool success = check_copy(mx1, mx2);
    assert(success);

    mx2.resize(8, 8);
    mx2.copy(mx1);

    success = check_copy(mx1, mx2);
    assert(success);

    // from a larger matrix to a smaller one.
    mx1.set(0, 0, string("test1"));
    mx2.set(0, 0, string("test2"));
    mx2.set(4, 4, true);
    mx2.set(7, 7, false);
    mx1.copy(mx2);
    success = check_copy(mx1, mx2);
    assert(success);

    // self assignment (should be no-op).
    mx1.copy(mx1);
    success = check_copy(mx1, mx1);
    assert(success);

    mx2.copy(mx2);
    success = check_copy(mx2, mx2);
    assert(success);
}

void mtm_test_copy_empty_destination()
{
    stack_printer __stack_printer__("::mtm_test_copy_empty_destination");

    mtx_type mx1, mx2(1, 1);
    mx1.copy(mx2); // This should not throw.

    mx2.copy(mx1); // This should not throw either.
}

void mtm_test_copy_from_array()
{
    stack_printer __stack_printer__("::mtm_test_copy_from_array");

    vector<double> src;
    src.reserve(9);
    for (size_t i = 0; i < 9; ++i)
        src.push_back(double(i));

    mtx_type mx(4, 4);
    mx.copy(3, 3, src.begin(), src.end());

    assert(mx.get<double>(0, 0) == 0.0);
    assert(mx.get<double>(1, 0) == 1.0);
    assert(mx.get<double>(2, 0) == 2.0);
    assert(mx.get<double>(0, 1) == 3.0);
    assert(mx.get<double>(1, 1) == 4.0);
    assert(mx.get<double>(2, 1) == 5.0);
    assert(mx.get<double>(0, 2) == 6.0);
    assert(mx.get<double>(1, 2) == 7.0);
    assert(mx.get<double>(2, 2) == 8.0);
    assert(mx.get_type(3, 0) == mtm::element_empty);
    assert(mx.get_type(3, 1) == mtm::element_empty);
    assert(mx.get_type(3, 2) == mtm::element_empty);
    assert(mx.get_type(3, 3) == mtm::element_empty);
    assert(mx.get_type(0, 3) == mtm::element_empty);
    assert(mx.get_type(1, 3) == mtm::element_empty);
    assert(mx.get_type(2, 3) == mtm::element_empty);
    assert(mx.get_type(3, 3) == mtm::element_empty);

    vector<std::string> src2;
    src2.reserve(4);
    src2.push_back("A");
    src2.push_back("B");
    src2.push_back("C");
    src2.push_back("D");

    mx.copy(2, 2, src2.begin(), src2.end());

    assert(mx.get<std::string>(0, 0) == "A");
    assert(mx.get<std::string>(1, 0) == "B");
    assert(mx.get<std::string>(0, 1) == "C");
    assert(mx.get<std::string>(1, 1) == "D");
    assert(mx.get<double>(2, 0) == 2.0);
    assert(mx.get<double>(2, 1) == 5.0);
    assert(mx.get<double>(0, 2) == 6.0);
    assert(mx.get<double>(1, 2) == 7.0);
    assert(mx.get<double>(2, 2) == 8.0);
    assert(mx.get_type(3, 0) == mtm::element_empty);
    assert(mx.get_type(3, 1) == mtm::element_empty);
    assert(mx.get_type(3, 2) == mtm::element_empty);
    assert(mx.get_type(3, 3) == mtm::element_empty);
    assert(mx.get_type(0, 3) == mtm::element_empty);
    assert(mx.get_type(1, 3) == mtm::element_empty);
    assert(mx.get_type(2, 3) == mtm::element_empty);
    assert(mx.get_type(3, 3) == mtm::element_empty);

    vector<bool> src3;
    src3.push_back(true);
    src3.push_back(false);
    src3.push_back(true);
    src3.push_back(false);

    mx.copy(4, 1, src3.begin(), src3.end());
    assert(mx.get<bool>(0, 0) == true);
    assert(mx.get<bool>(1, 0) == false);
    assert(mx.get<bool>(2, 0) == true);
    assert(mx.get<bool>(3, 0) == false);

    // Try to copy from an array of invalid type.
    vector<int8_t> src_invalid;
    src_invalid.push_back('a');
    src_invalid.push_back('b');

    try
    {
        mx.copy(2, 1, src_invalid.begin(), src_invalid.end());
        assert(!"type_error did not get thrown.");
    }
    catch (const mdds::type_error& e)
    {
        cout << "expected exception was thrown: '" << e.what() << "'" << endl;
    }
}

void mtm_test_assignment()
{
    stack_printer __stack_printer__("::mtm_test_assignment");
    mtx_type mx_orig(5, 5, 1.2);
    mtx_type mx_copied = mx_orig;
    assert(mx_orig == mx_copied);

    mx_copied = mx_copied; // self assignment.
    assert(mx_orig == mx_copied);

    mx_orig.set(2, 3, true);
    mx_orig.set(1, 1, string("foo"));
    mx_copied = mx_orig;
    assert(mx_orig == mx_copied);
}

void mtm_test_numeric()
{
    // Numeric elements only matrix is numeric.
    mtx_type mtx(2, 2, 1.1);
    assert(mtx.numeric());

    // Boolean element is numeric.
    mtx.set(0, 0, true);
    assert(mtx.numeric());

    // String element is not.
    mtx.set(1, 0, string("foo"));
    assert(!mtx.numeric());

    mtx.set(1, 0, 1.3);
    assert(mtx.numeric());

    // Empty element is not numeric.
    mtx.set_empty(1, 1);
    assert(!mtx.numeric());

    // Empty matrix is not numeric.
    mtx.clear();
    assert(!mtx.numeric());
}

void mtm_test_custom_string()
{
    stack_printer __stack_printer__("::mtm_test_custom_string");
    mtx_custom_type mtx(2, 2);
    mtx.set(0, 0, custom_string("foo"));
    assert(mtx.get_type(0, 0) == mtm::element_string);
    assert(mtx.get<custom_string>(0, 0) == custom_string("foo"));
    mtx.set(1, 1, 12.3);
    assert(mtx.get<double>(1, 1) == 12.3);
}

void mtm_test_position()
{
    stack_printer __stack_printer__("::mtm_test_position");
    mtx_type mtx(3, 2);
    mtx.set(0, 0, 1.0);
    mtx.set(0, 1, string("foo"));
    mtx.set(1, 0, 2.0);
    mtx.set(1, 1, 2.1);
    mtx.set(2, 0, true);
    mtx.set(2, 1, false);

    assert(mtx.get_type(0, 0) == mtm::element_numeric);
    assert(mtx.get_type(0, 1) == mtm::element_string);
    assert(mtx.get_type(1, 0) == mtm::element_numeric);
    assert(mtx.get_type(1, 1) == mtm::element_numeric);
    assert(mtx.get_type(2, 0) == mtm::element_boolean);
    assert(mtx.get_type(2, 1) == mtm::element_boolean);

    mtx_type::position_type pos = mtx.position(1, 1);
    assert(mtx.get_type(pos) == mtm::element_numeric);
    assert(mtx.get_numeric(pos) == 2.1);

    pos = mtx.position(2, 0);
    assert(mtx.get_type(pos) == mtm::element_boolean);
    assert(mtx.get_boolean(pos) == true);

    pos = mtx.position(0, 1);
    assert(mtx.get_type(pos) == mtm::element_string);
    assert(mtx.get_string(pos) == "foo");

    mtx.set_empty(pos);
    assert(mtx.get_type(0, 1) == mtm::element_empty);

    pos = mtx.position(1, 1);
    mtx.set(pos, false);
    assert(mtx.get_type(1, 1) == mtm::element_boolean);
    assert(mtx.get_boolean(1, 1) == false);

    pos = mtx.position(2, 0);
    mtx.set(pos, 12.3);
    assert(mtx.get_type(2, 0) == mtm::element_numeric);
    assert(mtx.get_numeric(2, 0) == 12.3);

    pos = mtx.position(2, 1);
    mtx.set(pos, string("ABC"));
    assert(mtx.get_type(2, 1) == mtm::element_string);
    assert(mtx.get_string(2, 1) == "ABC");

    // Start over, and test the traversal of position object.
    pos = mtx.position(0, 0);
    mtx_type::size_pair_type mtx_pos = mtx.matrix_position(pos);
    assert(mtx_pos.column == 0);
    assert(mtx_pos.row == 0);

    pos = mtx_type::next_position(pos);
    mtx_pos = mtx.matrix_position(pos);
    assert(mtx_pos.column == 0);
    assert(mtx_pos.row == 1);

    pos = mtx_type::next_position(pos);
    mtx_pos = mtx.matrix_position(pos);
    assert(mtx_pos.column == 0);
    assert(mtx_pos.row == 2);

    pos = mtx_type::next_position(pos);
    mtx_pos = mtx.matrix_position(pos);
    assert(mtx_pos.column == 1);
    assert(mtx_pos.row == 0);

    pos = mtx_type::next_position(pos);
    mtx_pos = mtx.matrix_position(pos);
    assert(mtx_pos.column == 1);
    assert(mtx_pos.row == 1);

    pos = mtx_type::next_position(pos);
    mtx_pos = mtx.matrix_position(pos);
    assert(mtx_pos.column == 1);
    assert(mtx_pos.row == 2);

    pos = mtx_type::next_position(pos);
    assert(pos == mtx.end_position());
}

void mtm_test_set_data_via_position()
{
    stack_printer __stack_printer__("::mtm_test_set_data_via_position");
    mtx_type mtx(5, 4);
    mtx_type::position_type pos = mtx.position(0, 1);
    vector<double> data;
    data.push_back(1.1);
    data.push_back(1.2);
    data.push_back(1.3);
    data.push_back(1.4);
    data.push_back(1.5);
    pos = mtx.set(pos, data.begin(), data.end());
    assert(mtx.get<double>(0, 1) == 1.1);
    assert(mtx.get<double>(1, 1) == 1.2);
    assert(mtx.get<double>(2, 1) == 1.3);
    assert(mtx.get<double>(3, 1) == 1.4);
    assert(mtx.get<double>(4, 1) == 1.5);

    mtx_type::size_pair_type mtx_pos = mtx.matrix_position(pos);
    assert(mtx_pos.row == 0);
    assert(mtx_pos.column == 1);
    pos = mtx.position(pos, 0, 2);
    pos = mtx.set(pos, string("test"));
    pos = mtx_type::next_position(pos);
    pos = mtx.set(pos, true);
    assert(mtx.get<string>(0, 2) == "test");
    assert(mtx.get<bool>(1, 2) == true);
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

struct sum_all_values
{
    double result;

    sum_all_values() : result(0.0)
    {}

    void operator()(const mtx_type::element_block_node_type& blk)
    {
        mtv::double_element_block::const_iterator it = mtv::double_element_block::cbegin(*blk.data);
        mtv::double_element_block::const_iterator it_end = mtv::double_element_block::cend(*blk.data);
        for (; it != it_end; ++it)
            result += *it;
    }
};

void mtm_perf_test_iterate_elements()
{
    cout << "measuring performance on iterating over all numeric elements." << endl;
    size_t rowsize = 100000;
    size_t colsize = 1000;
    cout << "row size: " << rowsize << "  column size: " << colsize << endl;
    mtx_type mx(rowsize, colsize, 0.0);
    {
        stack_watch sw;
        double val = 1.0;
        std::vector<double> vals;
        vals.reserve(rowsize * colsize);
        for (size_t i = 0; i < rowsize; ++i)
        {
            for (size_t j = 0; j < colsize; ++j)
            {
                vals.push_back(val);
                val += 0.001;
            }
        }
        mx.set(0, 0, vals.begin(), vals.end());
        cout << "element values inserted.  (duration: " << sw.get_duration() << " sec)" << endl;
    }

    {
        stack_watch sw;
        sum_all_values func;
        mx.walk(func);
        double val = func.result;
        cout << "all element values added.  (answer: " << val << ")  (duration: " << sw.get_duration() << " sec)"
             << endl;
    }
}

void mtm_perf_test_insert_via_position_object()
{
    size_t rowsize = 3000, colsize = 3000;
    {
        stack_printer __stack_printer__("::mtm_perf_test_insert_via_position_object (column and row positions)");
        mtx_type mx(rowsize, colsize);
        for (size_t i = 0; i < rowsize; ++i)
        {
            for (size_t j = 0; j < colsize; ++j)
            {
                mx.set(i, j, 1.1);
            }
        }
    }

    {
        stack_printer __stack_printer__("::mtm_perf_test_insert_via_position_object (position object)");
        mtx_type mx(rowsize, colsize);
        mtx_type::position_type pos = mx.position(0, 0);
        for (size_t i = 0; i < rowsize; ++i)
        {
            for (size_t j = 0; j < colsize; ++j)
            {
                pos = mx.set(pos, 1.1);
                pos = mtx_type::next_position(pos);
            }
        }
    }

    {
        stack_printer __stack_printer__("::mtm_perf_test_insert_via_position_object (position object)");
        mtx_type mx(rowsize, colsize);
        mtx_type::position_type pos = mx.position(0, 0);
        for (; pos != mx.end_position(); pos = mtx_type::next_position(pos))
            pos = mx.set(pos, 1.1);
    }
}

int main(int argc, char** argv)
{
    try
    {
        cmd_options opt;
        if (!parse_cmd_options(argc, argv, opt))
            return EXIT_FAILURE;

        if (opt.test_func)
        {
            mtm_test_construction();
            mtm_test_data_insertion();
            mtm_test_data_insertion_multiple();
            mtm_test_data_insertion_integer();
            mtm_test_set_empty();
            mtm_test_swap();
            mtm_test_transpose();
            mtm_test_resize();
            mtm_test_copy();
            mtm_test_copy_empty_destination();
            mtm_test_copy_from_array();
            mtm_test_assignment();
            mtm_test_numeric();
            mtm_test_custom_string();
            mtm_test_position();
            mtm_test_set_data_via_position();
        }

        if (opt.test_perf)
        {
            mtm_perf_test_storage_creation();
            mtm_perf_test_storage_set_numeric();
            mtm_perf_test_iterate_elements();
            mtm_perf_test_insert_via_position_object();
        }
    }
    catch (const std::exception& e)
    {
        fprintf(stdout, "Test failed: %s\n", e.what());
        return EXIT_FAILURE;
    }

    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}
