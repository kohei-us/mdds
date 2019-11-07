/*************************************************************************
 *
 * Copyright (c) 2012-2016 Kohei Yoshida
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

#include <string>
#include <ostream>
#include <memory>

using namespace mdds;
using namespace std;

// Standard matrix that uses std::string as its string type.
typedef multi_type_matrix<mtm::std_string_trait> mtx_type;

inline std::ostream& operator<< (std::ostream& os, const mtx_type::element_block_node_type& node)
{
    os << "(type=" << node.type << "; offset=" << node.offset << "; size=" << node.size << ")";
    return os;
}

template<typename _T>
struct print_element : std::unary_function<_T, void>
{
    void operator() (const _T& v) const
    {
        cout << v << endl;
    }
};

class walk_element_block
{
    size_t m_node_count = 0;

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

        ++m_node_count;
    }

    size_t get_node_count() const { return m_node_count; }
};

class walk_element_block_move_only
{
    size_t m_node_count = 0;

public:
    walk_element_block_move_only() {}

    walk_element_block_move_only(walk_element_block_move_only&& other) : m_node_count(other.m_node_count)
    {
        other.m_node_count = 0;
    }

    walk_element_block_move_only& operator= (walk_element_block_move_only&& other)
    {
        m_node_count = other.m_node_count;
        other.m_node_count = 0;
        return *this;
    }

    void operator() (const mtx_type::element_block_node_type& /*node*/)
    {
        ++m_node_count;
    }

    size_t get_node_count() const { return m_node_count; }
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
    func = mtx.walk(func);
    assert(func.get_node_count() == 5);

    walk_element_block_move_only func_mo;
    func_mo = mtx.walk(std::move(func_mo));
    assert(func_mo.get_node_count() == 5);
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
        func = mtx.walk(func, mtx_type::size_pair_type(1,1), mtx_type::size_pair_type(2, 2));
        assert(func.get_node_count() == 3);
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
        func = mtx.walk(func, mtx_type::size_pair_type(1,1), mtx_type::size_pair_type(2, 2));
        assert(func.get_node_count() == 3);
    }
}

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

template<>
void push_to_buffer<mtx_type::boolean_block_type>(const mtx_type::element_block_node_type& node, std::vector<string>& buf)
{
    using blk_type = mtx_type::boolean_block_type;
    auto it = node.begin<blk_type>();
    auto ite = node.end<blk_type>();
    std::for_each(it, ite,
        [&](bool v)
        {
            ostringstream os;
            os << (v ? "true" : "false");
            buf.push_back(os.str());
        }
    );
}

class parallel_walk_element_block
{
    using strlist_type = std::vector<string>;

    std::string m_name;
    std::shared_ptr<strlist_type> m_ls;
    std::shared_ptr<strlist_type> m_rs;

    void process_node(const mtx_type::element_block_node_type& node, strlist_type& buf)
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
    parallel_walk_element_block(std::string name) :
        m_name(std::move(name)),
        m_ls(std::make_shared<strlist_type>()),
        m_rs(std::make_shared<strlist_type>()) {}
    parallel_walk_element_block(const parallel_walk_element_block& other) :
        m_name(other.m_name),
        m_ls(other.m_ls), m_rs(other.m_rs) {}
    parallel_walk_element_block(parallel_walk_element_block&& other) :
        m_name(std::move(other.m_name)),
        m_ls(std::move(other.m_ls)), m_rs(std::move(other.m_rs)) {}

    parallel_walk_element_block& operator= (parallel_walk_element_block other)
    {
        swap(other);
        return *this;
    }

    void swap(parallel_walk_element_block& other)
    {
        m_ls.swap(other.m_ls);
        m_rs.swap(other.m_rs);
    }

    void operator() (const mtx_type::element_block_node_type& left, const mtx_type::element_block_node_type& right)
    {
        cout << "--" << endl;
        cout << "l: " << left << endl;
        cout << "r: " << right << endl;
        process_node(left, *m_ls);
        process_node(right, *m_rs);
    }

    strlist_type get_concat_buffer() const
    {
        strlist_type buf;
        assert(m_ls->size() == m_rs->size());
        auto it = m_ls->begin(), it2 = m_rs->begin();
        auto ite = m_ls->end();
        for (; it != ite; ++it, ++it2)
        {
            ostringstream os;
            os << *it << ":" << *it2;
            buf.push_back(os.str());
        }

        return buf;
    }

    void clear()
    {
        m_ls->clear();
        m_rs->clear();
    }

    const std::string& get_name() const { return m_name; }

    void set_name(std::string name) { m_name = std::move(name); }
};

bool check_concat_buffer(std::vector<string> concat, const char* expected[], size_t expected_size)
{
    if (concat.size() != expected_size)
    {
        cerr << "expected size: " << expected_size << ", actual size: " << concat.size() << endl;
        return false;
    }

    for (size_t i = 0; i < expected_size; ++i)
    {
        if (concat[i] != expected[i])
        {
            cerr << i << ": expected value: " << expected[i] << ", actual value: " << concat[i] << endl;
            return false;
        }
    }

    return true;
}

