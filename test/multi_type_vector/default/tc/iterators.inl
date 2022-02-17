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

void mtv_test_iterators()
{
    stack_printer __stack_printer__(__FUNCTION__);
    {
        mtv_type db(5);
        mtv_type::const_iterator it;
        it = db.begin();
        mtv_type::const_iterator it_end = db.end();
        size_t len = std::distance(it, it_end);
        assert(len == 1);
        assert(it != it_end);
        assert(it->type == mdds::mtv::element_type_empty);
        assert(it->size == 5);
        const mtv_type::const_iterator::value_type& val = *it;
        assert(val.type == it->type);
        assert(val.size == it->size);

        ++it;
        assert(it == it_end);
    }

    {
        mtv_type db(6);
        db.set(0, 1.1);
        db.set(1, 2.2);
        db.set(4, std::string("boo"));
        db.set(5, std::string("hoo"));
        assert(db.block_size() == 3);
        {
            // Forward iterator
            mtv_type::const_iterator it = db.begin(), it_end = db.end();
            size_t len = std::distance(it, it_end);
            assert(len == 3);
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_double);
            assert(it->size == 2);

            ++it;
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_empty);
            assert(it->size == 2);

            ++it;
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_string);
            assert(it->size == 2);

            ++it;
            assert(it == it_end);
        }

        {
            // Reverse iterator
            mtv_type::const_reverse_iterator it = db.rbegin(), it_end = db.rend();
            size_t len = std::distance(it, it_end);
            assert(len == 3);
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_string);
            assert(it->size == 2);

            ++it;
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_empty);
            assert(it->size == 2);

            ++it;
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_double);
            assert(it->size == 2);

            ++it;
            assert(it == it_end);
        }

        {
            // Reverse iterator (C++11)
            mtv_type::const_reverse_iterator it = db.crbegin(), it_end = db.crend();
            size_t len = std::distance(it, it_end);
            assert(len == 3);
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_string);
            assert(it->size == 2);

            ++it;
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_empty);
            assert(it->size == 2);

            ++it;
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_double);
            assert(it->size == 2);

            ++it;
            assert(it == it_end);
        }
    }

    {
        // Make sure that decrementing the iterator calculates the position correctly.
        mtv_type db(10);
        db.set(0, true);
        mtv_type::const_iterator it = db.begin();
        assert(it->position == 0);
        assert(it->size == 1);
        ++it;
        assert(it->position == 1);
        assert(it->size == 9);
        --it;
        assert(it->position == 0);
        assert(it->size == 1);
        assert(it == db.begin());
    }
}

void mtv_test_iterators_element_block()
{
    stack_printer __stack_printer__(__FUNCTION__);

    mtv_type db(10);
    db.set(0, 1.1);
    db.set(1, 1.2);
    db.set(2, 1.3);
    db.set(4, std::string("A"));
    db.set(5, std::string("B"));
    db.set(6, std::string("C"));
    db.set(7, std::string("D"));
    mtv_type::const_iterator it_blk = db.begin(), it_blk_end = db.end();

    // First block is a numeric block.
    assert(it_blk != it_blk_end);
    assert(it_blk->type == mdds::mtv::element_type_double);
    assert(it_blk->size == 3);
    assert(it_blk->data);
    {
        mdds::mtv::double_element_block::const_iterator it_data = mdds::mtv::double_element_block::begin(*it_blk->data);
        mdds::mtv::double_element_block::const_iterator it_data_end =
            mdds::mtv::double_element_block::end(*it_blk->data);
        assert(it_data != it_data_end);
        assert(*it_data == 1.1);
        ++it_data;
        assert(*it_data == 1.2);
        ++it_data;
        assert(*it_data == 1.3);
        ++it_data;
        assert(it_data == it_data_end);

        assert(mdds::mtv::double_element_block::at(*it_blk->data, 0) == 1.1);
        assert(mdds::mtv::double_element_block::at(*it_blk->data, 1) == 1.2);
        assert(mdds::mtv::double_element_block::at(*it_blk->data, 2) == 1.3);

        // Access the underlying data array directly.
        const double* array = &mdds::mtv::double_element_block::at(*it_blk->data, 0);
        assert(*array == 1.1);
        ++array;
        assert(*array == 1.2);
        ++array;
        assert(*array == 1.3);
    }

    // Next block is empty.
    ++it_blk;
    assert(it_blk->type == mdds::mtv::element_type_empty);
    assert(it_blk->size == 1);
    assert(it_blk->data == nullptr);

    // Next block is a string block.
    ++it_blk;
    assert(it_blk->type == mdds::mtv::element_type_string);
    assert(it_blk->size == 4);
    assert(it_blk->data);
    {
        mdds::mtv::string_element_block::const_reverse_iterator it_data =
            mdds::mtv::string_element_block::rbegin(*it_blk->data);
        mdds::mtv::string_element_block::const_reverse_iterator it_data_end =
            mdds::mtv::string_element_block::rend(*it_blk->data);
        assert(it_data != it_data_end);
        assert(*it_data == "D");
        ++it_data;
        assert(*it_data == "C");
        ++it_data;
        assert(*it_data == "B");
        ++it_data;
        assert(*it_data == "A");
        ++it_data;
        assert(it_data == it_data_end);
    }

    {
        // Test crbegin() and crend() too, which should be identical to the
        // const variants of rbegin() and rend() from above.
        mdds::mtv::string_element_block::const_reverse_iterator it_data =
            mdds::mtv::string_element_block::crbegin(*it_blk->data);
        mdds::mtv::string_element_block::const_reverse_iterator it_data_end =
            mdds::mtv::string_element_block::crend(*it_blk->data);
        assert(it_data != it_data_end);
        assert(*it_data == "D");
        ++it_data;
        assert(*it_data == "C");
        ++it_data;
        assert(*it_data == "B");
        ++it_data;
        assert(*it_data == "A");
        ++it_data;
        assert(it_data == it_data_end);
    }

    // Another empty block follows.
    ++it_blk;
    assert(it_blk->type == mdds::mtv::element_type_empty);
    assert(it_blk->size == 2);
    assert(it_blk->data == nullptr);

    ++it_blk;
    assert(it_blk == it_blk_end);
}

