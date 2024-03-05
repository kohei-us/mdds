/*************************************************************************
 *
 * Copyright (c) 2010-2023 Kohei Yoshida
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

#include "test_main.hpp"

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

using namespace mdds;

template<typename key_type, typename value_type>
void build_and_dump(segment_tree<key_type, value_type>& db)
{
    db.build_tree();
    std::cout << db.to_string() << std::endl;
}

struct test_data
{
    std::string name; // data structure expects the data to have 'name' data member.

    test_data(const std::string& s) : name(s)
    {}

    /**
     * Use this to sort instances of test_data by name, in ascending order.
     */
    struct sort_by_name
    {
        bool operator()(const test_data* left, const test_data* right) const
        {
            return left->name < right->name;
        }
    };

    struct name_printer
    {
        void operator()(const test_data* p) const
        {
            cout << p->name << " ";
        }
    };
};

std::ostream& operator<<(std::ostream& os, const test_data* v)
{
    os << v->name;
    return os;
}

std::ostream& operator<<(std::ostream& os, const test_data& v)
{
    os << v.name;
    return os;
}

template<typename StoreT>
struct segment_data_type
{
    using key_type = typename StoreT::key_type;
    using value_type = typename StoreT::value_type;

    key_type begin_key;
    key_type end_key;
    value_type value;

    segment_data_type(key_type _beg, key_type _end, value_type p) : begin_key(_beg), end_key(_end), value(p)
    {}

    bool operator==(const segment_data_type& r) const
    {
        return begin_key == r.begin_key && end_key == r.end_key && value == r.value;
    }

    bool operator!=(const segment_data_type& r) const
    {
        return !operator==(r);
    }
};

template<typename value_type>
bool check_against_expected(const std::list<value_type>& test, value_type* expected)
{
    size_t i = 0;
    value_type p = expected[i++];
    typename std::list<value_type>::const_iterator itr = test.begin(), itr_end = test.end();
    while (p)
    {
        if (itr == itr_end)
            // data chain ended prematurely.
            return false;

        if (*itr != p)
            // the value is not as expected.
            return false;

        p = expected[i++];
        ++itr;
    }
    if (itr != itr_end)
        // data chain is too long.
        return false;

    return true;
}

template<typename ValueT, typename ResT>
std::vector<ValueT> to_vector(const ResT& results)
{
    std::vector<ValueT> vec;
    for (const auto& v : results)
        vec.push_back(v.value);
    return vec;
}

/**
 * Only check the search result against expected result set.  The caller
 * needs to run search and pass the result to this function.
 */
template<typename KeyT, typename ValueT>
bool check_search_result_only(std::vector<ValueT> results, KeyT key, ValueT* expected)
{
    cout << "search key: " << key << " ";

    std::list<ValueT> test;
    std::copy(results.begin(), results.end(), std::back_inserter(test));
    test.sort(test_data::sort_by_name());

    cout << "search result (sorted): ";
    std::for_each(test.begin(), test.end(), test_data::name_printer());
    cout << endl;

    return check_against_expected(test, expected);
}

/**
 * Run the search and check the search result.
 */
template<typename KeyT, typename ValueT>
bool check_search_result(const segment_tree<KeyT, ValueT>& db, KeyT key, ValueT* expected)
{
    cout << "search key: " << key << " ";

    auto results = db.search(key);
    return check_search_result_only(to_vector<ValueT>(results), key, expected);
}

template<typename key_type, typename value_type>
bool check_search_result_iterator(const segment_tree<key_type, value_type>& db, key_type key, value_type* expected)
{
    cout << "search key: " << key << " ";

    typedef segment_tree<key_type, value_type> db_type;
    typename db_type::search_results results = db.search(key);
    std::list<value_type> test;
    for (const auto& v : results)
        test.push_back(v.value);

    test.sort(test_data::sort_by_name());

    cout << "search result (sorted): ";
    std::for_each(test.begin(), test.end(), test_data::name_printer());
    cout << endl;

    return check_against_expected(test, expected);
}

template<typename TreeT>
bool check_integrity(const TreeT& db, const typename TreeT::integrity_check_properties& props)
{
    try
    {
        db.check_integrity(props);
    }
    catch (const mdds::integrity_error& e)
    {
        std::cout << e.what() << std::endl;
        return false;
    }

    return true;
}