void mtm_test_parallel_walk()
{
    stack_printer __stack_printer__("::mtm_test_parallel_walk");

    parallel_walk_element_block func("test0");
    mtx_type left(10, 1), right(10, 1, string("'+'"));

    right.set(2, 0, 1.2);
    right.set(8, 0, false);
    right.set(9, 0, true);

    left.set(0, 0, 122.0);
    left.set(4, 0, string("A12"));
    left.set(5, 0, string("A25"));

    {
        func = left.walk(func, right);
        assert(func.get_name() == "test0");

        const char* expected[] = {
            "122:'+'",
            "' ':'+'",
            "' ':1.2",
            "' ':'+'",
            "A12:'+'",
            "A25:'+'",
            "' ':'+'",
            "' ':'+'",
            "' ':false",
            "' ':true",
        };

        size_t n = MDDS_N_ELEMENTS(expected);
        assert(check_concat_buffer(func.get_concat_buffer(), expected, n));
    }

    func.clear();

    {
        func.set_name("test0-2");
        func = left.walk(func, right, mtx_type::size_pair_type(2, 0), mtx_type::size_pair_type(8, 0));
        assert(func.get_name() == "test0-2");

        const char* expected[] = {
            "' ':1.2",
            "' ':'+'",
            "A12:'+'",
            "A25:'+'",
            "' ':'+'",
            "' ':'+'",
            "' ':false",
        };

        size_t n = MDDS_N_ELEMENTS(expected);
        assert(check_concat_buffer(func.get_concat_buffer(), expected, n));
    }

    func.clear();

    {
        // Only one row.
        func.set_name("test0-3");
        func = left.walk(func, right, mtx_type::size_pair_type(4, 0), mtx_type::size_pair_type(4, 0));
        assert(func.get_name() == "test0-3");

        const char* expected[] = {
            "A12:'+'",
        };

        size_t n = MDDS_N_ELEMENTS(expected);
        assert(check_concat_buffer(func.get_concat_buffer(), expected, n));
    }
}

void mtm_test_parallel_walk_non_equal_size()
{
    stack_printer __stack_printer__("::mtm_test_parallel_walk_non_equal_size");

    mtx_type left(3, 3), right(2, 2);

    left.set(0, 0, 10.0);
    left.set(1, 0, 20.0);
    left.set(2, 0, 30.0);
    left.set(0, 1, 40.0);
    left.set(1, 1, 50.0);
    left.set(2, 1, 60.0);
    left.set(0, 2, 70.0);
    left.set(1, 2, 80.0);
    left.set(2, 2, 90.0);

    right.set(0, 0, string("A"));
    right.set(1, 0, string("B"));
    right.set(0, 1, string("C"));
    right.set(1, 1, string("D"));

    {
        // Only walk the top-left 2x2 range.
        parallel_walk_element_block func("test1");
        func = left.walk(func, right, mtx_type::size_pair_type(0, 0), mtx_type::size_pair_type(1, 1));
        assert(func.get_name() == "test1");

        const char* expected[] = {
            "10:A",
            "20:B",
            "40:C",
            "50:D",
        };

        size_t n = MDDS_N_ELEMENTS(expected);
        assert(check_concat_buffer(func.get_concat_buffer(), expected, n));
    }

    // Break up the blocks a little.
    left.set(1, 0, true);
    left.set(0, 1, false);
    right.set(0, 0, -99.0);
    right.set(1, 1, -9.9);

    {
        // Only walk the top-left 2x2 range.
        parallel_walk_element_block func("test2");
        func = left.walk(func, right, mtx_type::size_pair_type(0, 0), mtx_type::size_pair_type(1, 1));
        assert(func.get_name() == "test2");

        const char* expected[] = {
            "10:-99",
            "true:B",
            "false:C",
            "50:-9.9",
        };

        size_t n = MDDS_N_ELEMENTS(expected);
        assert(check_concat_buffer(func.get_concat_buffer(), expected, n));
    }
}

/**
 * Make sure the walk methods can take lambdas.
 */
void mtm_test_walk_with_lambda()
{
    stack_printer __stack_printer__("::mtm_test_walk_with_lambda");
    vector<double> values = { 1.1, 1.2, 1.3, 1.4 };
    mtx_type mtx(2, 2, values.begin(), values.end());

    mtx.walk(
        [](const mtx_type::element_block_node_type& node)
        {
            assert(node.type == mdds::mtm::element_numeric);
            assert(node.offset == 0);
            assert(node.size == 4);
        }
    );

    struct section
    {
        mdds::mtm::element_t type;
        size_t offset;
        size_t size;

        bool operator== (const section& other) const
        {
            return type == other.type && offset == other.offset && size == other.size;
        }
    };

    std::vector<section> expected = {
        { mdds::mtm::element_numeric, 0, 1 },
        { mdds::mtm::element_numeric, 2, 1 },
    };

    std::vector<section> actual;

    mtx.walk(
        [&](const mtx_type::element_block_node_type& node)
        {
            actual.emplace_back();
            actual.back().type = node.type;
            actual.back().offset = node.offset;
            actual.back().size = node.size;
        }
        ,
        { 0, 0 }, { 0, 1 } // (row=0, column=0) to (row=0, column=1)
    );

    assert(expected == actual);
}

