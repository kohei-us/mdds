/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

struct event_block_counter
{
    size_t block_count; // number of element (data) blocks
    size_t block_count_numeric;
    size_t block_count_string;
    size_t block_count_int16;
    size_t block_count_uint16;
    size_t block_count_int32;
    size_t block_count_uint32;
    size_t block_count_int64;
    size_t block_count_uint64;
    size_t block_count_boolean;
    size_t block_count_int8;
    size_t block_count_uint8;

    event_block_counter()
        : block_count(0), block_count_numeric(0), block_count_string(0), block_count_int16(0), block_count_uint16(0),
          block_count_int32(0), block_count_uint32(0), block_count_int64(0), block_count_uint64(0),
          block_count_boolean(0), block_count_int8(0), block_count_uint8(0)
    {}

    /**
     * This copy constructor intentionally does not copy the counters.
     */
    event_block_counter(const event_block_counter&)
        : block_count(0), block_count_numeric(0), block_count_string(0), block_count_int16(0), block_count_uint16(0),
          block_count_int32(0), block_count_uint32(0), block_count_int64(0), block_count_uint64(0),
          block_count_boolean(0), block_count_int8(0), block_count_uint8(0)
    {}

    void element_block_acquired(const mdds::mtv::base_element_block* block)
    {
        ++block_count;

        switch (mdds::mtv::get_block_type(*block))
        {
            case mdds::mtv::element_type_double:
                ++block_count_numeric;
                break;
            case mdds::mtv::element_type_string:
                ++block_count_string;
                break;
            case mdds::mtv::element_type_int16:
                ++block_count_int16;
                break;
            case mdds::mtv::element_type_uint16:
                ++block_count_uint16;
                break;
            case mdds::mtv::element_type_int32:
                ++block_count_int32;
                break;
            case mdds::mtv::element_type_uint32:
                ++block_count_uint32;
                break;
            case mdds::mtv::element_type_int64:
                ++block_count_int64;
                break;
            case mdds::mtv::element_type_uint64:
                ++block_count_uint64;
                break;
            case mdds::mtv::element_type_boolean:
                ++block_count_boolean;
                break;
            case mdds::mtv::element_type_int8:
                ++block_count_int8;
                break;
            case mdds::mtv::element_type_uint8:
                ++block_count_uint8;
                break;
            default:;
        }
    }

    void element_block_released(const mdds::mtv::base_element_block* block)
    {
        --block_count;

        switch (mdds::mtv::get_block_type(*block))
        {
            case mdds::mtv::element_type_double:
                --block_count_numeric;
                break;
            case mdds::mtv::element_type_string:
                --block_count_string;
                break;
            case mdds::mtv::element_type_int16:
                --block_count_int16;
                break;
            case mdds::mtv::element_type_uint16:
                --block_count_uint16;
                break;
            case mdds::mtv::element_type_int32:
                --block_count_int32;
                break;
            case mdds::mtv::element_type_uint32:
                --block_count_uint32;
                break;
            case mdds::mtv::element_type_int64:
                --block_count_int64;
                break;
            case mdds::mtv::element_type_uint64:
                --block_count_uint64;
                break;
            case mdds::mtv::element_type_boolean:
                --block_count_boolean;
                break;
            case mdds::mtv::element_type_int8:
                --block_count_int8;
                break;
            case mdds::mtv::element_type_uint8:
                --block_count_uint8;
                break;
            default:;
        }
    }
};

struct eb_counter_trait : public mdds::mtv::standard_element_blocks_traits
{
    using event_func = event_block_counter;

    constexpr static mdds::mtv::lu_factor_t loop_unrolling = mdds::mtv::lu_factor_t::lu8;
};

