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

mdds::mtv::element_t mdds_mtv_get_element_type(const custom_string&)
{
    return element_type_custom_string;
}

void mdds_mtv_set_value(mtv::base_element_block& block, size_t pos, const custom_string& val)
{
    custom_string_block::set_value(block, pos, val);
}

void mdds_mtv_get_value(const mtv::base_element_block& block, size_t pos, custom_string& val)
{
    custom_string_block::get_value(block, pos, val);
}

template<typename _Iter>
void mdds_mtv_set_values(
    mtv::base_element_block& block, size_t pos, const custom_string&, const _Iter& it_begin, const _Iter& it_end)
{
    custom_string_block::set_values(block, pos, it_begin, it_end);
}

void mdds_mtv_append_value(mtv::base_element_block& block, const custom_string& val)
{
    custom_string_block::append_value(block, val);
}

void mdds_mtv_prepend_value(mtv::base_element_block& block, const custom_string& val)
{
    custom_string_block::prepend_value(block, val);
}

template<typename _Iter>
void mdds_mtv_prepend_values(mtv::base_element_block& block, const custom_string&, const _Iter& it_begin, const _Iter& it_end)
{
    custom_string_block::prepend_values(block, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_append_values(mtv::base_element_block& block, const custom_string&, const _Iter& it_begin, const _Iter& it_end)
{
    custom_string_block::append_values(block, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_assign_values(mtv::base_element_block& dest, const custom_string&, const _Iter& it_begin, const _Iter& it_end)
{
    custom_string_block::assign_values(dest, it_begin, it_end);
}

void mdds_mtv_get_empty_value(custom_string& val)
{
    val = custom_string();
}

template<typename _Iter>
void mdds_mtv_insert_values(
    mtv::base_element_block& block, size_t pos, const custom_string&, const _Iter& it_begin, const _Iter& it_end)
{
    custom_string_block::insert_values(block, pos, it_begin, it_end);
}

mtv::base_element_block* mdds_mtv_create_new_block(size_t init_size, const custom_string& val)
{
    return custom_string_block::create_block_with_value(init_size, val);
}

struct custom_string_trait
{
    typedef custom_string string_type;
    typedef custom_string_block string_element_block;

    static const mdds::mtv::element_t string_type_identifier = element_type_custom_string;

    struct element_block_func
    {
        static mdds::mtv::base_element_block* create_new_block(
            mdds::mtv::element_t type, size_t init_size)
        {
            switch (type)
            {
                case element_type_custom_string:
                    return string_element_block::create_block(init_size);
                default:
                    return mdds::mtv::element_block_func::create_new_block(type, init_size);
            }
        }

        static mdds::mtv::base_element_block* clone_block(const mdds::mtv::base_element_block& block)
        {
            switch (mtv::get_block_type(block))
            {
                case element_type_custom_string:
                    return string_element_block::clone_block(block);
                default:
                    return mdds::mtv::element_block_func::clone_block(block);
            }
        }

        static void delete_block(mdds::mtv::base_element_block* p)
        {
            if (!p)
                return;

            switch (mtv::get_block_type(*p))
            {
                case element_type_custom_string:
                    string_element_block::delete_block(p);
                break;
                default:
                    mdds::mtv::element_block_func::delete_block(p);
            }
        }

        static void resize_block(mdds::mtv::base_element_block& block, size_t new_size)
        {
            switch (mtv::get_block_type(block))
            {
                case element_type_custom_string:
                    string_element_block::resize_block(block, new_size);
                break;
                default:
                    mdds::mtv::element_block_func::resize_block(block, new_size);
            }
        }

        static void print_block(const mdds::mtv::base_element_block& block)
        {
            switch (mtv::get_block_type(block))
            {
                case element_type_custom_string:
                    string_element_block::print_block(block);
                break;
                default:
                    mdds::mtv::element_block_func::print_block(block);
            }
        }

        static void erase(mdds::mtv::base_element_block& block, size_t pos)
        {
            switch (mtv::get_block_type(block))
            {
                case element_type_custom_string:
                    string_element_block::erase_block(block, pos);
                break;
                default:
                    mdds::mtv::element_block_func::erase(block, pos);
            }
        }

        static void erase(mdds::mtv::base_element_block& block, size_t pos, size_t size)
        {
            switch (mtv::get_block_type(block))
            {
                case element_type_custom_string:
                    string_element_block::erase_block(block, pos, size);
                break;
                default:
                    mdds::mtv::element_block_func::erase(block, pos, size);
            }
        }

        static void append_values_from_block(
            mdds::mtv::base_element_block& dest, const mdds::mtv::base_element_block& src)
        {
            switch (mtv::get_block_type(dest))
            {
                case element_type_custom_string:
                    string_element_block::append_values_from_block(dest, src);
                break;
                default:
                    mdds::mtv::element_block_func::append_values_from_block(dest, src);
            }
        }

        static void append_values_from_block(
            mdds::mtv::base_element_block& dest, const mdds::mtv::base_element_block& src,
            size_t begin_pos, size_t len)
        {
            switch (mtv::get_block_type(dest))
            {
                case element_type_custom_string:
                    string_element_block::append_values_from_block(dest, src, begin_pos, len);
                break;
                default:
                    mdds::mtv::element_block_func::append_values_from_block(dest, src, begin_pos, len);
            }
        }

        static void assign_values_from_block(
            mdds::mtv::base_element_block& dest, const mdds::mtv::base_element_block& src,
            size_t begin_pos, size_t len)
        {
            switch (mtv::get_block_type(dest))
            {
                case element_type_custom_string:
                    string_element_block::assign_values_from_block(dest, src, begin_pos, len);
                break;
                default:
                    mdds::mtv::element_block_func::assign_values_from_block(dest, src, begin_pos, len);
            }
        }

        static bool equal_block(
            const mdds::mtv::base_element_block& left, const mdds::mtv::base_element_block& right)
        {
            if (mtv::get_block_type(left) == element_type_custom_string)
            {
                if (mtv::get_block_type(right) != element_type_custom_string)
                    return false;

                return string_element_block::get(left) == string_element_block::get(right);
            }
            else if (mtv::get_block_type(right) == element_type_custom_string)
                return false;

            return mdds::mtv::element_block_func::equal_block(left, right);
        }

        static void overwrite_values(mdds::mtv::base_element_block& block, size_t pos, size_t len)
        {
            switch (mtv::get_block_type(block))
            {
                case element_type_custom_string:
                    // Do nothing.  The client code manages the life cycle of these cells.
                break;
                default:
                    mdds::mtv::element_block_func::overwrite_values(block, pos, len);
            }
        }
    };
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
                mtx_type::boolean_block_type::const_iterator it_end = mtx_type::boolean_block_type::end(*node.data);
                std::for_each(it, it_end, print_element<bool>());
            }
            break;
            case mtm::element_string:
            {
                mtx_type::string_block_type::const_iterator it = mtx_type::string_block_type::begin(*node.data);
                mtx_type::string_block_type::const_iterator it_end = mtx_type::string_block_type::end(*node.data);
                std::for_each(it, it_end, print_element<mtx_type::string_type>());
            }
            break;
            case mtm::element_numeric:
            {
                mtx_type::numeric_block_type::const_iterator it = mtx_type::numeric_block_type::begin(*node.data);
                mtx_type::numeric_block_type::const_iterator it_end = mtx_type::numeric_block_type::end(*node.data);
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
        mtm_test_data_insertion_multiple();
        mtm_test_set_empty();
        mtm_test_swap();
        mtm_test_transpose();
        mtm_test_resize();
        mtm_test_copy();
        mtm_test_numeric();
        mtm_test_walk();
        mtm_test_custom_string();
    }

    if (opt.test_perf)
    {
        mtm_perf_test_storage_creation();
        mtm_perf_test_storage_set_numeric();
    }

    return EXIT_SUCCESS;
}