void mtm_test_parallel_walk_with_lambda()
{
    stack_printer __stack_printer__("::mtm_test_parallel_walk_with_lambda");

    vector<double> values = { 1.1, 1.2, 1.3, 1.4 };
    mtx_type mtx1(2, 2, values.begin(), values.end());

    mtx_type mtx2(2, 2);
    mtx2.set(0, 0, 2.2);
    mtx2.set(1, 0, 2.5);
    mtx2.set(0, 1, true);
    mtx2.set(1, 1, false);

    struct section
    {
        mdds::mtm::element_t type;
        size_t offset;
        size_t size;

        bool operator== (const section& other) const
        {
            return type == other.type && offset == other.offset && size == other.size;
        }
    };

    struct section_pair
    {
        section left;
        section right;

        bool operator== (const section_pair& other) const
        {
            return left == other.left && right == other.right;
        }
    };

    std::vector<section_pair> expected = {
        { { mdds::mtm::element_numeric, 0, 2 }, { mdds::mtm::element_numeric, 0, 2 } },
        { { mdds::mtm::element_numeric, 2, 2 }, { mdds::mtm::element_boolean, 0, 2 } },
    };

    std::vector<section_pair> actual;

    cout << "--" << endl;

    mtx1.walk(
        [&](const mtx_type::element_block_node_type& l, const mtx_type::element_block_node_type& r)
        {
            cout << "left: " << l << "; right: " << r << endl;
            actual.emplace_back();
            actual.back().left.type   = l.type;
            actual.back().left.offset = l.offset;
            actual.back().left.size   = l.size;
            actual.back().right.type   = r.type;
            actual.back().right.offset = r.offset;
            actual.back().right.size   = r.size;
        },
        mtx2
    );

    assert(expected == actual);

    mtx_type mtx3(4, 4, 1.0), mtx4(4, 4, std::string("A"));
    mtx3.set(2, 0, true);
    mtx3.set(3, 0, true);
    mtx4.set(0, 1, 2.2);
    mtx4.set(1, 1, 2.3);
    mtx4.set(0, 2, 1.1);
    mtx4.set(1, 2, 1.1);
    mtx3.set(1, 2, true);
    mtx3.set(2, 2, false);

    expected = {
        { { mdds::mtm::element_numeric, 0, 2 }, { mdds::mtm::element_string, 0, 2 } },
        { { mdds::mtm::element_boolean, 0, 1 }, { mdds::mtm::element_string, 2, 1 } },

        { { mdds::mtm::element_numeric, 0, 2 }, { mdds::mtm::element_numeric, 0, 2 } },
        { { mdds::mtm::element_numeric, 2, 1 }, { mdds::mtm::element_string, 0, 1 } },

        { { mdds::mtm::element_numeric, 4, 1 }, { mdds::mtm::element_numeric, 0, 1 } },
        { { mdds::mtm::element_boolean, 0, 1 }, { mdds::mtm::element_numeric, 1, 1 } },
        { { mdds::mtm::element_boolean, 1, 1 }, { mdds::mtm::element_string, 0, 1 } },
    };

    actual.clear();

    cout << "--" << endl;

    mtx3.walk(
        [&](const mtx_type::element_block_node_type& l, const mtx_type::element_block_node_type& r)
        {
            cout << "left: " << l << "; right: " << r << endl;
            actual.emplace_back();
            actual.back().left.type   = l.type;
            actual.back().left.offset = l.offset;
            actual.back().left.size   = l.size;
            actual.back().right.type   = r.type;
            actual.back().right.offset = r.offset;
            actual.back().right.size   = r.size;
        },
        mtx4,
        { 0, 0 }, { 2, 2 }
    );

    assert(expected.size() == actual.size());
    assert(expected[0] == actual[0]);
    assert(expected[1] == actual[1]);
    assert(expected[2] == actual[2]);
    assert(expected[3] == actual[3]);
    assert(expected[4] == actual[4]);
    assert(expected[5] == actual[5]);
    assert(expected[6] == actual[6]);
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
            mtm_test_walk();
            mtm_test_walk_subset();
            mtm_test_parallel_walk();
            mtm_test_parallel_walk_non_equal_size();
            mtm_test_walk_with_lambda();
            mtm_test_parallel_walk_with_lambda();
        }

        if (opt.test_perf)
        {
            // no perf test yet.
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
