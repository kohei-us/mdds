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

/**
 * Test the variant of set() method that takes iterators.
 */
void mtv_test_iterators_set()
{
    stack_printer __stack_printer__(__FUNCTION__);

    // single element only
    mtv_type db(1);
    mtv_type::iterator it = db.set(0, 1.1);
    assert(it == db.begin());

    // Set value to the top of the only block.
    db.clear();
    db.resize(3);
    it = db.set(0, 1.2);
    assert(it == db.begin());

    // Set value to the bottom of the only block.
    db.clear();
    db.resize(3);
    it = db.set(2, 1.3);
    mtv_type::iterator check = db.begin();
    ++check;
    assert(it == check);
    check = db.end();
    --check;
    assert(it == check);

    // Set value to the middle of the only block.
    db.clear();
    db.resize(3);
    it = db.set(1, 1.4);
    check = db.begin();
    ++check;
    assert(it == check);
    check = db.end();
    std::advance(check, -2);
    assert(it == check);
    assert(it->position == 1);
    assert(it->__private_data.block_index == 1);

    // Set value to the top empty block of size 1 followed by a non-empty block.
    db.clear();
    db.resize(2);
    db.set(1, 2.1);
    it = db.set(0, 2.2); // same type as that of the following block.
    assert(it == db.begin());
    assert(it->size == 2);
    assert(it->position == 0);
    assert(it->__private_data.block_index == 0);
    db.set_empty(0, 0);
    it = db.set(0, true); // different type from that of the following block.
    assert(it == db.begin());
    assert(it->size == 1);
    assert(it->position == 0);
    assert(it->__private_data.block_index == 0);

    // Set value to the top of the top empty block (not size 1) followed by a non-empty block.
    db.clear();
    db.resize(3);
    db.set(2, true);
    it = db.set(0, 5.1);
    assert(it == db.begin());

    // Set value to the bottom of the top empty block (not size 1) followed by a non-empty block.
    db.clear();
    db.resize(3);
    db.set(2, 6.1);
    it = db.set(1, 6.2); // same type as that of the following block.
    check = db.begin();
    ++check;
    assert(it == check);
    check = db.end();
    --check;
    assert(it == check);
    assert(it->size == 2);
    assert(it->position == 1);
    assert(it->__private_data.block_index == 1);
    db.set_empty(0, 1);
    it = db.set(1, true); // different type from that of the following block.
    check = db.begin();
    ++check;
    assert(it == check);
    check = db.end();
    std::advance(check, -2);
    assert(it == check);
    assert(it->size == 1);
    assert(it->position == 1);
    assert(it->__private_data.block_index == 1);

    // Set value to the middle of the top empty block (not size 1) followed by a non-empty block.
    db.clear();
    db.resize(6);
    db.set(5, 1.1);
    it = db.set(3, 1.2);
    check = db.begin();
    ++check;
    assert(it == check);
    assert(it->size == 1);
    assert(it->position == 3);
    assert(it->__private_data.block_index == 1);

    // Set value to an empty block of size 1 immediately below a non-empty block.
    db.clear();
    db.resize(2);
    db.set(0, true);
    it = db.set(1, false); // same type as that of the previous block.
    assert(it == db.begin());

    // Set value to an empty block of size 1 between non-empty blocks of the same type.
    db = mtv_type(3, true);
    db.set_empty(1, 1);
    it = db.set(1, false);
    assert(it == db.begin());
    assert(it->size == 3);
    ++it;
    assert(it == db.end());

    // Set value to an empty block of size 1 between non-empty blocks. The
    // previous block is of the same type as that of the inserted value.
    db = mtv_type(3, 1.1);
    db.set_empty(0, 1);
    db.set(0, true);
    it = db.set(1, false);
    assert(it == db.begin());
    assert(it->size == 2);
    std::advance(it, 2);
    assert(it == db.end());

    // Set value to the top of an empty block (not of size 1) following a
    // non-empty block of the same type.
    db.clear();
    db.resize(3);
    db.set(0, true);
    it = db.set(1, false);
    assert(it == db.begin());
    assert(it->size == 2);

    // Set value to an empty block of size 1, following a non-empty block of different type.
    db = mtv_type(2);
    db.set(0, true);
    it = db.set(1, 1.1);
    ++it;
    assert(it == db.end());

    // Set value to an empty block of size 1, following a non-empty block of
    // different type and followed by a non-empty block of different type.
    db = mtv_type(3, true);
    db.set_empty(1, 1);
    it = db.set(1, 2.1);
    std::advance(it, 2);
    assert(it == db.end());

    // Set value to an empty block of size 1, following a non-empty block of
    // different type but followed by a non-empty block of the same type.
    db.clear();
    db.resize(3);
    db.set(0, true);
    it = db.set(2, 2.1);
    ++it;
    assert(it == db.end());
    it = db.set(1, 2.2); // same type as that of the following block.
    assert(it->size == 2);
    ++it;
    assert(it == db.end());

    // Set value to the top of an empty block (not of size 1) between
    // non-empty blocks. The previous block is of different type.
    db.clear();
    db.resize(4);
    db.set(0, true);
    db.set(3, false);
    it = db.set(1, 2.2);
    check = db.begin();
    ++check;
    assert(it == check);
    assert(it->size == 1);
    assert(it->position == 1);

    // Set value to the bottom of an empty block (not of size 1) between
    // non-empty blocks.
    db = mtv_type(7, false);
    db.set_empty(2, 4);
    it = db.set(4, true); // Same type as that of the following block.
    assert(it->size == 3);
    assert(it->position == 4);
    assert(it->__private_data.block_index == 2);
    ++it;
    assert(it == db.end());

    db.set_empty(2, 4);
    it = db.set(4, 1.1); // Different type from that of the following block.
    assert(it->size == 1);
    assert(it->position == 4);
    assert(it->__private_data.block_index == 2);
    std::advance(it, 2);
    assert(it == db.end());

    // Set value to the middle of an empty block between non-empty blocks.
    db = mtv_type(12, true);
    db.set_empty(3, 7);
    it = db.set(5, 4.3);
    assert(it->size == 1);
    check = db.begin();
    std::advance(check, 2);
    assert(check == it);
    ++it;
    assert(it->size == 2);
    assert(it->type == mdds::mtv::element_type_empty);
    ++it;
    assert(it->size == 4);
    assert(it->type == mdds::mtv::element_type_boolean);
    ++it;
    assert(it == db.end());

    // Set value to existing block of the same type.
    it = db.set(5, 4.5);
    check = db.begin();
    std::advance(check, 2);
    assert(it == check);
    assert(it->size == 1);
    std::advance(it, 3);
    assert(it == db.end());

    // Set value to the top of an existing topmost block of different type.
    db = mtv_type(5, true);
    it = db.set(0, 1.1);
    assert(it == db.begin());
    std::advance(it, 2);
    assert(it == db.end());

    // Set value to the top of an existing block of different type.  The block
    // is below an empty block.
    db = mtv_type(10, true);
    db.set_empty(0, 4);
    it = db.set(5, 2.1);
    check = db.begin();
    ++check;
    assert(it == check);
    std::advance(it, 2);
    assert(it == db.end());

    // Set value to the top of an existing block of different type.  The block
    // is below a non-empty block.
    db = mtv_type(10, true);
    std::vector<double> doubles(3, 1.1);
    db.set(2, doubles.begin(), doubles.end()); // set double's to 2 thru 4.
    it = db.set(5, 2.1); // append to the previous block.
    check = db.begin();
    ++check;
    assert(it == check);
    ++it;
    assert(it->size == 4);
    assert(it->type == mdds::mtv::element_type_boolean);
    ++it;
    assert(it == db.end());

    db = mtv_type(10, true);
    db.set(2, doubles.begin(), doubles.end()); // set double's to 2 thru 4.
    it = db.set(5, std::string("foo")); // type different from that of the previous block.
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_string);
    check = db.begin();
    std::advance(check, 2);
    assert(it == check);
    std::advance(it, 2);
    assert(it == db.end());

    // Set value to the middle of an existing non-empty block.
    db = mtv_type(10, true);
    doubles.clear();
    doubles.resize(3, 2.3);
    db.set(0, doubles.begin(), doubles.end()); // set double's to 0 thru 2.
    it = db.set(6, std::string("text"));
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_string);
    assert(it->position == 6);
    check = db.begin();
    std::advance(check, 2);
    assert(it == check);
    ++it;
    assert(it->size == 3);
    assert(it->type == mdds::mtv::element_type_boolean);
    ++it;
    assert(it == db.end());

    // Set value to the bottom of the only block that's non-empty.
    db = mtv_type(10, false);
    it = db.set(9, 2.1);
    check = db.begin();
    ++check;
    assert(it == check);
    ++it;
    assert(it == db.end());

    // Set value to the bottom of the topmost non-empty block which is
    // followed by an empty block.
    db = mtv_type(10, false);
    db.set_empty(5, 9);
    it = db.set(4, 1.1);
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_double);
    assert(it->position == 4);
    check = db.begin();
    ++check;
    assert(it == check);
    ++it;
    assert(it->size == 5);
    assert(it->type == mdds::mtv::element_type_empty);
    ++it;
    assert(it == db.end());

    // This time the following block is not empty but is of different type
    // than that of the value being set.
    db = mtv_type(10, false);
    doubles.clear();
    doubles.resize(5, 2.1);
    db.set(5, doubles.begin(), doubles.end()); // numeric at 5 thru 9
    it = db.set(4, std::string("foo"));
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_string);
    check = db.begin();
    ++check;
    assert(it == check);
    ++it;
    assert(it->size == 5);
    assert(it->type == mdds::mtv::element_type_double);
    ++it;
    assert(it == db.end());

    // Same as before, but the value being set is of the same type as that of
    // the following block.
    db = mtv_type(10, false);
    doubles.clear();
    doubles.resize(5, 2.1);
    db.set(5, doubles.begin(), doubles.end()); // numeric at 5 thru 9
    it = db.set(4, 4.5); // same type as the following block.
    assert(it->size == 6);
    assert(it->type == mdds::mtv::element_type_double);
    ++it;
    assert(it == db.end());

    // Set value to the bottom of the last non-empty block.
    db = mtv_type(10, false);
    doubles.clear();
    doubles.resize(4, 3.1);
    db.set(6, doubles.begin(), doubles.end()); // numeric at 6 thru 9
    it = db.set(9, true);
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_boolean);
    ++it;
    assert(it == db.end());

    // Set value to the bottom of an non-empty block followed by an empty block.
    db = mtv_type(10, false);
    doubles.clear();
    doubles.resize(3, 3.3);
    db.set(2, doubles.begin(), doubles.end()); // numeric at 2 thru 4.
    db.set_empty(5, 9);
    it = db.set(4, std::string("foo"));
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_string);
    assert(it->position == 4);
    assert(it->__private_data.block_index == 2);
    ++it;
    assert(it->size == 5);
    assert(it->type == mdds::mtv::element_type_empty);
    ++it;
    assert(it == db.end());

    // Same as before, except the following block isn't empty but of different type.
    db = mtv_type(10, false);
    db.set(4, doubles.begin(), doubles.end()); // numeric at 4 thru 6.
    it = db.set(6, std::string("foo")); // 7 thru 9 is boolean.
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_string);
    assert(it->position == 6);
    assert(it->__private_data.block_index == 2);
    ++it;
    assert(it->size == 3);
    assert(it->type == mdds::mtv::element_type_boolean);
    assert(it->position == 7);
    ++it;
    assert(it == db.end());

    // Same as before, except the following block is now of the same type.
    db = mtv_type(10, false);
    db.set(4, doubles.begin(), doubles.end()); // numeric at 4 thru 6.
    it = db.set(6, true); // 7 thru 9 is boolean.

    // Set value to the only block (non-empty) of size 1.
    db = mtv_type(1, true);
    it = db.set(0, 1.1);
    assert(it == db.begin());
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_double);

    // Set value to the topmost non-empty block of size 1, followed by an empty block.
    db.resize(5);
    it = db.set(0, std::string("foo"));
    assert(it == db.begin());
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_string);
    ++it;
    assert(it->size == 4);
    assert(it->type == mdds::mtv::element_type_empty);
    ++it;
    assert(it == db.end());

    // Set value to the topmost non-empty block of size 1, followed by a non-empty block.
    db = mtv_type(5, true);
    db.set(0, 1.1);
    it = db.set(0, std::string("foo"));
    assert(it == db.begin());
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_string);
    ++it;
    assert(it->size == 4);
    assert(it->type == mdds::mtv::element_type_boolean);
    ++it;
    assert(it == db.end());

    // This time set value whose type is the same as that of the following block.
    it = db.set(0, false);
    assert(it == db.begin());
    assert(it->size == 5);
    assert(it->type == mdds::mtv::element_type_boolean);
    ++it;
    assert(it == db.end());

    // Set value to the topmost non-empty block of size 1, preceded by an empty block.
    db = mtv_type(5);
    db.set(4, true);
    it = db.set(4, 1.2);
    check = db.begin();
    ++check;
    assert(it == check);
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_double);
    ++it;
    assert(it == db.end());

    // This time the preceding block is not empty, but of different type.
    db = mtv_type(5, false);
    db.set(0, std::string("baa"));
    db.set(4, std::string("foo"));
    it = db.set(4, 1.2);
    check = db.begin();
    std::advance(check, 2);
    assert(it == check);
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_double);
    ++it;
    assert(it == db.end());

    it = db.set(4, true); // Now set value whose type is the same as that of the preceding block.
    check = db.end();
    --check;
    assert(it == check);
    assert(it->size == 4);
    assert(it->type == mdds::mtv::element_type_boolean);
    --it;
    assert(it == db.begin());

    // Set value to a non-empty block of size 1 that lies between existing blocks.
    db = mtv_type(10);
    db.set(7, true);
    it = db.set(7, 1.1); // Both preceding and following blocks are empty.
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_double);
    check = db.begin();
    ++check;
    assert(it == check);
    ++it;
    assert(it->size == 2);
    assert(it->type == mdds::mtv::element_type_empty);
    ++it;
    assert(it == db.end());

    db = mtv_type(10, true);
    doubles.clear();
    doubles.resize(8, 2.1);
    db.set(2, doubles.begin(), doubles.end()); // Set 2 thru 9 numeric.
    db.set(6, false);
    it = db.set(6, std::string("foo")); // Both preceding and following blocks are non-empty.
    check = db.end();
    std::advance(check, -2);
    assert(it == check);
    --it;
    assert(it->size == 4);
    assert(it->type == mdds::mtv::element_type_double);
    assert(it->position == 2);
    --it;
    assert(it->size == 2);
    assert(it->type == mdds::mtv::element_type_boolean);
    assert(it->position == 0);
    assert(it == db.begin());

    it = db.set(6, 4.5); // Same type as those of the preceding and following blocks.
    assert(it->size == 8);
    assert(it->type == mdds::mtv::element_type_double);
    assert(it->position == 2);
    assert(it->__private_data.block_index == 1);
    check = db.begin();
    ++check;
    assert(it == check);
    ++it;
    assert(it == db.end());

    db = mtv_type(10, true);
    db.set(4, static_cast<int32_t>(34));
    doubles.resize(5, 2.3);
    db.set(5, doubles.begin(), doubles.end());
    it = db.set(4, false); // Same type as that of the preceding block.
    assert(it == db.begin());
    assert(it->size == 5);
    assert(it->type == mdds::mtv::element_type_boolean);
    std::advance(it, 2);
    assert(it == db.end());

    db.set(4, static_cast<int32_t>(35)); // Reset to previous state.
    it = db.set(4, 4.5); // Same type as that of the following block.
    assert(it->size == 6);
    assert(it->type == mdds::mtv::element_type_double);
    ++it;
    assert(it == db.end());

    db.set(4, static_cast<int32_t>(36)); // Reset again.
    it = db.set(4, static_cast<int16_t>(28)); // Different type from either of the blocks.
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_int16);
    assert(it->position == 4);
    assert(it->__private_data.block_index == 1);
    std::advance(it, 2);
    assert(it == db.end());

    // Preceding block is empty, and the following block is non-empty.
    db = mtv_type(10);
    doubles.resize(3, 1.1);
    db.set(7, doubles.begin(), doubles.end()); // 7 thru 9 to be numeric.
    db.set(6, static_cast<int32_t>(23));
    it = db.set(6, std::string("foo")); // Type different from that of the following block.
    check = db.begin();
    ++check;
    assert(it == check);
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_string);
    assert(it->position == 6);
    ++it;
    assert(it->size == 3);
    assert(it->type == mdds::mtv::element_type_double);
    assert(it->position == 7);
    ++it;
    assert(it == db.end());

    db.set(6, static_cast<int32_t>(24)); // Reset.
    it = db.set(6, 4.5); // This time the same type as that of the following block.
    check = db.begin();
    ++check;
    assert(it == check);
    assert(it->size == 4);
    assert(it->type == mdds::mtv::element_type_double);
    ++it;
    assert(it == db.end());

    // Now, the preceding block is not empty while the following block is.
    db = mtv_type(10, static_cast<uint16_t>(10));
    db.set_empty(4, 6);
    db.set(3, 1.2);
    it = db.set(3, static_cast<uint16_t>(11)); // Same as the previous block.
    assert(it == db.begin());
    assert(it->size == 4);
    assert(it->type == mdds::mtv::element_type_uint16);
    std::advance(it, 3);
    assert(it == db.end());

    db.set(3, 1.3); // Reset
    it = db.set(3, std::string("foo")); // This time, different from the previous block.
    check = db.begin();
    ++check;
    assert(it == check);
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_string);
    std::advance(it, 3);
    assert(it == db.end());
}

