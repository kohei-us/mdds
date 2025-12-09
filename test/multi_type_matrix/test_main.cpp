// SPDX-FileCopyrightText: 2012 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.

#include <mdds/multi_type_matrix.hpp>

#include <string>
#include <ostream>
#include <functional>

using namespace mdds;

// Standard matrix that uses std::string as its string type.
typedef multi_type_matrix<mtm::std_string_traits> mtx_type;

// Custom string code --------------------------------------------------------

class custom_string
{
    std::string m_val;

public:
    custom_string()
    {}
    custom_string(const std::string& val) : m_val(val)
    {}
    custom_string(const custom_string& r) = default;
    custom_string& operator=(const custom_string& r) = default;

    const std::string& get() const
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

std::ostream& operator<<(std::ostream& os, const custom_string& str)
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
};

typedef mdds::multi_type_matrix<custom_string_trait> mtx_custom_type;

namespace {

template<typename _T>
void check_value(mtx_type& mtx, size_t row, size_t col, const _T& val)
{
    mtx.set(row, col, val);
    _T test = mtx.get<_T>(row, col);
    TEST_ASSERT(test == val);
}

bool check_copy(const mtx_type& mx1, const mtx_type& mx2)
{
    size_t row_count = std::min(mx1.size().row, mx2.size().row);
    size_t col_count = std::min(mx1.size().column, mx2.size().column);
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
                        std::cout << "check_copy: (row=" << i << ",column=" << j << ") different string values."
                                  << std::endl;
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
    MDDS_TEST_FUNC_SCOPE;

    {
        // default constructor.
        mtx_type mtx;
        mtx_type::size_pair_type sz = mtx.size();
        TEST_ASSERT(sz.row == 0 && sz.column == 0);
    }

    {
        // construction to a specific size.
        mtx_type mtx(2, 5);
        mtx_type::size_pair_type sz = mtx.size();
        TEST_ASSERT(sz.row == 2 && sz.column == 5);
    }

    {
        // construction to a specific size with default value.
        mtx_type mtx(2, 5, std::string("foo"));
        mtx_type::size_pair_type sz = mtx.size();
        TEST_ASSERT(sz.row == 2 && sz.column == 5);
        TEST_ASSERT(mtx.get_type(0, 0) == mtm::element_string);
        TEST_ASSERT(mtx.get_string(0, 0) == "foo");
        TEST_ASSERT(mtx.get_type(1, 4) == mtm::element_string);
        TEST_ASSERT(mtx.get_string(1, 4) == "foo");
    }

    {
        // construct with an array of data.
        std::vector<double> vals;
        vals.push_back(1.1);
        vals.push_back(1.2);
        vals.push_back(1.3);
        vals.push_back(1.4);
        mtx_type mtx(2, 2, vals.begin(), vals.end());
        mtx_type::size_pair_type sz = mtx.size();
        TEST_ASSERT(sz.row == 2 && sz.column == 2);
        TEST_ASSERT(mtx.get_numeric(0, 0) == 1.1);
        TEST_ASSERT(mtx.get_numeric(1, 0) == 1.2);
        TEST_ASSERT(mtx.get_numeric(0, 1) == 1.3);
        TEST_ASSERT(mtx.get_numeric(1, 1) == 1.4);

        try
        {
            mtx_type mtx2(3, 2, vals.begin(), vals.end());
            TEST_ASSERT(!"Construction of this matrix should have failed!");
        }
        catch (const invalid_arg_error& e)
        {
            // Good.
            std::cout << "exception caught (as expected) which says: " << e.what() << std::endl;
        }

        try
        {
            // Trying to initialize a matrix with array of unsupported data
            // type should end with an exception thrown.
            std::vector<int8_t> vals_ptr(4, 22);
            mtx_type mtx3(2, 2, vals_ptr.begin(), vals_ptr.end());
            TEST_ASSERT(!"Construction of this matrix should have failed!");
        }
        catch (const std::exception& e)
        {
            std::cout << "exception caught (as expected) which says: " << e.what() << std::endl;
        }
    }

    {
        // Construct with an array of custom string type.
        std::vector<custom_string> vals;
        vals.push_back(custom_string("A"));
        vals.push_back(custom_string("B"));
        vals.push_back(custom_string("C"));
        vals.push_back(custom_string("D"));
        mtx_custom_type mtx(1, 4, vals.begin(), vals.end());
        TEST_ASSERT(mtx.get_string(0, 0).get() == "A");
        TEST_ASSERT(mtx.get_string(0, 1).get() == "B");
        TEST_ASSERT(mtx.get_string(0, 2).get() == "C");
        TEST_ASSERT(mtx.get_string(0, 3).get() == "D");
        TEST_ASSERT(mtx.get_type(0, 0) == mtm::element_string);
        TEST_ASSERT(mtx.get_type(0, 1) == mtm::element_string);
        TEST_ASSERT(mtx.get_type(0, 2) == mtm::element_string);
        TEST_ASSERT(mtx.get_type(0, 3) == mtm::element_string);
    }

    {
        // Construct size_pair_type from initializer list.
        mtx_type::size_pair_type sz({3, 4});
        TEST_ASSERT(sz.row == 3);
        TEST_ASSERT(sz.column == 4);
    }
}

void mtm_test_data_insertion()
{
    MDDS_TEST_FUNC_SCOPE;
    {
        // Create with empty elements.
        mtx_type mtx(3, 4);
        mtx_type::size_pair_type sz = mtx.size();
        TEST_ASSERT(sz.row == 3 && sz.column == 4);
        TEST_ASSERT(!mtx.empty());
        TEST_ASSERT(mtx.get_type(0, 0) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(2, 3) == mtm::element_empty);
        check_value(mtx, 1, 1, 1.2);
        check_value(mtx, 2, 1, true);
        check_value(mtx, 3, 1, false);
        check_value(mtx, 0, 2, std::string("foo"));
        check_value(mtx, 1, 2, 23.4);

        // Overwrite
        TEST_ASSERT(mtx.get_type(1, 1) == mtm::element_numeric);
        check_value(mtx, 1, 1, std::string("baa"));

        // Setting empty.
        TEST_ASSERT(mtx.get_type(1, 1) == mtm::element_string);
        mtx.set_empty(1, 1);
        TEST_ASSERT(mtx.get_type(1, 1) == mtm::element_empty);

        mtx.clear();
        TEST_ASSERT(mtx.size().row == 0);
        TEST_ASSERT(mtx.size().column == 0);
        TEST_ASSERT(mtx.empty());
    }
}

void mtm_test_data_insertion_multiple()
{
    MDDS_TEST_FUNC_SCOPE;

    {
        mtx_type mtx(3, 5);

        // data shorter than column length
        std::vector<double> vals;
        vals.push_back(1.1);
        vals.push_back(1.2);
        mtx.set_column(2, vals.begin(), vals.end());
        TEST_ASSERT(mtx.get_numeric(0, 2) == 1.1);
        TEST_ASSERT(mtx.get_numeric(1, 2) == 1.2);
        TEST_ASSERT(mtx.get_type(2, 2) == mtm::element_empty);

        // data exatly at column length
        vals.clear();
        vals.push_back(2.1);
        vals.push_back(2.2);
        vals.push_back(2.3);
        mtx.set_column(2, vals.begin(), vals.end());
        TEST_ASSERT(mtx.get_numeric(0, 2) == 2.1);
        TEST_ASSERT(mtx.get_numeric(1, 2) == 2.2);
        TEST_ASSERT(mtx.get_numeric(2, 2) == 2.3);
        TEST_ASSERT(mtx.get_type(0, 3) == mtm::element_empty);

        // data longer than column length.  The excess data should be ignored.
        vals.clear();
        vals.push_back(3.1);
        vals.push_back(3.2);
        vals.push_back(3.3);
        vals.push_back(3.4);
        mtx.set_column(2, vals.begin(), vals.end());
        TEST_ASSERT(mtx.get_numeric(0, 2) == 3.1);
        TEST_ASSERT(mtx.get_numeric(1, 2) == 3.2);
        TEST_ASSERT(mtx.get_numeric(2, 2) == 3.3);
        TEST_ASSERT(mtx.get_type(0, 3) == mtm::element_empty);
    }
}

void mtm_test_data_insertion_integer()
{
    MDDS_TEST_FUNC_SCOPE;

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

    TEST_ASSERT(mtx.get_type(0, 0) == mtm::element_integer);
    TEST_ASSERT(mtx.get_type(1, 1) == mtm::element_integer);
    TEST_ASSERT(mtx.get_type(0, 1) == mtm::element_numeric);
    TEST_ASSERT(mtx.get_type(1, 0) == mtm::element_numeric);

    TEST_ASSERT(mtx.get_integer(0, 0) == 0);
    TEST_ASSERT(mtx.get_integer(1, 1) == 22);
    TEST_ASSERT(mtx.get_integer(0, 1) == 10);
    TEST_ASSERT(mtx.get_integer(1, 0) == 22);

    TEST_ASSERT(mtx.get_numeric(0, 0) == 0.0);
    TEST_ASSERT(mtx.get_numeric(1, 1) == 22.0);
    TEST_ASSERT(mtx.get_numeric(0, 1) == 10.0);
    TEST_ASSERT(mtx.get_numeric(1, 0) == 22.0);

    TEST_ASSERT(mtx.get_boolean(0, 0) == false);
    TEST_ASSERT(mtx.get_boolean(1, 1) == true);
    TEST_ASSERT(mtx.get_boolean(0, 1) == true);
    TEST_ASSERT(mtx.get_boolean(1, 0) == true);

    TEST_ASSERT(mtx.numeric()); // integers are considered numeric.

    TEST_ASSERT(mtx.get<int>(0, 0) == 0);
    TEST_ASSERT(mtx.get<int>(1, 1) == 22);

    mtx_type::position_type pos = mtx.position(0, 1);
    mtx.set(pos, int(987));
    TEST_ASSERT(mtx.get<int>(0, 1) == 987);

    // +--------+--------+
    // |  int   |  int   |
    // +--------+--------+
    // | double |  int   |
    // +--------+--------+

    std::vector<mtx_type::element_block_node_type> nodes;

    std::function<void(const mtx_type::element_block_node_type&)> f =
        [&nodes](const mtx_type::element_block_node_type& node) { nodes.push_back(node); };

    mtx.walk(f);

    TEST_ASSERT(nodes.size() == 3);
    TEST_ASSERT(nodes[0].type == mtm::element_integer);
    TEST_ASSERT(nodes[0].size == 1);
    TEST_ASSERT(nodes[1].type == mtm::element_numeric);
    TEST_ASSERT(nodes[1].size == 1);
    TEST_ASSERT(nodes[2].type == mtm::element_integer);
    TEST_ASSERT(nodes[2].size == 2);

    {
        auto it = nodes[2].begin<mtx_type::integer_block_type>();
        auto ite = nodes[2].end<mtx_type::integer_block_type>();
        TEST_ASSERT(*it == 987);
        ++it;
        TEST_ASSERT(*it == 22);
        ++it;
        TEST_ASSERT(it == ite);
    }
}

void mtm_test_set_empty()
{
    MDDS_TEST_FUNC_SCOPE;

    {
        // set whole column empty.
        mtx_type mtx(3, 5, 1.2);
        cout << "setting whole column 2 empty..." << endl;
        mtx.set_column_empty(2);
        TEST_ASSERT(mtx.get_type(0, 1) != mtm::element_empty);
        TEST_ASSERT(mtx.get_type(1, 1) != mtm::element_empty);
        TEST_ASSERT(mtx.get_type(2, 1) != mtm::element_empty);
        TEST_ASSERT(mtx.get_type(0, 2) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(1, 2) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(2, 2) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(0, 3) != mtm::element_empty);
        TEST_ASSERT(mtx.get_type(1, 3) != mtm::element_empty);
        TEST_ASSERT(mtx.get_type(2, 3) != mtm::element_empty);
    }

    {
        // set whole row empty.
        mtx_type mtx(3, 5, 1.2);
        cout << "setting whole row 1 empty..." << endl;
        mtx.set_row_empty(1);
        TEST_ASSERT(mtx.get_type(0, 0) != mtm::element_empty);
        TEST_ASSERT(mtx.get_type(0, 1) != mtm::element_empty);
        TEST_ASSERT(mtx.get_type(0, 2) != mtm::element_empty);
        TEST_ASSERT(mtx.get_type(0, 3) != mtm::element_empty);
        TEST_ASSERT(mtx.get_type(0, 4) != mtm::element_empty);
        TEST_ASSERT(mtx.get_type(1, 0) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(1, 1) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(1, 2) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(1, 3) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(1, 4) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(2, 0) != mtm::element_empty);
        TEST_ASSERT(mtx.get_type(2, 1) != mtm::element_empty);
        TEST_ASSERT(mtx.get_type(2, 2) != mtm::element_empty);
        TEST_ASSERT(mtx.get_type(2, 3) != mtm::element_empty);
        TEST_ASSERT(mtx.get_type(2, 4) != mtm::element_empty);
    }

    {
        // Set a range of elements empty.
        mtx_type mtx(5, 3, std::string("A"));
        cout << "setting element (0,1) to (1,2) empty..." << endl;
        mtx.set_empty(1, 0, 6); // rows 1-4 in column 0 and rows 0-1 in column 1.
        TEST_ASSERT(mtx.get_type(0, 0) == mtm::element_string);
        TEST_ASSERT(mtx.get_type(1, 0) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(2, 0) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(3, 0) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(4, 0) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(0, 1) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(1, 1) == mtm::element_empty);
        TEST_ASSERT(mtx.get_type(2, 1) == mtm::element_string);
        TEST_ASSERT(mtx.get_type(3, 1) == mtm::element_string);
        TEST_ASSERT(mtx.get_type(4, 1) == mtm::element_string);
        TEST_ASSERT(mtx.get_type(0, 2) == mtm::element_string);
        TEST_ASSERT(mtx.get_type(1, 2) == mtm::element_string);
        TEST_ASSERT(mtx.get_type(2, 2) == mtm::element_string);
        TEST_ASSERT(mtx.get_type(3, 2) == mtm::element_string);
        TEST_ASSERT(mtx.get_type(4, 2) == mtm::element_string);

        try
        {
            mtx.set_empty(2, 2, 0);
            TEST_ASSERT(false);
        }
        catch (const std::exception&)
        {
            cout << "exception thrown on length of zero as expected" << endl;
        }
    }
}

void mtm_test_swap()
{
    MDDS_TEST_FUNC_SCOPE;

    mtx_type mtx1(3, 6), mtx2(7, 2);
    mtx1.set(0, 0, 1.1);
    mtx1.set(2, 5, 1.9);
    mtx2.set(0, 0, 2.1);
    mtx2.set(6, 1, 2.9);
    mtx1.swap(mtx2);

    TEST_ASSERT(mtx1.size().row == 7);
    TEST_ASSERT(mtx1.size().column == 2);
    TEST_ASSERT(mtx1.get<double>(0, 0) == 2.1);
    TEST_ASSERT(mtx1.get<double>(6, 1) == 2.9);

    TEST_ASSERT(mtx2.size().row == 3);
    TEST_ASSERT(mtx2.size().column == 6);
    TEST_ASSERT(mtx2.get<double>(0, 0) == 1.1);
    TEST_ASSERT(mtx2.get<double>(2, 5) == 1.9);
}

void mtm_test_transpose()
{
    MDDS_TEST_FUNC_SCOPE;

    mtx_type mtx(3, 6);
    mtx.set(0, 0, 1.1);
    mtx.set(1, 0, 1.2);
    mtx.set(2, 0, 1.3);
    mtx.set(1, 5, std::string("foo"));
    mtx.set(2, 3, true);
    mtx.transpose();
    TEST_ASSERT(mtx.size().row == 6);
    TEST_ASSERT(mtx.size().column == 3);
    TEST_ASSERT(mtx.get<double>(0, 0) == 1.1);
    TEST_ASSERT(mtx.get<double>(0, 1) == 1.2);
    TEST_ASSERT(mtx.get<double>(0, 2) == 1.3);
    TEST_ASSERT(mtx.get<std::string>(5, 1) == "foo");
    TEST_ASSERT(mtx.get<bool>(3, 2) == true);
}

void mtm_test_resize()
{
    MDDS_TEST_FUNC_SCOPE;

    mtx_type mtx(0, 0);
    TEST_ASSERT(mtx.size().row == 0);
    TEST_ASSERT(mtx.size().column == 0);
    TEST_ASSERT(mtx.empty());

    mtx.resize(1, 3);
    TEST_ASSERT(mtx.size().row == 1);
    TEST_ASSERT(mtx.size().column == 3);
    TEST_ASSERT(!mtx.empty());
    TEST_ASSERT(mtx.get_type(0, 0) == mtm::element_empty);
    TEST_ASSERT(mtx.get_type(0, 1) == mtm::element_empty);
    TEST_ASSERT(mtx.get_type(0, 2) == mtm::element_empty);

    mtx.set(0, 0, 1.1);
    mtx.set(0, 1, std::string("foo"));
    mtx.set(0, 2, true);
    TEST_ASSERT(mtx.get<double>(0, 0) == 1.1);
    TEST_ASSERT(mtx.get<std::string>(0, 1) == "foo");
    TEST_ASSERT(mtx.get<bool>(0, 2) == true);

    // This shouldn't alter the original content.
    mtx.resize(2, 4);
    TEST_ASSERT(mtx.size().row == 2);
    TEST_ASSERT(mtx.size().column == 4);
    TEST_ASSERT(mtx.get<double>(0, 0) == 1.1);
    TEST_ASSERT(mtx.get<std::string>(0, 1) == "foo");
    TEST_ASSERT(mtx.get<bool>(0, 2) == true);
    TEST_ASSERT(mtx.get_type(1, 3) == mtm::element_empty);

    mtx.resize(2, 2);
    TEST_ASSERT(mtx.size().row == 2);
    TEST_ASSERT(mtx.size().column == 2);
    TEST_ASSERT(mtx.get<double>(0, 0) == 1.1);
    TEST_ASSERT(mtx.get<std::string>(0, 1) == "foo");
    TEST_ASSERT(mtx.get_type(1, 0) == mtm::element_empty);
    TEST_ASSERT(mtx.get_type(1, 1) == mtm::element_empty);

    // Three ways to resize to empty matrix.
    mtx.resize(2, 0);
    TEST_ASSERT(mtx.size().row == 0);
    TEST_ASSERT(mtx.size().column == 0);

    mtx.resize(2, 2);
    mtx.resize(0, 2);
    TEST_ASSERT(mtx.size().row == 0);
    TEST_ASSERT(mtx.size().column == 0);

    mtx.resize(2, 2);
    mtx.resize(0, 0);
    TEST_ASSERT(mtx.size().row == 0);
    TEST_ASSERT(mtx.size().column == 0);

    // Resize with initial value when the matrix becomes larger.
    mtx.resize(3, 2, 12.5);
    TEST_ASSERT(mtx.size().row == 3);
    TEST_ASSERT(mtx.size().column == 2);
    TEST_ASSERT(mtx.get<double>(2, 1) == 12.5);
    TEST_ASSERT(mtx.get<double>(2, 0) == 12.5);
    TEST_ASSERT(mtx.get<double>(0, 0) == 12.5);
    TEST_ASSERT(mtx.get<double>(0, 1) == 12.5);

    // The initial value should be ignored when shrinking.
    mtx.resize(2, 1, true);
    TEST_ASSERT(mtx.size().row == 2);
    TEST_ASSERT(mtx.size().column == 1);
    TEST_ASSERT(mtx.get<double>(1, 0) == 12.5);

    // Resize again with initial value of different type.
    mtx.resize(3, 2, std::string("extra"));
    TEST_ASSERT(mtx.size().row == 3);
    TEST_ASSERT(mtx.size().column == 2);
    TEST_ASSERT(mtx.get<double>(0, 0) == 12.5);
    TEST_ASSERT(mtx.get<double>(1, 0) == 12.5);
    TEST_ASSERT(mtx.get<std::string>(2, 1) == "extra");
    TEST_ASSERT(mtx.get<std::string>(2, 0) == "extra");
    TEST_ASSERT(mtx.get<std::string>(1, 1) == "extra");
    TEST_ASSERT(mtx.get<std::string>(0, 1) == "extra");
}

void mtm_test_copy()
{
    MDDS_TEST_FUNC_SCOPE;

    // Assigning from a smaller matrix to a bigger one.
    mtx_type mx1(5, 5), mx2(2, 2);
    mx2.set(0, 0, 1.2);
    mx2.set(1, 1, true);
    mx2.set(0, 1, std::string("test"));
    mx2.set(1, 0, std::string("foo"));
    mx1.copy(mx2);

    bool success = check_copy(mx1, mx2);
    TEST_ASSERT(success);

    mx2.resize(8, 8);
    mx2.copy(mx1);

    success = check_copy(mx1, mx2);
    TEST_ASSERT(success);

    // from a larger matrix to a smaller one.
    mx1.set(0, 0, std::string("test1"));
    mx2.set(0, 0, std::string("test2"));
    mx2.set(4, 4, true);
    mx2.set(7, 7, false);
    mx1.copy(mx2);
    success = check_copy(mx1, mx2);
    TEST_ASSERT(success);

    // self assignment (should be no-op).
    mx1.copy(mx1);
    success = check_copy(mx1, mx1);
    TEST_ASSERT(success);

    mx2.copy(mx2);
    success = check_copy(mx2, mx2);
    TEST_ASSERT(success);
}

void mtm_test_copy_empty_destination()
{
    MDDS_TEST_FUNC_SCOPE;

    mtx_type mx1, mx2(1, 1);
    mx1.copy(mx2); // This should not throw.

    mx2.copy(mx1); // This should not throw either.
}

void mtm_test_copy_from_array()
{
    MDDS_TEST_FUNC_SCOPE;

    std::vector<double> src;
    src.reserve(9);
    for (size_t i = 0; i < 9; ++i)
        src.push_back(double(i));

    mtx_type mx(4, 4);
    mx.copy(3, 3, src.begin(), src.end());

    TEST_ASSERT(mx.get<double>(0, 0) == 0.0);
    TEST_ASSERT(mx.get<double>(1, 0) == 1.0);
    TEST_ASSERT(mx.get<double>(2, 0) == 2.0);
    TEST_ASSERT(mx.get<double>(0, 1) == 3.0);
    TEST_ASSERT(mx.get<double>(1, 1) == 4.0);
    TEST_ASSERT(mx.get<double>(2, 1) == 5.0);
    TEST_ASSERT(mx.get<double>(0, 2) == 6.0);
    TEST_ASSERT(mx.get<double>(1, 2) == 7.0);
    TEST_ASSERT(mx.get<double>(2, 2) == 8.0);
    TEST_ASSERT(mx.get_type(3, 0) == mtm::element_empty);
    TEST_ASSERT(mx.get_type(3, 1) == mtm::element_empty);
    TEST_ASSERT(mx.get_type(3, 2) == mtm::element_empty);
    TEST_ASSERT(mx.get_type(3, 3) == mtm::element_empty);
    TEST_ASSERT(mx.get_type(0, 3) == mtm::element_empty);
    TEST_ASSERT(mx.get_type(1, 3) == mtm::element_empty);
    TEST_ASSERT(mx.get_type(2, 3) == mtm::element_empty);
    TEST_ASSERT(mx.get_type(3, 3) == mtm::element_empty);

    std::vector<std::string> src2;
    src2.reserve(4);
    src2.push_back("A");
    src2.push_back("B");
    src2.push_back("C");
    src2.push_back("D");

    mx.copy(2, 2, src2.begin(), src2.end());

    TEST_ASSERT(mx.get<std::string>(0, 0) == "A");
    TEST_ASSERT(mx.get<std::string>(1, 0) == "B");
    TEST_ASSERT(mx.get<std::string>(0, 1) == "C");
    TEST_ASSERT(mx.get<std::string>(1, 1) == "D");
    TEST_ASSERT(mx.get<double>(2, 0) == 2.0);
    TEST_ASSERT(mx.get<double>(2, 1) == 5.0);
    TEST_ASSERT(mx.get<double>(0, 2) == 6.0);
    TEST_ASSERT(mx.get<double>(1, 2) == 7.0);
    TEST_ASSERT(mx.get<double>(2, 2) == 8.0);
    TEST_ASSERT(mx.get_type(3, 0) == mtm::element_empty);
    TEST_ASSERT(mx.get_type(3, 1) == mtm::element_empty);
    TEST_ASSERT(mx.get_type(3, 2) == mtm::element_empty);
    TEST_ASSERT(mx.get_type(3, 3) == mtm::element_empty);
    TEST_ASSERT(mx.get_type(0, 3) == mtm::element_empty);
    TEST_ASSERT(mx.get_type(1, 3) == mtm::element_empty);
    TEST_ASSERT(mx.get_type(2, 3) == mtm::element_empty);
    TEST_ASSERT(mx.get_type(3, 3) == mtm::element_empty);

    std::vector<bool> src3;
    src3.push_back(true);
    src3.push_back(false);
    src3.push_back(true);
    src3.push_back(false);

    mx.copy(4, 1, src3.begin(), src3.end());
    TEST_ASSERT(mx.get<bool>(0, 0) == true);
    TEST_ASSERT(mx.get<bool>(1, 0) == false);
    TEST_ASSERT(mx.get<bool>(2, 0) == true);
    TEST_ASSERT(mx.get<bool>(3, 0) == false);

    // Try to copy from an array of invalid type.
    std::vector<int8_t> src_invalid;
    src_invalid.push_back('a');
    src_invalid.push_back('b');

    try
    {
        mx.copy(2, 1, src_invalid.begin(), src_invalid.end());
        TEST_ASSERT(!"type_error did not get thrown.");
    }
    catch (const mdds::type_error& e)
    {
        cout << "expected exception was thrown: '" << e.what() << "'" << endl;
    }
}

void mtm_test_assignment()
{
    MDDS_TEST_FUNC_SCOPE;

    mtx_type mx_orig(5, 5, 1.2);
    mtx_type mx_copied = mx_orig;
    TEST_ASSERT(mx_orig == mx_copied);

    mx_copied = mx_copied; // self assignment.
    TEST_ASSERT(mx_orig == mx_copied);

    mx_orig.set(2, 3, true);
    mx_orig.set(1, 1, std::string("foo"));
    mx_copied = mx_orig;
    TEST_ASSERT(mx_orig == mx_copied);
}

void mtm_test_numeric()
{
    MDDS_TEST_FUNC_SCOPE;

    // Numeric elements only matrix is numeric.
    mtx_type mtx(2, 2, 1.1);
    TEST_ASSERT(mtx.numeric());

    // Boolean element is numeric.
    mtx.set(0, 0, true);
    TEST_ASSERT(mtx.numeric());

    // String element is not.
    mtx.set(1, 0, std::string("foo"));
    TEST_ASSERT(!mtx.numeric());

    mtx.set(1, 0, 1.3);
    TEST_ASSERT(mtx.numeric());

    // Empty element is not numeric.
    mtx.set_empty(1, 1);
    TEST_ASSERT(!mtx.numeric());

    // Empty matrix is not numeric.
    mtx.clear();
    TEST_ASSERT(!mtx.numeric());
}

void mtm_test_custom_string()
{
    MDDS_TEST_FUNC_SCOPE;

    mtx_custom_type mtx(2, 2);
    mtx.set(0, 0, custom_string("foo"));
    TEST_ASSERT(mtx.get_type(0, 0) == mtm::element_string);
    TEST_ASSERT(mtx.get<custom_string>(0, 0) == custom_string("foo"));
    mtx.set(1, 1, 12.3);
    TEST_ASSERT(mtx.get<double>(1, 1) == 12.3);
}

void mtm_test_position()
{
    MDDS_TEST_FUNC_SCOPE;

    mtx_type mtx(3, 2);
    mtx.set(0, 0, 1.0);
    mtx.set(0, 1, std::string("foo"));
    mtx.set(1, 0, 2.0);
    mtx.set(1, 1, 2.1);
    mtx.set(2, 0, true);
    mtx.set(2, 1, false);

    TEST_ASSERT(mtx.get_type(0, 0) == mtm::element_numeric);
    TEST_ASSERT(mtx.get_type(0, 1) == mtm::element_string);
    TEST_ASSERT(mtx.get_type(1, 0) == mtm::element_numeric);
    TEST_ASSERT(mtx.get_type(1, 1) == mtm::element_numeric);
    TEST_ASSERT(mtx.get_type(2, 0) == mtm::element_boolean);
    TEST_ASSERT(mtx.get_type(2, 1) == mtm::element_boolean);

    mtx_type::position_type pos = mtx.position(1, 1);
    TEST_ASSERT(mtx.get_type(pos) == mtm::element_numeric);
    TEST_ASSERT(mtx.get_numeric(pos) == 2.1);

    pos = mtx.position(2, 0);
    TEST_ASSERT(mtx.get_type(pos) == mtm::element_boolean);
    TEST_ASSERT(mtx.get_boolean(pos) == true);

    pos = mtx.position(0, 1);
    TEST_ASSERT(mtx.get_type(pos) == mtm::element_string);
    TEST_ASSERT(mtx.get_string(pos) == "foo");

    mtx.set_empty(pos);
    TEST_ASSERT(mtx.get_type(0, 1) == mtm::element_empty);

    pos = mtx.position(1, 1);
    mtx.set(pos, false);
    TEST_ASSERT(mtx.get_type(1, 1) == mtm::element_boolean);
    TEST_ASSERT(mtx.get_boolean(1, 1) == false);

    pos = mtx.position(2, 0);
    mtx.set(pos, 12.3);
    TEST_ASSERT(mtx.get_type(2, 0) == mtm::element_numeric);
    TEST_ASSERT(mtx.get_numeric(2, 0) == 12.3);

    pos = mtx.position(2, 1);
    mtx.set(pos, std::string("ABC"));
    TEST_ASSERT(mtx.get_type(2, 1) == mtm::element_string);
    TEST_ASSERT(mtx.get_string(2, 1) == "ABC");

    // Start over, and test the traversal of position object.
    pos = mtx.position(0, 0);
    mtx_type::size_pair_type mtx_pos = mtx.matrix_position(pos);
    TEST_ASSERT(mtx_pos.column == 0);
    TEST_ASSERT(mtx_pos.row == 0);

    pos = mtx_type::next_position(pos);
    mtx_pos = mtx.matrix_position(pos);
    TEST_ASSERT(mtx_pos.column == 0);
    TEST_ASSERT(mtx_pos.row == 1);

    pos = mtx_type::next_position(pos);
    mtx_pos = mtx.matrix_position(pos);
    TEST_ASSERT(mtx_pos.column == 0);
    TEST_ASSERT(mtx_pos.row == 2);

    pos = mtx_type::next_position(pos);
    mtx_pos = mtx.matrix_position(pos);
    TEST_ASSERT(mtx_pos.column == 1);
    TEST_ASSERT(mtx_pos.row == 0);

    pos = mtx_type::next_position(pos);
    mtx_pos = mtx.matrix_position(pos);
    TEST_ASSERT(mtx_pos.column == 1);
    TEST_ASSERT(mtx_pos.row == 1);

    pos = mtx_type::next_position(pos);
    mtx_pos = mtx.matrix_position(pos);
    TEST_ASSERT(mtx_pos.column == 1);
    TEST_ASSERT(mtx_pos.row == 2);

    pos = mtx_type::next_position(pos);
    TEST_ASSERT(pos == mtx.end_position());
}

void mtm_test_set_data_via_position()
{
    MDDS_TEST_FUNC_SCOPE;

    mtx_type mtx(5, 4);
    mtx_type::position_type pos = mtx.position(0, 1);
    std::vector<double> data;
    data.push_back(1.1);
    data.push_back(1.2);
    data.push_back(1.3);
    data.push_back(1.4);
    data.push_back(1.5);
    pos = mtx.set(pos, data.begin(), data.end());
    TEST_ASSERT(mtx.get<double>(0, 1) == 1.1);
    TEST_ASSERT(mtx.get<double>(1, 1) == 1.2);
    TEST_ASSERT(mtx.get<double>(2, 1) == 1.3);
    TEST_ASSERT(mtx.get<double>(3, 1) == 1.4);
    TEST_ASSERT(mtx.get<double>(4, 1) == 1.5);

    mtx_type::size_pair_type mtx_pos = mtx.matrix_position(pos);
    TEST_ASSERT(mtx_pos.row == 0);
    TEST_ASSERT(mtx_pos.column == 1);
    pos = mtx.position(pos, 0, 2);
    pos = mtx.set(pos, std::string("test"));
    pos = mtx_type::next_position(pos);
    pos = mtx.set(pos, true);
    TEST_ASSERT(mtx.get<std::string>(0, 2) == "test");
    TEST_ASSERT(mtx.get<bool>(1, 2) == true);
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
                std::vector<double> vals;
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
        std::vector<double> vals;
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
                std::vector<double> vals;
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
        std::vector<double> vals;
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
