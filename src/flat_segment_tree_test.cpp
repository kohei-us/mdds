/*************************************************************************
 *
 * Copyright (c) 2008-2012 Kohei Yoshida
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

#include "mdds/flat_segment_tree.hpp"
#include "test_global.hpp"

#include <list>
#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <iterator>
#include <algorithm>

#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])

using namespace std;
using namespace mdds;

void print_title(const char* msg)
{
    cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    cout << " " << msg << endl;
    cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
}

void fst_test_leaf_search()
{
    {
        print_title("Simple insert test");
        flat_segment_tree<int, int> int_ranges(0, 100, -1);
        for (int i = 0; i < 20; ++i)
        {
            int start = i*5;
            int end = start + 5;
            int_ranges.insert_front(start, end, i);
        }
        int_ranges.dump_leaf_nodes();
    }

    {
        print_title("Merge test 1");
        flat_segment_tree<int, int> merge_test(0, 100, -1);
        merge_test.insert_front(10, 20, 5);
        merge_test.dump_leaf_nodes();
        merge_test.insert_front(15, 30, 5);
        merge_test.dump_leaf_nodes();
        merge_test.insert_front(30, 50, 5);
        merge_test.dump_leaf_nodes();
        merge_test.insert_front(8, 11, 5);
        merge_test.dump_leaf_nodes();
        merge_test.insert_front(5, 8, 5);
        merge_test.dump_leaf_nodes();
    }

    {
        print_title("Merge test 2");
        flat_segment_tree<int, int> merge_test(0, 100, -1);

        // This should not change the node configuration.
        merge_test.insert_front(10, 90, -1);
        merge_test.dump_leaf_nodes();

        for (int i = 10; i <= 80; i += 10)
            merge_test.insert_front(i, i+10, i);
        merge_test.dump_leaf_nodes();
        merge_test.insert_front(10, 90, -1);
        merge_test.dump_leaf_nodes();

        for (int i = 10; i <= 80; i += 10)
            merge_test.insert_front(i, i+10, i);
        merge_test.dump_leaf_nodes();
        merge_test.insert_front(8, 92, -1);
        merge_test.dump_leaf_nodes();

        for (int i = 10; i <= 80; i += 10)
            merge_test.insert_front(i, i+10, i);
        merge_test.dump_leaf_nodes();
        merge_test.insert_front(12, 88, 25);
        merge_test.dump_leaf_nodes();
    }

    {
        print_title("Search test");
        flat_segment_tree<int, int> db(0, 100, -1);
        for (int i = 0; i < 10; ++i)
        {
            int key = i*10;
            int val = i*5;
            db.insert_front(key, key+10, val);
        }
        db.dump_leaf_nodes();
        for (int i = 0; i <= 100; ++i)
        {
            int val = 0;
            if (db.search(i, val).second)
                cout << "key = " << i << "; value = " << val << endl;
            else
                cout << "key = " << i << "; (value not found)" << endl;
        }
        for (int i = 0; i <= 100; ++i)
        {
            int val = 0, start, end;
            if (db.search(i, val, &start, &end).second)
                cout << "key = " << i << "; value = " << val << "(span: " << start << " - " << end << ")" << endl;
            else
                cout << "key = " << i << "; (value not found)" << endl;
        }
    }
}

/**
 * Test tree construction of flat_segment_tree.
 */
void fst_test_tree_build()
{
    stack_printer __stack_printer__("::fst_test_tree_build");
    {
        int lower = 0, upper = 100, delta = 10;
        flat_segment_tree<int, int> db(lower, upper, 0);
        {
            stack_printer __stack_printer__("::fst_test_tree_build insertion");
            for (int i = lower; i < upper; i+=delta)
                db.insert_front(i, i+delta, i*2);
        }
        db.dump_leaf_nodes();

        {
            stack_printer __stack_printer__("::fst_test_tree_build tree construction");
            db.build_tree();
            db.dump_tree();
        }
    }

    {
        flat_segment_tree<int, int> db(0, 10, 0);
        db.dump_leaf_nodes();
    }
}

void fst_perf_test_search_leaf()
{
    stack_printer __stack_printer__("fst_perf_test_search_leaf");

    int lower = 0, upper = 50000;
    flat_segment_tree<int, int> db(lower, upper, 0);
    for (int i = upper-1; i >= lower; --i)
        db.insert_front(i, i+1, i);

    int success = 0, failure = 0;
    int val;
    for (int i = lower; i < upper; ++i)
    {
        if (db.search(i, val).second)
            ++success;
        else
            ++failure;
    }
    fprintf(stdout, "fst_perf_test_search_leaf:   success (%d)  failure (%d)\n", success, failure);
}

void fst_perf_test_search_tree()
{
    stack_printer __stack_printer__("fst_perf_test_leaf_search");

    int lower = 0, upper = 5000000;
    flat_segment_tree<int, int> db(lower, upper, 0);
    for (int i = upper-1; i >= lower; --i)
        db.insert_front(i, i+1, i);

    {
        stack_printer sp2("::fst_perf_test_search_tree (build tree)");
        db.build_tree();
    }

    int success = 0, failure = 0;
    {
        stack_printer sp2("::fst_perf_test_search_tree (search tree)");
        int val;
        for (int i = lower; i < upper; ++i)
        {
            if (db.search_tree(i, val).second)
                ++success;
            else
                ++failure;
        }
    }

    fprintf(stdout, "fst_perf_test_search:   success (%d)  failure (%d)\n", success, failure);
}

void fst_test_tree_search()
{
    stack_printer __stack_printer__("::fst_test_tree_search");
    typedef flat_segment_tree<int, int> fst_type;
    int lower = 0, upper = 200, delta = 5;
    fst_type db(lower, upper, 0);
    for (int i = lower; i < upper; i += delta)
        db.insert_front(i, i+delta, i);

    db.build_tree();
    db.dump_tree();
    db.dump_leaf_nodes();

    int val, start, end;
    int success = 0, failure = 0;
    for (int i = lower-10; i < upper+10; ++i)
    {
        if (db.search_tree(i, val, &start, &end).second)
        {
            cout << "key = " << i << "; value = " << val << " (" << start << "-" << end << ")" << endl;
            ++success;
        }
        else
        {
            ++failure;
            cout << "key = " << i << " (search failed)" << endl;
        }
    }
    cout << "search: success (" << success << ")  failure (" << failure << ")" << endl;

    // Make sure search_tree() returns correct iterator position.
    db.clear();
    db.insert_back(5, 10, 2);
    db.insert_back(15, 18, 3);
    db.insert_back(23, 28, 4);
    db.build_tree();

    typedef pair<fst_type::const_iterator,bool> ret_type;
    ret_type ret = db.search_tree(0, val, &start, &end);
    assert(ret.second);
    assert(start == 0 && end == 5 && val == 0);
    assert(ret.first == db.begin());

    ret = db.search_tree(6, val, &start, &end);
    assert(ret.second);
    assert(start == 5 && end == 10 && val == 2);
    fst_type::const_iterator check = db.begin();
    ++check; // 5-10 is the 2nd segment from the top.
    assert(ret.first == check);

    ret = db.search_tree(17, val, &start, &end);
    assert(ret.second);
    assert(start == 15 && end == 18 && val == 3);
    std::advance(check, 2);
    assert(ret.first == check);

    ret = db.search_tree(55, val, &start, &end);
    assert(ret.second);
    assert(start == 28 && end == upper && val == 0);
    std::advance(check, 3);
    assert(ret.first == check);

    ret = db.search_tree(upper+10, val, &start, &end);
    assert(!ret.second); // This search should fail.
    assert(ret.first == db.end());
}