void st_test_insert_search_removal()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef long key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    db_type db;
    value_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");

    build_and_dump(db);
    assert(db_type::node::get_instance_count() == 0);

    db.insert(0, 10, &A);
    build_and_dump(db);
    {
        assert(db_type::node::get_instance_count() == db.leaf_size());

        db_type::integrity_check_properties props;
        // clang-format off
        props.leaf_nodes = {
            // key, value chain
            {0u, {&A,}},
            {10u, {}},
        };
        // clang-format on

        assert(check_integrity(db, props));
    }

    db.insert(0, 5, &B);
    build_and_dump(db);
    {
        assert(db_type::node::get_instance_count() == db.leaf_size());

        db_type::integrity_check_properties props;
        // clang-format off
        props.leaf_nodes = {
            // key, value chain
            {0u, {&A, &B,}},
            {5u, {&A,}},
            {10u, {}},
        };
        // clang-format on

        assert(check_integrity(db, props));
    }

    db.insert(5, 12, &C);
    build_and_dump(db);
    {
        assert(db_type::node::get_instance_count() == db.leaf_size());

        db_type::integrity_check_properties props;
        // clang-format off
        props.leaf_nodes = {
            // key, value chain
            {0u, {&A, &B,}},
            {5u, {&A, &C}},
            {10u, {&C,}},
            {12u, {}},
        };
        // clang-format on

        assert(check_integrity(db, props));
    }

    db.insert(10, 24, &D);
    build_and_dump(db);
    {
        assert(db_type::node::get_instance_count() == db.leaf_size());

        db_type::integrity_check_properties props;
        // clang-format off
        props.leaf_nodes = {
            // key, value chain
            {0u, {&A, &B,}},
            {5u, {&A, &C}},
            {10u, {&C, &D}},
            {12u, {&D,}},
            {24u, {}},
        };
        // clang-format on

        assert(check_integrity(db, props));
    }

    db.insert(4, 24, &E);
    build_and_dump(db);
    {
        assert(db_type::node::get_instance_count() == db.leaf_size());

        db_type::integrity_check_properties props;
        // clang-format off
        props.leaf_nodes = {
            // key, value chain
            {0u, {&B,}},
            {4u, {&B, &E}},
            {5u, {&A, &C}},
            {10u, {&C, &D}},
            {12u, {&D, &E}},
            {24u, {}},
        };
        // clang-format on

        assert(check_integrity(db, props));
    }

    db.insert(0, 26, &F);
    build_and_dump(db);
    {
        assert(db_type::node::get_instance_count() == db.leaf_size());

        db_type::integrity_check_properties props;
        // clang-format off
        props.leaf_nodes = {
            // key, value chain
            {0u, {&B,}},
            {4u, {&B, &E}},
            {5u, {&A, &C}},
            {10u, {&C, &D}},
            {12u, {&D, &E, &F}},
            {24u, {&F,}},
            {26u, {}},
        };
        // clang-format on

        assert(check_integrity(db, props));
    }

    db.insert(12, 26, &G);
    build_and_dump(db);
    {
        assert(db_type::node::get_instance_count() == db.leaf_size());

        db_type::integrity_check_properties props;
        // clang-format off
        props.leaf_nodes = {
            // key, value chain
            {0u, {&B,}},
            {4u, {&B, &E}},
            {5u, {&A, &C}},
            {10u, {&C, &D}},
            {12u, {&D, &E, &F, &G}},
            {24u, {&F, &G}},
            {26u, {}},
        };
        // clang-format on

        assert(check_integrity(db, props));
    }

    // Search tests.  Test boundary cases.

    for (key_type i = -10; i <= 30; ++i)
    {
        auto results = db.search(i);
        std::cout << "search key " << i << ": ";
        for (const auto& result : results)
            std::cout << result.value->name << " ";

        std::cout << std::endl;
    }

    {
        key_type key = -1;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 0;
        value_type* expected[] = {&A, &B, &F, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 4;
        value_type* expected[] = {&A, &B, &E, &F, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 5;
        value_type* expected[] = {&A, &C, &E, &F, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 10;
        value_type* expected[] = {&C, &D, &E, &F, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 12;
        value_type* expected[] = {&D, &E, &F, &G, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 24;
        value_type* expected[] = {&F, &G, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 30;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 9999;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    // Remove E, F and G and check search results.
    // E: 4-24; F: 0-26; G: 12-26
    {
        auto count_before = db.size();
        auto res = db.search(10);
        assert(!res.empty());
        bool erased = false;
        for (auto it = res.begin(); it != res.end(); ++it)
        {
            if (it->value == &E)
            {
                db.erase(it);
                erased = true;
                break;
            }
        }
        assert(erased);
        assert(db.size() == count_before - 1);
    }

    {
        auto count_before = db.size();
        auto res = db.search(10);
        assert(!res.empty());
        bool erased = false;
        for (auto it = res.begin(); it != res.end(); ++it)
        {
            if (it->value == &F)
            {
                db.erase(it);
                erased = true;
                break;
            }
        }
        assert(erased);
        assert(db.size() == count_before - 1);
    }

    {
        auto count_before = db.size();
        auto res = db.search(20);
        assert(!res.empty());
        bool erased = false;
        for (auto it = res.begin(); it != res.end(); ++it)
        {
            if (it->value == &G)
            {
                db.erase(it);
                erased = true;
                break;
            }
        }
        assert(erased);
        assert(db.size() == count_before - 1);
    }

    cout << "removed: E F G" << endl;
    cout << db.to_string() << endl;

    for (key_type i = -10; i <= 30; ++i)
    {
        auto results = db.search(i);
        std::cout << "search key " << i << ": ";
        for (const auto& result : results)
            std::cout << result.value->name << " ";
        std::cout << std::endl;
    }

    {
        key_type key = -1;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 0;
        value_type* expected[] = {&A, &B, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 4;
        value_type* expected[] = {&A, &B, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 5;
        value_type* expected[] = {&A, &C, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 10;
        value_type* expected[] = {&C, &D, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 12;
        value_type* expected[] = {&D, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 24;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 30;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 9999;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    // Re-build the tree and check the search results once again, to make sure
    // we get the same results.

    db.build_tree();
    cout << db.to_string() << endl;

    {
        key_type key = -1;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 0;
        value_type* expected[] = {&A, &B, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 4;
        value_type* expected[] = {&A, &B, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 5;
        value_type* expected[] = {&A, &C, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 10;
        value_type* expected[] = {&C, &D, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 12;
        value_type* expected[] = {&D, 0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 24;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }

    {
        key_type key = 30;
        value_type* expected[] = {0};
        assert(check_search_result(db, key, expected));
    }
}

void st_test_invalid_insertion()
{
    MDDS_TEST_FUNC_SCOPE;

    using db_type = segment_tree<int, std::string>;
    db_type db;

    try
    {
        db.insert(2, 1, "some value");
        assert(!"exception didn't get thrown!");
    }
    catch (const std::invalid_argument&)
    {
        // expected
    }
    catch (...)
    {
        assert(!"wrong exception caught");
    }
}

void st_test_copy_constructor()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef long key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;
    using segment_data = segment_data_type<db_type>;

    db_type db;
    value_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");
    std::vector<segment_data> segments;
    segments.push_back(segment_data(0, 10, &A));
    segments.push_back(segment_data(0, 5, &B));
    segments.push_back(segment_data(5, 12, &C));
    segments.push_back(segment_data(10, 24, &D));
    segments.push_back(segment_data(4, 24, &E));
    segments.push_back(segment_data(0, 26, &F));
    segments.push_back(segment_data(12, 26, &G));
    segments.push_back(segment_data(0, 0, nullptr)); // null-terminated

    // Copy before the tree is built.

    cout << "--" << endl;
    cout << db.to_string() << endl;

    db_type db_copied(db);
    cout << "--" << endl;
    cout << db_copied.to_string() << endl;
    assert(db.valid_tree() == db_copied.valid_tree());
    assert(db == db_copied);

    db_type db_assigned;
    db_assigned = db_copied; // copy assignment
    assert(db_assigned.valid_tree() == db_copied.valid_tree());
    assert(db_assigned == db_copied);

    // Copy after the tree is built.
    db.build_tree();
    db_type db_copied_tree(db);
    cout << "--" << endl;
    cout << db_copied_tree.to_string() << endl;
    assert(db.valid_tree() == db_copied_tree.valid_tree());
    assert(db == db_copied_tree);

    db_assigned = db_copied_tree; // copy assignment
    assert(db_assigned.valid_tree() == db_copied_tree.valid_tree());
    assert(db_assigned == db_copied_tree);
}

void st_test_equality()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef uint32_t key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    value_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");
    {
        db_type db1, db2;
        db1.insert(0, 10, &A);
        db2.insert(0, 10, &A);
        assert(db1 == db2);
        db2.insert(5, 12, &B);
        assert(db1 != db2);
        db1.insert(5, 12, &C);
        assert(db1 != db2);
        auto n_removed = db1.erase_if([&C](key_type, key_type, const auto& value) { return value == &C; });
        assert(n_removed == 1);
        n_removed = db2.erase_if([&B](key_type, key_type, const auto& value) { return value == &B; });
        assert(n_removed == 1);
        assert(db1 == db2);
        db1.insert(4, 20, &D);
        db2.insert(4, 20, &D);
        assert(db1 == db2);
        db1.insert(3, 12, &E);
        db2.insert(3, 15, &E);
        assert(db1 != db2);
    }

    {
        // different insertion orders
        db_type db1, db2;
        assert(db1 == db2);
        db1.insert(0, 10, &A);
        db1.insert(10, 20, &B);
        db2.insert(10, 20, &B);
        db2.insert(0, 10, &A);
        assert(db1 == db2);
    }

    {
        // one contains a deleted segment
        db_type db1, db2;
        db1.insert(0, 10, &C);
        assert(!db1.empty());
        assert(db2.empty());
        assert(db1 != db2);
        db1.erase_if([](key_type start, key_type, const auto&) { return start == 0; });
        assert(db1 == db2);
        assert(db1.empty());
        assert(db2.empty());
    }
}

void st_test_clear()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef uint8_t key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;
    using segment_data = segment_data_type<db_type>;

    value_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");

    std::vector<segment_data> segments;
    segments.push_back(segment_data(0, 10, &A));
    segments.push_back(segment_data(0, 5, &B));
    segments.push_back(segment_data(5, 12, &C));
    segments.push_back(segment_data(10, 24, &D));
    segments.push_back(segment_data(4, 24, &E));
    segments.push_back(segment_data(0, 26, &F));
    segments.push_back(segment_data(12, 26, &G));
    segments.push_back(segment_data(0, 0, nullptr)); // null-terminated

    db_type db;
    for (size_t i = 0; segments[i].value; ++i)
        db.insert(segments[i].begin_key, segments[i].end_key, segments[i].value);

    assert(!db.empty());
    assert(db.size() == 7);
    cout << "size of db is " << db.size() << endl;

    db.clear();
    assert(db.empty());
    assert(db.size() == 0);

    // Insert the same data set once again, but this time build tree afterwards.
    for (size_t i = 0; segments[i].value; ++i)
        db.insert(segments[i].begin_key, segments[i].end_key, segments[i].value);

    db.build_tree();
    assert(!db.empty());
    assert(db.size() == 7);

    db.clear();
    assert(db.empty());
    assert(db.size() == 0);
}

void st_test_duplicate_insertion()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef short key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    value_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");

    db_type db;
    db.insert(0, 10, &A);
    db.insert(0, 10, &A); // duplicate segments are allowed
    db.insert(2, 30, &A);
    db.insert(0, 10, &B);
    assert(db.size() == 4);
    auto n_removed = db.erase_if(
        [&A](key_type start, key_type end, const auto& value) { return start == 0 && end == 10 && value == &A; });
    assert(n_removed == 2);
    assert(db.size() == 2);
    db.insert(2, 30, &A);
    assert(db.size() == 3);
    build_and_dump(db);
    assert(db.size() == 3);
    assert(db.leaf_size() == 4); // 0, 2, 10, 30
}

/**
 * When the number of segments is not a multiple of 2, it creates a tree
 * where the right side becomes "cut off".  Make sure the search works
 * correctly under those conditions.
 */
void st_test_search_on_uneven_tree()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef int16_t key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    for (key_type data_count = 10; data_count < 20; ++data_count)
    {
        std::vector<std::unique_ptr<test_data>> data_store;
        data_store.reserve(data_count);
        for (key_type i = 0; i < data_count; ++i)
        {
            std::ostringstream os;
            os << std::hex << std::showbase << i;
            data_store.emplace_back(new test_data(os.str()));
        }
        assert(data_store.size() == static_cast<size_t>(data_count));

        db_type db;
        for (key_type i = 0; i < data_count; ++i)
        {
            test_data* p = data_store[i].get();
            db.insert(0, i + 1, p);
        }
        assert(db.size() == static_cast<size_t>(data_count));

        db.build_tree();

        for (key_type i = -1; i < data_count + 1; ++i)
        {
            auto results = db.search(i);
            std::cout << "search key: " << i << "  result: ";
            for (const auto& result : results)
                std::cout << result.value->name << " ";
            std::cout << endl;
        }
    }
}

void st_test_perf_insertion()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef uint32_t key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    key_type data_count = 1000000;

    // First, create test data instances and store them into a vector.
    std::vector<std::unique_ptr<test_data>> data_store;
    {
        stack_printer __stack_printer2__("::st_test_perf_insertion:: data array creation");
        data_store.reserve(data_count);
        for (key_type i = 0; i < data_count; ++i)
        {
            std::ostringstream os;
            os << std::hex << i;
            data_store.emplace_back(new test_data(os.str()));
        }
    }
    assert(data_store.size() == data_count);

    db_type db;
    {
        stack_printer __stack_printer2__("::st_test_perf_insertion:: data array insertion into segment tree");
        for (key_type i = 0; i < data_count; ++i)
        {
            test_data* p = data_store[i].get();
            db.insert(0, i + 1, p);
        }
    }
    assert(db.size() == data_count);

    {
        stack_printer __stack_printer2__("::st_test_perf_insertion:: build tree");
        db.build_tree();
    }
    assert(db.valid_tree());

    {
        stack_printer __stack_printer2__("::st_test_perf_insertion:: 200 searches with max results");
        for (key_type i = 0; i < 200; ++i)
        {
            auto results = db.search(0);
            assert(results.size() == data_count);
            auto it = results.begin();
            ++it;
            assert(it->value);
        }
    }

    {
        stack_printer __stack_printer2__("::st_test_perf_insertion:: 200 searches with median results");
        for (key_type i = 0; i < 200; ++i)
        {
            auto results = db.search(data_count / 2);
            assert(results.size() == data_count / 2);
            auto it = results.begin();
            ++it;
            assert(it->value);
        }
    }

    {
        stack_printer __stack_printer2__("::st_test_perf_insertion:: 200 searches with empty results");
        for (key_type i = 0; i < 200; ++i)
        {
            auto results = db.search(data_count);
            assert(results.size() == 0);
            assert(results.begin() == results.end());
        }
    }

    {
        auto db_copy = db;
        stack_printer __stack_printer2__("::st_test_perf_insertion:: first 500 segment removals");
        for (key_type i = 0; i < 500; ++i)
        {
            test_data* p = data_store[i].get();
            db_copy.erase_if([p](key_type, key_type, const auto& value) { return value == p; });
        }
    }
    assert(db.size() == data_count);

    {
        auto db_copy = db;
        stack_printer __stack_printer2__("::st_test_perf_insertion:: last 500 segment removals");
        for (key_type i = 0; i < 500; ++i)
        {
            test_data* p = data_store[data_store.size() - i - 1].get();
            db_copy.erase_if([p](key_type, key_type, const auto& value) { return value == p; });
        }
    }
    assert(db.size() == data_count);

    {
        stack_printer __stack_printer2__("::st_test_perf_insertion:: clear");
        db.clear();
    }
}

void st_test_aggregated_search_results()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef uint16_t key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;
    using segment_data = segment_data_type<db_type>;

    value_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");

    std::vector<segment_data> segments;
    segments.push_back(segment_data(0, 10, &A));
    segments.push_back(segment_data(0, 5, &B));
    segments.push_back(segment_data(5, 12, &C));
    segments.push_back(segment_data(10, 24, &D));
    segments.push_back(segment_data(4, 24, &E));
    segments.push_back(segment_data(0, 26, &F));
    segments.push_back(segment_data(12, 26, &G));
    segments.push_back(segment_data(0, 0, nullptr)); // null-terminated

    db_type db;
    for (size_t i = 0; segments[i].value; ++i)
        db.insert(segments[i].begin_key, segments[i].end_key, segments[i].value);

    db.build_tree();

    std::vector<test_data*> results;
    {
        key_type key = 0;
        auto res = db.search(key);
        for (const auto& v : res)
            results.push_back(v.value);

        value_type* expected[] = {&A, &B, &F, 0};
        assert(check_search_result_only(results, key, expected));
    }

    {
        key_type key = 10;
        auto res = db.search(key);
        for (const auto& v : res)
            results.push_back(v.value);

        // Note the duplicated F's in the search result.
        value_type* expected[] = {&A, &B, &C, &D, &E, &F, &F, 0};
        assert(check_search_result_only(results, key, expected));
    }

    {
        key_type key = 5;
        auto res = db.search(key);
        for (const auto& v : res)
            results.push_back(v.value);

        value_type* expected[] = {&A, &A, &B, &C, &C, &D, &E, &E, &F, &F, &F, 0};
        assert(check_search_result_only(results, key, expected));
    }

    {
        results.clear(); // clear the accumulated result set.
        key_type key = 5;
        auto res = db.search(key);
        for (const auto& v : res)
            results.push_back(v.value);

        value_type* expected[] = {&A, &C, &E, &F, 0};
        assert(check_search_result_only(results, key, expected));
    }
}

void st_test_dense_tree_search()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef uint16_t key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    value_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");
    db_type db;
    db.insert(0, 1, &A);
    db.insert(0, 2, &B);
    db.insert(0, 3, &C);
    db.insert(0, 4, &D);
    db.insert(0, 5, &E);
    db.insert(0, 6, &F);
    db.insert(0, 7, &G);
    db.build_tree();
    cout << "--" << endl;
    cout << db.to_string() << endl;

    {
        db_type::value_type expected[] = {&A, &B, &C, &D, &E, &F, &G, 0};
        bool success = check_search_result<key_type, value_type*>(db, 0, expected);
        assert(success);
    }
    {
        db_type::value_type expected[] = {&B, &C, &D, &E, &F, &G, 0};
        bool success = check_search_result<key_type, value_type*>(db, 1, expected);
        assert(success);
    }
    {
        db_type::value_type expected[] = {&C, &D, &E, &F, &G, 0};
        bool success = check_search_result<key_type, value_type*>(db, 2, expected);
        assert(success);
    }
    {
        db_type::value_type expected[] = {&D, &E, &F, &G, 0};
        bool success = check_search_result<key_type, value_type*>(db, 3, expected);
        assert(success);
    }
    {
        db_type::value_type expected[] = {&E, &F, &G, 0};
        bool success = check_search_result<key_type, value_type*>(db, 4, expected);
        assert(success);
    }
    {
        db_type::value_type expected[] = {&F, &G, 0};
        bool success = check_search_result<key_type, value_type*>(db, 5, expected);
        assert(success);
    }
    {
        db_type::value_type expected[] = {&G, 0};
        bool success = check_search_result<key_type, value_type*>(db, 6, expected);
        assert(success);
    }
    {
        db_type::value_type expected[] = {0};
        bool success = check_search_result<key_type, value_type*>(db, 7, expected);
        assert(success);
    }
}

void st_test_search_on_empty_set()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef uint16_t key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    db_type db;
    db.build_tree();

    // Search on an empty set should still be considered a success as long as
    // the tree is built beforehand.
    auto results = db.search(0);
    assert(results.size() == 0);
    assert(results.empty());
    assert(results.begin() == results.end());
}

void st_test_search_iterator_basic()
{
    MDDS_TEST_FUNC_SCOPE;
    typedef uint16_t key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    value_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");
    db_type db;
    db.insert(0, 1, &A);
    db.insert(0, 2, &B);
    db.insert(0, 3, &C);
    db.insert(0, 4, &D);
    db.insert(0, 5, &E);
    db.insert(0, 6, &F);
    db.insert(0, 7, &G);
    db.build_tree();
    cout << "--" << endl;
    cout << db.to_string() << endl;

    db_type::search_results results = db.search(0);
    assert(results.size() == 7);
    db_type::search_results::const_iterator itr;
    db_type::search_results::const_iterator itr_beg = results.begin();
    db_type::search_results::const_iterator itr_end = results.end();
    cout << "Iterate through the search results." << endl;
    for (itr = itr_beg; itr != itr_end; ++itr)
        cout << (*itr).value->name << " ";
    cout << endl;

    cout << "Do it again." << endl;
    for (itr = itr_beg; itr != itr_end; ++itr)
        cout << (*itr).value->name << " ";
    cout << endl;

    cout << "Iterate backwards" << endl;
    do
    {
        --itr;
        cout << (*itr).value->name << " ";
    } while (itr != itr_beg);
    cout << endl;

    cout << "Get the last item from the end position." << endl;
    itr = itr_end;
    --itr;
    cout << (*itr).value->name << endl;

    cout << "Use std::for_each to print names." << endl;
    std::for_each(itr_beg, itr_end, [](const auto& v) { std::cout << v.value->name; });
    cout << endl;
}

void st_test_search_iterator_result_check()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef uint16_t key_type;
    typedef test_data value_type;
    typedef segment_tree<key_type, value_type*> db_type;

    value_type A("A"), B("B"), C("C"), D("D"), E("E"), F("F"), G("G");
    db_type db;
    db.insert(0, 1, &A);
    db.insert(0, 2, &B);
    db.insert(0, 3, &C);
    db.insert(0, 4, &D);
    db.insert(0, 5, &E);
    db.insert(0, 6, &F);
    db.insert(0, 7, &G);
    db.build_tree();

    {
        value_type* expected[] = {&A, &B, &C, &D, &E, &F, &G, 0};
        bool success = check_search_result_iterator<key_type, value_type*>(db, 0, expected);
        assert(success);
    }
    {
        value_type* expected[] = {&B, &C, &D, &E, &F, &G, 0};
        bool success = check_search_result_iterator<key_type, value_type*>(db, 1, expected);
        assert(success);
    }
    {
        value_type* expected[] = {&C, &D, &E, &F, &G, 0};
        bool success = check_search_result_iterator<key_type, value_type*>(db, 2, expected);
        assert(success);
    }
    {
        value_type* expected[] = {&D, &E, &F, &G, 0};
        bool success = check_search_result_iterator<key_type, value_type*>(db, 3, expected);
        assert(success);
    }
    {
        value_type* expected[] = {&E, &F, &G, 0};
        bool success = check_search_result_iterator<key_type, value_type*>(db, 4, expected);
        assert(success);
    }
    {
        value_type* expected[] = {&F, &G, 0};
        bool success = check_search_result_iterator<key_type, value_type*>(db, 5, expected);
        assert(success);
    }
    {
        value_type* expected[] = {&G, 0};
        bool success = check_search_result_iterator<key_type, value_type*>(db, 6, expected);
        assert(success);
    }
    {
        value_type* expected[] = {0};
        bool success = check_search_result_iterator<key_type, value_type*>(db, 7, expected);
        assert(success);
    }
}

/**
 * When calling search() on empty tree, even without calling build_tree()
 * should still return a valid search_result instance with a size of 0.
 */
void st_test_empty_result_set()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef segment_tree<long, std::string*> db_type;
    db_type db;
    db_type::search_results results = db.search(0);
    cout << "size of empty result set: " << results.size() << endl;
    assert(results.size() == 0);
    assert(results.empty());
}

void st_test_non_pointer_data()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef uint16_t key_type;
    typedef size_t value_type;
    typedef segment_tree<key_type, value_type> db_type;

    db_type db;
    db.insert(0, 1, 10);
    db.build_tree();

    db_type::search_results results = db.search(0);
    assert(results.size() == 1);
    assert(!results.empty());
    assert(results.begin()->value == 10);
}

void st_test_erase_on_invalid_tree()
{
    MDDS_TEST_FUNC_SCOPE;

    using db_type = segment_tree<int, std::string>;

    db_type db;
    db.insert(0, 5, "A");
    db.insert(-10, 2, "B");
    db.insert(2, 10, "C");
    db.build_tree();
    assert(db.valid_tree());

    {
        // It should contains the "C" segment.
        auto results = db.search(5);
        auto it = std::find_if(results.begin(), results.end(), [](const auto& v) { return v.value == "C"; });
        assert(it != results.end());
    }

    db.insert(3, 15, "D");
    assert(!db.valid_tree());
    // Remove a segment while the tree is invalid.
    db.erase_if([](int, int, const auto& v) { return v == "C"; });
    db.build_tree();

    {
        // It should no longer contain the "C" segment.
        auto results = db.search(5);
        auto it = std::find_if(results.begin(), results.end(), [](const auto& v) { return v.value == "C"; });
        assert(it == results.end());
    }
}

void st_test_boundary_keys()
{
    MDDS_TEST_FUNC_SCOPE;

    using db_type = segment_tree<int16_t, bool>;

    db_type db;
    auto keys = db.boundary_keys();
    assert(keys.empty());

    auto expected = keys;
    expected = {1, 3};

    db.insert(1, 3, true);
    assert(db.size() == 1);
    keys = db.boundary_keys();
    assert(keys == expected);

    db.insert(3, 5, true);
    assert(db.size() == 2);
    keys = db.boundary_keys();
    expected = {1, 3, 5};
    assert(keys == expected);

    db.insert(7, 10, false);
    assert(db.size() == 3);
    keys = db.boundary_keys();
    expected = {1, 3, 5, 7, 10};
    assert(keys == expected);

    db.insert(-5, -2, true);
    assert(db.size() == 4);
    keys = db.boundary_keys();
    expected = {-5, -2, 1, 3, 5, 7, 10};
    assert(keys == expected);

    db.insert(-2, 10, true); // no new keys
    assert(db.size() == 5);
    keys = db.boundary_keys();
    assert(keys == expected);

    db.build_tree();
    keys = db.boundary_keys();
    assert(keys == expected);

    // erase segments with false value i.e. 7-10.
    db.erase_if([](int16_t, int16_t, bool value) { return !value; });
    assert(db.size() == 4);
    keys = db.boundary_keys();
    expected = {-5, -2, 1, 3, 5, 10};
    assert(keys == expected);

    db.build_tree(); // purge the deleted segment
    keys = db.boundary_keys();
    assert(keys == expected);

    // erase all the other segments
    db.erase_if([](int16_t, int16_t, bool value) { return value; });
    assert(db.empty());
    keys = db.boundary_keys();
    expected.clear();
    assert(keys == expected);

    db.build_tree(); // purge all deleted segments
    assert(!db.valid_tree());
    keys = db.boundary_keys();
    assert(keys == expected);
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
            st_test_insert_search_removal();
            st_test_invalid_insertion();
            st_test_copy_constructor();
            st_test_equality();
            st_test_clear();
            st_test_duplicate_insertion();
            st_test_search_on_uneven_tree();
            st_test_aggregated_search_results();
            st_test_dense_tree_search();
            st_test_search_on_empty_set();
            st_test_search_iterator_basic();
            st_test_search_iterator_result_check();
            st_test_empty_result_set();
            st_test_non_pointer_data();
            st_test_erase_on_invalid_tree();
            st_test_boundary_keys();

            st_test_move_constructor();
            st_test_move_equality();
        }

        if (opt.test_perf)
        {
            st_test_perf_insertion();
        }

        // At this point, all of the nodes created during the test run should have
        // been destroyed.  If not, we are leaking memory.
        typedef segment_tree<uint32_t, void*> db_type;
        assert(db_type::node::get_instance_count() == 0);
    }
    catch (const std::exception& e)
    {
        fprintf(stdout, "Test failed: %s\n", e.what());
        return EXIT_FAILURE;
    }
    fprintf(stdout, "Test finished successfully!\n");
    return EXIT_SUCCESS;
}