void mtv_test_iterators_mutable_element_block()
{
    stack_printer __stack_printer__(__FUNCTION__);

    /**
     * This function is just to ensure that even the non-const iterator can be
     * dereferenced via const reference.
     *
     * @param it this is passed as a const reference, yet it should still allow
     *           being dereferenced as long as no data is modified.
     */
    auto check_block_iterator = [](const mtv_type::iterator& it, mdds::mtv::element_t expected) {
        mdds::mtv::element_t actual = it->type;
        const mtv_type::element_block_type* data = (*it).data;
        assert(actual == expected);
        assert(data != nullptr);
    };

    mtv_type db(1);
    db.set(0, 1.2);
    mtv_type::iterator it_blk = db.begin(), it_blk_end = db.end();
    size_t n = std::distance(it_blk, it_blk_end);
    assert(n == 1);
    check_block_iterator(it_blk, mdds::mtv::element_type_double);

    mdds::mtv::double_element_block::iterator it = mdds::mtv::double_element_block::begin(*it_blk->data);
    mdds::mtv::double_element_block::iterator it_end = mdds::mtv::double_element_block::end(*it_blk->data);
    n = std::distance(it, it_end);
    assert(n == 1);
    assert(*it == 1.2);

    *it = 2.3; // write via iterator.
    assert(db.get<double>(0) == 2.3);

    db.resize(3);
    db.set(1, 2.4);
    db.set(2, 2.5);

    it_blk = db.begin();
    it_blk_end = db.end();
    n = std::distance(it_blk, it_blk_end);
    assert(n == 1);
    check_block_iterator(it_blk, mdds::mtv::element_type_double);

    it = mdds::mtv::double_element_block::begin(*it_blk->data);
    it_end = mdds::mtv::double_element_block::end(*it_blk->data);
    n = std::distance(it, it_end);
    assert(n == 3);
    *it = 3.1;
    ++it;
    *it = 3.2;
    ++it;
    *it = 3.3;

    assert(db.get<double>(0) == 3.1);
    assert(db.get<double>(1) == 3.2);
    assert(db.get<double>(2) == 3.3);
}

void mtv_test_iterators_private_data()
{
    stack_printer __stack_printer__(__FUNCTION__);

    // What the end position iterator stores in the private data area is
    // intentionally undefined.

    mtv_type db(9);

    // With only a single block

    mtv_type::iterator it = db.begin();
    assert(it->position == 0);
    assert(it->__private_data.block_index == 0);

    it = db.end();
    --it;
    assert(it->position == 0);
    assert(it->__private_data.block_index == 0);

    // With 3 blocks (sizes of 4, 3, and 2 in this order)

    db.set(4, 1.1);
    db.set(5, 1.1);
    db.set(6, 1.1);

    it = db.begin();
    assert(it->size == 4);
    assert(it->position == 0);
    assert(it->__private_data.block_index == 0);
    ++it;
    assert(it->size == 3);
    assert(it->position == 4);
    assert(it->__private_data.block_index == 1);
    ++it;
    assert(it->size == 2);
    assert(it->position == 7);
    assert(it->__private_data.block_index == 2);

    ++it;
    assert(it == db.end()); // end position reached.

    // Go in reverse direction.
    --it;
    assert(it->size == 2);
    assert(it->position == 7);
    assert(it->__private_data.block_index == 2);
    --it;
    assert(it->size == 3);
    assert(it->position == 4);
    assert(it->__private_data.block_index == 1);
    --it;
    assert(it->size == 4);
    assert(it->position == 0);
    assert(it->__private_data.block_index == 0);
    assert(it == db.begin());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