void test_single_tree_search(const flat_segment_tree<int, int>& db, int key, int val, int start, int end)
{
    int r_val, r_start, r_end;
    if (db.search_tree(key, r_val, &r_start, &r_end).second)
        assert(r_val == val && r_start == start && r_end == end);
    else
        assert(!"tree search failed!");
}

template<typename key_type, typename value_type>
void build_and_dump(flat_segment_tree<key_type, value_type>&db)
{
    db.build_tree();
    db.dump_tree();
    db.dump_leaf_nodes();
}

template<typename key_type, typename value_type>
bool check_leaf_nodes(
    const flat_segment_tree<key_type, value_type>& db,
    const key_type* keys, const value_type* values, size_t key_size)
{
    if (key_size <= 1)
        return false;

    vector<key_type> key_checks;
    key_checks.reserve(key_size);
    for (size_t i = 0; i < key_size; ++i)
        key_checks.push_back(keys[i]);

    if (!db.verify_keys(key_checks))
        return false;

    vector<value_type> value_checks;
    value_checks.reserve(key_size-1);
    for (size_t i = 0; i < key_size-1; ++i)
        value_checks.push_back(values[i]);

    if (!db.verify_values(value_checks))
        return false;

    return true;
}

template<typename key_type, typename value_type>
bool is_iterator_valid(
    const typename flat_segment_tree<key_type, value_type>::const_iterator& beg,
    const typename flat_segment_tree<key_type, value_type>::const_iterator& end,
    const key_type* keys, const value_type* values, size_t key_size)
{
    assert(key_size > 1);

    typedef flat_segment_tree<key_type, value_type> container;
    typename container::const_iterator itr;
    size_t idx = 0;
    for (itr = beg; itr != end; ++itr, ++idx)
    {
        if (idx >= key_size)
            // out-of-bound index
            return false;

        // Check the key first.
        if (keys[idx] != itr->first)
            return false;

        if (idx < key_size - 1)
        {
            // Check the value only if it's not the last node.  The last node
            // may have an arbitrary value.
            if (values[idx] != itr->second)
                return false;
        }
    }

    // At this point, the iterator should be at the end position.
    if (itr != end)
        return false;

    // Check the keys and values again but go to the opposite direction.
    do
    {
        --itr;
        --idx;

        // key
        if (keys[idx] != itr->first)
            return false;

        // value
        if (idx < key_size - 1)
        {
            if (values[idx] != itr->second)
                return false;
        }
    }
    while (itr != beg);

    return true;
}

template<typename key_type, typename value_type>
bool is_iterator_valid(
    const typename flat_segment_tree<key_type, value_type>::const_reverse_iterator& beg,
    const typename flat_segment_tree<key_type, value_type>::const_reverse_iterator& end,
    const key_type* keys, const value_type* values, size_t key_size)
{
    assert(key_size > 1);

    typedef flat_segment_tree<key_type, value_type> container;
    typename container::const_reverse_iterator itr;
    size_t idx = key_size - 1;
    for (itr = beg; itr != end; ++itr, --idx)
    {
        if (idx >= key_size)
            // out-of-bound index
            return false;

        // Check the key first.
        if (keys[idx] != itr->first)
            return false;

        if (idx < key_size - 1)
        {
            // Check the value only if it's not the last node.  The last node
            // may have an arbitrary value.
            if (values[idx] != itr->second)
                return false;
        }
    }

    // At this point, the iterator should be at the end position.
    if (itr != end)
        return false;

    // Check the keys and values again but go to the opposite direction.
    do
    {
        --itr;
        ++idx;

        // key
        if (keys[idx] != itr->first)
            return false;

        // value
        if (idx < key_size - 1)
        {
            if (values[idx] != itr->second)
                return false;
        }
    }
    while (itr != beg);

    return true;
}

void fst_test_insert_search_mix()
{
    stack_printer __stack_printer__("fst_test_insert_search_mix");
    typedef flat_segment_tree<int, int> db_type;
    db_type db(0, 100, 0);

    build_and_dump(db);
    assert(db_type::node::get_instance_count() == db.leaf_size());
    assert(db.is_tree_valid());
    test_single_tree_search(db, 0, 0, 0, 100);
    test_single_tree_search(db, 99, 0, 0, 100);

    db.insert_front(0, 10, 1);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    assert(db_type::node::get_instance_count() == db.leaf_size());
    assert(db.is_tree_valid());
    test_single_tree_search(db, 0, 1, 0, 10);
    test_single_tree_search(db, 5, 1, 0, 10);
    test_single_tree_search(db, 9, 1, 0, 10);
    test_single_tree_search(db, 10, 0, 10, 100);

    db.insert_front(0, 100, 0);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    assert(db_type::node::get_instance_count() == db.leaf_size());
    assert(db.is_tree_valid());
    test_single_tree_search(db, 0, 0, 0, 100);
    test_single_tree_search(db, 99, 0, 0, 100);

    db.insert_front(10, 20, 5);
    db.insert_front(30, 40, 5);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    assert(db_type::node::get_instance_count() == db.leaf_size());
    assert(db.is_tree_valid());
    test_single_tree_search(db, 10, 5, 10, 20);
    test_single_tree_search(db, 20, 0, 20, 30);
    test_single_tree_search(db, 30, 5, 30, 40);
    test_single_tree_search(db, 40, 0, 40, 100);

    db.insert_front(18, 22, 6);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    assert(db_type::node::get_instance_count() == db.leaf_size());
    assert(db.is_tree_valid());
    test_single_tree_search(db, 18, 6, 18, 22);
    test_single_tree_search(db, 22, 0, 22, 30);
    test_single_tree_search(db, 30, 5, 30, 40);

    db.insert_front(19, 30, 5);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    assert(db_type::node::get_instance_count() == db.leaf_size());
    assert(db.is_tree_valid());
    test_single_tree_search(db, 19, 5, 19, 40);

    db.insert_front(-100, 500, 999);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    assert(db_type::node::get_instance_count() == db.leaf_size());
    assert(db.is_tree_valid());
    test_single_tree_search(db, 30, 999, 0, 100);
}

