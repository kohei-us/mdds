/*************************************************************************
 *
 * Copyright (c) 2011-2018 Kohei Yoshida
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
#include "test_main.hpp"

using namespace std;
using namespace mdds;

namespace {

enum test_mtv_type {
    _mtv_bool, _mtv_int8, _mtv_uint8, _mtv_int16, _mtv_uint16, _mtv_int32, _mtv_uint32, _mtv_int64, _mtv_uint64, _mtv_float, _mtv_double, _mtv_string,
};

#define TEST_TYPE(_type_,_type_enum_) test_mtv_type test_type(_type_) { return _type_enum_; }
TEST_TYPE(bool,     _mtv_bool)
TEST_TYPE(int8_t,   _mtv_int8)
TEST_TYPE(uint8_t,  _mtv_uint8)
TEST_TYPE(int16_t,  _mtv_int16)
TEST_TYPE(uint16_t, _mtv_uint16)
TEST_TYPE(int32_t,  _mtv_int32)
TEST_TYPE(uint32_t, _mtv_uint32)
TEST_TYPE(int64_t,  _mtv_int64)
TEST_TYPE(uint64_t, _mtv_uint64)
TEST_TYPE(float,    _mtv_float)
TEST_TYPE(double,   _mtv_double)
TEST_TYPE(string,   _mtv_string)

void mtv_test_types()
{
    // Test function overloading of standard types.

    stack_printer __stack_printer__(__FUNCTION__);
    {
        bool val = false;
        assert(test_type(val) == _mtv_bool);
        cout << "bool is good" << endl;
    }
    {
        int16_t val = 0;
        assert(test_type(val) == _mtv_int16);
        cout << "int16 is good" << endl;
    }
    {
        uint16_t val = 0;
        assert(test_type(val) == _mtv_uint16);
        cout << "uint16 is good" << endl;
    }
    {
        int32_t val = 0;
        assert(test_type(val) == _mtv_int32);
        cout << "int32 is good" << endl;
    }
    {
        uint32_t val = 0;
        assert(test_type(val) == _mtv_uint32);
        cout << "uint32 is good" << endl;
    }
    {
        int64_t val = 0;
        assert(test_type(val) == _mtv_int64);
        cout << "int64 is good" << endl;
    }
    {
        uint64_t val = 0;
        assert(test_type(val) == _mtv_uint64);
        cout << "uint64 is good" << endl;
    }
    {
        float val = 0;
        assert(test_type(val) == _mtv_float);
        cout << "float is good" << endl;
    }
    {
        double val = 0;
        assert(test_type(val) == _mtv_double);
        cout << "double is good" << endl;
    }
    {
        string val;
        assert(test_type(val) == _mtv_string);
        cout << "string is good" << endl;
    }
    {
        int8_t val = 0;
        assert(test_type(val) == _mtv_int8);
        cout << "int8 is good" << endl;
    }
    {
        uint8_t val = 0;
        assert(test_type(val) == _mtv_uint8);
        cout << "uint8 is good" << endl;
    }
}

void mtv_test_swap_range()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db1(10), db2(10);
    db1.set(2, 1.1);
    db1.set(3, 1.2);
    db1.set(4, 1.3);

    db2.set(2, 2.1);
    db2.set(3, 2.2);
    db2.set(4, 2.3);

    db1.swap(2, 4, db2, 2);
    assert(db1.get<double>(2) == 2.1);
    assert(db1.get<double>(3) == 2.2);
    assert(db1.get<double>(4) == 2.3);
    assert(db2.get<double>(2) == 1.1);
    assert(db2.get<double>(3) == 1.2);
    assert(db2.get<double>(4) == 1.3);

    // Source is empty but destination is not.
    db1 = mtv_type(3);
    db2 = mtv_type(3, 12.3);
    db1.swap(1, 2, db2, 1);
    assert(db1.is_empty(0));
    assert(db1.get<double>(1) == 12.3);
    assert(db1.get<double>(2) == 12.3);
    assert(db1.block_size() == 2);
    assert(db2.get<double>(0) == 12.3);
    assert(db2.is_empty(1));
    assert(db2.is_empty(2));
    assert(db2.block_size() == 2);

    // Go to the opposite direction.
    db1.swap(1, 2, db2, 1);
    assert(db1.block_size() == 1);
    mtv_type::iterator it = db1.begin();
    assert(it->type == mtv::element_type_empty);
    assert(it->size == 3);
    it = db2.begin();
    assert(it->type == mtv::element_type_double);
    assert(it->size == 3);

    int32_t int_val = 2;
    int16_t short_val = 5;
    db1 = mtv_type(5, int_val);
    db2 = mtv_type(5, short_val);
    db1.set(1, 2.3);
    db1.set(2, 2.4);
    db2.set(3, string("abc"));
    db2.set(4, string("def"));
    db1.swap(1, 2, db2, 3); // Swap 1-2 of source with 3-4 of destination.
    assert(db1.get<int32_t>(0) == int_val);
    assert(db1.get<string>(1) == "abc");
    assert(db1.get<string>(2) == "def");
    assert(db1.get<int32_t>(3) == int_val);
    assert(db1.get<int32_t>(4) == int_val);
    assert(db1.block_size() == 3);

    assert(db2.get<int16_t>(0) == short_val);
    assert(db2.get<int16_t>(1) == short_val);
    assert(db2.get<int16_t>(2) == short_val);
    assert(db2.get<double>(3) == 2.3);
    assert(db2.get<double>(4) == 2.4);
    assert(db2.block_size() == 2);

    // Merge with the next block in the destination.
    db1 = mtv_type(5, int_val);
    db2 = mtv_type(5, 12.3);
    db2.set(0, string("A"));
    db2.set(1, string("B"));
    db1.set(3, 1.1);
    db1.set(4, 1.2);
    db1.swap(3, 4, db2, 0);
    assert(db1.get<int32_t>(0) == int_val);
    assert(db1.get<int32_t>(1) == int_val);
    assert(db1.get<int32_t>(2) == int_val);
    assert(db1.get<string>(3) == "A");
    assert(db1.get<string>(4) == "B");
    assert(db1.block_size() == 2);

    assert(db2.get<double>(0) == 1.1);
    assert(db2.get<double>(1) == 1.2);
    assert(db2.get<double>(2) == 12.3);
    assert(db2.get<double>(3) == 12.3);
    assert(db2.get<double>(4) == 12.3);
    assert(db2.block_size() == 1);

    // Merge with the previous block in the destination.
    db1 = mtv_type(5, int_val);
    db1.set(2, string("A"));
    db1.set(3, string("B"));

    db2 = mtv_type(5, string("default"));
    db2.set(3, short_val);
    db2.set(4, short_val);

    db1.swap(2, 3, db2, 3);
    assert(db1.get<int32_t>(0) == int_val);
    assert(db1.get<int32_t>(1) == int_val);
    assert(db1.get<int16_t>(2) == short_val);
    assert(db1.get<int16_t>(3) == short_val);
    assert(db1.get<int32_t>(4) == int_val);
    assert(db1.block_size() == 3);

    assert(db2.get<string>(0) == "default");
    assert(db2.get<string>(1) == "default");
    assert(db2.get<string>(2) == "default");
    assert(db2.get<string>(3) == "A");
    assert(db2.get<string>(4) == "B");
    assert(db2.block_size() == 1);

    // Merge with both the previous and next blocks in the destination.
    db1 = mtv_type(5, int_val);
    db1.set(2, string("C"));
    db1.set(3, string("D"));

    db2 = mtv_type(6, string("default"));
    db2.set(3, short_val);
    db2.set(4, short_val);

    db1.swap(2, 3, db2, 3);
    assert(db1.get<int32_t>(0) == int_val);
    assert(db1.get<int32_t>(1) == int_val);
    assert(db1.get<int16_t>(2) == short_val);
    assert(db1.get<int16_t>(3) == short_val);
    assert(db1.get<int32_t>(4) == int_val);
    assert(db1.block_size() == 3);

    assert(db2.get<string>(0) == "default");
    assert(db2.get<string>(1) == "default");
    assert(db2.get<string>(2) == "default");
    assert(db2.get<string>(3) == "C");
    assert(db2.get<string>(4) == "D");
    assert(db2.get<string>(5) == "default");
    assert(db2.block_size() == 1);

    // Set the new elements to the top of a block in the destination.
    db1 = mtv_type(5, int_val);
    db1.set(3, string("E"));
    db1.set(4, string("F"));
    db2 = mtv_type(5, short_val);
    db1.swap(3, 4, db2, 0);
    assert(db1.get<int32_t>(0) == int_val);
    assert(db1.get<int32_t>(1) == int_val);
    assert(db1.get<int32_t>(2) == int_val);
    assert(db1.get<int16_t>(3) == short_val);
    assert(db1.get<int16_t>(4) == short_val);
    assert(db1.block_size() == 2);
    assert(db2.get<string>(0) == "E");
    assert(db2.get<string>(1) == "F");
    assert(db2.get<int16_t>(2) == short_val);
    assert(db2.get<int16_t>(3) == short_val);
    assert(db2.get<int16_t>(4) == short_val);
    assert(db2.block_size() == 2);

    // Do the same as before, but merge with the previous block.
    db1 = mtv_type(5, int_val);
    db1.set(3, string("G"));
    db1.set(4, string("H"));
    db2 = mtv_type(5, short_val);
    db2.set(0, string("F"));
    db1.swap(3, 4, db2, 1);
    assert(db1.get<int32_t>(0) == int_val);
    assert(db1.get<int32_t>(1) == int_val);
    assert(db1.get<int32_t>(2) == int_val);
    assert(db1.get<int16_t>(3) == short_val);
    assert(db1.get<int16_t>(4) == short_val);
    assert(db1.block_size() == 2);
    assert(db2.get<string>(0) == "F");
    assert(db2.get<string>(1) == "G");
    assert(db2.get<string>(2) == "H");
    assert(db2.get<int16_t>(3) == short_val);
    assert(db2.get<int16_t>(4) == short_val);
    assert(db2.block_size() == 2);

    // Set the new element to the middle of a destination block.
    db1 = mtv_type(5, int_val);
    db1.set(3, string("I"));
    db1.set(4, string("J"));
    db2 = mtv_type(5, short_val);
    db2.set(0, string("top"));
    db1.swap(3, 4, db2, 2);
    assert(db1.get<int32_t>(0) == int_val);
    assert(db1.get<int32_t>(1) == int_val);
    assert(db1.get<int32_t>(2) == int_val);
    assert(db1.get<int16_t>(3) == short_val);
    assert(db1.get<int16_t>(4) == short_val);
    assert(db1.block_size() == 2);

    assert(db2.get<string>(0) == "top");
    assert(db2.get<int16_t>(1) == short_val);
    assert(db2.get<string>(2) == "I");
    assert(db2.get<string>(3) == "J");
    assert(db2.get<int16_t>(4) == short_val);
    assert(db2.block_size() == 4);

    // Set the new element to the lower part of a destination block.
    db1 = mtv_type(5, int_val);
    db1.set(0, string("A1"));
    db1.set(1, string("A2"));
    db2 = mtv_type(5, short_val);
    db1.swap(0, 1, db2, 3);

    assert(db1.get<int16_t>(0) == short_val);
    assert(db1.get<int16_t>(1) == short_val);
    assert(db1.get<int32_t>(2) == int_val);
    assert(db1.get<int32_t>(3) == int_val);
    assert(db1.get<int32_t>(4) == int_val);
    assert(db1.block_size() == 2);

    assert(db2.get<int16_t>(0) == short_val);
    assert(db2.get<int16_t>(1) == short_val);
    assert(db2.get<int16_t>(2) == short_val);
    assert(db2.get<string>(3) == "A1");
    assert(db2.get<string>(4) == "A2");
    assert(db2.block_size() == 2);

    // Same as above, except that the new element will be merged with the next
    // block in the destination.
    db1 = mtv_type(5, int_val);
    db1.set(0, string("A1"));
    db1.set(1, string("A2"));
    db2 = mtv_type(6, short_val);
    db2.set(5, string("A3"));
    db1.swap(0, 1, db2, 3);

    assert(db1.get<int16_t>(0) == short_val);
    assert(db1.get<int16_t>(1) == short_val);
    assert(db1.get<int32_t>(2) == int_val);
    assert(db1.get<int32_t>(3) == int_val);
    assert(db1.get<int32_t>(4) == int_val);
    assert(db1.block_size() == 2);

    assert(db2.get<int16_t>(0) == short_val);
    assert(db2.get<int16_t>(1) == short_val);
    assert(db2.get<int16_t>(2) == short_val);
    assert(db2.get<string>(3) == "A1");
    assert(db2.get<string>(4) == "A2");
    assert(db2.get<string>(5) == "A3");
    assert(db2.block_size() == 2);

    // Swap the top part of source block.
    db1 = mtv_type(5, int_val);
    db2 = mtv_type(5, short_val);
    db1.swap(0, 1, db2, 0);

    assert(db1.get<int16_t>(0) == short_val);
    assert(db1.get<int16_t>(1) == short_val);
    assert(db1.get<int32_t>(2) == int_val);
    assert(db1.get<int32_t>(3) == int_val);
    assert(db1.get<int32_t>(4) == int_val);

    assert(db2.get<int32_t>(0) == int_val);
    assert(db2.get<int32_t>(1) == int_val);
    assert(db2.get<int16_t>(2) == short_val);
    assert(db2.get<int16_t>(3) == short_val);
    assert(db2.get<int16_t>(4) == short_val);

    // Do the same, and merge with the previous block in the source.
    db1 = mtv_type(5, int_val);
    db1.set(0, string("A"));
    db2 = mtv_type(5, short_val);
    db2.set(0, string("B"));
    db2.set(1, string("C"));
    db1.swap(1, 2, db2, 0);

    assert(db1.get<string>(0) == "A");
    assert(db1.get<string>(1) == "B");
    assert(db1.get<string>(2) == "C");
    assert(db1.get<int32_t>(3) == int_val);
    assert(db1.get<int32_t>(4) == int_val);
    assert(db1.block_size() == 2);

    assert(db2.get<int32_t>(0) == int_val);
    assert(db2.get<int32_t>(1) == int_val);
    assert(db2.get<int16_t>(2) == short_val);
    assert(db2.get<int16_t>(3) == short_val);
    assert(db2.get<int16_t>(4) == short_val);
    assert(db2.block_size() == 2);

    // Replace the bottom part of existing source block.
    db1 = mtv_type(2, true);
    db2 = mtv_type(1, int_val);
    db1.swap(1, 1, db2, 0);
    assert(db1.get<bool>(0) == true);
    assert(db1.get<int32_t>(1) == int_val);
    assert(db2.get<bool>(0) == true);

    // Do the same, but merge with the next block in the source.
    db1 = mtv_type(3, true);
    db1.set<int32_t>(2, int_val+1);
    db2 = mtv_type(1, int_val);
    db1.swap(1, 1, db2, 0);
    assert(db1.get<bool>(0) == true);
    assert(db1.get<int32_t>(1) == int_val);
    assert(db1.get<int32_t>(2) == int_val+1);
    assert(db2.get<bool>(0) == true);

    // Replace the middle of existing source block.
    db1 = mtv_type(5);
    db1.set<int8_t>(0, 'a');
    db1.set<int8_t>(1, 'b');
    db1.set<int8_t>(2, 'c');
    db1.set<int8_t>(3, 'd');
    db1.set<int8_t>(4, 'e');
    db2 = mtv_type(2);
    db2.set(0, 1.1);
    db2.set(1, -1.1);
    db1.swap(2, 3, db2, 0);
    assert(db1.get<int8_t>(0) == 'a');
    assert(db1.get<int8_t>(1) == 'b');
    assert(db1.get<double>(2) == 1.1);
    assert(db1.get<double>(3) == -1.1);
    assert(db1.get<int8_t>(4) == 'e');
    assert(db1.block_size() == 3);

    assert(db2.get<int8_t>(0) == 'c');
    assert(db2.get<int8_t>(1) == 'd');
    assert(db2.block_size() == 1);

    // Swap single empty block with multiple destination blocks.
    db1 = mtv_type(5);
    db2 = mtv_type(5);
    db2.set(0, 1.1);
    db2.set(1, 2.1);
    db2.set(2, 3.1);
    db2.set(3, string("abc"));
    db2.set(4, string("def"));
    db1.swap(0, 3, db2, 1);
    assert(db1.get<double>(0) == 2.1);
    assert(db1.get<double>(1) == 3.1);
    assert(db1.get<string>(2) == "abc");
    assert(db1.get<string>(3) == "def");
    assert(db1.is_empty(4));
    assert(db1.block_size() == 3);
    assert(db2.get<double>(0) == 1.1);
    assert(db2.is_empty(1));
    assert(db2.is_empty(2));
    assert(db2.is_empty(3));
    assert(db2.is_empty(4));
    assert(db2.block_size() == 2);

    // Swap non-empty single block with multiple destination blocks.
    db1 = mtv_type(4, int_val);
    db2 = mtv_type(5);
    db2.set(0, 1.1);
    db2.set(1, 2.1);
    db2.set(2, 3.1);
    db2.set(3, string("abc"));
    db2.set(4, string("def"));
    db1.swap(0, 3, db2, 1);
    assert(db1.get<double>(0) == 2.1);
    assert(db1.get<double>(1) == 3.1);
    assert(db1.get<string>(2) == "abc");
    assert(db1.get<string>(3) == "def");
    assert(db1.block_size() == 2);
    assert(db2.get<double>(0) == 1.1);
    assert(db2.get<int32_t>(1) == int_val);
    assert(db2.get<int32_t>(2) == int_val);
    assert(db2.get<int32_t>(3) == int_val);
    assert(db2.get<int32_t>(4) == int_val);
    assert(db2.block_size() == 2);

    // Another scenario.
    db1 = mtv_type(2, short_val);
    db2 = mtv_type(2);
    db2.set(0, string("A"));
    db2.set<int8_t>(1, 'A');
    db1.swap(0, 1, db2, 0);
    assert(db1.get<string>(0) == "A");
    assert(db1.get<int8_t>(1) == 'A');
    assert(db2.get<int16_t>(0) == short_val);
    assert(db2.get<int16_t>(1) == short_val);

    // Another scenario.
    db1 = mtv_type(2, 3.14);
    db2 = mtv_type(3);
    db2.set(0, string("abc"));
    db2.set<uint8_t>(1, 'z');
    db2.set<uint8_t>(2, 'y');
    db1.swap(0, 1, db2, 0);
    assert(db1.get<string>(0) == "abc");
    assert(db1.get<uint8_t>(1) == 'z');
    assert(db2.get<double>(0) == 3.14);
    assert(db2.get<double>(1) == 3.14);
    assert(db2.get<uint8_t>(2) == 'y');

    // Another scenario.
    db1 = mtv_type(5);
    db1.set<int32_t>(0, 1);
    db1.set<int32_t>(1, 2);
    db1.set<int32_t>(2, 3);
    db1.set<int32_t>(3, 4);
    db1.set<int32_t>(4, 5);
    db2 = mtv_type(3);
    db2.set(0, 2.3);
    db2.set<int8_t>(1, 'B');
    db2.set<int64_t>(2, 123);
    db1.swap(0, 2, db2, 0);
    assert(db1.get<double>(0) == 2.3);
    assert(db1.get<int8_t>(1) == 'B');
    assert(db1.get<int64_t>(2) == 123);
    assert(db1.get<int32_t>(3) == 4);
    assert(db1.get<int32_t>(4) == 5);
    assert(db2.get<int32_t>(0) == 1);
    assert(db2.get<int32_t>(1) == 2);
    assert(db2.get<int32_t>(2) == 3);
    assert(db2.block_size() == 1);

    // Another one.
    db1 = mtv_type(3, string("test"));
    db2 = mtv_type(2);
    db2.set(0, -99.1);
    db2.set(1, string("foo"));
    db1.swap(1, 2, db2, 0);
    assert(db1.get<string>(0) == "test");
    assert(db1.get<double>(1) == -99.1);
    assert(db1.get<string>(2) == "foo");
    assert(db2.get<string>(0) == "test");
    assert(db2.get<string>(1) == "test");

    // The source range is in the middle of a block.
    db1 = mtv_type(8);
    for (int32_t i = 0; i < 8; ++i)
        db1.set<int32_t>(i, i+2);
    db2 = mtv_type(4);
    db2.set<int32_t>(0, 12);
    db2.set<int16_t>(1, 13);
    db2.set<int64_t>(2, 14);
    db2.set<double>(3, 15.0);
    db1.swap(3, 5, db2, 1);

    assert(db1.get<int32_t>(0) == 2);
    assert(db1.get<int32_t>(1) == 3);
    assert(db1.get<int32_t>(2) == 4);
    assert(db1.get<int16_t>(3) == 13);
    assert(db1.get<int64_t>(4) == 14);
    assert(db1.get<double>(5) == 15.0);
    assert(db1.get<int32_t>(6) == 8);
    assert(db1.get<int32_t>(7) == 9);

    assert(db2.get<int32_t>(0) == 12);
    assert(db2.get<int32_t>(1) == 5);
    assert(db2.get<int32_t>(2) == 6);
    assert(db2.get<int32_t>(3) == 7);
    assert(db2.block_size() == 1);

    // Try swapping in a multi-to-single block direction.
    db1 = mtv_type(2);
    db1.set(0, 1.2);
    db1.set(1, string("123"));
    db2 = mtv_type(10, short_val);
    db1.swap(0, 1, db2, 4);
    assert(db1.get<int16_t>(0) == short_val);
    assert(db1.get<int16_t>(1) == short_val);

    assert(db2.get<int16_t>(0) == short_val);
    assert(db2.get<int16_t>(1) == short_val);
    assert(db2.get<int16_t>(2) == short_val);
    assert(db2.get<int16_t>(3) == short_val);
    assert(db2.get<double>(4) == 1.2);
    assert(db2.get<string>(5) == "123");
    assert(db2.get<int16_t>(6) == short_val);
    assert(db2.get<int16_t>(7) == short_val);
    assert(db2.get<int16_t>(8) == short_val);
    assert(db2.get<int16_t>(9) == short_val);

    // Multi-to-multi block swapping. Very simple case.
    db1 = mtv_type(2);
    db1.set(0, 2.1);
    db1.set(1, string("test"));
    db2 = mtv_type(2);
    db2.set(0, int_val);
    db2.set(1, short_val);
    db1.swap(0, 1, db2, 0);

    assert(db1.get<int32_t>(0) == int_val);
    assert(db1.get<int16_t>(1) == short_val);
    assert(db2.get<double>(0) == 2.1);
    assert(db2.get<string>(1) == "test");

    // More complex case.
    db1 = mtv_type(10);
    db1.set<int32_t>(0, 2);
    db1.set<int32_t>(1, 3);
    db1.set<int32_t>(2, 4);
    db1.set<string>(3, "A");
    db1.set<string>(4, "B");
    db1.set<string>(5, "C");
    // Leave some empty range.
    db2 = mtv_type(10);
    for (int32_t i = 0; i < 10; ++i)
        db2.set<int32_t>(i, 10+i);
    db2.set<int8_t>(5, 'Z');
    db1.swap(1, 7, db2, 2);

    assert(db1.get<int32_t>(0) == 2);
    assert(db1.get<int32_t>(1) == 12);
    assert(db1.get<int32_t>(2) == 13);
    assert(db1.get<int32_t>(3) == 14);
    assert(db1.get<int8_t>(4) == 'Z');
    assert(db1.get<int32_t>(5) == 16);
    assert(db1.get<int32_t>(6) == 17);
    assert(db1.get<int32_t>(7) == 18);
    assert(db1.is_empty(8));
    assert(db1.is_empty(9));
    assert(db1.block_size() == 4);

    assert(db2.get<int32_t>(0) == 10);
    assert(db2.get<int32_t>(1) == 11);
    assert(db2.get<int32_t>(2) == 3);
    assert(db2.get<int32_t>(3) == 4);
    assert(db2.get<string>(4) == "A");
    assert(db2.get<string>(5) == "B");
    assert(db2.get<string>(6) == "C");
    assert(db2.is_empty(7));
    assert(db2.is_empty(8));
    assert(db2.get<int32_t>(9) == 19);
    assert(db2.block_size() == 4);
}

struct block_node_printer
{
    void operator() (const mtv_type::value_type& node) const
    {
        cout << "type: " << node.type << "  size: " << node.size << "  data: " << node.data << endl;
    }
};

void mtv_test_value_type()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db(5);
    db.set(0, 1.1);
    db.set(1, string("A"));
    db.set(2, string("B"));
    db.set(3, int32_t(12));
    db.set(4, int16_t(8));
    for_each(db.begin(), db.end(), block_node_printer());
}

void mtv_test_block_identifier()
{
    stack_printer __stack_printer__(__FUNCTION__);
    assert(mtv::double_element_block::block_type == mtv::element_type_double);
    assert(mtv::string_element_block::block_type == mtv::element_type_string);
    assert(mtv::int16_element_block::block_type == mtv::element_type_int16);
    assert(mtv::uint16_element_block::block_type == mtv::element_type_uint16);
    assert(mtv::int32_element_block::block_type == mtv::element_type_int32);
    assert(mtv::uint32_element_block::block_type == mtv::element_type_uint32);
    assert(mtv::int64_element_block::block_type == mtv::element_type_int64);
    assert(mtv::uint64_element_block::block_type == mtv::element_type_uint64);
    assert(mtv::boolean_element_block::block_type == mtv::element_type_boolean);
    assert(mtv::int8_element_block::block_type == mtv::element_type_int8);
    assert(mtv::uint8_element_block::block_type == mtv::element_type_uint8);
}

void mtv_test_transfer()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db1(5), db2(5);
    db1.set(0, 1.0);
    db1.set(1, 2.0);
    mtv_type::iterator it = db1.transfer(1, 2, db2, 1);

    assert(db1.get<double>(0) == 1.0);
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.is_empty(3));
    assert(db1.is_empty(4));

    assert(db2.is_empty(0));
    assert(db2.get<double>(1) == 2.0);
    assert(db2.is_empty(2));
    assert(db2.is_empty(3));
    assert(db2.is_empty(4));

    assert(it->size == 4);
    assert(it->__private_data.block_index == 1);
    assert(it->position == 1);

    // Reset and start over.
    db1.clear();
    db1.resize(5);
    db2.clear();
    db2.resize(5);

    db1.set(2, 1.2);
    db1.set(3, 1.3);

    // Transfer 1:2 in db1 to 2:3 in db2.
    it = db1.transfer(1, 2, db2, 2);

    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.get<double>(3) == 1.3);
    assert(db1.is_empty(4));

    assert(db2.is_empty(0));
    assert(db2.is_empty(1));
    assert(db2.is_empty(2));
    assert(db2.get<double>(3) == 1.2);
    assert(db2.is_empty(4));

    assert(it->size == 3);
    assert(it->position == 0);
    assert(it->__private_data.block_index == 0);

    // Reset and start over.
    db1.clear();
    db1.resize(4);
    db2.clear();
    db2.resize(4);

    db1.set(0, string("A"));
    db1.set(1, string("B"));
    db1.set(2, 11.1);
    db1.set(3, 11.2);

    it = db1.transfer(1, 2, db2, 1);

    assert(db1.get<string>(0) == "A");
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.get<double>(3) == 11.2);

    assert(db2.is_empty(0));
    assert(db2.get<string>(1) == "B");
    assert(db2.get<double>(2) == 11.1);
    assert(db2.is_empty(3));

    assert(it->size == 2);
    assert(it->position == 1);
    assert(it->__private_data.block_index == 1);

    // Reset and start over.
    db1.clear();
    db1.resize(4);
    db2.clear();
    db2.resize(4);

    db1.set(2, 12.8);
    it = db1.transfer(1, 2, db2, 1);

    // Reset and start over.
    db1.clear();
    db1.resize(20);
    db2.clear();
    db2.resize(20);

    db1.set(9, 1.1);

    db2.set(10, 1.2);
    db2.set(11, 1.3);

    it = db1.transfer(9, 9, db2, 9);

    // The source should be entirely empty after the transfer.
    assert(db1.block_size() == 1);
    assert(it == db1.begin());
    assert(it->__private_data.block_index == 0);
    assert(it->size == 20);
    assert(it->type == mtv::element_type_empty);
    ++it;
    assert(it == db1.end());

    // Check the destination as well.
    assert(db2.block_size() == 3);
    it = db2.begin();
    assert(it->size == 9);
    assert(it->__private_data.block_index == 0);
    assert(it->type == mtv::element_type_empty);
    ++it;
    assert(it->size == 3);
    assert(it->__private_data.block_index == 1);
    assert(it->type == mtv::element_type_double);
    ++it;
    assert(it->size == 8);
    assert(it->__private_data.block_index == 2);
    assert(it->type == mtv::element_type_empty);
    ++it;
    assert(it == db2.end());
    assert(db2.get<double>(9) == 1.1);
    assert(db2.get<double>(10) == 1.2);
    assert(db2.get<double>(11) == 1.3);

    // Reset and start over.
    db1.clear();
    db1.resize(20);
    db2.clear();
    db2.resize(20);

    db1.set(8, 1.0);
    db1.set(9, 1.1);

    db2.set(10, 1.2);
    db2.set(11, 1.3);

    it = db1.transfer(9, 9, db2, 9);
    assert(it->__private_data.block_index == 2);
    assert(db1.block_size() == 3);
    assert(db1.get<double>(8) == 1.0);
    it = db1.begin();
    assert(it->size == 8);
    assert(it->type == mtv::element_type_empty);
    ++it;
    assert(it->size == 1);
    assert(it->type == mtv::element_type_double);
    ++it;
    assert(it->size == 11);
    assert(it->type == mtv::element_type_empty);
    ++it;
    assert(it == db1.end());

    // Reset and start over.
    db1.clear();
    db1.resize(8);
    db2 = mtv_type(8, true);

    db1.transfer(0, 4, db2, 1);

    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.is_empty(3));
    assert(db1.is_empty(4));
    assert(db1.is_empty(5));
    assert(db1.is_empty(6));
    assert(db1.is_empty(7));

    assert(db2.get<bool>(0) == true);
    assert(db2.is_empty(1));
    assert(db2.is_empty(2));
    assert(db2.is_empty(3));
    assert(db2.is_empty(4));
    assert(db2.is_empty(5));
    assert(db2.get<bool>(6) == true);
    assert(db2.get<bool>(7) == true);

    // Start over.
    db1 = mtv_type(8, int16_t(43));
    db2 = mtv_type(8);
    db1.transfer(3, 5, db2, 0);
    assert(db1.get<int16_t>(0) == 43);
    assert(db1.get<int16_t>(1) == 43);
    assert(db1.get<int16_t>(2) == 43);
    assert(db1.is_empty(3));
    assert(db1.is_empty(4));
    assert(db1.is_empty(5));
    assert(db1.get<int16_t>(6) == 43);
    assert(db1.get<int16_t>(7) == 43);

    assert(db2.get<int16_t>(0) == 43);
    assert(db2.get<int16_t>(1) == 43);
    assert(db2.get<int16_t>(2) == 43);
    assert(db2.is_empty(3));
    assert(db2.is_empty(4));
    assert(db2.is_empty(5));
    assert(db2.is_empty(6));
    assert(db2.is_empty(7));

    // Start over.
    db1 = mtv_type(5);
    db2 = mtv_type(5);
    db1.set<double>(3, 1.1);
    db1.set<double>(4, 1.2);
    db2.set<int32_t>(0, 1);
    db2.set<int32_t>(1, 1);

    db1.transfer(3, 4, db2, 0);

    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.is_empty(3));
    assert(db1.is_empty(4));

    assert(db2.get<double>(0) == 1.1);
    assert(db2.get<double>(1) == 1.2);
    assert(db2.is_empty(2));
    assert(db2.is_empty(3));
    assert(db2.is_empty(4));

    // Start over.
    db1 = mtv_type(5);
    db2 = mtv_type(5);
    db1.set<double>(3, 1.1);
    db1.set<int32_t>(4, 892);
    db2.set<int32_t>(0, 1);
    db2.set<int32_t>(1, 1);

    db1.transfer(3, 4, db2, 0);

    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.is_empty(3));
    assert(db1.is_empty(4));

    assert(db2.get<double>(0) == 1.1);
    assert(db2.get<int32_t>(1) == 892);
    assert(db2.is_empty(2));
    assert(db2.is_empty(3));
    assert(db2.is_empty(4));

    // Start over.
    db1 = mtv_type(5, false);
    db2 = mtv_type(5);
    db1.set<int32_t>(4, 234);
    db2.set<int8_t>(0, 32);
    db2.set<uint16_t>(1, 11);
    db2.set<uint16_t>(2, 22);
    db2.set<int8_t>(3, 56);

    db1.transfer(3, 4, db2, 1);

    assert(db1.get<bool>(0) == false);
    assert(db1.get<bool>(1) == false);
    assert(db1.get<bool>(2) == false);
    assert(db1.is_empty(3));
    assert(db1.is_empty(4));

    assert(db2.get<int8_t>(0) == 32);
    assert(db2.get<bool>(1) == false);
    assert(db2.get<int32_t>(2) == 234);
    assert(db2.get<int8_t>(3) == 56);
    assert(db2.is_empty(4));

    // Start over.
    db1 = mtv_type(5, false);
    db2 = mtv_type(6);
    db1.set<int32_t>(4, 234);
    db2.set<uint32_t>(0, 2345);

    db1.transfer(3, 4, db2, 4);

    assert(db1.get<bool>(0) == false);
    assert(db1.get<bool>(1) == false);
    assert(db1.get<bool>(2) == false);
    assert(db1.is_empty(3));
    assert(db1.is_empty(4));

    assert(db2.get<uint32_t>(0) == 2345);
    assert(db2.is_empty(1));
    assert(db2.is_empty(2));
    assert(db2.is_empty(3));
    assert(db2.get<bool>(4) == false);
    assert(db2.get<int32_t>(5) == 234);

    // Start over
    db1.clear();
    db2 = mtv_type(10);
    db1.push_back<int8_t>(5);
    db1.push_back<int16_t>(5);
    db1.push_back<int32_t>(5);
    db1.push_back<double>(5);

    db1.transfer(0, 3, db2, 6);

    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.is_empty(3));

    assert(db2.is_empty(0));
    assert(db2.is_empty(1));
    assert(db2.is_empty(2));
    assert(db2.is_empty(3));
    assert(db2.is_empty(4));
    assert(db2.is_empty(5));
    assert(db2.get<int8_t>(6) == 5);
    assert(db2.get<int16_t>(7) == 5);
    assert(db2.get<int32_t>(8) == 5);
    assert(db2.get<double>(9) == 5.0);

    // Start over.
    db1 = mtv_type(5, true);
    db2 = mtv_type(6);

    db1.transfer(0, 2, db2, 1);

    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.get<bool>(3) == true);
    assert(db1.get<bool>(4) == true);

    assert(db2.is_empty(0));
    assert(db2.get<bool>(1) == true);
    assert(db2.get<bool>(2) == true);
    assert(db2.get<bool>(3) == true);
    assert(db2.is_empty(4));
    assert(db2.is_empty(5));

    // Start over.
    db1 = mtv_type(5, int32_t(987));
    db1.set(3, 1.1);
    db1.set(4, 1.2);

    db2 = mtv_type(5);
    db2.set(0, true);
    db2.set(1, false);

    db1.transfer(0, 2, db2, 2);

    assert(db1.is_empty(0));
    assert(db1.is_empty(1));
    assert(db1.is_empty(2));
    assert(db1.get<double>(3) == 1.1);
    assert(db1.get<double>(4) == 1.2);

    assert(db2.get<bool>(0) == true);
    assert(db2.get<bool>(1) == false);
    assert(db2.get<int32_t>(2) == 987);
    assert(db2.get<int32_t>(3) == 987);
    assert(db2.get<int32_t>(4) == 987);
}

void mtv_test_push_back()
{
    stack_printer __stack_printer__(__FUNCTION__);

    mtv_type db;
    assert(db.size() == 0);
    assert(db.block_size() == 0);

    // Append an empty element into an empty container.
    mtv_type::iterator it = db.push_back_empty();
    assert(db.size() == 1);
    assert(db.block_size() == 1);
    assert(it->size == 1);
    assert(it->type == mtv::element_type_empty);
    assert(it->__private_data.block_index == 0);
    assert(it == db.begin());
    ++it;
    assert(it == db.end());

    // ... and again.
    it = db.push_back_empty();
    assert(db.size() == 2);
    assert(db.block_size() == 1);
    assert(it->size == 2);
    assert(it->type == mtv::element_type_empty);
    assert(it->__private_data.block_index == 0);
    assert(it == db.begin());
    ++it;
    assert(it == db.end());

    // Append non-empty this time.
    it = db.push_back(1.1);
    assert(db.size() == 3);
    assert(db.block_size() == 2);
    assert(it->size == 1);
    assert(it->type == mtv::element_type_double);
    assert(it->__private_data.block_index == 1);
    mtv_type::iterator check = it;
    --check;
    assert(check == db.begin());
    ++it;
    assert(it == db.end());

    // followed by an empty element again.
    it = db.push_back_empty();
    assert(db.size() == 4);
    assert(db.block_size() == 3);
    assert(it->size == 1);
    assert(it->type == mtv::element_type_empty);
    assert(it->__private_data.block_index == 2);
    check = it;
    --check;
    --check;
    assert(check == db.begin());
    ++it;
    assert(it == db.end());

    // Check the values.
    assert(db.is_empty(0));
    assert(db.is_empty(1));
    assert(db.get<double>(2) == 1.1);
    assert(db.is_empty(3));

    // Empty the container and push back a non-empty element.
    db.clear();
    it = db.push_back(string("push me"));
    assert(db.size() == 1);
    assert(db.block_size() == 1);
    assert(it->size == 1);
    assert(it->type == mtv::element_type_string);
    assert(it->__private_data.block_index == 0);
    assert(it == db.begin());
    ++it;
    assert(it == db.end());
    assert(db.get<string>(0) == "push me");

    // Push back a non-empty element of the same type.
    it = db.push_back(string("again"));
    assert(db.size() == 2);
    assert(db.block_size() == 1);
    assert(it->size == 2);
    assert(it->type == mtv::element_type_string);
    assert(it->__private_data.block_index == 0);
    assert(it == db.begin());
    ++it;
    assert(it == db.end());

    assert(db.get<string>(0) == "push me");
    assert(db.get<string>(1) == "again");

    // Push back another non-empty element of a different type.
    it = db.push_back(23.4);
    assert(db.size() == 3);
    assert(db.block_size() == 2);
    assert(it->size == 1);
    assert(it->type == mtv::element_type_double);
}

void mtv_test_capacity()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db(10, 1.1);
    assert(db.block_size() == 1);
    mtv_type::const_iterator it = db.begin();
    assert(it->type == mtv::element_type_double);
    size_t cap = mtv::double_element_block::capacity(*it->data);
    assert(cap == 10);

    db.set_empty(3, 3);
    assert(db.block_size() == 3);
    db.shrink_to_fit();
    it = db.begin();
    assert(it->type == mtv::element_type_double);
    cap = mtv::double_element_block::capacity(*it->data);
    assert(cap == 3);
}

struct mtv_test_position_type_end_position
{
    template<typename T>
    void run(T& db)
    {
        auto pos1 = db.position(9); // last valid position.
        pos1 = mtv_type::next_position(pos1);
        auto pos2 = db.position(10); // end position - one position past the last valid position
        assert(pos1 == pos2);

        // Move back from the end position by one. It should point to the last
        // valid position.
        pos2 = mtv_type::advance_position(pos2, -1);
        pos1 = db.position(9);
        assert(pos1 == pos2);

        // Try the variant of position() method that takes position hint as its first argument.
        pos1 = db.position(db.begin(), 10);
        pos1 = mtv_type::advance_position(pos1, -10);
        pos2 = db.position(0);
        assert(pos1 == pos2);

        // A position more than one past the last valid position is considered out-of-range.

        try
        {
            pos1 = db.position(11);
            assert(!"No exceptions thrown, but one was expected to be thrown.");
        }
        catch (const std::out_of_range&)
        {
            // good.
            cout << "Out of range exception was thrown as expected." << endl;
        }
        catch (...)
        {
            assert(!"An unexpected exception was thrown.");
        }

        // Try again with the variant that takes a position hint.

        try
        {
            pos1 = db.position(db.begin(), 11);
            assert(!"No exceptions thrown, but one was expected to be thrown.");
        }
        catch (const std::out_of_range&)
        {
            // good.
            cout << "Out of range exception was thrown as expected." << endl;
        }
        catch (...)
        {
            assert(!"An unexpected exception was thrown.");
        }
    }

    mtv_test_position_type_end_position()
    {
        stack_printer __stack_printer__(__FUNCTION__);
        mtv_type db(10);
        const mtv_type& cdb = db;
        cout << "* position_type" << endl;
        run(db);
        cout << "* const_position_type" << endl;
        run(cdb);
    }
};

/**
 * Ensure that we test block adjustment case with adjusted block size
 * greater than 8.
 */