void mtv_test_block_counter()
{
    MDDS_TEST_FUNC_SCOPE;

    using mtv_type = mtv_template_type<eb_counter_trait>;

    {
        // Initializing with an empty block should not create any element block.
        mtv_type db(10);
        TEST_ASSERT(db.event_handler().block_count == 0);
    }

    {
        // Initializing with one element block of size 10.
        mtv_type db(10, 1.2);
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
        db.clear();
        TEST_ASSERT(db.event_handler().block_count == 0);
        TEST_ASSERT(db.event_handler().block_count_numeric == 0);

        db.push_back(5.5); // create a new block.
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
        db.push_back(6.6); // no new block creation.
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
        db.push_back(std::string("foo")); // another new block.
        TEST_ASSERT(db.event_handler().block_count == 2);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
        TEST_ASSERT(db.event_handler().block_count_string == 1);

        // This should remove the last string block.
        db.resize(2);
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
        TEST_ASSERT(db.event_handler().block_count_string == 0);

        // This should have no effect on the block count.
        db.resize(1);
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);

        // This should remove the last remaining block.
        db.resize(0);
        TEST_ASSERT(db.event_handler().block_count == 0);
        TEST_ASSERT(db.event_handler().block_count_numeric == 0);
    }

    {
        mtv_type db(5);
        TEST_ASSERT(db.event_handler().block_count == 0);

        db.set(0, true);
        TEST_ASSERT(db.event_handler().block_count == 1);
        db.set(1, 12.2);
        TEST_ASSERT(db.event_handler().block_count == 2);
        TEST_ASSERT(db.event_handler().block_count_boolean == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);

        db.set(4, std::string("foo"));
        TEST_ASSERT(db.event_handler().block_count == 3);
        TEST_ASSERT(db.event_handler().block_count_string == 1);
        db.set(3, std::string("bar"));
        TEST_ASSERT(db.event_handler().block_count == 3);
        TEST_ASSERT(db.event_handler().block_count_string == 1);

        // This should delete the top two element blocks.
        db.set_empty(0, 1);
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_boolean == 0);
        TEST_ASSERT(db.event_handler().block_count_numeric == 0);

        // Now, delete the bottom one.
        db.set_empty(3, 4);
        TEST_ASSERT(db.event_handler().block_count == 0);
        TEST_ASSERT(db.event_handler().block_count_string == 0);

        // Create and delete a block in the middle.
        db.set(3, false);
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_boolean == 1);
        db.set_empty(3, 3);
        TEST_ASSERT(db.event_handler().block_count == 0);
        TEST_ASSERT(db.event_handler().block_count_boolean == 0);

        db.set(2, 10.5);
        db.set(3, std::string("hmm"));
        TEST_ASSERT(db.event_handler().block_count == 2);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
        TEST_ASSERT(db.event_handler().block_count_string == 1);
        db.set_empty(3, 3);
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
        TEST_ASSERT(db.event_handler().block_count_string == 0);

        // Start over.
        db.clear();
        TEST_ASSERT(db.event_handler().block_count == 0);

        db.push_back(1.1);
        db.push_back(1.2);
        db.push_back(1.3);
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);

        // Put empty block in the middle.
        db.set_empty(1, 1);
        TEST_ASSERT(db.event_handler().block_count == 2);
        TEST_ASSERT(db.event_handler().block_count_numeric == 2);
    }

    {
        mtv_type db(4, 1.2);
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);

        // Split the block into two.
        db.insert_empty(2, 2);
        TEST_ASSERT(db.event_handler().block_count == 2);
        TEST_ASSERT(db.event_handler().block_count_numeric == 2);
    }

    {
        mtv_type db(2);
        db.set(1, 1.2); // This creates a new element block.
        TEST_ASSERT(db.event_handler().block_count == 1);
        db.set(0, 1.1); // The element block count should not change.
        TEST_ASSERT(db.event_handler().block_count == 1);
    }

    {
        mtv_type db(2);
        db.set(1, std::string("test"));
        TEST_ASSERT(db.event_handler().block_count == 1);
        db.set(0, 1.1);
        TEST_ASSERT(db.event_handler().block_count == 2);
        db.set(0, true);
        TEST_ASSERT(db.event_handler().block_count == 2);
        TEST_ASSERT(db.event_handler().block_count_boolean == 1);
        TEST_ASSERT(db.event_handler().block_count_string == 1);

        db.set(0, std::string("foo"));
        TEST_ASSERT(db.event_handler().block_count == 1);

        db.set(1, 1.2);
        TEST_ASSERT(db.event_handler().block_count == 2);
        db.set(1, std::string("bar"));
        TEST_ASSERT(db.event_handler().block_count == 1);
    }

    {
        mtv_type db(2);
        db.set(0, std::string("test")); // This creates a new string block.
        TEST_ASSERT(db.event_handler().block_count == 1);
        db.set(1, std::string("foo")); // This appends to the existing string block.
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_string == 1);
    }

    {
        mtv_type db(3);
        db.set(0, std::string("test"));
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_string == 1);
        db.set(2, std::string("foo"));
        TEST_ASSERT(db.event_handler().block_count == 2);
        TEST_ASSERT(db.event_handler().block_count_string == 2);
        db.set(1, std::string("bar")); // This merges all data into a single string block.
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_string == 1);
    }

    {
        mtv_type db(4);
        db.set(0, std::string("test"));
        TEST_ASSERT(db.event_handler().block_count == 1);
        db.set(2, std::string("foo1"));
        TEST_ASSERT(db.event_handler().block_count == 2);
        db.set(3, std::string("foo2"));
        TEST_ASSERT(db.event_handler().block_count == 2);
        db.set(1, std::string("bar")); // This merges all data into a single string block.
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_string == 1);
    }

    {
        mtv_type db(3);
        db.set(0, std::string("test"));
        TEST_ASSERT(db.event_handler().block_count == 1);
        db.set(2, 1.2);
        TEST_ASSERT(db.event_handler().block_count == 2);
        db.set(1, std::string("bar"));
        TEST_ASSERT(db.event_handler().block_count == 2);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
        TEST_ASSERT(db.event_handler().block_count_string == 1);
    }

    {
        mtv_type db(3);
        db.set(0, std::string("test"));
        TEST_ASSERT(db.event_handler().block_count == 1);
        db.set(2, 1.2);
        TEST_ASSERT(db.event_handler().block_count == 2);
        db.set(1, 1.1); // This will get prepended to the next numeric block.
        TEST_ASSERT(db.event_handler().block_count == 2);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
        TEST_ASSERT(db.event_handler().block_count_string == 1);
    }

    {
        std::vector<double> vals = {1.1, 1.2, 1.3};
        mtv_type db(vals.size(), vals.begin(), vals.end());
        TEST_ASSERT(db.event_handler().block_count == 1);

        mtv_type db2(db);
        TEST_ASSERT(db2.event_handler().block_count == 1);
        db2.push_back(std::string("foo"));
        TEST_ASSERT(db2.event_handler().block_count == 2);
        mtv_type db3 = db2;
        TEST_ASSERT(db3.event_handler().block_count == 2);

        mtv_type db4(3);
        db4.insert(0, vals.begin(), vals.end());
        TEST_ASSERT(db4.event_handler().block_count == 1);

        mtv_type db5(3, int64_t(10));
        TEST_ASSERT(db5.event_handler().block_count == 1);
        db5.insert(0, vals.begin(), vals.end());
        TEST_ASSERT(db5.event_handler().block_count == 2);

        mtv_type db6(2, int32_t(30));
        TEST_ASSERT(db6.event_handler().block_count == 1);
        db6.insert(1, vals.begin(), vals.end()); // Insert to split the block.
        TEST_ASSERT(db6.event_handler().block_count == 3);
        TEST_ASSERT(db6.event_handler().block_count_int32 == 2);
        TEST_ASSERT(db6.event_handler().block_count_numeric == 1);
    }

    {
        mtv_type db(3);
        db.set(1, 1.1);
        db.set(2, true);
        TEST_ASSERT(db.event_handler().block_count == 2);
        db.set(1, false);
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_boolean == 1);
    }

    {
        mtv_type db(3);
        db.set(1, 1.1);
        db.set(0, true);
        TEST_ASSERT(db.event_handler().block_count == 2);
        TEST_ASSERT(db.event_handler().block_count_boolean == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
        db.set(1, false);
        TEST_ASSERT(db.event_handler().block_count == 1);
    }

    {
        mtv_type db(3);
        db.set(0, true);
        db.set(1, 1.1);
        db.set(2, false);
        TEST_ASSERT(db.event_handler().block_count == 3);
        TEST_ASSERT(db.event_handler().block_count_boolean == 2);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
        db.set(1, true);
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_boolean == 1);

        db.set(1, 1.1);
        TEST_ASSERT(db.event_handler().block_count == 3);
        db.set(2, int64_t(10));
        db.set(1, true);
        TEST_ASSERT(db.event_handler().block_count == 2);
        TEST_ASSERT(db.event_handler().block_count_boolean == 1);
        TEST_ASSERT(db.event_handler().block_count_int64 == 1);

        db.set(1, 1.1);
        TEST_ASSERT(db.event_handler().block_count == 3);
        TEST_ASSERT(db.event_handler().block_count_boolean == 1);
        TEST_ASSERT(db.event_handler().block_count_int64 == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
        db.set(1, int64_t(20));
        TEST_ASSERT(db.event_handler().block_count == 2);

        db.release();
        TEST_ASSERT(db.event_handler().block_count == 0);
    }

    {
        mtv_type db;
        db.push_back(1.1);
        db.push_back(int64_t(10));
        db.push_back(std::string("foo"));
        TEST_ASSERT(db.event_handler().block_count == 3);
        TEST_ASSERT(db.event_handler().block_count_int64 == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
        TEST_ASSERT(db.event_handler().block_count_string == 1);

        db.erase(0, 2);
        TEST_ASSERT(db.event_handler().block_count == 0);
    }

    {
        mtv_type db;
        db.push_back(1.1);
        db.push_back_empty();
        TEST_ASSERT(db.event_handler().block_count == 1);
        db.erase(0, 0);
        TEST_ASSERT(db.event_handler().block_count == 0);
    }

    {
        mtv_type db(3);
        db.set(0, std::string("top"));
        db.set(2, std::string("bottom"));
        TEST_ASSERT(db.event_handler().block_count == 2);
        TEST_ASSERT(db.event_handler().block_count_string == 2);
        db.erase(1, 1);
        TEST_ASSERT(db.event_handler().block_count == 1);
    }

    {
        mtv_type db(3);
        db.set(1, 1.1);
        TEST_ASSERT(db.event_handler().block_count == 1);
        db.erase(1, 1);
        TEST_ASSERT(db.event_handler().block_count == 0);
    }

    {
        std::vector<double> vals = {1.1, 1.2};
        mtv_type db(4);
        db.set(0, 0.1);
        db.set(1, 0.2);
        db.set(2, std::string("foo"));
        db.set(3, std::string("bar"));
        TEST_ASSERT(db.event_handler().block_count == 2);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
        TEST_ASSERT(db.event_handler().block_count_string == 1);
        db.set(2, vals.begin(), vals.end()); // remove a block and append to previous one.
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
    }

    {
        std::vector<double> vals = {1.1, 1.2};
        mtv_type db(4);
        db.set(0, int32_t(5));
        db.set(1, int32_t(10));
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_int32 == 1);
        db.set(2, vals.begin(), vals.end()); // set to empty block.
        TEST_ASSERT(db.event_handler().block_count == 2);
        TEST_ASSERT(db.event_handler().block_count_int32 == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
    }

    {
        std::vector<double> vals = {1.1, 1.2};
        mtv_type db(4);
        db.set(0, int32_t(5));
        db.set(1, int32_t(10));
        db.set(2, std::string("foo"));
        db.set(3, std::string("bar"));
        TEST_ASSERT(db.event_handler().block_count == 2);
        TEST_ASSERT(db.event_handler().block_count_int32 == 1);
        TEST_ASSERT(db.event_handler().block_count_string == 1);
        db.set(2, vals.begin(), vals.end()); // replace a block.
        TEST_ASSERT(db.event_handler().block_count == 2);
        TEST_ASSERT(db.event_handler().block_count_int32 == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
    }

    {
        std::vector<double> vals = {1.1, 1.2};
        mtv_type db(4, std::string("foo"));
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_string == 1);
        db.set(0, vals.begin(), vals.end()); // replace the upper part of a block.
        TEST_ASSERT(db.event_handler().block_count == 2);
        TEST_ASSERT(db.event_handler().block_count_string == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
    }

    {
        std::vector<double> vals = {1.1, 1.2};
        mtv_type db(4, std::string("foo"));
        TEST_ASSERT(db.event_handler().block_count == 1);
        db.set(2, vals.begin(), vals.end()); // replace the lower part of the last block.
        TEST_ASSERT(db.event_handler().block_count == 2);
    }

    {
        std::vector<double> vals = {1.1, 1.2};
        mtv_type db(4, std::string("foo"));
        db.push_back(int64_t(100));
        TEST_ASSERT(db.event_handler().block_count == 2);
        db.set(2, vals.begin(), vals.end()); // replace the lower part of a block.
        TEST_ASSERT(db.event_handler().block_count == 3);
        TEST_ASSERT(db.event_handler().block_count_string == 1);
        TEST_ASSERT(db.event_handler().block_count_int64 == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
    }

    {
        std::vector<double> vals = {1.1, 1.2};
        mtv_type db(6, std::string("foo"));
        TEST_ASSERT(db.event_handler().block_count == 1);
        db.set(2, vals.begin(), vals.end()); // set the values to the middle of a block.
        TEST_ASSERT(db.event_handler().block_count == 3);
        TEST_ASSERT(db.event_handler().block_count_string == 2);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
    }

    {
        mtv_type db(1, 0.1);
        db.push_back(int16_t(1));
        db.push_back(int32_t(20));
        TEST_ASSERT(db.event_handler().block_count == 3);
        TEST_ASSERT(db.event_handler().block_count_int16 == 1);
        TEST_ASSERT(db.event_handler().block_count_int32 == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);

        std::vector<double> vals = {1.1, 1.2, 1.3}; // same type as the top block.
        db.set(0, vals.begin(), vals.end()); // overwrite multiple blocks.
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
    }

    {
        mtv_type db(1, std::string("foo"));
        db.push_back(int16_t(1));
        db.push_back(int32_t(20));
        TEST_ASSERT(db.event_handler().block_count == 3);
        TEST_ASSERT(db.event_handler().block_count_int16 == 1);
        TEST_ASSERT(db.event_handler().block_count_int32 == 1);
        TEST_ASSERT(db.event_handler().block_count_string == 1);

        std::vector<double> vals = {1.1, 1.2, 1.3}; // differene type from that of the top block.
        db.set(0, vals.begin(), vals.end()); // overwrite multiple blocks.
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
    }

    {
        mtv_type db(6);
        db.set(2, 1.1);
        db.set(3, int32_t(22));
        TEST_ASSERT(db.event_handler().block_count == 2);
        TEST_ASSERT(db.event_handler().block_count_int32 == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
        db.erase(2, 3);
        TEST_ASSERT(db.event_handler().block_count == 0);
    }

    {
        mtv_type db(6, int8_t('a'));
        db.set(2, 1.1);
        db.set(3, int32_t(22));
        TEST_ASSERT(db.event_handler().block_count == 4);
        TEST_ASSERT(db.event_handler().block_count_int32 == 1);
        TEST_ASSERT(db.event_handler().block_count_numeric == 1);
        TEST_ASSERT(db.event_handler().block_count_int8 == 2);
        db.erase(2, 3);
        TEST_ASSERT(db.event_handler().block_count == 1);
        TEST_ASSERT(db.event_handler().block_count_int8 == 1);
    }

    {
        mtv_type src(6, int8_t('a')), dst(6);
        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count == 0);
        src.transfer(0, 2, dst, 0);
        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count == 1);

        src.transfer(3, 5, dst, 3);
        TEST_ASSERT(src.event_handler().block_count == 0);
        TEST_ASSERT(dst.event_handler().block_count == 1);
    }

    {
        mtv_type src(6), dst(6);
        src.set(0, int8_t('z'));
        src.set(1, int32_t(10));
        src.set(2, int16_t(5));
        dst.set(3, 1.1);
        TEST_ASSERT(src.event_handler().block_count == 3);
        TEST_ASSERT(src.event_handler().block_count_int8 == 1);
        TEST_ASSERT(src.event_handler().block_count_int32 == 1);
        TEST_ASSERT(src.event_handler().block_count_int16 == 1);
        TEST_ASSERT(dst.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count_numeric == 1);

        src.transfer(0, 2, dst, 0);
        TEST_ASSERT(src.event_handler().block_count == 0);
        TEST_ASSERT(dst.event_handler().block_count == 4);
        TEST_ASSERT(dst.event_handler().block_count_numeric == 1);
        TEST_ASSERT(dst.event_handler().block_count_int8 == 1);
        TEST_ASSERT(dst.event_handler().block_count_int32 == 1);
        TEST_ASSERT(dst.event_handler().block_count_int16 == 1);
    }

    {
        mtv_type src(6), dst(6);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);
        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count == 0);

        src.transfer(1, 3, dst, 1);
        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count == 1);
    }

    {
        mtv_type src(6), dst(6);
        src.set(3, 1.1);
        src.set(4, 1.2);
        src.set(5, 1.3);
        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count == 0);

        src.transfer(1, 3, dst, 1);
        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count == 1);
    }

    {
        mtv_type src(3), dst(3);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);

        dst.set(0, std::string("2.1"));
        dst.set(1, std::string("2.2"));
        dst.set(2, std::string("2.3"));

        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count == 1);

        src.swap(0, 2, dst, 0);
        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count == 1);
    }

    {
        mtv_type src(4), dst(3);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);
        src.set(3, 1.4);

        dst.set(0, std::string("2.1"));
        dst.set(1, std::string("2.2"));
        dst.set(2, std::string("2.3"));

        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count == 1);

        src.swap(0, 2, dst, 0);
        TEST_ASSERT(src.event_handler().block_count == 2);
        TEST_ASSERT(dst.event_handler().block_count == 1);
    }

    {
        mtv_type src(5), dst(3);
        src.set(0, std::string("2.0"));
        src.set(1, 1.1);
        src.set(2, 1.2);
        src.set(3, 1.3);
        src.set(4, 1.4);

        dst.set(0, std::string("2.1"));
        dst.set(1, std::string("2.2"));
        dst.set(2, std::string("2.3"));

        TEST_ASSERT(src.event_handler().block_count == 2);
        TEST_ASSERT(dst.event_handler().block_count == 1);

        src.swap(1, 3, dst, 0);
        TEST_ASSERT(src.event_handler().block_count == 2);
        TEST_ASSERT(dst.event_handler().block_count == 1);
    }

    {
        mtv_type src(4), dst(3);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);
        src.set(3, 1.4);

        dst.set(0, std::string("2.1"));
        dst.set(1, std::string("2.2"));
        dst.set(2, std::string("2.3"));

        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count == 1);

        src.swap(1, 3, dst, 0);
        TEST_ASSERT(src.event_handler().block_count == 2);
        TEST_ASSERT(dst.event_handler().block_count == 1);
    }

    {
        mtv_type src(5), dst(3);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);
        src.set(3, 1.4);
        src.set(4, std::string("2.4"));

        dst.set(0, std::string("2.1"));
        dst.set(1, std::string("2.2"));
        dst.set(2, std::string("2.3"));

        TEST_ASSERT(src.event_handler().block_count == 2);
        TEST_ASSERT(dst.event_handler().block_count == 1);

        src.swap(1, 3, dst, 0);
        TEST_ASSERT(src.event_handler().block_count == 2);
        TEST_ASSERT(dst.event_handler().block_count == 1);
    }

    {
        mtv_type src(5), dst(3);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);
        src.set(3, 1.4);
        src.set(4, 1.5);

        dst.set(0, std::string("2.1"));
        dst.set(1, std::string("2.2"));
        dst.set(2, std::string("2.3"));

        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count == 1);

        src.swap(1, 3, dst, 0);
        TEST_ASSERT(src.event_handler().block_count == 3);
        TEST_ASSERT(dst.event_handler().block_count == 1);
    }

    {
        mtv_type src(3), dst(3);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);

        dst.set(0, std::string("2.1"));
        dst.set(1, int32_t(100));
        dst.set(2, std::string("2.3"));

        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(src.event_handler().block_count_numeric == 1);
        TEST_ASSERT(dst.event_handler().block_count == 3);
        TEST_ASSERT(dst.event_handler().block_count_string == 2);
        TEST_ASSERT(dst.event_handler().block_count_int32 == 1);

        src.swap(0, 2, dst, 0);
        TEST_ASSERT(src.event_handler().block_count == 3);
        TEST_ASSERT(src.event_handler().block_count_string == 2);
        TEST_ASSERT(src.event_handler().block_count_int32 == 1);
        TEST_ASSERT(dst.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count_numeric == 1);
    }

    {
        mtv_type src(5), dst(5);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);
        src.set(3, 1.4);
        src.set(4, 1.5);

        dst.set(0, std::string("2.1"));
        dst.set(1, std::string("2.2"));
        dst.set(2, int32_t(100));
        dst.set(3, std::string("2.3"));
        dst.set(4, std::string("2.4"));

        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(src.event_handler().block_count_numeric == 1);
        TEST_ASSERT(dst.event_handler().block_count == 3);
        TEST_ASSERT(dst.event_handler().block_count_string == 2);
        TEST_ASSERT(dst.event_handler().block_count_int32 == 1);

        src.swap(1, 3, dst, 1);
        TEST_ASSERT(src.event_handler().block_count == 5);
        TEST_ASSERT(src.event_handler().block_count_numeric == 2);
        TEST_ASSERT(src.event_handler().block_count_string == 2);
        TEST_ASSERT(src.event_handler().block_count_int32 == 1);
        TEST_ASSERT(dst.event_handler().block_count == 3);
        TEST_ASSERT(dst.event_handler().block_count_string == 2);
        TEST_ASSERT(dst.event_handler().block_count_numeric == 1);
    }

    {
        mtv_type src(3), dst(4);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);

        dst.set(0, 2.1);
        dst.set(1, int32_t(5));
        dst.set(2, int32_t(6));
        dst.set(3, int32_t(7));

        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(src.event_handler().block_count_numeric == 1);
        TEST_ASSERT(dst.event_handler().block_count == 2);
        TEST_ASSERT(dst.event_handler().block_count_numeric == 1);
        TEST_ASSERT(dst.event_handler().block_count_int32 == 1);

        src.swap(0, 2, dst, 1);
        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(src.event_handler().block_count_int32 == 1);
        TEST_ASSERT(dst.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count_numeric == 1);
    }

    {
        mtv_type src(3), dst(5);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);

        dst.set(0, 2.1);
        dst.set(1, int32_t(5));
        dst.set(2, int32_t(6));
        dst.set(3, int32_t(7));
        dst.set(4, 2.2);

        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(src.event_handler().block_count_numeric == 1);
        TEST_ASSERT(dst.event_handler().block_count == 3);
        TEST_ASSERT(dst.event_handler().block_count_numeric == 2);
        TEST_ASSERT(dst.event_handler().block_count_int32 == 1);

        src.swap(0, 2, dst, 1);
        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(src.event_handler().block_count_int32 == 1);
        TEST_ASSERT(dst.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count_numeric == 1);
    }

    {
        mtv_type src(3), dst(4);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);

        dst.set(0, int32_t(5));
        dst.set(1, int32_t(6));
        dst.set(2, int32_t(7));
        dst.set(3, 2.1);

        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(src.event_handler().block_count_numeric == 1);
        TEST_ASSERT(dst.event_handler().block_count == 2);
        TEST_ASSERT(dst.event_handler().block_count_int32 == 1);
        TEST_ASSERT(dst.event_handler().block_count_numeric == 1);

        src.swap(0, 2, dst, 0);
        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(src.event_handler().block_count_int32 == 1);
        TEST_ASSERT(dst.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count_numeric == 1);
    }

    {
        mtv_type src(3), dst(4);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);

        dst.set(0, int32_t(5));
        dst.set(1, int32_t(6));
        dst.set(2, int32_t(7));
        dst.set(3, int32_t(8));

        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count == 1);

        src.swap(0, 2, dst, 0);
        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count == 2);
        TEST_ASSERT(dst.event_handler().block_count_int32 == 1);
        TEST_ASSERT(dst.event_handler().block_count_numeric == 1);
    }

    {
        mtv_type src(3), dst(5);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);

        dst.set(0, 2.1);
        dst.set(1, int32_t(5));
        dst.set(2, int32_t(6));
        dst.set(3, int32_t(7));
        dst.set(4, int32_t(8));

        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(src.event_handler().block_count_numeric == 1);
        TEST_ASSERT(dst.event_handler().block_count == 2);
        TEST_ASSERT(dst.event_handler().block_count_numeric == 1);
        TEST_ASSERT(dst.event_handler().block_count_int32 == 1);

        src.swap(0, 2, dst, 1);
        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(src.event_handler().block_count_int32 == 1);
        TEST_ASSERT(dst.event_handler().block_count == 2);
        TEST_ASSERT(dst.event_handler().block_count_numeric == 1);
        TEST_ASSERT(dst.event_handler().block_count_int32 == 1);
    }

    {
        mtv_type src(3), dst(5);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);

        dst.set(0, int32_t(4));
        dst.set(1, int32_t(5));
        dst.set(2, int32_t(6));
        dst.set(3, int32_t(7));
        dst.set(4, int32_t(8));

        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count == 1);

        src.swap(0, 2, dst, 1);
        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(src.event_handler().block_count_int32 == 1);
        TEST_ASSERT(dst.event_handler().block_count == 3);
        TEST_ASSERT(dst.event_handler().block_count_int32 == 2);
        TEST_ASSERT(dst.event_handler().block_count_numeric == 1);
    }

    {
        mtv_type src(3), dst(4);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);

        dst.set(0, int32_t(4));
        dst.set(1, int32_t(5));
        dst.set(2, int32_t(6));
        dst.set(3, int32_t(7));

        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(src.event_handler().block_count_numeric == 1);
        TEST_ASSERT(dst.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count_int32 == 1);

        src.swap(0, 2, dst, 1);
        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(src.event_handler().block_count_int32 == 1);
        TEST_ASSERT(dst.event_handler().block_count == 2);
        TEST_ASSERT(dst.event_handler().block_count_int32 == 1);
        TEST_ASSERT(dst.event_handler().block_count_numeric == 1);
    }

    {
        mtv_type src(3), dst(5);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);

        dst.set(0, int32_t(4));
        dst.set(1, int32_t(5));
        dst.set(2, int32_t(6));
        dst.set(3, int32_t(7));
        dst.set(4, 2.1);

        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(src.event_handler().block_count_numeric == 1);
        TEST_ASSERT(dst.event_handler().block_count == 2);
        TEST_ASSERT(dst.event_handler().block_count_numeric == 1);
        TEST_ASSERT(dst.event_handler().block_count_int32 == 1);

        src.swap(0, 2, dst, 1);
        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(src.event_handler().block_count_int32 == 1);
        TEST_ASSERT(dst.event_handler().block_count == 2);
        TEST_ASSERT(dst.event_handler().block_count_int32 == 1);
        TEST_ASSERT(dst.event_handler().block_count_numeric == 1);
    }

    {
        mtv_type src(3), dst(5);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);

        dst.set(0, 2.1);
        dst.set(4, 2.2);

        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count == 2);

        src.transfer(0, 2, dst, 1);
        TEST_ASSERT(src.event_handler().block_count == 0);
        TEST_ASSERT(dst.event_handler().block_count == 1);
    }

    {
        mtv_type src(20), dst(20);
        src.set(9, 1.2);

        dst.set(10, 2.1);
        dst.set(11, 2.2);

        TEST_ASSERT(src.event_handler().block_count == 1);
        TEST_ASSERT(dst.event_handler().block_count == 1);

        src.transfer(9, 9, dst, 9);
        TEST_ASSERT(src.event_handler().block_count == 0);
        TEST_ASSERT(dst.event_handler().block_count == 1);
    }

    {
        mtv_type db1(10);
        db1.set<int32_t>(0, 2);
        db1.set<int32_t>(1, 3);
        db1.set<int32_t>(2, 4);
        db1.set<std::string>(3, "A");
        db1.set<std::string>(4, "B");
        db1.set<std::string>(5, "C");

        // Leave some empty range.
        mtv_type db2(10);
        for (int32_t i = 0; i < 10; ++i)
            db2.set<int32_t>(i, 10 + i);
        db2.set<int8_t>(5, 'Z');

        TEST_ASSERT(db1.event_handler().block_count == 2);
        TEST_ASSERT(db1.event_handler().block_count_int32 == 1);
        TEST_ASSERT(db1.event_handler().block_count_string == 1);

        TEST_ASSERT(db2.event_handler().block_count == 3);
        TEST_ASSERT(db2.event_handler().block_count_int32 == 2);
        TEST_ASSERT(db2.event_handler().block_count_int8 == 1);

        db1.swap(1, 7, db2, 2);

        TEST_ASSERT(db1.event_handler().block_count == 3);
        TEST_ASSERT(db1.event_handler().block_count_int32 == 2);
        TEST_ASSERT(db1.event_handler().block_count_string == 0);
        TEST_ASSERT(db1.event_handler().block_count_int8 == 1);

        TEST_ASSERT(db2.event_handler().block_count == 3);
        TEST_ASSERT(db2.event_handler().block_count_int32 == 2);
        TEST_ASSERT(db2.event_handler().block_count_int8 == 0);
        TEST_ASSERT(db2.event_handler().block_count_string == 1);
    }
}

void mtv_test_block_counter_clone()
{
    MDDS_TEST_FUNC_SCOPE;

    using mtv_type = mtv_template_type<eb_counter_trait>;

    mtv_type src;
    src.emplace_back<std::string>("v1");
    src.emplace_back<std::string>("v2");
    src.emplace_back<std::string>("v3");
    TEST_ASSERT(src.event_handler().block_count_string == 1);
    src.push_back_empty();
    src.emplace_back<std::string>("v4");
    TEST_ASSERT(src.event_handler().block_count_string == 2);

    auto cloned = src.clone();
    TEST_ASSERT(cloned.event_handler().block_count_string == 2);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
