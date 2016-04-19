/*************************************************************************
 *
 * Copyright (c) 2012-2013 Kohei Yoshida
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
#include <mdds/multi_type_vector_custom_func1.hpp>

#include <string>
#include <ostream>

using namespace mdds;
using namespace std;

// Standard matrix that uses std::string as its string type.
typedef multi_type_matrix<mtm::std_string_trait> mtx_type;

// Custom string code --------------------------------------------------------

class custom_string
{
    string m_val;
public:
    custom_string() {}
    custom_string(const string& val) : m_val(val) {}
    custom_string(const custom_string& r) : m_val(r.m_val) {}
    const string& get() const { return m_val; }
    bool operator== (const custom_string& r) const { return m_val == r.m_val; }
    bool operator!= (const custom_string& r) const { return !operator==(r); }
};

ostream& operator<< (ostream& os, const custom_string& str)
{
    os << str.get();
    return os;
}

const mtv::element_t element_type_custom_string = mdds::mtv::element_type_user_start;
typedef mtv::default_element_block<element_type_custom_string, custom_string> custom_string_block;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(custom_string, element_type_custom_string, custom_string(), custom_string_block)

struct custom_string_trait
{
    typedef custom_string string_type;
    typedef custom_string_block string_element_block;

    static const mdds::mtv::element_t string_type_identifier = element_type_custom_string;

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
    size_t row_count = min(mx1.size().row,  mx2.size().row);
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
                default:
                    ;
            }
        }
    }
    return true;
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
        assert(mtx.get_type(0,0) == mtm::element_string);
        assert(mtx.get_string(0,0) == "foo");
        assert(mtx.get_type(1,4) == mtm::element_string);
        assert(mtx.get_string(1,4) == "foo");
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
        assert(mtx.get_numeric(0,0) == 1.1);
        assert(mtx.get_numeric(1,0) == 1.2);
        assert(mtx.get_numeric(0,1) == 1.3);
        assert(mtx.get_numeric(1,1) == 1.4);

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
            vector<size_t> vals_ptr(4, 22);
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
        assert(mtx.get_string(0,0).get() == "A");
        assert(mtx.get_string(0,1).get() == "B");
        assert(mtx.get_string(0,2).get() == "C");
        assert(mtx.get_string(0,3).get() == "D");
        assert(mtx.get_type(0,0) == mtm::element_string);
        assert(mtx.get_type(0,1) == mtm::element_string);
        assert(mtx.get_type(0,2) == mtm::element_string);
        assert(mtx.get_type(0,3) == mtm::element_string);
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
        assert(mtx.get_type(0,0) == mtm::element_empty);
        assert(mtx.get_type(2,3) == mtm::element_empty);
        check_value(mtx, 1, 1, 1.2);
        check_value(mtx, 2, 1, true);
        check_value(mtx, 3, 1, false);
        check_value(mtx, 0, 2, string("foo"));
        check_value(mtx, 1, 2, 23.4);

        // Overwrite
        assert(mtx.get_type(1,1) == mtm::element_numeric);
        check_value(mtx, 1, 1, string("baa"));

        // Setting empty.
        assert(mtx.get_type(1,1) == mtm::element_string);
        mtx.set_empty(1, 1);
        assert(mtx.get_type(1,1) == mtm::element_empty);

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

template<typename _T>
struct print_element : std::unary_function<_T, void>
{
    void operator() (const _T& v) const
    {
        cout << v << endl;
    }
};

class walk_element_block : std::unary_function<mtx_type::element_block_node_type, void>
{
public:
    void operator() (const mtx_type::element_block_node_type& node)
    {
        switch (node.type)
        {
            case mtm::element_boolean:
            {
                mtx_type::boolean_block_type::const_iterator it = mtx_type::boolean_block_type::begin(*node.data);
                std::advance(it, node.offset);
                mtx_type::boolean_block_type::const_iterator it_end = it;
                std::advance(it_end, node.size);
                std::for_each(it, it_end, print_element<bool>());
            }
            break;
            case mtm::element_string:
            {
                mtx_type::string_block_type::const_iterator it = mtx_type::string_block_type::begin(*node.data);
                std::advance(it, node.offset);
                mtx_type::string_block_type::const_iterator it_end = it;
                std::advance(it_end, node.size);
                std::for_each(it, it_end, print_element<mtx_type::string_type>());
            }
            break;
            case mtm::element_numeric:
            {
                mtx_type::numeric_block_type::const_iterator it = mtx_type::numeric_block_type::begin(*node.data);
                std::advance(it, node.offset);
                mtx_type::numeric_block_type::const_iterator it_end = it;
                std::advance(it_end, node.size);
                std::for_each(it, it_end, print_element<double>());
            }
            break;
            case mtm::element_empty:
                cout << "- empty block -" << endl;
            break;
            default:
                ;
        }
    }
};

void mtm_test_walk()
{
    stack_printer __stack_printer__("::mtm_test_walk");
    mtx_type mtx(12, 1); // single column matrix to make it easier.
    mtx.set(2, 0, 1.1);
    mtx.set(3, 0, 1.2);
    mtx.set(4, 0, 1.3);
    mtx.set(5, 0, 1.4);
    mtx.set(7, 0, string("A"));
    mtx.set(8, 0, string("B"));
    mtx.set(9, 0, string("C"));
    mtx.set(10, 0, false);
    mtx.set(11, 0, true);
    walk_element_block func;
    mtx.walk(func);
}

void mtm_test_walk_subset()
{
    {
        stack_printer __stack_printer__("::mtm_test_walk_subset test1");
        mtx_type mtx(4, 4);
        mtx.set(1, 1, 1.1);
        mtx.set(2, 1, 1.2);
        mtx.set(3, 1, 1.3);
        mtx.set(0, 2, string("A1"));
        mtx.set(1, 2, string("A2"));
        mtx.set(2, 2, false);
        walk_element_block func;
        mtx.walk(func, mtx_type::size_pair_type(1,1), mtx_type::size_pair_type(2, 2));
    }
    {
        stack_printer __stack_printer__("::mtm_test_walk_subset test2");
        mtx_type mtx(4, 4);
        mtx.set(0, 1, 1.0);
        mtx.set(1, 1, 1.1);
        mtx.set(0, 2, string("A1"));
        mtx.set(1, 2, string("A2"));
        mtx.set(2, 2, string("A3"));
        mtx.set(3, 2, string("A4"));
        walk_element_block func;
        mtx.walk(func, mtx_type::size_pair_type(1,1), mtx_type::size_pair_type(2, 2));
    }
}

class parallel_walk_element_block : std::binary_function<mtx_type::element_block_node_type, mtx_type::element_block_node_type, void>
{
    std::vector<string> m_ls;
    std::vector<string> m_rs;

    template<typename _Blk>
    void push_to_buffer(const mtx_type::element_block_node_type& node, std::vector<string>& buf)
    {
        auto it = node.begin<_Blk>();
        auto ite = node.end<_Blk>();
        std::for_each(it, ite,
            [&](const typename _Blk::value_type& v)
            {
                ostringstream os;
                os << v;
                buf.push_back(os.str());
            }
        );
    }

    void process_node(const mtx_type::element_block_node_type& node, std::vector<string>& buf)
    {
        switch (node.type)
        {
            case mtm::element_boolean:
                push_to_buffer<mtx_type::boolean_block_type>(node, buf);
            break;
            case mtm::element_string:
                push_to_buffer<mtx_type::string_block_type>(node, buf);
            break;
            case mtm::element_numeric:
                push_to_buffer<mtx_type::numeric_block_type>(node, buf);
            break;
            case mtm::element_empty:
                for (size_t i = 0; i < node.size; ++i)
                    buf.push_back("' '");
            break;
            default:
                ;
        }
    }
public:
    void operator() (const mtx_type::element_block_node_type& left, const mtx_type::element_block_node_type& right)
    {
        cout << "--" << endl;
        cout << "l: offset=" << left.offset << ", size=" << left.size << ", type=" << left.type << endl;
        cout << "r: offset=" << right.offset << ", size=" << right.size << ", type=" << right.type << endl;
        process_node(left, m_ls);
        process_node(right, m_rs);
    }

    std::vector<string> get_concat_buffer() const
    {
        std::vector<string> buf;
        assert(m_ls.size() == m_rs.size());
        auto it = m_ls.begin(), it2 = m_rs.begin();
        auto ite = m_ls.end();
        for (; it != ite; ++it, ++it2)
        {
            ostringstream os;
            os << *it << ":" << *it2;
            buf.push_back(os.str());
        }

        return buf;
    }
};

void mtm_test_parallel_walk()
{
    stack_printer __stack_printer__("::mtm_test_parallel_walk");

    parallel_walk_element_block func;
    mtx_type left(10, 1), right(10, 1, string("'+'"));

    right.set(2, 0, 1.2);
    right.set(8, 0, false);
    right.set(9, 0, true);

    left.set(0, 0, 122.0);
    left.set(4, 0, string("A12"));
    left.set(5, 0, string("A25"));

    left.walk(func, right);

    const char* expected[] = {
        "122:'+'",
        "' ':'+'",
        "' ':1.2",
        "' ':'+'",
        "A12:'+'",
        "A25:'+'",
        "' ':'+'",
        "' ':'+'",
        "' ':0",
        "' ':1",
    };

    size_t n = MDDS_N_ELEMENTS(expected);

    std::vector<string> concat = func.get_concat_buffer();
    assert(concat.size() == n);
    for (size_t i = 0; i < n; ++i)
        assert(concat[i] == expected[i]);
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

    sum_all_values() : result(0.0) {}

    void operator() (const mtx_type::element_block_node_type& blk)
    {
        mtv::numeric_element_block::const_iterator it = mtv::numeric_element_block::begin(*blk.data);
        mtv::numeric_element_block::const_iterator it_end = mtv::numeric_element_block::end(*blk.data);
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
        vals.reserve(rowsize*colsize);
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
        cout << "all element values added.  (answer: " << val << ")  (duration: " << sw.get_duration() << " sec)" << endl;
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

int main (int argc, char **argv)
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
            mtm_test_set_empty();
            mtm_test_swap();
            mtm_test_transpose();
            mtm_test_resize();
            mtm_test_copy();
            mtm_test_assignment();
            mtm_test_numeric();
            mtm_test_walk();
            mtm_test_walk_subset();
            mtm_test_parallel_walk();
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