void fst_test_shift_left()
{
    stack_printer __stack_printer__("fst_test_shift_segment_left");
    typedef flat_segment_tree<int, int> db_type;
    db_type db(0, 100, 0);
    db.insert_front(20, 40, 5);
    db.insert_front(50, 60, 10);
    db.insert_front(70, 80, 15);
    build_and_dump(db);

    // invalid segment ranges -- these should not modify the state of the
    // tree, hence the tree should remain valid.
    db.shift_left(5, 0);
    assert(db.is_tree_valid());

    db.shift_left(95, 120);
    assert(db.is_tree_valid());

    db.shift_left(105, 120);
    assert(db.is_tree_valid());

    db.shift_left(-10, -5);
    assert(db.is_tree_valid());

    db.shift_left(-10, 5);
    assert(db.is_tree_valid());

    // shift without removing nodes (including the lower bound).
    db.shift_left(0, 5);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        vector<int> key_checks;
        key_checks.push_back(0);
        key_checks.push_back(15);
        key_checks.push_back(35);
        key_checks.push_back(45);
        key_checks.push_back(55);
        key_checks.push_back(65);
        key_checks.push_back(75);
        key_checks.push_back(100);
        assert(db.verify_keys(key_checks));
    }

    // shift without removing nodes (not including the lower bound).
    db.shift_left(1, 6);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        vector<int> key_checks;
        key_checks.push_back(0);
        key_checks.push_back(10);
        key_checks.push_back(30);
        key_checks.push_back(40);
        key_checks.push_back(50);
        key_checks.push_back(60);
        key_checks.push_back(70);
        key_checks.push_back(100);
        assert(db.verify_keys(key_checks));
    }

    // shift without removing nodes (the upper bound of the removed segment
    // coincides with a node).
    db.shift_left(5, 10);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        vector<int> key_checks;
        key_checks.push_back(0);
        key_checks.push_back(5);
        key_checks.push_back(25);
        key_checks.push_back(35);
        key_checks.push_back(45);
        key_checks.push_back(55);
        key_checks.push_back(65);
        key_checks.push_back(100);
        assert(db.verify_keys(key_checks));
    }

    // shift with one overlapping node.
    db.shift_left(1, 11);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 1, 15, 25, 35, 45, 55, 100};
        int vals[] = {0, 5,  0, 10,  0, 15,  0};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // shift with two overlapping nodes.
    db.shift_left(2, 30);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 1,  2, 7, 17, 27, 100};
        int vals[] = {0, 5, 10, 0, 15,  0};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // shift with both ends at existing nodes, but no nodes in between.
    db.shift_left(0, 1);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0,  1, 6, 16, 26, 100};
        int vals[] = {5, 10, 0, 15,  0};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // shift with both ends at existing nodes, no nodes in between, and
    // removing the segment results in two consecutive segments with identical
    // value.  The segments should get combined into one.
    db.shift_left(16, 26);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0,  1, 6, 100};
        int vals[] = {5, 10, 0};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // insert two new segments for the next test....
    db.insert_front(10, 20, 400);
    db.insert_front(30, 40, 400);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0,  1, 6,  10, 20,  30, 40, 100};
        int vals[] = {5, 10, 0, 400,  0, 400,  0};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // same test as the previous one, but the value of the combined segment
    // differs from the value of the rightmost leaf node.
    db.shift_left(20, 30);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0,  1, 6,  10, 30, 100};
        int vals[] = {5, 10, 0, 400,  0};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // remove all.
    db.shift_left(0, 100);
    assert(!db.is_tree_valid());
    build_and_dump(db);
}

void fst_test_shift_left_right_edge()
{
    stack_printer __stack_printer__("fst_test_shift_segment_left_right_edge");
    flat_segment_tree<int, bool> db(0, 100, false);
    build_and_dump(db);

    // This should not change the tree state.
    db.shift_left(2, 100);
    build_and_dump(db);
    {
        int  keys[] = {0, 100};
        bool vals[] = {false};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    db.insert_front(20, 100, true);
    build_and_dump(db);
    {
        int  keys[] = {    0,   20, 100};
        bool vals[] = {false, true};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // This should insert a new segment at the end with the initial base value.
    db.shift_left(80, 100);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int  keys[] = {    0,   20,    80, 100};
        bool vals[] = {false, true, false};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // This should not modify the tree since the removed segment already has
    // the initial base value.
    db.shift_left(85, 100);
    assert(db.is_tree_valid()); // tree must still be valid.
    build_and_dump(db);
    {
        int  keys[] = {    0,   20,    80, 100};
        bool vals[] = {false, true, false};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // Insert a new segment at the end with the value 'true' again...
    db.insert_front(85, 100, true);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int  keys[] = {    0,   20,    80,   85, 100};
        bool vals[] = {false, true, false, true};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    db.shift_left(90, 95);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int  keys[] = {    0,   20,    80,   85,    95, 100};
        bool vals[] = {false, true, false, true, false};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }
}

void fst_test_shift_left_append_new_segment()
{
    stack_printer __stack_printer__("fst_test_shift_segment_left_append_new_segment");
    flat_segment_tree<int, bool> db(0, 100, false);
    db.insert_front(0, 100, true);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int  keys[] = {   0, 100};
        bool vals[] = {true};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    db.shift_left(10, 20);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int  keys[] = {   0,    90, 100};
        bool vals[] = {true, false};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    db.insert_front(0, 10, true);
    db.insert_front(10, 20, false);
    db.insert_front(20, 60, true);
    db.insert_front(60, 80, false);
    db.insert_front(80, 100, true);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int  keys[] = {   0,    10,   20,    60,   80, 100};
        bool vals[] = {true, false, true, false, true};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    db.shift_left(0, 70);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int  keys[] = {   0,    10,    30, 100};
        bool vals[] = {false, true, false};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }
}

void fst_test_shift_right_init0()
{
    stack_printer __stack_printer__("fst_test_shift_segment_right_init0");

    flat_segment_tree<int, int> db(0, 100, 0);
    db.insert_front(0,  10,  15);
    db.insert_front(10, 20,  1);
    db.insert_front(20, 30,  2);
    db.insert_front(30, 40,  3);
    db.insert_front(40, 50,  4);
    db.insert_front(50, 60,  5);
    db.insert_front(60, 70,  6);
    db.insert_front(70, 80,  7);
    db.insert_front(80, 90,  8);
    assert(!db.is_tree_valid());
    build_and_dump(db);

    // shifting position is at the lower bound.  The leftmost segment has a
    // non-zero value which needs to be preserved after the shift by adding a
    // new node.
    db.shift_right(0, 5, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0,  5, 15, 25, 35, 45, 55, 65, 75, 85, 95,100};
        int vals[] = {0, 15,  1,  2,  3,  4,  5,  6,  7,  8,  0};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // shifting position is at the lower bound, and after the shift, the upper
    // bound of the last non-zero segment (10) becomes the upper bound of the
    // global range.
    db.shift_right(0, 5, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        int vals[] = {0, 15,  1,  2,  3,  4,  5,  6,  7,  8};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // Shift by some odd number.
    db.shift_right(0, 49, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 59, 69, 79, 89, 99, 100};
        int vals[] = {0, 15,  1,  2,  3,  4};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // Shift so that the 2nd node from the right-most node becomes the new
    // right-most node.
    db.shift_right(0, 11, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 70, 80, 90, 100};
        int vals[] = {0, 15,  1,  2};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // This should remove all segments.
    db.shift_right(0, 30, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 100};
        int vals[] = {0};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // Insert a few new segments for the next series of tests...
    db.insert_front(5, 10, 5);
    db.insert_front(20, 30, 5);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 5, 10, 20, 30, 100};
        int vals[] = {0, 5,  0,  5,  0};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // Inserting at a non-node position.  This should simply extend that
    // segment and shift all the others.
    db.shift_right(6, 20, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 5, 30, 40, 50, 100};
        int vals[] = {0, 5,  0,  5,  0};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // Inserting at a node position.
    db.shift_right(5, 20, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 25, 50, 60, 70, 100};
        int vals[] = {0, 5,  0,  5,  0};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // Inserting at a non-node position, pushing a node out-of-bound.
    db.shift_right(65, 40, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 25, 50, 60, 100};
        int vals[] = {0, 5,  0,  5};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // Inserting at a node position, pushing a node out-of-bound.
    db.shift_right(50, 40, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 25, 90, 100};
        int vals[] = {0,  5,  0};
        assert(check_leaf_nodes(db, keys, vals, ARRAY_SIZE(keys)));
    }
}

