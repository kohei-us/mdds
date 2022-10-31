/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2022 Kohei Yoshida
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
#include <iostream>

#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#include <mdds/multi_type_vector/soa/main.hpp>

using mdds::mtv::trace_method_t;

namespace {

bool starts_with(std::string_view s, std::string_view test)
{
    std::string_view ss = s.substr(0, test.size());
    return ss == test;
}

struct checked_method_props
{
    const void* instance = nullptr;
    std::string function_name;
    trace_method_t type = trace_method_t::unspecified;

    bool operator==(const checked_method_props& other) const
    {
        return instance == other.instance && function_name == other.function_name && type == other.type;
    }
};

std::ostream& operator<<(std::ostream& os, const checked_method_props& v)
{
    os << "{instance: " << v.instance << ", function_name: " << v.function_name << ", type: " << int(v.type);
    return os;
}

bool compare(const checked_method_props& lhs, const checked_method_props& rhs)
{
    bool res = lhs.instance == rhs.instance && lhs.function_name == rhs.function_name && lhs.type == rhs.type;
    if (!res)
    {
        std::cerr << "The two are not equal!" << std::endl;
        std::cerr << "lhs: " << lhs << std::endl;
        std::cerr << "rhs: " << rhs << std::endl;
    }
    return res;
}

class test_scope
{
    std::vector<checked_method_props> m_expected;
    std::vector<checked_method_props>& m_observed;
    int m_line_number;

public:
    test_scope(std::vector<checked_method_props>& observed, int line_number)
        : m_observed(observed), m_line_number(line_number)
    {
        std::cout << "--" << std::endl;
    }

    ~test_scope()
    {
        if (!std::equal(m_expected.cbegin(), m_expected.cend(), m_observed.cbegin(), compare))
        {
            std::cerr << "test failed (line=" << m_line_number << ")" << std::endl;
            assert(false);
        }
        m_observed.clear();
    }

    auto& expected()
    {
        return m_expected;
    }
};

std::vector<checked_method_props> observed;

struct mtv_custom_trait : public mdds::mtv::standard_element_blocks_traits
{
    static void trace(const mdds::mtv::trace_method_properties_t& props)
    {
        std::cout << "[" << props.instance << "]: {" << props.function_name << ": " << props.function_args
                  << "}; type=" << int(props.type) << std::endl;

        // Some compilers put an extra space after the 'operator'. Let's delete that extra space char.
        std::string func_name = props.function_name;
        if (starts_with(func_name, "operator "))
        {
            std::string rest = func_name.substr(9);
            func_name = "operator" + rest;
        }
        observed.push_back({props.instance, func_name, props.type});
    }
};

using mtv_type = mdds::mtv::soa::multi_type_vector<mtv_custom_trait>;

} // anonymous namespace

