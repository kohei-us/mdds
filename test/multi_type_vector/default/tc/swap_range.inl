/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2021 Kohei Yoshida
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
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->size == 3);
    it = db2.begin();
    assert(it->type == mdds::mtv::element_type_double);
    assert(it->size == 3);

    int32_t int_val = 2;
    int16_t short_val = 5;
    db1 = mtv_type(5, int_val);
    db2 = mtv_type(5, short_val);
    db1.set(1, 2.3);
    db1.set(2, 2.4);
    db2.set(3, std::string("abc"));
    db2.set(4, std::string("def"));
    db1.swap(1, 2, db2, 3); // Swap 1-2 of source with 3-4 of destination.
    assert(db1.get<int32_t>(0) == int_val);
    assert(db1.get<std::string>(1) == "abc");
    assert(db1.get<std::string>(2) == "def");
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
    db2.set(0, std::string("A"));
    db2.set(1, std::string("B"));
    db1.set(3, 1.1);
    db1.set(4, 1.2);
    db1.swap(3, 4, db2, 0);
    assert(db1.get<int32_t>(0) == int_val);
    assert(db1.get<int32_t>(1) == int_val);
    assert(db1.get<int32_t>(2) == int_val);
    assert(db1.get<std::string>(3) == "A");
    assert(db1.get<std::string>(4) == "B");
    assert(db1.block_size() == 2);

    assert(db2.get<double>(0) == 1.1);
    assert(db2.get<double>(1) == 1.2);
    assert(db2.get<double>(2) == 12.3);
    assert(db2.get<double>(3) == 12.3);
    assert(db2.get<double>(4) == 12.3);
    assert(db2.block_size() == 1);

    // Merge with the previous block in the destination.
    db1 = mtv_type(5, int_val);
    db1.set(2, std::string("A"));
    db1.set(3, std::string("B"));

    db2 = mtv_type(5, std::string("default"));
    db2.set(3, short_val);
    db2.set(4, short_val);

    db1.swap(2, 3, db2, 3);
    assert(db1.get<int32_t>(0) == int_val);
    assert(db1.get<int32_t>(1) == int_val);
    assert(db1.get<int16_t>(2) == short_val);
    assert(db1.get<int16_t>(3) == short_val);
    assert(db1.get<int32_t>(4) == int_val);
    assert(db1.block_size() == 3);

    assert(db2.get<std::string>(0) == "default");
    assert(db2.get<std::string>(1) == "default");
    assert(db2.get<std::string>(2) == "default");
    assert(db2.get<std::string>(3) == "A");
    assert(db2.get<std::string>(4) == "B");
    assert(db2.block_size() == 1);

    // Merge with both the previous and next blocks in the destination.
    db1 = mtv_type(5, int_val);
    db1.set(2, std::string("C"));
    db1.set(3, std::string("D"));

    db2 = mtv_type(6, std::string("default"));
    db2.set(3, short_val);
    db2.set(4, short_val);

    db1.swap(2, 3, db2, 3);
    assert(db1.get<int32_t>(0) == int_val);
    assert(db1.get<int32_t>(1) == int_val);
    assert(db1.get<int16_t>(2) == short_val);
    assert(db1.get<int16_t>(3) == short_val);
    assert(db1.get<int32_t>(4) == int_val);
    assert(db1.block_size() == 3);

    assert(db2.get<std::string>(0) == "default");
    assert(db2.get<std::string>(1) == "default");
    assert(db2.get<std::string>(2) == "default");
    assert(db2.get<std::string>(3) == "C");
    assert(db2.get<std::string>(4) == "D");
    assert(db2.get<std::string>(5) == "default");
    assert(db2.block_size() == 1);

    // Set the new elements to the top of a block in the destination.
    db1 = mtv_type(5, int_val);
    db1.set(3, std::string("E"));
    db1.set(4, std::string("F"));
    db2 = mtv_type(5, short_val);
    db1.swap(3, 4, db2, 0);
    assert(db1.get<int32_t>(0) == int_val);
    assert(db1.get<int32_t>(1) == int_val);
    assert(db1.get<int32_t>(2) == int_val);
    assert(db1.get<int16_t>(3) == short_val);
    assert(db1.get<int16_t>(4) == short_val);
    assert(db1.block_size() == 2);
    assert(db2.get<std::string>(0) == "E");
    assert(db2.get<std::string>(1) == "F");
    assert(db2.get<int16_t>(2) == short_val);
    assert(db2.get<int16_t>(3) == short_val);
    assert(db2.get<int16_t>(4) == short_val);
    assert(db2.block_size() == 2);

    // Do the same as before, but merge with the previous block.
    db1 = mtv_type(5, int_val);
    db1.set(3, std::string("G"));
    db1.set(4, std::string("H"));
    db2 = mtv_type(5, short_val);
    db2.set(0, std::string("F"));
    db1.swap(3, 4, db2, 1);
    assert(db1.get<int32_t>(0) == int_val);
    assert(db1.get<int32_t>(1) == int_val);
    assert(db1.get<int32_t>(2) == int_val);
    assert(db1.get<int16_t>(3) == short_val);
    assert(db1.get<int16_t>(4) == short_val);
    assert(db1.block_size() == 2);
    assert(db2.get<std::string>(0) == "F");
    assert(db2.get<std::string>(1) == "G");
    assert(db2.get<std::string>(2) == "H");
    assert(db2.get<int16_t>(3) == short_val);
    assert(db2.get<int16_t>(4) == short_val);
    assert(db2.block_size() == 2);

    // Set the new element to the middle of a destination block.
    db1 = mtv_type(5, int_val);
    db1.set(3, std::string("I"));
    db1.set(4, std::string("J"));
    db2 = mtv_type(5, short_val);
    db2.set(0, std::string("top"));
    db1.swap(3, 4, db2, 2);
    assert(db1.get<int32_t>(0) == int_val);
    assert(db1.get<int32_t>(1) == int_val);
    assert(db1.get<int32_t>(2) == int_val);
    assert(db1.get<int16_t>(3) == short_val);
    assert(db1.get<int16_t>(4) == short_val);
    assert(db1.block_size() == 2);

    assert(db2.get<std::string>(0) == "top");
    assert(db2.get<int16_t>(1) == short_val);
    assert(db2.get<std::string>(2) == "I");
    assert(db2.get<std::string>(3) == "J");
    assert(db2.get<int16_t>(4) == short_val);
    assert(db2.block_size() == 4);

    // Set the new element to the lower part of a destination block.
    db1 = mtv_type(5, int_val);
    db1.set(0, std::string("A1"));
    db1.set(1, std::string("A2"));
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
    assert(db2.get<std::string>(3) == "A1");
    assert(db2.get<std::string>(4) == "A2");
    assert(db2.block_size() == 2);

    // Same as above, except that the new element will be merged with the next
    // block in the destination.
    db1 = mtv_type(5, int_val);
    db1.set(0, std::string("A1"));
    db1.set(1, std::string("A2"));
    db2 = mtv_type(6, short_val);
    db2.set(5, std::string("A3"));
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
    assert(db2.get<std::string>(3) == "A1");
    assert(db2.get<std::string>(4) == "A2");
    assert(db2.get<std::string>(5) == "A3");
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
    db1.set(0, std::string("A"));
    db2 = mtv_type(5, short_val);
    db2.set(0, std::string("B"));
    db2.set(1, std::string("C"));
    db1.swap(1, 2, db2, 0);

    assert(db1.get<std::string>(0) == "A");
    assert(db1.get<std::string>(1) == "B");
    assert(db1.get<std::string>(2) == "C");
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
    db1.set<int32_t>(2, int_val + 1);
    db2 = mtv_type(1, int_val);
    db1.swap(1, 1, db2, 0);
    assert(db1.get<bool>(0) == true);
    assert(db1.get<int32_t>(1) == int_val);
    assert(db1.get<int32_t>(2) == int_val + 1);
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
    db2.set(3, std::string("abc"));
    db2.set(4, std::string("def"));
    db1.swap(0, 3, db2, 1);
    assert(db1.get<double>(0) == 2.1);
    assert(db1.get<double>(1) == 3.1);
    assert(db1.get<std::string>(2) == "abc");
    assert(db1.get<std::string>(3) == "def");
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
    db2.set(3, std::string("abc"));
    db2.set(4, std::string("def"));
    db1.swap(0, 3, db2, 1);
    assert(db1.get<double>(0) == 2.1);
    assert(db1.get<double>(1) == 3.1);
    assert(db1.get<std::string>(2) == "abc");
    assert(db1.get<std::string>(3) == "def");
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
    db2.set(0, std::string("A"));
    db2.set<int8_t>(1, 'A');
    db1.swap(0, 1, db2, 0);
    assert(db1.get<std::string>(0) == "A");
    assert(db1.get<int8_t>(1) == 'A');
    assert(db2.get<int16_t>(0) == short_val);
    assert(db2.get<int16_t>(1) == short_val);

    // Another scenario.
    db1 = mtv_type(2, 3.14);
    db2 = mtv_type(3);
    db2.set(0, std::string("abc"));
    db2.set<uint8_t>(1, 'z');
    db2.set<uint8_t>(2, 'y');
    db1.swap(0, 1, db2, 0);
    assert(db1.get<std::string>(0) == "abc");
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
    db1 = mtv_type(3, std::string("test"));
    db2 = mtv_type(2);
    db2.set(0, -99.1);
    db2.set(1, std::string("foo"));
    db1.swap(1, 2, db2, 0);
    assert(db1.get<std::string>(0) == "test");
    assert(db1.get<double>(1) == -99.1);
    assert(db1.get<std::string>(2) == "foo");
    assert(db2.get<std::string>(0) == "test");
    assert(db2.get<std::string>(1) == "test");

    // The source range is in the middle of a block.
    db1 = mtv_type(8);
    for (int32_t i = 0; i < 8; ++i)
        db1.set<int32_t>(i, i + 2);
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
    db1.set(1, std::string("123"));
    db2 = mtv_type(10, short_val);
    db1.swap(0, 1, db2, 4);
    assert(db1.get<int16_t>(0) == short_val);
    assert(db1.get<int16_t>(1) == short_val);

    assert(db2.get<int16_t>(0) == short_val);
    assert(db2.get<int16_t>(1) == short_val);
    assert(db2.get<int16_t>(2) == short_val);
    assert(db2.get<int16_t>(3) == short_val);
    assert(db2.get<double>(4) == 1.2);
    assert(db2.get<std::string>(5) == "123");
    assert(db2.get<int16_t>(6) == short_val);
    assert(db2.get<int16_t>(7) == short_val);
    assert(db2.get<int16_t>(8) == short_val);
    assert(db2.get<int16_t>(9) == short_val);

    // Multi-to-multi block swapping. Very simple case.
    db1 = mtv_type(2);
    db1.set(0, 2.1);
    db1.set(1, std::string("test"));
    db2 = mtv_type(2);
    db2.set(0, int_val);
    db2.set(1, short_val);
    db1.swap(0, 1, db2, 0);

    assert(db1.get<int32_t>(0) == int_val);
    assert(db1.get<int16_t>(1) == short_val);
    assert(db2.get<double>(0) == 2.1);
    assert(db2.get<std::string>(1) == "test");

    // More complex case.
    db1 = mtv_type(10);
    db1.set<int32_t>(0, 2);
    db1.set<int32_t>(1, 3);
    db1.set<int32_t>(2, 4);
    db1.set<std::string>(3, "A");
    db1.set<std::string>(4, "B");
    db1.set<std::string>(5, "C");
    // Leave some empty range.
    db2 = mtv_type(10);
    for (int32_t i = 0; i < 10; ++i)
        db2.set<int32_t>(i, 10 + i);
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
    assert(db2.get<std::string>(4) == "A");
    assert(db2.get<std::string>(5) == "B");
    assert(db2.get<std::string>(6) == "C");
    assert(db2.is_empty(7));
    assert(db2.is_empty(8));
    assert(db2.get<int32_t>(9) == 19);
    assert(db2.block_size() == 4);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