void fst_test_shift_right_init999()
{
    stack_printer __stack_printer__("fst_test_shift_segment_right_init999");

    // Initialize the tree with a default value of 999.
    flat_segment_tree<int, int> db(0, 100, 999);
    db.insert_front(0, 10, 0);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int k[] = {0, 10, 100};
        int v[] = {0, 999};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    // This should only extend the first segment.
    db.shift_right(1, 10, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int k[] = {0, 20, 100};
        int v[] = {0, 999};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    // Inserting at the leftmost node position should create a new segment
    // with a default value of 999.
    db.shift_right(0, 10, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int k[] = {0, 10, 30, 100};
        int v[] = {999, 0, 999};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    // Invalid shifts -- these should not invalidate the tree.
    db.shift_right(-10, 10, false);
    assert(db.is_tree_valid());

    db.shift_right(100, 10, false);
    assert(db.is_tree_valid());

    db.shift_right(0, 0, false);
    assert(db.is_tree_valid());
}

void fst_test_shift_right_bool()
{
    flat_segment_tree<long, bool> db(0, 1048576, false);
    db.insert_front(3, 7, true);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        long k[] = {0, 3, 7, 1048576};
        bool v[] = {false, true, false};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    db.shift_right(1, 1, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        long k[] = {0, 4, 8, 1048576};
        bool v[] = {false, true, false};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }
}

void fst_test_shift_right_skip_start_node()
{
    stack_printer __stack_printer__("fst_test_shift_segment_right_skip_start_node");

    flat_segment_tree<long, short> db(0, 1048576, 0);
    db.insert_front(3, 7, 5);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        long  k[] = {0, 3, 7, 1048576};
        short v[] = {0, 5, 0};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    db.shift_right(3, 2, true);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        long  k[] = {0, 3, 9, 1048576};
        short v[] = {0, 5, 0};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    // shift_right from the leftmost node should not change its value
    db.insert_front(0, 4, 2);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        long  k[] = {0, 4, 9, 1048576};
        short v[] = {2, 5, 0};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    db.shift_right(0, 2, true);
    build_and_dump(db);
    {
        long  k[] = {0, 6, 11, 1048576};
        short v[] = {2, 5, 0};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }
}

/**
 * Right all nodes right so that all existing nodes get pushed out of the
 * range.
 */
void fst_test_shift_right_all_nodes()
{
    typedef flat_segment_tree<unsigned, unsigned> fst_type;

    fst_type db(0, 10, 0);
    {
        unsigned k[] = {0, 10};
        unsigned v[] = {0};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    db.insert_back(0, 8, 2);
    db.dump_leaf_nodes();
    {
        unsigned k[] = {0, 8, 10};
        unsigned v[] = {2, 0};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    // Shift all nodes out of range.  After this, there should be only the
    // left and right most nodes left.
    db.shift_right(0, 15, false);
    db.dump_leaf_nodes();
    {
        unsigned k[] = {0, 10};
        unsigned v[] = {0};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }
}

template<typename key_type, typename value_type>
struct leaf_node_functor : public unary_function<void, pair<key_type, value_type> >
{
    void operator() (const pair<key_type, value_type>&) const
    {
    }
};

void fst_test_const_iterator()
{
    stack_printer __stack_printer__("::fst_test_const_iterator");

    {
        typedef unsigned int    key_type;
        typedef unsigned short  value_type;
        unsigned short max_value = numeric_limits<value_type>::max();
        typedef flat_segment_tree<key_type, value_type> container_type;

        container_type db(0, 1000, max_value);

        build_and_dump(db);
        {
            unsigned int   k[] = {0, 1000};
            unsigned short v[] = {max_value};
            assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
        }

        db.insert_front(10, 20, 10);
        db.insert_front(20, 50, 20);
        db.insert_front(100, 300, 55);
        build_and_dump(db);
        {
            unsigned int   k[] = {0, 10, 20, 50, 100, 300, 1000};
            unsigned short v[] = {max_value, 10, 20, max_value, 55, max_value};
            assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
            fprintf(stdout, "fst_test_const_iterator:   leaf nodes valid\n");

            // Check the forward iterator's integrity.
            assert(is_iterator_valid(db.begin(), db.end(), k, v, ARRAY_SIZE(k)));
            fprintf(stdout, "fst_test_const_iterator:   forward iterator valid\n");

            // Check the reverse iterator's integrity.
            assert(is_iterator_valid(db.rbegin(), db.rend(), k, v, ARRAY_SIZE(k)));
            fprintf(stdout, "fst_test_const_iterator:   reverse iterator valid\n");
        }

        // Make sure it works with for_each.
        for_each(db.begin(), db.end(), leaf_node_functor<key_type, value_type>());
    }

    {
        typedef flat_segment_tree<int, bool> container_type;
        container_type db(0, 100, true);
        db.insert_front(0, 50, false);

        {
            cout << "-- forward" << endl;
            container_type::const_iterator it = db.begin(), it_end = db.end();
            // 0 -> 50 -> 100 -> end
            cout << "key: " << it->first << "  value: " << it->second << endl;
            assert(it->first == 0);
            assert(it->second == false);
            ++it;
            cout << "key: " << it->first << "  value: " << it->second << endl;
            assert(it->first == 50);
            assert(it->second == true);
            ++it;
            cout << "key: " << it->first << "  value: " << it->second << endl;
            assert(it->first == 100);
            assert(it != it_end);
            ++it;
            assert(it == it_end);
        }
        {
            cout << "-- reverse" << endl;
            container_type::const_reverse_iterator it = db.rbegin(), it_end = db.rend();
            // 100 -> 50 -> 0 -> end
            cout << "key: " << it->first << "  value: " << it->second << endl;
            assert(it->first == 100);
            ++it;
            cout << "key: " << it->first << "  value: " << it->second << endl;
            assert(it->first == 50);
            assert(it->second == true);
            ++it;
            cout << "key: " << it->first << "  value: " << it->second << endl;
            assert(it->first == 0);
            assert(it->second == false);
            assert(it != it_end);
            ++it;
            assert(it == it_end);
        }
    }
}

template<typename key_type, typename value_type>
void fst_test_insert_front_back(key_type start_key, key_type end_key, value_type default_value)
{
    typedef flat_segment_tree<key_type, value_type> container_type;
    typedef typename container_type::const_iterator itr_type;

    value_type val = 0;

    // insert a series of segments from the front.
    container_type db_front(start_key, end_key, default_value);
    for (key_type i = start_key; i < end_key - 10; ++i)
    {
        itr_type itr = db_front.insert_front(i, i+1, val).first;
        assert(itr->first == i);
        assert(itr->second == val);
        if (++val > 10)
            val = 0;
    }

    // insert the same series of segments from the back.
    container_type db_back(start_key, end_key, default_value);
    val = 0;
    for (key_type i = start_key; i < end_key - 10; ++i)
    {
        itr_type itr = db_back.insert_back(i, i+1, val).first;
        assert(itr->first == i);
        assert(itr->second == val);
        if (++val > 10)
            val = 0;
    }

    // Now, these two must be identical.
    if (db_front != db_back)
    {
        // They are not identical!
        db_front.dump_leaf_nodes();
        db_back.dump_leaf_nodes();
        cout << "start_key = " << start_key << "  end_key = " << end_key << "  default_value = " << default_value << endl;
        assert(!"Contents of the two containers are not identical!");
    }
}

void fst_perf_test_insert_front_back()
{
    typedef unsigned long key_type;
    typedef int           value_type;
    typedef flat_segment_tree<key_type, value_type> container_type;
    key_type upper_bound = 20000;
    {
        stack_printer __stack_printer__("::fst_perf_test_insert (front insertion)");
        container_type db(0, upper_bound, 0);
        value_type val = 0;
        for (key_type i = 0; i < upper_bound; ++i)
        {
            db.insert_front(i, i+1, val);
            if (++val > 10)
                val = 0;
        }
    }

    {
        stack_printer __stack_printer__("::fst_perf_test_insert (back insertion)");
        container_type db(0, upper_bound, 0);
        value_type val = 0;
        for (key_type i = 0; i < upper_bound; ++i)
        {
            db.insert_back(i, i+1, val);
            if (++val > 10)
                val = 0;
        }
    }
}

void fst_test_copy_ctor()
{
    stack_printer __stack_printer__("::fst_test_copy_ctor");
    typedef unsigned long key_type;
    typedef int           value_type;
    typedef flat_segment_tree<key_type, value_type> fst;

    // Test copy construction of node first.

    {
        // Original node.
        fst::node_ptr node1(new fst::node);
        node1->value_leaf.key   = 10;
        node1->value_leaf.value = 500;
        assert(node1->is_leaf);
        assert(!node1->parent);
        assert(!node1->prev);
        assert(!node1->next);

        // Copy it to new node.
        fst::node_ptr node2(new fst::node(*node1));
        assert(node2->is_leaf);
        assert(!node2->parent);
        assert(!node2->prev);
        assert(!node2->next);
        assert(node2->value_leaf.key == 10);
        assert(node2->value_leaf.value == 500);

        // Changing the values of the original should not modify the second node.
        node1->value_leaf.key   = 35;
        node1->value_leaf.value = 200;
        assert(node2->value_leaf.key == 10);
        assert(node2->value_leaf.value == 500);
    }

    {
        // Test non-leaf node objects.
        fst::nonleaf_node node1;
        node1.value_nonleaf.low  = 123;
        node1.value_nonleaf.high = 789;

        // Test the copying of non-leaf values.
        fst::nonleaf_node node2(node1);
        assert(!node2.is_leaf);
        assert(!node2.parent);
        assert(!node2.left);
        assert(!node2.right);
        assert(node2.value_nonleaf.low == 123);
        assert(node2.value_nonleaf.high == 789);
    }

    // Now, test the copy construction of the flat_segment_tree.

    // Simple copying by copy construction.
    fst db(0, 100, 5);
    fst db_copied(db);
    assert(db == db_copied);
    {
        key_type   k[] = {0, 100};
        value_type v[] = {5};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    // Inserting the same segment value to both instances.  They should still
    // be equal.
    db.insert_front(5, 10, 0);
    db_copied.insert_front(5, 10, 0);
    assert(db == db_copied);
    {
        key_type   k[] = {0, 5, 10, 100};
        value_type v[] = {5, 0,  5};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    // Inserting a new segment only to the 2nd instance.  They should differ.
    db_copied.insert_front(15, 20, 35);
    assert(db != db_copied);
    {
        key_type   k[] = {0, 5, 10, 15, 20, 100};
        value_type v[] = {5, 0,  5, 35,  5};
        assert(check_leaf_nodes(db_copied, k, v, ARRAY_SIZE(k)));
    }

    // Make sure that copying will leave the tree invalid.
    assert(!db_copied.is_tree_valid());
    db_copied.build_tree();
    assert(db_copied.is_tree_valid());
    fst db_copied_again(db_copied);
    assert(db_copied == db_copied_again);
    assert(!db_copied_again.is_tree_valid());
    assert(!db_copied_again.get_root_node());

    // Make sure we can still perform tree search correctly.
    value_type answer = 0;
    db_copied_again.build_tree();
    db_copied_again.search_tree(18, answer);
    assert(db_copied_again.is_tree_valid());
    assert(answer == 35);
}

void fst_test_equality()
{
    stack_printer __stack_printer__("::fst_test_equality");
    typedef unsigned long key_type;
    typedef int           value_type;
    typedef flat_segment_tree<key_type, value_type> container_type;

    container_type db1(0, 100, 0);
    container_type db2(0, 100, 0);
    assert(db1 == db2);

    db1.insert_front(0, 1, 1);
    assert(db1 != db2);

    db2.insert_front(0, 1, 1);
    assert(db1 == db2);

    // Same node count, but with different value.
    db2.insert_front(0, 1, 2);
    assert(db1 != db2);

    // Set the value back.
    db2.insert_front(0, 1, 1);
    assert(db1 == db2);

    db1.insert_front(4, 10, 10);
    db1.insert_front(4, 10, 0);
    assert(db1 == db2);

    db1.insert_front(20, 40, 5);
    db1.insert_front(30, 35, 6);
    assert(db1 != db2);

    db2.insert_front(20, 30, 5);
    db2.insert_front(30, 35, 6);
    db2.insert_front(35, 40, 5);
    assert(db1 == db2);
}

void fst_test_back_insert()
{
    stack_printer __stack_printer__("::fst_test_back_insert");
    typedef unsigned int   key_type;
    typedef unsigned short value_type;
    typedef flat_segment_tree<key_type, value_type> container_type;
    container_type db(0, 100, 0);
    db.insert_back(1, 2, 1);
    {
        unsigned int   k[] = {0, 1, 2, 100};
        unsigned short v[] = {0, 1, 0};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    db.insert_back(3, 4, 2);
    {
        unsigned int   k[] = {0, 1, 2, 3, 4, 100};
        unsigned short v[] = {0, 1, 0, 2, 0};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    db.insert_back(4, 5, 2);
    {
        unsigned int   k[] = {0, 1, 2, 3, 5, 100};
        unsigned short v[] = {0, 1, 0, 2, 0};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    db.insert_back(90, 120, 10);
    {
        unsigned int   k[] = {0, 1, 2, 3, 5, 90, 100};
        unsigned short v[] = {0, 1, 0, 2, 0, 10};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    db.insert_back(0, 10, 20);
    {
        unsigned int   k[] = {0, 10, 90, 100};
        unsigned short v[] = {20, 0, 10};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    db.insert_back(5, 20, 20);
    {
        unsigned int   k[] = {0, 20, 90, 100};
        unsigned short v[] = {20, 0, 10};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    db.insert_back(15, 30, 5);
    {
        unsigned int   k[] = {0, 15, 30, 90, 100};
        unsigned short v[] = {20, 5,  0, 10};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    db.insert_back(0, 1, 2);
    {
        unsigned int   k[] = {0,  1, 15, 30, 90, 100};
        unsigned short v[] = {2, 20,  5,  0, 10};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }

    db.dump_leaf_nodes();
}

template<typename A, typename B>
void print_iterator(typename flat_segment_tree<A,B>::const_iterator& itr)
{
    cout << "iterator: (key=" << itr->first << ",value=" << itr->second << ")" << endl;
}

void fst_test_insert_iterator()
{
    stack_printer __stack_printer__("::fst_test_insert_iterator");
    typedef long key_type;
    typedef short value_type;
    typedef flat_segment_tree<key_type, value_type> db_type;

    db_type db(0, 100, 0);
    db_type::const_iterator itr;

    itr = db.insert_front(0, 5, 4).first;
    assert(itr->first == 0);
    assert(itr->second == 4);
    print_iterator<key_type, value_type>(itr);

    itr = db.insert_front(3, 10, 100).first;
    assert(itr->first == 3);
    assert(itr->second == 100);
    print_iterator<key_type, value_type>(itr);

    itr = db.insert_front(5, 8, 100).first;
    assert(itr->first == 3);
    assert(itr->second == 100);
    print_iterator<key_type, value_type>(itr);

    itr = db.insert_front(5, 8, 50).first;
    assert(itr->first == 5);
    assert(itr->second == 50);
    print_iterator<key_type, value_type>(itr);

    itr = db.insert_front(6, 9, 50).first;
    assert(itr->first == 5);
    assert(itr->second == 50);
    print_iterator<key_type, value_type>(itr);

    itr = db.insert_front(9, 20, 24).first;
    assert(itr->first == 9);
    assert(itr->second == 24);
    print_iterator<key_type, value_type>(itr);

    itr = db.insert_front(19, 24, 34).first;
    assert(itr->first == 19);
    assert(itr->second == 34);
    print_iterator<key_type, value_type>(itr);

    itr = db.insert_front(24, 26, 0).first;
    assert(itr->first == 24);
    assert(itr->second == 0);
    print_iterator<key_type, value_type>(itr);

    itr = db.insert_front(30, 50, 2).first;
    assert(itr->first == 30);
    assert(itr->second == 2);
    print_iterator<key_type, value_type>(itr);

    itr = db.insert_front(120, 140, 34).first;
    assert(itr == db.end());

    itr = db.insert_front(-20, -10, 20).first;
    assert(itr == db.end());
}

void fst_test_insert_state_changed()
{
    stack_printer __stack_printer__("::fst_test_insert_state_changed");
    typedef long key_type;
    typedef short value_type;
    typedef flat_segment_tree<key_type, value_type> db_type;
    typedef pair<db_type::const_iterator, bool> ret_type;

    db_type db(0, 1000, 0);

    // Inserting a segment with the default value.  This should not change the
    // state.
    ret_type r = db.insert_front(10, 15, 0);
    assert(!r.second);

    // New segment with a different value.
    r = db.insert_front(0, 10, 1);
    assert(r.second);

    // Inserting the same segment should not change the state.
    r = db.insert_front(0, 10, 1);
    assert(!r.second);

    r = db.insert_front(0, 1, 1);
    assert(!r.second);

    r = db.insert_front(8, 10, 1);
    assert(!r.second);

    // This extends the segment, therefore the state should change.
    r = db.insert_front(8, 11, 1);
    assert(r.second);

    r = db.insert_front(11, 15, 0);
    assert(!r.second);

    // This extends the segment.  At this point, 0 - 15 should have a value of 1.
    r = db.insert_front(11, 15, 1);
    assert(r.second);
    {
        db_type::const_iterator itr = r.first;
        assert(itr->first == 0);
        assert(itr->second == 1);
        ++itr;
        assert(itr->first == 15);
    }

    r = db.insert_front(2, 4, 1);
    assert(!r.second);

    // Different value segment.  This should change the state.
    r = db.insert_front(2, 4, 0);
    assert(r.second);

    // Ditto.
    r = db.insert_front(2, 4, 1);
    assert(r.second);

    r = db.insert_front(1, 8, 1);
    assert(!r.second);

    // Different value segment.
    r = db.insert_front(1, 8, 2);
    assert(r.second);

    r = db.insert_front(8, 20, 2);
    assert(r.second);

    // The 0-1 segment should still have a value of 1.  So this won't change
    // the state.
    r = db.insert_front(0, 1, 1);
    assert(!r.second);

    // Partially out-of-bound segment, but this should modify the value of 0-2.
    r = db.insert_front(-50, 2, 10);
    assert(r.second);
    {
        db_type::const_iterator itr = r.first;
        assert(itr->first == 0);
        assert(itr->second == 10);
        ++itr;
        assert(itr->first == 2);
    }

    // Entirely out-of-bound.
    r = db.insert_front(-50, -2, 20);
    assert(!r.second);

    // Likewise, partially out-of-bound at the higher end.
    r = db.insert_front(800, 1200, 20);
    assert(r.second);

    // Entirely out-of-bound.
    r = db.insert_front(1300, 1400, 25);
    assert(!r.second);
}

void fst_perf_test_insert_position()
{
    typedef flat_segment_tree<long, bool> db_type;
    typedef pair<db_type::const_iterator, bool> ret_type;
    long upper = 60000;
    {
        stack_printer __stack_printer__("::fst_perf_test_insert_position (front)");
        // Much smaller upper boundary because front insertion is very slow.
        db_type db(0, upper, false);
        bool val = false;
        for (long i = 0; i < upper; ++i)
        {
            db.insert_front(i, i+1, val);
            val = !val;
        }
    }

    {
        stack_printer __stack_printer__("::fst_perf_test_insert_position (back)");
        db_type db(0, upper, false);
        bool val = false;
        for (long i = 0; i < upper; ++i)
        {
            db.insert_back(i, i+1, val);
            val = !val;
        }
    }

    {
        db_type db(0, upper, false);
        {
            stack_printer __stack_printer__("::fst_perf_test_insert_position (position)");
            db_type::const_iterator itr = db.begin();
            bool val = false;
            for (long i = 0; i < upper; ++i)
            {
                ret_type ret = db.insert(itr, i, i+1, val);
                val = !val;
                itr = ret.first;
            }
        }
        {
            stack_printer __stack_printer__("::fst_perf_test_insert_position (position re-insert)");
            db_type::const_iterator itr = db.begin();
            bool val = true;
            for (long i = 0; i < upper; ++i)
            {
                ret_type ret = db.insert(itr, i, i+1, val);
                val = !val;
                itr = ret.first;
            }
        }
    }
}

void fst_perf_test_position_search()
{
    typedef flat_segment_tree<long, bool> db_type;
    typedef pair<db_type::const_iterator, bool> ret_type;
    long upper = 60000;
    db_type db(0, upper, false);

    // Fill the leaf nodes first.
    db_type::const_iterator itr = db.begin();
    bool val = false;
    for (long i = 0; i < upper; ++i)
    {
        ret_type ret = db.insert(itr, i, i+1, val);
        val = !val;
        itr = ret.first;
    }

    {
        stack_printer __stack_printer__("::fst_perf_test_position_search (normal)");
        for (long i = 0; i < upper; ++i)
        {
            bool val;
            ret_type ret = db.search(i, val);
            assert(ret.second);
        }
    }

    {
        stack_printer __stack_printer__("::fst_perf_test_position_search (positioned)");
        itr = db.begin();
        for (long i = 0; i < upper; ++i)
        {
            bool val;
            ret_type ret = db.search(itr, i, val);
            assert(ret.second);
            itr = ret.first;
        }
    }
}

template<typename K, typename V>
bool check_pos_search_result(
    const flat_segment_tree<K, V>& db,
    typename flat_segment_tree<K, V>::const_iterator& itr,
    K key, K start_expected, K end_expected, V value_expected)
{
    typedef flat_segment_tree<K, V> db_type;
    typedef pair<typename db_type::const_iterator, bool> ret_type;

    V _val;
    K _start = -1, _end = -1;

    ret_type r = db.search(itr, key, _val, &_start, &_end);

    cout << "expected: start=" << start_expected << " end=" << end_expected << " value=" << value_expected << endl;
    cout << "observed: start=" << _start << " end=" << _end << " value=" << _val << endl;

    bool result =
        _start == start_expected && _end == end_expected && _val == value_expected &&
        r.first->first == start_expected && r.first->second == value_expected;
    itr = r.first;
    return result;
}

void fst_test_position_search()
{
    stack_printer __stack_printer__("::fst_test_position_search");
    typedef flat_segment_tree<long, short> db_type;
    typedef pair<db_type::const_iterator, bool> ret_type;

    db_type db(0, 100, 0);
    db.insert_front(10, 20, 1);
    db.insert_front(30, 50, 5);

    db_type db2(-10, 10, 1);

    struct {
        long start_range;
        long end_range;
        short value_expected;
    } params[] = {
        {  0,  10, 0 },
        { 10,  20, 1 },
        { 20,  30, 0 },
        { 30,  50, 5 },
        { 50, 100, 0 }
    };

    size_t n = sizeof(params) / sizeof(params[0]);

    cout << "Testing for searches with various valid and invalid iterators." << endl;
    for (size_t i = 0; i < n; ++i)
    {
        for (long j = params[i].start_range; j < params[i].end_range; ++j)
        {
            db_type::const_iterator itr;
            bool success = false;

            // empty iterator - should fall back to normal search.
            success = check_pos_search_result(
                db, itr, j, params[i].start_range, params[i].end_range, params[i].value_expected);
            assert(success);

            // iterator returned from the previous search.
            success = check_pos_search_result(
                db, itr, j, params[i].start_range, params[i].end_range, params[i].value_expected);
            assert(success);

            // begin iterator.
            itr = db.begin();
            success = check_pos_search_result(
                db, itr, j, params[i].start_range, params[i].end_range, params[i].value_expected);
            assert(success);

            // end iterator.
            itr = db.end();
            success = check_pos_search_result(
                db, itr, j, params[i].start_range, params[i].end_range, params[i].value_expected);
            assert(success);

            // iterator from another container - should fall back to normal search.
            itr = db2.begin();
            success = check_pos_search_result(
                db, itr, j, params[i].start_range, params[i].end_range, params[i].value_expected);
            assert(success);
        }
    }

    cout << "Testing for continuous searching by re-using the iteraotr from the previous search." << endl;
    db_type::const_iterator itr;
    short val;
    long start = 0, end = 0;

    for (size_t i = 0; i < n; ++i)
    {
        ret_type r = db.search(itr, end, val, &start, &end);
        assert(start == params[i].start_range);
        assert(end == params[i].end_range);
        assert(val == params[i].value_expected);
        assert(r.second);
        itr = r.first;
    }
}

void fst_test_min_max_default()
{
    typedef flat_segment_tree<long, short> db_type;
    db_type db(0, 100, 2);
    assert(db.min_key() == 0);
    assert(db.max_key() == 100);
    assert(db.default_value() == 2);
}

void fst_test_swap()
{
    typedef flat_segment_tree<long, int> db_type;
    db_type db1(0, 200, 20);
    db_type db2(20, 40, 0);
    db1.insert_back(20, 30, 1);
    db1.insert_back(30, 40, 2);
    db1.insert_back(40, 50, 3);
    db1.build_tree();

    // Check the content of db1.
    {
        db_type::key_type   k[] = {0,  20, 30, 40, 50, 200};
        db_type::value_type v[] = {20,  1,  2,  3, 20};
        assert(check_leaf_nodes(db1, k, v, ARRAY_SIZE(k)));
    }
    assert(db1.min_key() == 0);
    assert(db1.max_key() == 200);
    assert(db1.default_value() == 20);
    assert(db1.is_tree_valid());

    db1.swap(db2);

    // Now db2 should inherit the content of db1.
    {
        db_type::key_type   k[] = {0,  20, 30, 40, 50, 200};
        db_type::value_type v[] = {20,  1,  2,  3, 20};
        assert(check_leaf_nodes(db2, k, v, ARRAY_SIZE(k)));
    }
    assert(db2.min_key() == 0);
    assert(db2.max_key() == 200);
    assert(db2.default_value() == 20);
    assert(db2.is_tree_valid());

    // Tree search should work on db2.
    db_type::value_type val = 0;
    assert(db2.search_tree(35, val).second);
    assert(val == 2);
}

void fst_test_clear()
{
    stack_printer __stack_printer__("::fst_test_clear");

    typedef flat_segment_tree<long, int> db_type;
    db_type db(0, 100, 42);
    db.insert_back(0, 10, 0);
    db.insert_back(10, 20, 1);
    db.insert_back(20, 30, 2);
    db.build_tree();

    {
        db_type::key_type k[] = {0, 10, 20, 30, 100};
        db_type::value_type v[] = {0, 1, 2, 42};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }
    assert(db.min_key() == 0);
    assert(db.max_key() == 100);
    assert(db.default_value() == 42);
    assert(db.is_tree_valid());

    db.clear();

    {
        db_type::key_type k[] = {0, 100};
        db_type::value_type v[] = {42};
        assert(check_leaf_nodes(db, k, v, ARRAY_SIZE(k)));
    }
    assert(db.min_key() == 0);
    assert(db.max_key() == 100);
    assert(db.default_value() == 42);
    assert(!db.is_tree_valid());
}

void fst_test_assignment()
{
    stack_printer __stack_printer__("::fst_test_assignment");

    typedef flat_segment_tree<long, int> db_type;
    db_type db1(0, 100, 42);
    db1.insert_back(0, 10, 0);
    db1.insert_back(10, 20, 1);
    db1.insert_back(20, 30, 2);
    db1.build_tree();

    {
        db_type::key_type k[] = {0, 10, 20, 30, 100};
        db_type::value_type v[] = {0, 1, 2, 42};
        assert(check_leaf_nodes(db1, k, v, ARRAY_SIZE(k)));
    }
    assert(db1.min_key() == 0);
    assert(db1.max_key() == 100);
    assert(db1.default_value() == 42);
    assert(db1.is_tree_valid());

    db_type db2(20, 40, 0);
    db2.insert_back(20, 30, 8);
    db2.build_tree();

    {
        db_type::key_type k[] = {20, 30, 40};
        db_type::value_type v[] = {8, 0};
        assert(check_leaf_nodes(db2, k, v, ARRAY_SIZE(k)));
    }
    assert(db2.min_key() == 20);
    assert(db2.max_key() == 40);
    assert(db2.default_value() == 0);
    assert(db2.is_tree_valid());

    db_type db3(10, 80, 4);
    db3.build_tree();

    {
        db_type::key_type k[] = {10, 80};
        db_type::value_type v[] = {4};
        assert(check_leaf_nodes(db3, k, v, ARRAY_SIZE(k)));
    }
    assert(db3.min_key() == 10);
    assert(db3.max_key() == 80);
    assert(db3.default_value() == 4);
    assert(db3.is_tree_valid());

    db3 = db2 = db1;

    {
        db_type::key_type k[] = {0, 10, 20, 30, 100};
        db_type::value_type v[] = {0, 1, 2, 42};
        assert(check_leaf_nodes(db1, k, v, ARRAY_SIZE(k)));
    }
    assert(db1.min_key() == 0);
    assert(db1.max_key() == 100);
    assert(db1.default_value() == 42);
    assert(db1.is_tree_valid());

    {
        db_type::key_type k[] = {0, 10, 20, 30, 100};
        db_type::value_type v[] = {0, 1, 2, 42};
        assert(check_leaf_nodes(db2, k, v, ARRAY_SIZE(k)));
    }
    assert(db2.min_key() == 0);
    assert(db2.max_key() == 100);
    assert(db2.default_value() == 42);
    assert(!db2.is_tree_valid());

    {
        db_type::key_type k[] = {0, 10, 20, 30, 100};
        db_type::value_type v[] = {0, 1, 2, 42};
        assert(check_leaf_nodes(db3, k, v, ARRAY_SIZE(k)));
    }
    assert(db3.min_key() == 0);
    assert(db3.max_key() == 100);
    assert(db3.default_value() == 42);
    assert(!db3.is_tree_valid());
}

void fst_test_non_numeric_value()
{
    stack_printer __stack_printer__("::fst_test_non_numeric_value");

    typedef flat_segment_tree<int, std::string> db_type;
    db_type db(0, 4, "42");
    db.insert_back(1, 2, "hello world");

    assert(db.default_value() == "42");

    std::string result;
    db.search(1, result);

    assert(result == "hello world");

    db_type db2(db);

    assert(db == db2);
}

void fst_test_insert_out_of_bound()
{
    stack_printer __stack_printer__("::fst_test_insert_out_of_bound");

    typedef flat_segment_tree<int, bool> db_type;
    db_type db(0, 10, false);

    // An out-of-bound range, whether it's in part or in its entirety, should
    // be handled gracefully without throwing exceptions or causing segfaults.

    // ranges that are entirely out-of-bound.

    auto ret = db.insert_front(-10, -8, false);
    assert(!ret.second);
    db.insert_back(12, 13, false);
    assert(!ret.second);

    db_type::const_iterator pos = db.end();

    ret = db.insert(pos, -10, -8, false);
    assert(!ret.second);
    pos = ret.first;

    ret = db.insert(pos, 12, 13, false);
    assert(!ret.second);
    pos = ret.first;

    // partial overflows.

    ret = db.insert(pos, -2, 2, true);
    assert(ret.second); // content modified
    pos = ret.first;

    ret = db.insert(pos, 8, 20, true);
    assert(ret.second); // content modified
    pos = ret.first;
}

void fst_test_segment_iterator()
{
    stack_printer __stack_printer__("::fst_test_segment_iterator");

    typedef flat_segment_tree<int16_t, bool> db_type;
    db_type db(0, 100, false);

    db_type::const_segment_iterator it = db.begin_segment();
    db_type::const_segment_iterator ite = db.end_segment();

    assert(it != ite);
    assert(it->start == 0);
    assert(it->end == 100);
    assert(it->value == false);

    const auto& v = *it;
    assert(v.start == 0);
    assert(v.end == 100);
    assert(v.value == false);

    ++it;
    assert(it == ite);

    --it;
    assert(it != ite);
    assert(it->start == 0);
    assert(it->end == 100);
    assert(it->value == false);

    db_type::const_segment_iterator it2; // default constructor
    it2 = it; // assignment operator
    assert(it2 == it);
    assert(it2->start == 0);
    assert(it2->end == 100);
    assert(it2->value == false);

    auto it3(it2); // copy constructor
    assert(it3 == it2);
    assert(it3->start == 0);
    assert(it3->end == 100);
    assert(it3->value == false);

    db.insert_back(20, 50, true); // this invalidates the iterators.

    it = db.begin_segment();
    ite = db.end_segment();

    assert(it->start == 0);
    assert(it->end == 20);
    assert(it->value == false);

    it2 = it++; // post-increment

    assert(it2->start == 0);
    assert(it2->end == 20);
    assert(it2->value == false);

    assert(it->start == 20);
    assert(it->end == 50);
    assert(it->value == true);

    ++it;
    assert(it->start == 50);
    assert(it->end == 100);
    assert(it->value == false);

    ++it;
    assert(it == ite);

    it2 = it--; // post-decrement.
    assert(it2 == ite);
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
            fst_test_equality();
            fst_test_copy_ctor();
            fst_test_back_insert();
            {
                typedef unsigned int   key_type;
                typedef unsigned short value_type;
                for (value_type i = 0; i <= 100; ++i)
                    fst_test_insert_front_back<key_type, value_type>(0, 100, i);
            }

            {
                typedef int   key_type;
                typedef short value_type;
                for (value_type i = 0; i <= 100; ++i)
                    fst_test_insert_front_back<key_type, value_type>(0, 100, i);
            }

            {
                typedef long         key_type;
                typedef unsigned int value_type;
                for (value_type i = 0; i <= 100; ++i)
                    fst_test_insert_front_back<key_type, value_type>(0, 100, i);
            }

            fst_test_leaf_search();
            fst_test_tree_build();
            fst_test_tree_search();
            fst_test_insert_search_mix();
            fst_test_shift_left();
            fst_test_shift_left_right_edge();
            fst_test_shift_left_append_new_segment();
            fst_test_shift_right_init0();
            fst_test_shift_right_init999();
            fst_test_shift_right_bool();
            fst_test_shift_right_skip_start_node();
            fst_test_shift_right_all_nodes();
            fst_test_const_iterator();
            fst_test_insert_iterator();
            fst_test_insert_state_changed();
            fst_test_position_search();
            fst_test_min_max_default();
            fst_test_swap();
            fst_test_clear();
            fst_test_assignment();
            fst_test_non_numeric_value();
            fst_test_insert_out_of_bound();
            fst_test_segment_iterator();
        }

        if (opt.test_perf)
        {
            fst_perf_test_search_leaf();
            fst_perf_test_search_tree();
            fst_perf_test_insert_front_back();
            fst_perf_test_insert_position();
            fst_perf_test_position_search();
        }
    }
    catch (const std::exception& e)
    {
        fprintf(stdout, "Test failed: %s\n", e.what());
        return EXIT_FAILURE;
    }

    fprintf(stdout, "Test finished successfully!\n");
    return 0;
}

