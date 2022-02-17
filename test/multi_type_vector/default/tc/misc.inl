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

enum test_mtv_type
{
    _mtv_bool,
    _mtv_int8,
    _mtv_uint8,
    _mtv_int16,
    _mtv_uint16,
    _mtv_int32,
    _mtv_uint32,
    _mtv_int64,
    _mtv_uint64,
    _mtv_float,
    _mtv_double,
    _mtv_string,
};

#define TEST_TYPE(_type_, _type_enum_) \
    test_mtv_type test_type(_type_) \
    { \
        return _type_enum_; \
    }
TEST_TYPE(bool, _mtv_bool)
TEST_TYPE(int8_t, _mtv_int8)
TEST_TYPE(uint8_t, _mtv_uint8)
TEST_TYPE(int16_t, _mtv_int16)
TEST_TYPE(uint16_t, _mtv_uint16)
TEST_TYPE(int32_t, _mtv_int32)
TEST_TYPE(uint32_t, _mtv_uint32)
TEST_TYPE(int64_t, _mtv_int64)
TEST_TYPE(uint64_t, _mtv_uint64)
TEST_TYPE(float, _mtv_float)
TEST_TYPE(double, _mtv_double)
TEST_TYPE(std::string, _mtv_string)

void mtv_test_misc_types()
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
        std::string val;
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

void mtv_test_misc_swap()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db1(3), db2(5);
    db1.set(0, 1.0);
    db1.set(1, 2.0);
    db1.set(2, 3.0);

    db2.set(0, 4.0);
    db2.set(1, 5.0);
    db2.set(4, std::string("foo"));
    db1.swap(db2);

    assert(db1.size() == 5 && db1.block_size() == 3);
    assert(db2.size() == 3 && db2.block_size() == 1);
}

void mtv_test_misc_equality()
{
    stack_printer __stack_printer__(__FUNCTION__);
    {
        // Two columns of equal size.
        mtv_type db1(3), db2(3);
        assert(db1 == db2);
        db1.set(0, 1.0);
        assert(db1 != db2);
        db2.set(0, 1.0);
        assert(db1 == db2);
        db2.set(0, 1.2);
        assert(db1 != db2);
        db1.set(0, 1.2);
        assert(db1 == db2);
    }

    {
        // Two columns of different sizes.  They are always non-equal no
        // matter what.
        mtv_type db1(3), db2(4);
        assert(db1 != db2);
        db1.set(0, 1.2);
        db2.set(0, 1.2);
        assert(db1 != db2);

        // Comparison to self.
        assert(db1 == db1);
        assert(db2 == db2);
    }

    {
        // Two containers both consisting of numeric - empty - numeric blocks,
        // and the last numeric blocks differ.

        mtv_type db1(5), db2(5);
        db1.set(0, 1.1);
        db1.set(3, 2.1);
        db1.set(4, 2.2);

        db2.set(0, 1.1);
        db2.set(3, 2.1);
        db2.set(4, 2.3); // different value

        assert(db1 != db2);
    }

    {
        mtv_type db1(2), db2(2);
        db1.set(1, 10.1);
        db2.set(1, 10.1);
        assert(db1 == db2);

        db2.set(0, std::string("foo"));
        assert(db1 != db2);

        db1.set(0, std::string("foo"));
        assert(db1 == db2);

        db2.set_empty(0, 0);
        assert(db1 != db2);
    }
}

void mtv_test_misc_clone()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db1(3);
    db1.set(0, 3.4);
    db1.set(1, std::string("foo"));
    db1.set(2, true);

    // copy construction

    mtv_type db2(db1);
    assert(db1.size() == db2.size());
    assert(db1.block_size() == db2.block_size());
    assert(db1 == db2);

    {
        double test1, test2;
        db1.get(0, test1);
        db2.get(0, test2);
        assert(test1 == test2);
    }
    {
        std::string test1, test2;
        db1.get(1, test1);
        db2.get(1, test2);
        assert(test1 == test2);
    }

    {
        bool test1, test2;
        db1.get(2, test1);
        db2.get(2, test2);
        assert(test1 == test2);
    }

    // assignment

    mtv_type db3 = db1;
    assert(db3 == db1);
    db3.set(0, std::string("alpha"));
    assert(db3 != db1);

    mtv_type db4, db5;
    db4 = db5 = db3;
    assert(db4 == db5);
    assert(db3 == db5);
    assert(db3 == db4);
}

void mtv_test_misc_resize()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db(0);
    assert(db.size() == 0);
    assert(db.empty());

    // Resize to create initial empty block.
    db.resize(3);
    assert(db.size() == 3);
    assert(db.block_size() == 1);

    // Resize to increase the existing empty block.
    db.resize(5);
    assert(db.size() == 5);
    assert(db.block_size() == 1);

    for (long row = 0; row < 5; ++row)
        db.set(row, static_cast<double>(row));

    assert(db.size() == 5);
    assert(db.block_size() == 1);

    // Increase its size by one.  This should append an empty cell block of size one.
    db.resize(6);
    assert(db.size() == 6);
    assert(db.block_size() == 2);
    assert(db.is_empty(5));

    // Do it again.
    db.resize(7);
    assert(db.size() == 7);
    assert(db.block_size() == 2);

    // Now, reduce its size to eliminate the last empty block.
    db.resize(5);
    assert(db.size() == 5);
    assert(db.block_size() == 1);

    // Reset.
    db.resize(7);
    assert(db.size() == 7);
    assert(db.block_size() == 2);

    // Now, resize across multiple blocks.
    db.resize(4);
    assert(db.size() == 4);
    assert(db.block_size() == 1);
    double test;
    db.get(3, test);
    assert(test == 3.0);

    // Empty it.
    db.resize(0);
    assert(db.size() == 0);
    assert(db.block_size() == 0);
    assert(db.empty());
}