void mtv_test_block_pos_adjustments()
{
    stack_printer __stack_printer__(__FUNCTION__);

    mtv_type db(1);

    for (int i = 0; i < 20; ++i)
    {
        std::vector<double> vs(2, 1.1);
        db.insert(0, vs.begin(), vs.end());

        std::vector<int16_t> is(2, 34);
        db.insert(0, is.begin(), is.end());
    }

    assert(db.size() == 81);
    assert(db.block_size() == 41);
    assert(db.check_block_integrity());
}

}

int main (int argc, char **argv)
{
    try
    {
        mtv_test_types();
        mtv_test_construction();
        mtv_test_basic();
        mtv_test_basic_numeric();
        mtv_test_empty_cells();
        mtv_test_misc_swap();
        mtv_test_misc_equality();
        mtv_test_misc_clone();
        mtv_test_misc_resize();
        mtv_test_erase();
        mtv_test_insert_empty();
        mtv_test_set_cells();
        mtv_test_insert_cells();
        mtv_test_iterators();
        mtv_test_iterators_element_block();
        mtv_test_iterators_mutable_element_block();
        mtv_test_iterators_private_data();
        mtv_test_iterators_set();
        mtv_test_iterators_set_2();
        mtv_test_iterators_insert();
        mtv_test_iterators_insert_empty();
        mtv_test_iterators_set_empty();
        mtv_test_hints_set();
        mtv_test_hints_set_cells();
        mtv_test_hints_insert_cells();
        mtv_test_hints_set_empty();
        mtv_test_hints_insert_empty();
        mtv_test_position();
        mtv_test_position_next();
        mtv_test_position_advance();
        mtv_test_swap_range();
        mtv_test_value_type();
        mtv_test_block_identifier();
        mtv_test_transfer();
        mtv_test_push_back();
        mtv_test_capacity();
        mtv_test_position_type_end_position();
        mtv_test_block_pos_adjustments();
    }
    catch (const std::exception& e)
    {
        cout << "Test failed: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}
