/*************************************************************************
 *
 * Copyright (c) 2008-2009 Kohei Yoshida
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

#if 0 // Disabled until I re-write priority search tree in a template.
#include "rangetree.hxx"
#include "segmenttree.hxx"
#endif
#include "flatsegmenttree.hxx"

#include <list>
#include <iostream>
#include <vector>
#include <limits>
#include <iterator>
#include <algorithm>

#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])

using namespace std;
using namespace mdds;


#include <stdio.h>
#include <string>
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

void printTitle(const char* msg)
{
    cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    cout << " " << msg << endl;
    cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
}

#if 0 // Disabled until I re-write priority search tree in a template.
void testPrioSearchTree()
{
    RangeTree emptyDB;
    emptyDB.insertPoint("Raleigh", 90, 34);
    emptyDB.build();
//  emptyDB.dump();
    emptyDB.search(0, 100);
    emptyDB.search(10, 20);

    RangeTree rangeDB;
    rangeDB.insertPoint("North Pole", -35, -42);
    rangeDB.insertPoint("Chicago", 35, 42);
    rangeDB.insertPoint("Mobile", 52, 10);
    rangeDB.insertPoint("Toronto", 62, 77);
    rangeDB.insertPoint("Buffalo", 82, 65);
    rangeDB.insertPoint("Denver", 5, 45);
    rangeDB.insertPoint("Provo", 5, 46);
    rangeDB.insertPoint("Provo", 5, 46);
    rangeDB.insertPoint("Provo", 5, 46);
    rangeDB.insertPoint("Provo", 5, 46);
    rangeDB.insertPoint("Omaha", 27, 35);
    rangeDB.insertPoint("Atlanta", 85, 15);
    rangeDB.insertPoint("Miami", 90, 5);
    rangeDB.insertPoint("Raleigh", 90, 34);
    rangeDB.insertPoint("New York", 102, 44);
    rangeDB.insertPoint("Seattle", 2, 1029);
    rangeDB.insertPoint("Bemidji", 23, 113);
    rangeDB.build();
//  rangeDB.dump();

    rangeDB.search(30, 200);
    rangeDB.printSearchResult();
    rangeDB.search(0, 52);
    rangeDB.printSearchResult();
    rangeDB.search(102, 150);
    rangeDB.printSearchResult();

    rangeDB.search(0, 100, 30);
    rangeDB.printSearchResult();
    rangeDB.search(0, 50, 100);
    rangeDB.printSearchResult();
    rangeDB.search(40, 100, 10);
    rangeDB.printSearchResult();
    rangeDB.search(-100, 10000, -100);
    rangeDB.printSearchResult();
    rangeDB.search(-100, -99, 1000);
    rangeDB.printSearchResult();
}

void testPrioSearchTree2()
{
    RangeTree db;
    for (int x = 0; x < 10; ++x)
        for (int y = 0; y < 10; ++y)
            db.insertPoint("point", x, y);
    db.build();

    for (int x = 0; x < 4; ++x)
    {
        db.search(x, x+1, 0);
        db.printSearchResult();
    }

    for (int x = 0; x < 4; ++x)
        for (int y = 0; y < 5; ++y)
        {
            db.search(x, x+1, y);
            db.printSearchResult();
        }
}

namespace {

struct SegmentName : public segment_tree::segment_data_type
{
    string name;
    SegmentName(const char* _name) :
        name(_name) {}

    virtual ~SegmentName() {}

    virtual size_t hash() const
    {
        size_t n = name.size();
        if (n > 10)
            // Put a cap at 10 characters.
            n = 10;

        size_t hash = 0;
        for (size_t i = 0; i < n; ++i)
        {
            char c = name[i];
            hash += static_cast<size_t>(c);
        }
        return hash;
    }

    virtual const char* what() const
    {
        return name.c_str();
    }
};

}

void testSegmentTree1()
{
    struct {
        segment_tree::value_type  low;
        segment_tree::value_type  high;
        SegmentName*            name;
    } points[] = {
        { 6, 36, new SegmentName("A")},
        {34, 38, new SegmentName("B")},
        {21, 36, new SegmentName("C")},
        {23, 27, new SegmentName("D")},
        { 3,  8, new SegmentName("E")},
        {15, 19, new SegmentName("F")},
        {11, 14, new SegmentName("G")}
    };

    segment_tree db;
    size_t n = sizeof(points)/sizeof(points[0]);
    for (size_t i = 0; i < n; ++i)
    {
        cout << "inserting segment into tree: " << points[i].low << " - " << points[i].high << " ("
             << points[i].name->what() << ")" << endl;
        db.insert_segment(points[i].low, points[i].high, points[i].name);
    }

    db.build();
    for (size_t i = 0; i < 40; ++i)
    {
        list<const segment_tree::segment_data_type*> hits;
        db.search(i, hits);
        cout << "segments that contains point " << i << ":";
        for (list<const segment_tree::segment_data_type*>::const_iterator itr = hits.begin(), itrEnd = hits.end();
              itr != itrEnd; ++itr)
        {
            const segment_tree::segment_data_type* p = *itr;
            cout << " " << static_cast<const SegmentName*>(p)->name;
        }
        cout << endl;
    }

    for (size_t i = 0; i < n; ++i)
        delete points[i].name;
}
#endif

void fst_test_leaf_search()
{
    {
        printTitle("Simple insert test");
        flat_segment_tree<int, int> int_ranges(0, 100, -1);
        for (int i = 0; i < 20; ++i)
        {
            int start = i*5;
            int end = start + 5;
            int_ranges.insert_segment(start, end, i);
        }
        int_ranges.dump_leaf_nodes();
    }

    {
        printTitle("Merge test 1");
        flat_segment_tree<int, int> merge_test(0, 100, -1);
        merge_test.insert_segment(10, 20, 5);
        merge_test.dump_leaf_nodes();
        merge_test.insert_segment(15, 30, 5);
        merge_test.dump_leaf_nodes();
        merge_test.insert_segment(30, 50, 5);
        merge_test.dump_leaf_nodes();
        merge_test.insert_segment(8, 11, 5);
        merge_test.dump_leaf_nodes();
        merge_test.insert_segment(5, 8, 5);
        merge_test.dump_leaf_nodes();
    }

    {
        printTitle("Merge test 2");
        flat_segment_tree<int, int> merge_test(0, 100, -1);

        // This should not change the node configuration.
        merge_test.insert_segment(10, 90, -1);
        merge_test.dump_leaf_nodes();

        for (int i = 10; i <= 80; i += 10)
            merge_test.insert_segment(i, i+10, i);
        merge_test.dump_leaf_nodes();
        merge_test.insert_segment(10, 90, -1);
        merge_test.dump_leaf_nodes();

        for (int i = 10; i <= 80; i += 10)
            merge_test.insert_segment(i, i+10, i);
        merge_test.dump_leaf_nodes();
        merge_test.insert_segment(8, 92, -1);
        merge_test.dump_leaf_nodes();

        for (int i = 10; i <= 80; i += 10)
            merge_test.insert_segment(i, i+10, i);
        merge_test.dump_leaf_nodes();
        merge_test.insert_segment(12, 88, 25);
        merge_test.dump_leaf_nodes();
    }

    {
        printTitle("Search test");
        flat_segment_tree<int, int> db(0, 100, -1);
        for (int i = 0; i < 10; ++i)
        {
            int key = i*10;
            int val = i*5;
            db.insert_segment(key, key+10, val);
        }
        db.dump_leaf_nodes();
        for (int i = 0; i <= 100; ++i)
        {
            int val = 0;
            if (db.search(i, val))
                cout << "key = " << i << "; value = " << val << endl;
            else
                cout << "key = " << i << "; (value not found)" << endl;
        }
        for (int i = 0; i <= 100; ++i)
        {
            int val = 0, start, end;
            if (db.search(i, val, &start, &end))
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
    StackPrinter __stack_printer__("::fst_test_tree_build");
    {
        int lower = 0, upper = 100, delta = 10;
        flat_segment_tree<int, int> db(lower, upper, 0);
        {
            StackPrinter __stack_printer__("::fst_test_tree_build insertion");
            for (int i = lower; i < upper; i+=delta)
                db.insert_segment(i, i+delta, i*2);
        }
        db.dump_leaf_nodes();
    
        {
            StackPrinter __stack_printer__("::fst_test_tree_build tree construction");
            db.build_tree();
            db.dump_tree();
        }    
    }

    {
        flat_segment_tree<int, int> db(0, 10, 0);
        db.dump_leaf_nodes();
    }
}

void fst_perf_test_search(bool tree_search)
{
    StackPrinter __stack_printer__("fst_perf_test_leaf_search");

    int lower = 0, upper = 50000;
    flat_segment_tree<int, int> db(lower, upper, 0);
    for (int i = upper-1; i >= lower; --i)
        db.insert_segment(i, i+1, i);

    int success = 0, failure = 0;
    if (tree_search)
    {
        fprintf(stdout, "fst_perf_test_search:   tree search\n");
        db.build_tree();

        int val;
        for (int i = lower; i < upper; ++i)
        {
            if (db.search_tree(i, val))
                ++success;
            else
                ++failure;
        }
    }
    else
    {
        fprintf(stdout, "fst_perf_test_search:   leaf search\n");
        int val;
        for (int i = lower; i < upper; ++i)
        {
            if (tree_search)
            {
                if (db.search_tree(i, val))
                    ++success;
                else
                    ++failure;
            }
            else
            {
                if (db.search(i, val))
                    ++success;
                else
                    ++failure;
            }
        }
    }
    fprintf(stdout, "fst_perf_test_search:   success (%d)  failure (%d)\n", success, failure);
}

void fst_test_tree_search()
{
    StackPrinter __stack_printer__("::fst_test_tree_search");
    int lower = 0, upper = 200, delta = 5;
    flat_segment_tree<int, int> db(lower, upper, 0);
    for (int i = lower; i < upper; i += delta)
        db.insert_segment(i, i+delta, i);

    db.build_tree();
    db.dump_tree();
    db.dump_leaf_nodes();

    int val, start, end;
    int success = 0, failure = 0;
    for (int i = lower-10; i < upper+10; ++i)
    {
        if (db.search_tree(i, val, &start, &end))
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
}

void test_single_tree_search(const flat_segment_tree<int, int>& db, int key, int val, int start, int end)
{
    int r_val, r_start, r_end;
    if (db.search_tree(key, r_val, &r_start, &r_end))
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
bool is_leaf_nodes_valid(
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
    StackPrinter __stack_printer__("fst_test_insert_search_mix");

    flat_segment_tree<int, int> db(0, 100, 0);

    build_and_dump(db);
    assert(node_base::get_instance_count() == 3);
    assert(db.is_tree_valid());
    test_single_tree_search(db, 0, 0, 0, 100);
    test_single_tree_search(db, 99, 0, 0, 100);

    db.insert_segment(0, 10, 1);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    assert(node_base::get_instance_count() == 6);
    assert(db.is_tree_valid());
    test_single_tree_search(db, 0, 1, 0, 10);
    test_single_tree_search(db, 5, 1, 0, 10);
    test_single_tree_search(db, 9, 1, 0, 10);
    test_single_tree_search(db, 10, 0, 10, 100);

    db.insert_segment(0, 100, 0);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    assert(node_base::get_instance_count() == 3);
    assert(db.is_tree_valid());
    test_single_tree_search(db, 0, 0, 0, 100);
    test_single_tree_search(db, 99, 0, 0, 100);

    db.insert_segment(10, 20, 5);
    db.insert_segment(30, 40, 5);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    assert(node_base::get_instance_count() == 12);
    assert(db.is_tree_valid());
    test_single_tree_search(db, 10, 5, 10, 20);
    test_single_tree_search(db, 20, 0, 20, 30);
    test_single_tree_search(db, 30, 5, 30, 40);
    test_single_tree_search(db, 40, 0, 40, 100);

    db.insert_segment(18, 22, 6);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    assert(node_base::get_instance_count() == 14);
    assert(db.is_tree_valid());
    test_single_tree_search(db, 18, 6, 18, 22);
    test_single_tree_search(db, 22, 0, 22, 30);
    test_single_tree_search(db, 30, 5, 30, 40);

    db.insert_segment(19, 30, 5);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    assert(node_base::get_instance_count() == 12);
    assert(db.is_tree_valid());
    test_single_tree_search(db, 19, 5, 19, 40);

    db.insert_segment(-100, 500, 999);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    assert(node_base::get_instance_count() == 3);
    assert(db.is_tree_valid());
    test_single_tree_search(db, 30, 999, 0, 100);
}

void fst_test_shift_segment_left()
{
    StackPrinter __stack_printer__("fst_test_shift_segment_left");

    flat_segment_tree<int, int> db(0, 100, 0);
    db.insert_segment(20, 40, 5);
    db.insert_segment(50, 60, 10);
    db.insert_segment(70, 80, 15);
    build_and_dump(db);

    // invalid segment ranges -- these should not modify the state of the 
    // tree, hence the tree should remain valid.
    db.shift_segment_left(5, 0);
    assert(db.is_tree_valid());

    db.shift_segment_left(95, 120);
    assert(db.is_tree_valid());

    db.shift_segment_left(105, 120);
    assert(db.is_tree_valid());

    db.shift_segment_left(-10, -5);
    assert(db.is_tree_valid());

    db.shift_segment_left(-10, 5);
    assert(db.is_tree_valid());

    // shift without removing nodes (including the lower bound).
    db.shift_segment_left(0, 5);
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
    db.shift_segment_left(1, 6);
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
    db.shift_segment_left(5, 10);
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
    db.shift_segment_left(1, 11);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 1, 15, 25, 35, 45, 55, 100};
        int vals[] = {0, 5,  0, 10,  0, 15,  0};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // shift with two overlapping nodes.
    db.shift_segment_left(2, 30);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 1,  2, 7, 17, 27, 100};
        int vals[] = {0, 5, 10, 0, 15,  0};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // shift with both ends at existing nodes, but no nodes in between.
    db.shift_segment_left(0, 1);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0,  1, 6, 16, 26, 100}; 
        int vals[] = {5, 10, 0, 15,  0};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // shift with both ends at existing nodes, no nodes in between, and
    // removing the segment results in two consecutive segments with identical
    // value.  The segments should get combined into one.
    db.shift_segment_left(16, 26);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0,  1, 6, 100}; 
        int vals[] = {5, 10, 0};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // insert two new segments for the next test....
    db.insert_segment(10, 20, 400);
    db.insert_segment(30, 40, 400);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0,  1, 6,  10, 20,  30, 40, 100}; 
        int vals[] = {5, 10, 0, 400,  0, 400,  0};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // same test as the previous one, but the value of the combined segment 
    // differs from the value of the rightmost leaf node.
    db.shift_segment_left(20, 30);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0,  1, 6,  10, 30, 100}; 
        int vals[] = {5, 10, 0, 400,  0};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // remove all.
    db.shift_segment_left(0, 100);
    assert(!db.is_tree_valid());
    build_and_dump(db);
}

void fst_test_shift_segment_left_right_edge()
{
    StackPrinter __stack_printer__("fst_test_shift_segment_left_right_edge");
    flat_segment_tree<int, bool> db(0, 100, false);
    build_and_dump(db);

    // This should not change the tree state.
    db.shift_segment_left(2, 100);
    build_and_dump(db);
    {
        int  keys[] = {0, 100};
        bool vals[] = {false};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    db.insert_segment(20, 100, true);
    build_and_dump(db);
    {
        int  keys[] = {    0,   20, 100};
        bool vals[] = {false, true};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // This should insert a new segment at the end with the initial base value.
    db.shift_segment_left(80, 100);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int  keys[] = {    0,   20,    80, 100};
        bool vals[] = {false, true, false};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // This should not modify the tree since the removed segment already has 
    // the initial base value.
    db.shift_segment_left(85, 100);
    assert(db.is_tree_valid()); // tree must still be valid.
    build_and_dump(db);
    {
        int  keys[] = {    0,   20,    80, 100};
        bool vals[] = {false, true, false};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // Insert a new segment at the end with the value 'true' again...
    db.insert_segment(85, 100, true);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int  keys[] = {    0,   20,    80,   85, 100};
        bool vals[] = {false, true, false, true};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    db.shift_segment_left(90, 95);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int  keys[] = {    0,   20,    80,   85,    95, 100};
        bool vals[] = {false, true, false, true, false};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }
}

void fst_test_shift_segment_left_append_new_segment()
{
    StackPrinter __stack_printer__("fst_test_shift_segment_left_append_new_segment");
    flat_segment_tree<int, bool> db(0, 100, false);
    db.insert_segment(0, 100, true);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int  keys[] = {   0, 100};
        bool vals[] = {true};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    db.shift_segment_left(10, 20);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int  keys[] = {   0,    90, 100};
        bool vals[] = {true, false};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    db.insert_segment(0, 10, true);
    db.insert_segment(10, 20, false);
    db.insert_segment(20, 60, true);
    db.insert_segment(60, 80, false);
    db.insert_segment(80, 100, true);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int  keys[] = {   0,    10,   20,    60,   80, 100};
        bool vals[] = {true, false, true, false, true};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    db.shift_segment_left(0, 70);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int  keys[] = {   0,    10,    30, 100};
        bool vals[] = {false, true, false};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }
}

void fst_test_shift_segment_right_init0()
{
    StackPrinter __stack_printer__("fst_test_shift_segment_right_init0");

    flat_segment_tree<int, int> db(0, 100, 0);
    db.insert_segment(0,  10,  15);
    db.insert_segment(10, 20,  1);
    db.insert_segment(20, 30,  2);
    db.insert_segment(30, 40,  3);
    db.insert_segment(40, 50,  4);
    db.insert_segment(50, 60,  5);
    db.insert_segment(60, 70,  6);
    db.insert_segment(70, 80,  7);
    db.insert_segment(80, 90,  8);
    assert(!db.is_tree_valid());
    build_and_dump(db);

    // shifting position is at the lower bound.  The leftmost segment has a 
    // non-zero value which needs to be preserved after the shift by adding a
    // new node.
    db.shift_segment_right(0, 5, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0,  5, 15, 25, 35, 45, 55, 65, 75, 85, 95,100};
        int vals[] = {0, 15,  1,  2,  3,  4,  5,  6,  7,  8,  0};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // shifting position is at the lower bound, and after the shift, the upper 
    // bound of the last non-zero segment (10) becomes the upper bound of the
    // global range.
    db.shift_segment_right(0, 5, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
        int vals[] = {0, 15,  1,  2,  3,  4,  5,  6,  7,  8};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // Shift by some odd number.
    db.shift_segment_right(0, 49, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 59, 69, 79, 89, 99, 100};
        int vals[] = {0, 15,  1,  2,  3,  4};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // Shift so that the 2nd node from the right-most node becomes the new 
    // right-most node.
    db.shift_segment_right(0, 11, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 70, 80, 90, 100};
        int vals[] = {0, 15,  1,  2};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // This should remove all segments.
    db.shift_segment_right(0, 30, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 100};
        int vals[] = {0};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // Insert a few new segments for the next series of tests...
    db.insert_segment(5, 10, 5);
    db.insert_segment(20, 30, 5);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 5, 10, 20, 30, 100};
        int vals[] = {0, 5,  0,  5,  0};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // Inserting at a non-node position.  This should simply extend that 
    // segment and shift all the others.
    db.shift_segment_right(6, 20, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 5, 30, 40, 50, 100};
        int vals[] = {0, 5,  0,  5,  0};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // Inserting at a node position.
    db.shift_segment_right(5, 20, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 25, 50, 60, 70, 100};
        int vals[] = {0, 5,  0,  5,  0};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // Inserting at a non-node position, pushing a node out-of-bound.
    db.shift_segment_right(65, 40, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 25, 50, 60, 100};
        int vals[] = {0, 5,  0,  5};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }

    // Inserting at a node position, pushing a node out-of-bound.
    db.shift_segment_right(50, 40, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int keys[] = {0, 25, 90, 100};
        int vals[] = {0,  5,  0};
        assert(is_leaf_nodes_valid(db, keys, vals, ARRAY_SIZE(keys)));
    }
}

void fst_test_shift_segment_right_init999()
{
    StackPrinter __stack_printer__("fst_test_shift_segment_right_init999");

    // Initialize the tree with a default value of 999.
    flat_segment_tree<int, int> db(0, 100, 999);
    db.insert_segment(0, 10, 0);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int k[] = {0, 10, 100};
        int v[] = {0, 999};
        assert(is_leaf_nodes_valid(db, k, v, ARRAY_SIZE(k)));
    }

    // This should only extend the first segment.
    db.shift_segment_right(1, 10, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int k[] = {0, 20, 100};
        int v[] = {0, 999};
        assert(is_leaf_nodes_valid(db, k, v, ARRAY_SIZE(k)));
    }

    // Inserting at the leftmost node position should create a new segment 
    // with a default value of 999.
    db.shift_segment_right(0, 10, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        int k[] = {0, 10, 30, 100};
        int v[] = {999, 0, 999};
        assert(is_leaf_nodes_valid(db, k, v, ARRAY_SIZE(k)));
    }

    // Invalid shifts -- these should not invalidate the tree.
    db.shift_segment_right(-10, 10, false);
    assert(db.is_tree_valid());

    db.shift_segment_right(100, 10, false);
    assert(db.is_tree_valid());

    db.shift_segment_right(0, 0, false);
    assert(db.is_tree_valid());
}

void fst_test_shift_segment_right_bool()
{
    flat_segment_tree<long, bool> db(0, 1048576, false);
    db.insert_segment(3, 7, true);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        long k[] = {0, 3, 7, 1048576};
        bool v[] = {false, true, false};
        assert(is_leaf_nodes_valid(db, k, v, ARRAY_SIZE(k)));
    }

    db.shift_segment_right(1, 1, false);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        long k[] = {0, 4, 8, 1048576};
        bool v[] = {false, true, false};
        assert(is_leaf_nodes_valid(db, k, v, ARRAY_SIZE(k)));
    }
}

void fst_test_shift_segment_right_skip_start_node()
{
    StackPrinter __stack_printer__("fst_test_shift_segment_right_skip_start_node");

    flat_segment_tree<long, short> db(0, 1048576, 0);
    db.insert_segment(3, 7, 5);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        long  k[] = {0, 3, 7, 1048576};
        short v[] = {0, 5, 0};
        assert(is_leaf_nodes_valid(db, k, v, ARRAY_SIZE(k)));
    }

    db.shift_segment_right(3, 2, true);
    assert(!db.is_tree_valid());
    build_and_dump(db);
    {
        long  k[] = {0, 3, 9, 1048576};
        short v[] = {0, 5, 0};
        assert(is_leaf_nodes_valid(db, k, v, ARRAY_SIZE(k)));
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
    StackPrinter __stack_printer__("::fst_test_const_reverse_iterator");

    typedef unsigned int    key_type;
    typedef unsigned short  value_type;
    unsigned short max_value = numeric_limits<value_type>::max();
    typedef flat_segment_tree<key_type, value_type> container_type;

    container_type db(0, 1000, max_value);

    build_and_dump(db);
    {
        unsigned int   k[] = {0, 1000};
        unsigned short v[] = {max_value};
        assert(is_leaf_nodes_valid(db, k, v, ARRAY_SIZE(k)));
    }

    db.insert_segment(10, 20, 10);
    db.insert_segment(20, 50, 20);
    db.insert_segment(100, 300, 55);
    build_and_dump(db);
    {
        unsigned int   k[] = {0, 10, 20, 50, 100, 300, 1000};
        unsigned short v[] = {max_value, 10, 20, max_value, 55, max_value};
        assert(is_leaf_nodes_valid(db, k, v, ARRAY_SIZE(k)));
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

void fst_test_insert_back()
{
    StackPrinter __stack_printer__("::fst_test_insert_back");
}

int main (int argc, char *argv[])
{
#if 0 // Disabled until I re-write priority search tree in a template.
    testPrioSearchTree();
    testPrioSearchTree2();
    testSegmentTree1();
#endif
    fst_test_insert_back();
    return 0;

    fst_test_leaf_search();
    fst_test_tree_build();
    fst_test_tree_search();
    fst_perf_test_search(false);
    fst_perf_test_search(true);
    fst_test_insert_search_mix();
    fst_test_shift_segment_left();
    fst_test_shift_segment_left_right_edge();
    fst_test_shift_segment_left_append_new_segment();
    fst_test_shift_segment_right_init0();
    fst_test_shift_segment_right_init999();
    fst_test_shift_segment_right_bool();
    fst_test_shift_segment_right_skip_start_node();
    fst_test_const_iterator();
    fprintf(stdout, "Test finished successfully!\n");
    return 0;
}