int main()
{
    {
        // Random assortment of calls (1)
        test_scope ts(observed, __LINE__);
        {
            mtv_type db(10);
            auto pos = db.begin();
            pos = db.set<std::string>(pos, 2, "str");
            pos = db.set<int32_t>(pos, 4, 23);

            [[maybe_unused]] std::string s = db.get<std::string>(2);
            [[maybe_unused]] bool b = db.is_empty(0);
            [[maybe_unused]] auto t = db.get_type(2);
            [[maybe_unused]] auto size = db.size();

            db.clear();

            ts.expected() = {
                {&db, "multi_type_vector", trace_method_t::constructor},
                {&db, "begin", trace_method_t::accessor},
                {&db, "set", trace_method_t::mutator_with_pos_hint},
                {&db, "set", trace_method_t::mutator_with_pos_hint},
                {&db, "get", trace_method_t::accessor},
                {&db, "is_empty", trace_method_t::accessor},
                {&db, "get_type", trace_method_t::accessor},
                {&db, "size", trace_method_t::accessor},
                {&db, "clear", trace_method_t::mutator},
                {&db, "~multi_type_vector", trace_method_t::destructor},
            };
        }
    }

    {
        // Random assortment of calls (2)
        test_scope ts(observed, __LINE__);
        {
            mtv_type db(10);
            db.set<std::string>(2, "str");
            db.set<int32_t>(4, 23);

            [[maybe_unused]] auto bs = db.block_size();
            [[maybe_unused]] bool b = db.empty();

            [[maybe_unused]] int32_t v;
            db.get(4, v);

            db.clear();

            [[maybe_unused]] auto it_end = db.end();

            ts.expected() = {
                {&db, "multi_type_vector", trace_method_t::constructor},
                {&db, "set", trace_method_t::mutator},
                {&db, "set", trace_method_t::mutator},
                {&db, "block_size", trace_method_t::accessor},
                {&db, "empty", trace_method_t::accessor},
                {&db, "get", trace_method_t::accessor},
                {&db, "clear", trace_method_t::mutator},
                {&db, "end", trace_method_t::accessor},
                {&db, "~multi_type_vector", trace_method_t::destructor},
            };
        }
    }

    {
        // constructors & event handler access
        test_scope ts(observed, __LINE__);
        {
            mtv_type db1;
            mtv_type db2{mtv_type::event_func()}; // move
            mtv_type::event_func ef;
            mtv_type db3(ef); // copy

            [[maybe_unused]] auto& ref_ef = db3.event_handler(); // non-const ref
            const mtv_type& cdb3 = db3;
            [[maybe_unused]] const auto& cref_ef = cdb3.event_handler(); // const ref

            mtv_type db4(20, true); // constructor with one init value
            std::vector<int32_t> values = {1, 2, 3, 4, 5};
            mtv_type db5(5, values.begin(), values.end()); // constructor with a series of values

            mtv_type db6(db5); // copy constructor
            mtv_type db7(std::move(db6)); // move constructor

            ts.expected() = {
                {&db1, "multi_type_vector", trace_method_t::constructor},
                {&db2, "multi_type_vector", trace_method_t::constructor},
                {&db3, "multi_type_vector", trace_method_t::constructor},
                {&db3, "event_handler", trace_method_t::accessor},
                {&db3, "event_handler", trace_method_t::accessor},
                {&db4, "multi_type_vector", trace_method_t::constructor},
                {&db5, "multi_type_vector", trace_method_t::constructor},
                {&db6, "multi_type_vector", trace_method_t::constructor},
                {&db7, "multi_type_vector", trace_method_t::constructor},

                {&db7, "~multi_type_vector", trace_method_t::destructor},
                {&db6, "~multi_type_vector", trace_method_t::destructor},
                {&db5, "~multi_type_vector", trace_method_t::destructor},
                {&db4, "~multi_type_vector", trace_method_t::destructor},
                {&db3, "~multi_type_vector", trace_method_t::destructor},
                {&db2, "~multi_type_vector", trace_method_t::destructor},
                {&db1, "~multi_type_vector", trace_method_t::destructor},
            };
        }
    }

    {
        // position methods
        test_scope ts(observed, __LINE__);
        {
            mtv_type db(10);
            const mtv_type& cdb = db; // const ref
            auto pos = db.position(0);
            auto pos_hint = db.begin();
            pos = db.position(pos_hint, 2);

            [[maybe_unused]] auto cpos = cdb.position(1); // const method
            cpos = cdb.position(pos_hint, 1);

            ts.expected() = {
                {&db, "multi_type_vector", trace_method_t::constructor},
                {&db, "position", trace_method_t::accessor},
                {&db, "begin", trace_method_t::accessor},
                {&db, "position", trace_method_t::accessor_with_pos_hint},
                {&db, "position", trace_method_t::accessor},
                {&db, "position", trace_method_t::accessor_with_pos_hint},
                {&db, "~multi_type_vector", trace_method_t::destructor},
            };
        }
    }

    {
        // set, push_back, insert, set_empty, erase, and insert_empty methods
        test_scope ts(observed, __LINE__);
        {
            mtv_type db(10);
            std::vector<uint8_t> values = {3, 4, 5, 6};
            auto pos_hint = db.set(2, values.begin(), values.end());
            pos_hint = db.set(pos_hint, 4, values.begin(), values.end());
            db.push_back<int16_t>(456);
            db.push_back_empty();
            pos_hint = db.insert(0, values.begin(), values.end());
            db.insert(pos_hint, 0, values.begin(), values.end());
            pos_hint = db.set_empty(0, 3);
            pos_hint = db.set_empty(pos_hint, 4, 5);
            db.erase(2, 3);
            pos_hint = db.insert_empty(3, 10);
            db.insert_empty(pos_hint, 15, 2);

            ts.expected() = {
                {&db, "multi_type_vector", trace_method_t::constructor},
                {&db, "set", trace_method_t::mutator},
                {&db, "set", trace_method_t::mutator_with_pos_hint},
                {&db, "push_back", trace_method_t::mutator},
                {&db, "push_back_empty", trace_method_t::mutator},
                {&db, "insert", trace_method_t::mutator},
                {&db, "insert", trace_method_t::mutator_with_pos_hint},
                {&db, "set_empty", trace_method_t::mutator},
                {&db, "set_empty", trace_method_t::mutator_with_pos_hint},
                {&db, "erase", trace_method_t::mutator},
                {&db, "insert_empty", trace_method_t::mutator},
                {&db, "insert_empty", trace_method_t::mutator_with_pos_hint},
                {&db, "~multi_type_vector", trace_method_t::destructor},
            };
        }
    }

    {
        // transfer
        test_scope ts(observed, __LINE__);
        {
            mtv_type src(10, true), dst(10);
            auto pos_hint = src.transfer(0, 3, dst, 0);
            pos_hint = src.transfer(pos_hint, 6, 8, dst, 6);

            ts.expected() = {
                {&src, "multi_type_vector", trace_method_t::constructor},
                {&dst, "multi_type_vector", trace_method_t::constructor},
                // transfer() calls destination's size() and set_empty() internally.
                {&src, "transfer", trace_method_t::mutator},
                {&dst, "size", trace_method_t::accessor},
                {&dst, "set_empty", trace_method_t::mutator},
                // same here...
                {&src, "transfer", trace_method_t::mutator_with_pos_hint},
                {&dst, "size", trace_method_t::accessor},
                {&dst, "set_empty", trace_method_t::mutator},
                {&dst, "~multi_type_vector", trace_method_t::destructor},
                {&src, "~multi_type_vector", trace_method_t::destructor},
            };
        }
    }

    {
        // release, which has 4 variants.
        test_scope ts(observed, __LINE__);
        {
            mtv_type db(10, int32_t(42));

            int32_t v = db.release<int32_t>(0); // variant 1
            auto pos_hint = db.release(1, v); // variant 2
            pos_hint = db.release(pos_hint, 2, v); // variant 3
            db.release(); // final variant

            ts.expected() = {
                {&db, "multi_type_vector", trace_method_t::constructor},
                {&db, "release", trace_method_t::mutator},
                {&db, "release", trace_method_t::mutator},
                {&db, "release", trace_method_t::mutator_with_pos_hint},
                {&db, "release", trace_method_t::mutator},
                {&db, "~multi_type_vector", trace_method_t::destructor},
            };
        }
    }

    {
        // release_range (2 variants)
        test_scope ts(observed, __LINE__);
        {
            mtv_type db(10, int32_t(42));
            auto pos_hint = db.release_range(0, 2);
            pos_hint = db.release_range(pos_hint, 5, 7);

            ts.expected() = {
                {&db, "multi_type_vector", trace_method_t::constructor},
                {&db, "release_range", trace_method_t::mutator},
                {&db, "release_range", trace_method_t::mutator_with_pos_hint},
                {&db, "~multi_type_vector", trace_method_t::destructor},
            };
        }
    }

    {
        // iterator accessors.
        test_scope ts(observed, __LINE__);
        {
            mtv_type db(10);
            auto it = db.begin();
            it = db.end();
            const mtv_type& cdb = db;
            auto cit = cdb.cbegin();
            cit = cdb.cend();

            cit = cdb.begin();
            cit = cdb.end();

            auto rit = db.rbegin();
            rit = db.rend();

            auto crit = cdb.rbegin();
            crit = cdb.rend();

            crit = cdb.crbegin();
            crit = cdb.crend();

            ts.expected() = {
                {&db, "multi_type_vector", trace_method_t::constructor},
                {&db, "begin", trace_method_t::accessor},
                {&db, "end", trace_method_t::accessor},
                {&db, "cbegin", trace_method_t::accessor},
                {&db, "cend", trace_method_t::accessor},
                {&db, "begin", trace_method_t::accessor},
                {&db, "end", trace_method_t::accessor},
                {&db, "rbegin", trace_method_t::accessor},
                {&db, "rend", trace_method_t::accessor},
                {&db, "rbegin", trace_method_t::accessor},
                {&db, "rend", trace_method_t::accessor},
                {&db, "crbegin", trace_method_t::accessor},
                {&db, "crend", trace_method_t::accessor},
                {&db, "~multi_type_vector", trace_method_t::destructor},
            };
        }
    }

    {
        // resize, swap, shrink_to_fit and (non-)equality operators.
        test_scope ts(observed, __LINE__);
        {
            mtv_type db;
            db.resize(10);

            mtv_type db2(10);
            db2.swap(db);

            db2.swap(0, 2, db, 0);
            db2.shrink_to_fit();

            [[maybe_unused]] bool b = db == db2;
            b = db != db2;

            ts.expected() = {
                {&db, "multi_type_vector", trace_method_t::constructor},
                {&db, "resize", trace_method_t::mutator},
                {&db2, "multi_type_vector", trace_method_t::constructor},
                {&db2, "swap", trace_method_t::mutator},
                {&db2, "swap", trace_method_t::mutator},
                {&db2, "shrink_to_fit", trace_method_t::mutator},
                {&db, "operator==", trace_method_t::accessor},
                {&db, "operator!=", trace_method_t::accessor},
                {&db2, "~multi_type_vector", trace_method_t::destructor},
                {&db, "~multi_type_vector", trace_method_t::destructor},
            };
        }
    }

    {
        std::cout << "--" << std::endl;

        // assignment operators.  These methods internally call other public
        // methods that cannot be tested, so we only check the first trace call.
        mtv_type db, db2;
        observed.clear();
        db = db2; // copy
        checked_method_props expected{&db, "operator=", trace_method_t::mutator};
        assert(compare(observed.at(0), expected));

        observed.clear();
        db = std::move(db2); // move
        assert(compare(observed.at(0), expected));

        observed.clear();
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
