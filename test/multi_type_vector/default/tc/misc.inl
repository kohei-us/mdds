/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

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
    MDDS_TEST_FUNC_SCOPE;

    // Test function overloading of standard types.
    {
        bool val = false;
        TEST_ASSERT(test_type(val) == _mtv_bool);
        cout << "bool is good" << endl;
    }
    {
        int16_t val = 0;
        TEST_ASSERT(test_type(val) == _mtv_int16);
        cout << "int16 is good" << endl;
    }
    {
        uint16_t val = 0;
        TEST_ASSERT(test_type(val) == _mtv_uint16);
        cout << "uint16 is good" << endl;
    }
    {
        int32_t val = 0;
        TEST_ASSERT(test_type(val) == _mtv_int32);
        cout << "int32 is good" << endl;
    }
    {
        uint32_t val = 0;
        TEST_ASSERT(test_type(val) == _mtv_uint32);
        cout << "uint32 is good" << endl;
    }
    {
        int64_t val = 0;
        TEST_ASSERT(test_type(val) == _mtv_int64);
        cout << "int64 is good" << endl;
    }
    {
        uint64_t val = 0;
        TEST_ASSERT(test_type(val) == _mtv_uint64);
        cout << "uint64 is good" << endl;
    }
    {
        float val = 0;
        TEST_ASSERT(test_type(val) == _mtv_float);
        cout << "float is good" << endl;
    }
    {
        double val = 0;
        TEST_ASSERT(test_type(val) == _mtv_double);
        cout << "double is good" << endl;
    }
    {
        std::string val;
        TEST_ASSERT(test_type(val) == _mtv_string);
        cout << "string is good" << endl;
    }
    {
        int8_t val = 0;
        TEST_ASSERT(test_type(val) == _mtv_int8);
        cout << "int8 is good" << endl;
    }
    {
        uint8_t val = 0;
        TEST_ASSERT(test_type(val) == _mtv_uint8);
        cout << "uint8 is good" << endl;
    }
}

void mtv_test_misc_swap()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type db1(3), db2(5);
    db1.set(0, 1.0);
    db1.set(1, 2.0);
    db1.set(2, 3.0);

    db2.set(0, 4.0);
    db2.set(1, 5.0);
    db2.set(4, std::string("foo"));
    db1.swap(db2);

    TEST_ASSERT(db1.size() == 5 && db1.block_size() == 3);
    TEST_ASSERT(db2.size() == 3 && db2.block_size() == 1);
}

void mtv_test_misc_equality()
{
    MDDS_TEST_FUNC_SCOPE;

    // Test function overloading of standard types.
    {
        // Two columns of equal size.
        mtv_type db1(3), db2(3);
        TEST_ASSERT(db1 == db2);
        db1.set(0, 1.0);
        TEST_ASSERT(db1 != db2);
        db2.set(0, 1.0);
        TEST_ASSERT(db1 == db2);
        db2.set(0, 1.2);
        TEST_ASSERT(db1 != db2);
        db1.set(0, 1.2);
        TEST_ASSERT(db1 == db2);
    }

    {
        // Two columns of different sizes.  They are always non-equal no
        // matter what.
        mtv_type db1(3), db2(4);
        TEST_ASSERT(db1 != db2);
        db1.set(0, 1.2);
        db2.set(0, 1.2);
        TEST_ASSERT(db1 != db2);

        // Comparison to self.
        TEST_ASSERT(db1 == db1);
        TEST_ASSERT(db2 == db2);
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

        TEST_ASSERT(db1 != db2);
    }

    {
        mtv_type db1(2), db2(2);
        db1.set(1, 10.1);
        db2.set(1, 10.1);
        TEST_ASSERT(db1 == db2);

        db2.set(0, std::string("foo"));
        TEST_ASSERT(db1 != db2);

        db1.set(0, std::string("foo"));
        TEST_ASSERT(db1 == db2);

        db2.set_empty(0, 0);
        TEST_ASSERT(db1 != db2);
    }
}

void mtv_test_misc_clone()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type db1(3);
    db1.set(0, 3.4);
    db1.set(1, std::string("foo"));
    db1.set(2, true);

    // copy construction

    mtv_type db2(db1);
    TEST_ASSERT(db1.size() == db2.size());
    TEST_ASSERT(db1.block_size() == db2.block_size());
    TEST_ASSERT(db1 == db2);

    {
        double test1, test2;
        db1.get(0, test1);
        db2.get(0, test2);
        TEST_ASSERT(test1 == test2);
    }
    {
        std::string test1, test2;
        db1.get(1, test1);
        db2.get(1, test2);
        TEST_ASSERT(test1 == test2);
    }

    {
        bool test1, test2;
        db1.get(2, test1);
        db2.get(2, test2);
        TEST_ASSERT(test1 == test2);
    }

    // assignment

    mtv_type db3 = db1;
    TEST_ASSERT(db3 == db1);
    db3.set(0, std::string("alpha"));
    TEST_ASSERT(db3 != db1);

    mtv_type db4, db5;
    db4 = db5 = db3;
    TEST_ASSERT(db4 == db5);
    TEST_ASSERT(db3 == db5);
    TEST_ASSERT(db3 == db4);
}