/**
 * Test the variant of set() method that takes iterators (part 2).
 */
void mtv_test_iterators_set_2()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type::iterator it, check;
    std::vector<double> doubles(3, 1.1);
    std::deque<bool> bools;
    std::vector<std::string> strings;

    // simple overwrite.
    mtv_type db(10, 2.3);
    db.set(0, true);
    db.set(1, std::string("foo"));
    it = db.set(2, doubles.begin(), doubles.end());
    check = db.begin();
    std::advance(check, 2);
    assert(it == check);
    ++it;
    assert(it == db.end());

    // Insert and merge with previous block.
    db = mtv_type(10, true);
    db.set(5, 1.1);
    db.set(6, 1.2);
    db.set(7, 1.3);
    db.set(8, std::string("foo"));
    bools.resize(3, false);
    it = db.set(5, bools.begin(), bools.end());
    assert(it == db.begin());
    assert(it->size == 8);
    assert(it->type == mdds::mtv::element_type_boolean);
    std::advance(it, 3);
    assert(it == db.end());

    // Insert and merge with previous and next blocks.
    db = mtv_type(10, true);
    db.set(0, std::string("foo"));
    db.set(5, 1.1);
    db.set(6, 1.2);
    db.set(7, 1.3);
    it = db.set(5, bools.begin(), bools.end());
    assert(db.block_size() == 2);
    check = db.begin();
    ++check;
    assert(it == check);
    assert(it->size == 9);
    assert(it->type == mdds::mtv::element_type_boolean);
    ++it;
    assert(it == db.end());

    // Insert and merge with next block only.
    db = mtv_type(10); // start empty.
    db.set(4, true);
    db.set(5, true);
    db.set(6, true);
    db.set(7, 1.1);
    db.set(8, 1.2);
    db.set(9, 1.3);
    doubles.resize(3, 2.2);
    it = db.set(4, doubles.begin(), doubles.end());
    check = db.begin();
    ++check;
    assert(it == check);
    assert(it->size == 6);
    assert(it->type == mdds::mtv::element_type_double);
    ++it;
    assert(it == db.end());

    // Replace the upper part of a block and merge with previous block.
    db = mtv_type(10, false);
    db.set(3, 1.2);
    db.set(4, 1.3);
    db.set(5, 1.4);
    db.set(6, 1.5);
    db.set(7, 1.6);
    bools.resize(3, true);
    it = db.set(3, bools.begin(), bools.end());
    assert(it == db.begin());
    assert(it->size == 6);
    assert(it->type == mdds::mtv::element_type_boolean);
    std::advance(it, 3);
    assert(it == db.end());

    // Replace the upper part of a block but don't merge with previous block.
    db = mtv_type(10, false);
    db.set(3, std::string("A"));
    db.set(4, std::string("B"));
    db.set(5, std::string("C"));
    db.set(6, std::string("D"));
    db.set(7, std::string("E"));
    doubles.resize(3, 1.1);
    it = db.set(3, doubles.begin(), doubles.end());
    check = db.begin();
    ++check;
    assert(it == check);
    assert(it->size == 3);
    assert(it->type == mdds::mtv::element_type_double);
    ++it;
    assert(it->size == 2);
    assert(it->type == mdds::mtv::element_type_string);
    ++it;
    assert(it->size == 2);
    assert(it->type == mdds::mtv::element_type_boolean);
    ++it;
    assert(it == db.end());

    // Overwrite the lower part of a block and merge it with the next block.
    db = mtv_type(10, false);
    db.set(0, 2.2);
    db.set(4, 1.1);
    db.set(5, 1.2);
    db.set(6, 1.3);
    assert(db.block_size() == 4);
    bools.resize(2, true);
    it = db.set(5, bools.begin(), bools.end()); // 5 to 6
    check = db.begin();
    std::advance(check, 3);
    assert(it == check);
    assert(it->size == 5);
    assert(it->type == mdds::mtv::element_type_boolean);
    ++it;
    assert(it == db.end());

    // Overwrite the lower part of a block but don't merge it with the next block.
    db = mtv_type(10, std::string("boo"));
    db.set(0, 1.1);
    db.set(5, true);
    db.set(6, true);
    db.set(7, true);
    doubles.resize(2, 2.2);
    it = db.set(6, doubles.begin(), doubles.end());
    check = db.begin();
    std::advance(check, 3);
    assert(it == check);
    assert(it->size == 2);
    assert(it->type == mdds::mtv::element_type_double);
    std::advance(it, 2);
    assert(it == db.end());

    // Overwrite the lower part of the last block.
    db = mtv_type(10, std::string("boo"));
    db.set(0, 1.1);
    doubles.resize(3, 2.2);
    it = db.set(7, doubles.begin(), doubles.end());
    check = db.begin();
    std::advance(check, 2);
    assert(it == check);
    ++it;
    assert(it->size == 3);
    assert(it->type == mdds::mtv::element_type_double);
    assert(it == db.end());

    // Overwrite the middle part of a block.
    db = mtv_type(10);
    bools.resize(5, true);
    it = db.set(3, bools.begin(), bools.end());
    check = db.begin();
    ++check;
    assert(check == it);
    assert(it->size == 5);
    assert(it->type == mdds::mtv::element_type_boolean);
    std::advance(it, 2);
    assert(it == db.end());

    // Overwrite multiple blocks with values whose type matches that of the top block.
    int32_t int_val = 255;
    db = mtv_type(10, int_val);
    bools.resize(6, true);
    db.set(4, bools.begin(), bools.end()); // set 4 thru 9 to bool.
    db.set(5, 1.1);
    db.set(7, std::string("foo"));
    assert(db.block_size() == 6);
    doubles.resize(4, 4.5);
    it = db.set(5, doubles.begin(), doubles.end()); // 5 thrugh 8.
    check = db.begin();
    assert(check->type == mdds::mtv::element_type_int32);
    ++check;
    assert(check->type == mdds::mtv::element_type_boolean);
    ++check;
    assert(it == check);
    assert(it->type == mdds::mtv::element_type_double);
    assert(it->size == 4);
    std::advance(it, 2);
    assert(it == db.end());

    // The same scenario, except that the values also match that of the bottom block.
    db = mtv_type(10, 1.1);
    db.set(5, true);
    assert(db.block_size() == 3);
    doubles.resize(3, 2.3);
    it = db.set(4, doubles.begin(), doubles.end());
    assert(db.block_size() == 1);
    assert(it == db.begin());
    assert(it->type == mdds::mtv::element_type_double);
    assert(it->size == 10);
    ++it;
    assert(it == db.end());

    // This time, the top block is of different type.
    db = mtv_type(10, false);
    doubles.resize(4, 4.5);
    db.set(3, doubles.begin(), doubles.end()); // 3 thru 6
    db.set(0, int32_t(1));
    strings.resize(4, std::string("test"));
    it = db.set(4, strings.begin(), strings.end()); // Overwrite the lower part of the top block.
    check = db.begin();
    assert(check->type == mdds::mtv::element_type_int32);
    ++check;
    assert(check->type == mdds::mtv::element_type_boolean);
    ++check;
    assert(check->type == mdds::mtv::element_type_double);
    ++check;
    assert(it == check);
    assert(it->type == mdds::mtv::element_type_string);
    assert(it->size == 4);
    ++it;
    assert(it->type == mdds::mtv::element_type_boolean);
    ++it;
    assert(it == db.end());

    db = mtv_type(10, false);
    db.set(0, 1.1);
    db.set(4, 1.2);
    db.set(5, 1.3);
    db.set(6, std::string("a"));
    db.set(7, std::string("b"));
    doubles.resize(3, 0.8);
    it = db.set(6, doubles.begin(), doubles.end()); // Merge with the upper block.
    check = db.begin();
    assert(check->type == mdds::mtv::element_type_double);
    ++check;
    assert(check->type == mdds::mtv::element_type_boolean);
    ++check;
    assert(it == check);
    assert(it->type == mdds::mtv::element_type_double);
    assert(it->size == 5);
    ++it;
    assert(it->type == mdds::mtv::element_type_boolean);
    assert(it->size == 1);
    ++it;
    assert(it == db.end());

    // Make sure this also works in scenarios where the values merge with lower block.
    db = mtv_type(20, false);
    doubles.resize(4, 3.4);
    db.set(5, doubles.begin(), doubles.end()); // 5 thru 8
    strings.resize(5, "expanded");
    db.set(11, strings.begin(), strings.end()); // 11 thru 15
    strings.clear();
    strings.resize(6, "overwriting");
    it = db.set(7, strings.begin(), strings.end()); // 7 thru 12

    // At this point, 7 thru 15 should be strings.
    assert(it->type == mdds::mtv::element_type_string);
    assert(it->size == 9);
    check = db.begin();
    assert(check->type == mdds::mtv::element_type_boolean);
    assert(check->size == 5); // 0 thru 4
    ++check;
    assert(check->type == mdds::mtv::element_type_double);
    assert(check->size == 2); // 5 thru 6
    ++check;
    assert(it == check);
    ++it;
    assert(it->type == mdds::mtv::element_type_boolean);
    assert(it->size == 4); // 16 thru 19
    ++it;
    assert(it == db.end());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