void mtv_test_misc_value_type()
{
    stack_printer __stack_printer__(__FUNCTION__);

    mtv_type db(5);
    db.set(0, 1.1);
    db.set(1, std::string("A"));
    db.set(2, std::string("B"));
    db.set(3, int32_t(12));
    db.set(4, int16_t(8));

    std::for_each(db.begin(), db.end(), [](const mtv_type::value_type& node) {
        cout << "type: " << node.type << "  size: " << node.size << "  data: " << node.data << endl;
    });
}

void mtv_test_misc_block_identifier()
{
    stack_printer __stack_printer__(__FUNCTION__);
    assert(mdds::mtv::double_element_block::block_type == mdds::mtv::element_type_double);
    assert(mdds::mtv::string_element_block::block_type == mdds::mtv::element_type_string);
    assert(mdds::mtv::int16_element_block::block_type == mdds::mtv::element_type_int16);
    assert(mdds::mtv::uint16_element_block::block_type == mdds::mtv::element_type_uint16);
    assert(mdds::mtv::int32_element_block::block_type == mdds::mtv::element_type_int32);
    assert(mdds::mtv::uint32_element_block::block_type == mdds::mtv::element_type_uint32);
    assert(mdds::mtv::int64_element_block::block_type == mdds::mtv::element_type_int64);
    assert(mdds::mtv::uint64_element_block::block_type == mdds::mtv::element_type_uint64);
    assert(mdds::mtv::boolean_element_block::block_type == mdds::mtv::element_type_boolean);
    assert(mdds::mtv::int8_element_block::block_type == mdds::mtv::element_type_int8);
    assert(mdds::mtv::uint8_element_block::block_type == mdds::mtv::element_type_uint8);
}

void mtv_test_misc_push_back()
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
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->__private_data.block_index == 0);
    assert(it == db.begin());
    ++it;
    assert(it == db.end());

    // ... and again.
    it = db.push_back_empty();
    assert(db.size() == 2);
    assert(db.block_size() == 1);
    assert(it->size == 2);
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->__private_data.block_index == 0);
    assert(it == db.begin());
    ++it;
    assert(it == db.end());

    // Append non-empty this time.
    it = db.push_back(1.1);
    assert(db.size() == 3);
    assert(db.block_size() == 2);
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_double);
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
    assert(it->type == mdds::mtv::element_type_empty);
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
    it = db.push_back(std::string("push me"));
    assert(db.size() == 1);
    assert(db.block_size() == 1);
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_string);
    assert(it->__private_data.block_index == 0);
    assert(it == db.begin());
    ++it;
    assert(it == db.end());
    assert(db.get<std::string>(0) == "push me");

    // Push back a non-empty element of the same type.
    it = db.push_back(std::string("again"));
    assert(db.size() == 2);
    assert(db.block_size() == 1);
    assert(it->size == 2);
    assert(it->type == mdds::mtv::element_type_string);
    assert(it->__private_data.block_index == 0);
    assert(it == db.begin());
    ++it;
    assert(it == db.end());

    assert(db.get<std::string>(0) == "push me");
    assert(db.get<std::string>(1) == "again");

    // Push back another non-empty element of a different type.
    it = db.push_back(23.4);
    assert(db.size() == 3);
    assert(db.block_size() == 2);
    assert(it->size == 1);
    assert(it->type == mdds::mtv::element_type_double);
}

void mtv_test_misc_capacity()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db(10, 1.1);
    assert(db.block_size() == 1);
    mtv_type::const_iterator it = db.begin();
    assert(it->type == mdds::mtv::element_type_double);
    size_t cap = mdds::mtv::double_element_block::capacity(*it->data);
    assert(cap == 10);

    db.set_empty(3, 3);
    assert(db.block_size() == 3);
    db.shrink_to_fit();
    it = db.begin();
    assert(it->type == mdds::mtv::element_type_double);
    cap = mdds::mtv::double_element_block::capacity(*it->data);
    assert(cap == 3);
}

void mtv_test_misc_position_type_end_position()
{
    stack_printer __stack_printer__(__FUNCTION__);

    auto run = [](auto& db) {
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
    };

    mtv_type db(10);
    const mtv_type& cdb = db;
    cout << "* position_type" << endl;
    run(db);
    cout << "* const_position_type" << endl;
    run(cdb);
}

/**
 * Ensure that we test block adjustment case with adjusted block size
 * greater than 8.
 */
void mtv_test_misc_block_pos_adjustments()
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