void mtv_test_misc_resize()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type db(0);
    TEST_ASSERT(db.size() == 0);
    TEST_ASSERT(db.empty());

    // Resize to create initial empty block.
    db.resize(3);
    TEST_ASSERT(db.size() == 3);
    TEST_ASSERT(db.block_size() == 1);

    // Resize to increase the existing empty block.
    db.resize(5);
    TEST_ASSERT(db.size() == 5);
    TEST_ASSERT(db.block_size() == 1);

    for (long row = 0; row < 5; ++row)
        db.set(row, static_cast<double>(row));

    TEST_ASSERT(db.size() == 5);
    TEST_ASSERT(db.block_size() == 1);

    // Increase its size by one.  This should append an empty cell block of size one.
    db.resize(6);
    TEST_ASSERT(db.size() == 6);
    TEST_ASSERT(db.block_size() == 2);
    TEST_ASSERT(db.is_empty(5));

    // Do it again.
    db.resize(7);
    TEST_ASSERT(db.size() == 7);
    TEST_ASSERT(db.block_size() == 2);

    // Now, reduce its size to eliminate the last empty block.
    db.resize(5);
    TEST_ASSERT(db.size() == 5);
    TEST_ASSERT(db.block_size() == 1);

    // Reset.
    db.resize(7);
    TEST_ASSERT(db.size() == 7);
    TEST_ASSERT(db.block_size() == 2);

    // Now, resize across multiple blocks.
    db.resize(4);
    TEST_ASSERT(db.size() == 4);
    TEST_ASSERT(db.block_size() == 1);
    double test;
    db.get(3, test);
    TEST_ASSERT(test == 3.0);

    // Empty it.
    db.resize(0);
    TEST_ASSERT(db.size() == 0);
    TEST_ASSERT(db.block_size() == 0);
    TEST_ASSERT(db.empty());
}

void mtv_test_misc_value_type()
{
    MDDS_TEST_FUNC_SCOPE;

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
    MDDS_TEST_FUNC_SCOPE;

    TEST_ASSERT(mdds::mtv::double_element_block::block_type == mdds::mtv::element_type_double);
    TEST_ASSERT(mdds::mtv::string_element_block::block_type == mdds::mtv::element_type_string);
    TEST_ASSERT(mdds::mtv::int16_element_block::block_type == mdds::mtv::element_type_int16);
    TEST_ASSERT(mdds::mtv::uint16_element_block::block_type == mdds::mtv::element_type_uint16);
    TEST_ASSERT(mdds::mtv::int32_element_block::block_type == mdds::mtv::element_type_int32);
    TEST_ASSERT(mdds::mtv::uint32_element_block::block_type == mdds::mtv::element_type_uint32);
    TEST_ASSERT(mdds::mtv::int64_element_block::block_type == mdds::mtv::element_type_int64);
    TEST_ASSERT(mdds::mtv::uint64_element_block::block_type == mdds::mtv::element_type_uint64);
    TEST_ASSERT(mdds::mtv::boolean_element_block::block_type == mdds::mtv::element_type_boolean);
    TEST_ASSERT(mdds::mtv::int8_element_block::block_type == mdds::mtv::element_type_int8);
    TEST_ASSERT(mdds::mtv::uint8_element_block::block_type == mdds::mtv::element_type_uint8);
}

void mtv_test_misc_push_back()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type db;
    TEST_ASSERT(db.size() == 0);
    TEST_ASSERT(db.block_size() == 0);

    // Append an empty element into an empty container.
    mtv_type::iterator it = db.push_back_empty();
    TEST_ASSERT(db.size() == 1);
    TEST_ASSERT(db.block_size() == 1);
    TEST_ASSERT(it->size == 1);
    TEST_ASSERT(it->type == mdds::mtv::element_type_empty);
    TEST_ASSERT(it->__private_data.block_index == 0);
    TEST_ASSERT(it == db.begin());
    ++it;
    TEST_ASSERT(it == db.end());

    // ... and again.
    it = db.push_back_empty();
    TEST_ASSERT(db.size() == 2);
    TEST_ASSERT(db.block_size() == 1);
    TEST_ASSERT(it->size == 2);
    TEST_ASSERT(it->type == mdds::mtv::element_type_empty);
    TEST_ASSERT(it->__private_data.block_index == 0);
    TEST_ASSERT(it == db.begin());
    ++it;
    TEST_ASSERT(it == db.end());

    // Append non-empty this time.
    it = db.push_back(1.1);
    TEST_ASSERT(db.size() == 3);
    TEST_ASSERT(db.block_size() == 2);
    TEST_ASSERT(it->size == 1);
    TEST_ASSERT(it->type == mdds::mtv::element_type_double);
    TEST_ASSERT(it->__private_data.block_index == 1);
    mtv_type::iterator check = it;
    --check;
    TEST_ASSERT(check == db.begin());
    ++it;
    TEST_ASSERT(it == db.end());

    // followed by an empty element again.
    it = db.push_back_empty();
    TEST_ASSERT(db.size() == 4);
    TEST_ASSERT(db.block_size() == 3);
    TEST_ASSERT(it->size == 1);
    TEST_ASSERT(it->type == mdds::mtv::element_type_empty);
    TEST_ASSERT(it->__private_data.block_index == 2);
    check = it;
    --check;
    --check;
    TEST_ASSERT(check == db.begin());
    ++it;
    TEST_ASSERT(it == db.end());

    // Check the values.
    TEST_ASSERT(db.is_empty(0));
    TEST_ASSERT(db.is_empty(1));
    TEST_ASSERT(db.get<double>(2) == 1.1);
    TEST_ASSERT(db.is_empty(3));

    // Empty the container and push back a non-empty element.
    db.clear();
    it = db.push_back(std::string("push me"));
    TEST_ASSERT(db.size() == 1);
    TEST_ASSERT(db.block_size() == 1);
    TEST_ASSERT(it->size == 1);
    TEST_ASSERT(it->type == mdds::mtv::element_type_string);
    TEST_ASSERT(it->__private_data.block_index == 0);
    TEST_ASSERT(it == db.begin());
    ++it;
    TEST_ASSERT(it == db.end());
    TEST_ASSERT(db.get<std::string>(0) == "push me");

    // Push back a non-empty element of the same type.
    it = db.push_back(std::string("again"));
    TEST_ASSERT(db.size() == 2);
    TEST_ASSERT(db.block_size() == 1);
    TEST_ASSERT(it->size == 2);
    TEST_ASSERT(it->type == mdds::mtv::element_type_string);
    TEST_ASSERT(it->__private_data.block_index == 0);
    TEST_ASSERT(it == db.begin());
    ++it;
    TEST_ASSERT(it == db.end());

    TEST_ASSERT(db.get<std::string>(0) == "push me");
    TEST_ASSERT(db.get<std::string>(1) == "again");

    // Push back another non-empty element of a different type.
    it = db.push_back(23.4);
    TEST_ASSERT(db.size() == 3);
    TEST_ASSERT(db.block_size() == 2);
    TEST_ASSERT(it->size == 1);
    TEST_ASSERT(it->type == mdds::mtv::element_type_double);
}

void mtv_test_misc_capacity()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type db(10, 1.1);
    TEST_ASSERT(db.block_size() == 1);
    mtv_type::const_iterator it = db.begin();
    TEST_ASSERT(it->type == mdds::mtv::element_type_double);
    size_t cap = mdds::mtv::double_element_block::capacity(*it->data);
    TEST_ASSERT(cap == 10);

    db.set_empty(3, 3);
    TEST_ASSERT(db.block_size() == 3);
    db.shrink_to_fit();
    it = db.begin();
    TEST_ASSERT(it->type == mdds::mtv::element_type_double);
    cap = mdds::mtv::double_element_block::capacity(*it->data);
    TEST_ASSERT(cap == 3);
}

void mtv_test_misc_position_type_end_position()
{
    MDDS_TEST_FUNC_SCOPE;

    auto run = [](auto& db) {
        auto pos1 = db.position(9); // last valid position.
        pos1 = mtv_type::next_position(pos1);
        auto pos2 = db.position(10); // end position - one position past the last valid position
        TEST_ASSERT(pos1 == pos2);

        // Move back from the end position by one. It should point to the last
        // valid position.
        pos2 = mtv_type::advance_position(pos2, -1);
        pos1 = db.position(9);
        TEST_ASSERT(pos1 == pos2);

        // Try the variant of position() method that takes position hint as its first argument.
        pos1 = db.position(db.begin(), 10);
        pos1 = mtv_type::advance_position(pos1, -10);
        pos2 = db.position(0);
        TEST_ASSERT(pos1 == pos2);

        // A position more than one past the last valid position is considered out-of-range.

        try
        {
            pos1 = db.position(11);
            TEST_ASSERT(!"No exceptions thrown, but one was expected to be thrown.");
        }
        catch (const std::out_of_range&)
        {
            // good.
            cout << "Out of range exception was thrown as expected." << endl;
        }
        catch (...)
        {
            TEST_ASSERT(!"An unexpected exception was thrown.");
        }

        // Try again with the variant that takes a position hint.

        try
        {
            pos1 = db.position(db.begin(), 11);
            TEST_ASSERT(!"No exceptions thrown, but one was expected to be thrown.");
        }
        catch (const std::out_of_range&)
        {
            // good.
            cout << "Out of range exception was thrown as expected." << endl;
        }
        catch (...)
        {
            TEST_ASSERT(!"An unexpected exception was thrown.");
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
    MDDS_TEST_FUNC_SCOPE;

    mtv_type db(1);

    for (int i = 0; i < 20; ++i)
    {
        std::vector<double> vs(2, 1.1);
        db.insert(0, vs.begin(), vs.end());

        std::vector<int16_t> is(2, 34);
        db.insert(0, is.begin(), is.end());
    }

    TEST_ASSERT(db.size() == 81);
    TEST_ASSERT(db.block_size() == 41);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
