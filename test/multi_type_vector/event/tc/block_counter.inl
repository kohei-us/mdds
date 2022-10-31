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
    stack_printer __stack_printer__(__FUNCTION__);

    using mtv_type = mtv_template_type<eb_counter_trait>;

    {
        // Initializing with an empty block should not create any element block.
        mtv_type db(10);
        assert(db.event_handler().block_count == 0);
    }

    {
        // Initializing with one element block of size 10.
        mtv_type db(10, 1.2);
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_numeric == 1);
        db.clear();
        assert(db.event_handler().block_count == 0);
        assert(db.event_handler().block_count_numeric == 0);

        db.push_back(5.5); // create a new block.
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_numeric == 1);
        db.push_back(6.6); // no new block creation.
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_numeric == 1);
        db.push_back(std::string("foo")); // another new block.
        assert(db.event_handler().block_count == 2);
        assert(db.event_handler().block_count_numeric == 1);
        assert(db.event_handler().block_count_string == 1);

        // This should remove the last string block.
        db.resize(2);
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_numeric == 1);
        assert(db.event_handler().block_count_string == 0);

        // This should have no effect on the block count.
        db.resize(1);
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_numeric == 1);

        // This should remove the last remaining block.
        db.resize(0);
        assert(db.event_handler().block_count == 0);
        assert(db.event_handler().block_count_numeric == 0);
    }

    {
        mtv_type db(5);
        assert(db.event_handler().block_count == 0);

        db.set(0, true);
        assert(db.event_handler().block_count == 1);
        db.set(1, 12.2);
        assert(db.event_handler().block_count == 2);
        assert(db.event_handler().block_count_boolean == 1);
        assert(db.event_handler().block_count_numeric == 1);

        db.set(4, std::string("foo"));
        assert(db.event_handler().block_count == 3);
        assert(db.event_handler().block_count_string == 1);
        db.set(3, std::string("bar"));
        assert(db.event_handler().block_count == 3);
        assert(db.event_handler().block_count_string == 1);

        // This should delete the top two element blocks.
        db.set_empty(0, 1);
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_boolean == 0);
        assert(db.event_handler().block_count_numeric == 0);

        // Now, delete the bottom one.
        db.set_empty(3, 4);
        assert(db.event_handler().block_count == 0);
        assert(db.event_handler().block_count_string == 0);

        // Create and delete a block in the middle.
        db.set(3, false);
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_boolean == 1);
        db.set_empty(3, 3);
        assert(db.event_handler().block_count == 0);
        assert(db.event_handler().block_count_boolean == 0);

        db.set(2, 10.5);
        db.set(3, std::string("hmm"));
        assert(db.event_handler().block_count == 2);
        assert(db.event_handler().block_count_numeric == 1);
        assert(db.event_handler().block_count_string == 1);
        db.set_empty(3, 3);
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_numeric == 1);
        assert(db.event_handler().block_count_string == 0);

        // Start over.
        db.clear();
        assert(db.event_handler().block_count == 0);

        db.push_back(1.1);
        db.push_back(1.2);
        db.push_back(1.3);
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_numeric == 1);

        // Put empty block in the middle.
        db.set_empty(1, 1);
        assert(db.event_handler().block_count == 2);
        assert(db.event_handler().block_count_numeric == 2);
    }

    {
        mtv_type db(4, 1.2);
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_numeric == 1);

        // Split the block into two.
        db.insert_empty(2, 2);
        assert(db.event_handler().block_count == 2);
        assert(db.event_handler().block_count_numeric == 2);
    }

    {
        mtv_type db(2);
        db.set(1, 1.2); // This creates a new element block.
        assert(db.event_handler().block_count == 1);
        db.set(0, 1.1); // The element block count should not change.
        assert(db.event_handler().block_count == 1);
    }

    {
        mtv_type db(2);
        db.set(1, std::string("test"));
        assert(db.event_handler().block_count == 1);
        db.set(0, 1.1);
        assert(db.event_handler().block_count == 2);
        db.set(0, true);
        assert(db.event_handler().block_count == 2);
        assert(db.event_handler().block_count_boolean == 1);
        assert(db.event_handler().block_count_string == 1);

        db.set(0, std::string("foo"));
        assert(db.event_handler().block_count == 1);

        db.set(1, 1.2);
        assert(db.event_handler().block_count == 2);
        db.set(1, std::string("bar"));
        assert(db.event_handler().block_count == 1);
    }

    {
        mtv_type db(2);
        db.set(0, std::string("test")); // This creates a new string block.
        assert(db.event_handler().block_count == 1);
        db.set(1, std::string("foo")); // This appends to the existing string block.
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_string == 1);
    }

    {
        mtv_type db(3);
        db.set(0, std::string("test"));
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_string == 1);
        db.set(2, std::string("foo"));
        assert(db.event_handler().block_count == 2);
        assert(db.event_handler().block_count_string == 2);
        db.set(1, std::string("bar")); // This merges all data into a single string block.
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_string == 1);
    }

    {
        mtv_type db(4);
        db.set(0, std::string("test"));
        assert(db.event_handler().block_count == 1);
        db.set(2, std::string("foo1"));
        assert(db.event_handler().block_count == 2);
        db.set(3, std::string("foo2"));
        assert(db.event_handler().block_count == 2);
        db.set(1, std::string("bar")); // This merges all data into a single string block.
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_string == 1);
    }

    {
        mtv_type db(3);
        db.set(0, std::string("test"));
        assert(db.event_handler().block_count == 1);
        db.set(2, 1.2);
        assert(db.event_handler().block_count == 2);
        db.set(1, std::string("bar"));
        assert(db.event_handler().block_count == 2);
        assert(db.event_handler().block_count_numeric == 1);
        assert(db.event_handler().block_count_string == 1);
    }

    {
        mtv_type db(3);
        db.set(0, std::string("test"));
        assert(db.event_handler().block_count == 1);
        db.set(2, 1.2);
        assert(db.event_handler().block_count == 2);
        db.set(1, 1.1); // This will get prepended to the next numeric block.
        assert(db.event_handler().block_count == 2);
        assert(db.event_handler().block_count_numeric == 1);
        assert(db.event_handler().block_count_string == 1);
    }

    {
        std::vector<double> vals = {1.1, 1.2, 1.3};
        mtv_type db(vals.size(), vals.begin(), vals.end());
        assert(db.event_handler().block_count == 1);

        mtv_type db2(db);
        assert(db2.event_handler().block_count == 1);
        db2.push_back(std::string("foo"));
        assert(db2.event_handler().block_count == 2);
        mtv_type db3 = db2;
        assert(db3.event_handler().block_count == 2);

        mtv_type db4(3);
        db4.insert(0, vals.begin(), vals.end());
        assert(db4.event_handler().block_count == 1);

        mtv_type db5(3, int64_t(10));
        assert(db5.event_handler().block_count == 1);
        db5.insert(0, vals.begin(), vals.end());
        assert(db5.event_handler().block_count == 2);

        mtv_type db6(2, int32_t(30));
        assert(db6.event_handler().block_count == 1);
        db6.insert(1, vals.begin(), vals.end()); // Insert to split the block.
        assert(db6.event_handler().block_count == 3);
        assert(db6.event_handler().block_count_int32 == 2);
        assert(db6.event_handler().block_count_numeric == 1);
    }

    {
        mtv_type db(3);
        db.set(1, 1.1);
        db.set(2, true);
        assert(db.event_handler().block_count == 2);
        db.set(1, false);
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_boolean == 1);
    }

    {
        mtv_type db(3);
        db.set(1, 1.1);
        db.set(0, true);
        assert(db.event_handler().block_count == 2);
        assert(db.event_handler().block_count_boolean == 1);
        assert(db.event_handler().block_count_numeric == 1);
        db.set(1, false);
        assert(db.event_handler().block_count == 1);
    }

    {
        mtv_type db(3);
        db.set(0, true);
        db.set(1, 1.1);
        db.set(2, false);
        assert(db.event_handler().block_count == 3);
        assert(db.event_handler().block_count_boolean == 2);
        assert(db.event_handler().block_count_numeric == 1);
        db.set(1, true);
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_boolean == 1);

        db.set(1, 1.1);
        assert(db.event_handler().block_count == 3);
        db.set(2, int64_t(10));
        db.set(1, true);
        assert(db.event_handler().block_count == 2);
        assert(db.event_handler().block_count_boolean == 1);
        assert(db.event_handler().block_count_int64 == 1);

        db.set(1, 1.1);
        assert(db.event_handler().block_count == 3);
        assert(db.event_handler().block_count_boolean == 1);
        assert(db.event_handler().block_count_int64 == 1);
        assert(db.event_handler().block_count_numeric == 1);
        db.set(1, int64_t(20));
        assert(db.event_handler().block_count == 2);

        db.release();
        assert(db.event_handler().block_count == 0);
    }

    {
        mtv_type db;
        db.push_back(1.1);
        db.push_back(int64_t(10));
        db.push_back(std::string("foo"));
        assert(db.event_handler().block_count == 3);
        assert(db.event_handler().block_count_int64 == 1);
        assert(db.event_handler().block_count_numeric == 1);
        assert(db.event_handler().block_count_string == 1);

        db.erase(0, 2);
        assert(db.event_handler().block_count == 0);
    }

    {
        mtv_type db;
        db.push_back(1.1);
        db.push_back_empty();
        assert(db.event_handler().block_count == 1);
        db.erase(0, 0);
        assert(db.event_handler().block_count == 0);
    }

    {
        mtv_type db(3);
        db.set(0, std::string("top"));
        db.set(2, std::string("bottom"));
        assert(db.event_handler().block_count == 2);
        assert(db.event_handler().block_count_string == 2);
        db.erase(1, 1);
        assert(db.event_handler().block_count == 1);
    }

    {
        mtv_type db(3);
        db.set(1, 1.1);
        assert(db.event_handler().block_count == 1);
        db.erase(1, 1);
        assert(db.event_handler().block_count == 0);
    }

    {
        std::vector<double> vals = {1.1, 1.2};
        mtv_type db(4);
        db.set(0, 0.1);
        db.set(1, 0.2);
        db.set(2, std::string("foo"));
        db.set(3, std::string("bar"));
        assert(db.event_handler().block_count == 2);
        assert(db.event_handler().block_count_numeric == 1);
        assert(db.event_handler().block_count_string == 1);
        db.set(2, vals.begin(), vals.end()); // remove a block and append to previous one.
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_numeric == 1);
    }

    {
        std::vector<double> vals = {1.1, 1.2};
        mtv_type db(4);
        db.set(0, int32_t(5));
        db.set(1, int32_t(10));
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_int32 == 1);
        db.set(2, vals.begin(), vals.end()); // set to empty block.
        assert(db.event_handler().block_count == 2);
        assert(db.event_handler().block_count_int32 == 1);
        assert(db.event_handler().block_count_numeric == 1);
    }

    {
        std::vector<double> vals = {1.1, 1.2};
        mtv_type db(4);
        db.set(0, int32_t(5));
        db.set(1, int32_t(10));
        db.set(2, std::string("foo"));
        db.set(3, std::string("bar"));
        assert(db.event_handler().block_count == 2);
        assert(db.event_handler().block_count_int32 == 1);
        assert(db.event_handler().block_count_string == 1);
        db.set(2, vals.begin(), vals.end()); // replace a block.
        assert(db.event_handler().block_count == 2);
        assert(db.event_handler().block_count_int32 == 1);
        assert(db.event_handler().block_count_numeric == 1);
    }

    {
        std::vector<double> vals = {1.1, 1.2};
        mtv_type db(4, std::string("foo"));
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_string == 1);
        db.set(0, vals.begin(), vals.end()); // replace the upper part of a block.
        assert(db.event_handler().block_count == 2);
        assert(db.event_handler().block_count_string == 1);
        assert(db.event_handler().block_count_numeric == 1);
    }

    {
        std::vector<double> vals = {1.1, 1.2};
        mtv_type db(4, std::string("foo"));
        assert(db.event_handler().block_count == 1);
        db.set(2, vals.begin(), vals.end()); // replace the lower part of the last block.
        assert(db.event_handler().block_count == 2);
    }

    {
        std::vector<double> vals = {1.1, 1.2};
        mtv_type db(4, std::string("foo"));
        db.push_back(int64_t(100));
        assert(db.event_handler().block_count == 2);
        db.set(2, vals.begin(), vals.end()); // replace the lower part of a block.
        assert(db.event_handler().block_count == 3);
        assert(db.event_handler().block_count_string == 1);
        assert(db.event_handler().block_count_int64 == 1);
        assert(db.event_handler().block_count_numeric == 1);
    }

    {
        std::vector<double> vals = {1.1, 1.2};
        mtv_type db(6, std::string("foo"));
        assert(db.event_handler().block_count == 1);
        db.set(2, vals.begin(), vals.end()); // set the values to the middle of a block.
        assert(db.event_handler().block_count == 3);
        assert(db.event_handler().block_count_string == 2);
        assert(db.event_handler().block_count_numeric == 1);
    }

    {
        mtv_type db(1, 0.1);
        db.push_back(int16_t(1));
        db.push_back(int32_t(20));
        assert(db.event_handler().block_count == 3);
        assert(db.event_handler().block_count_int16 == 1);
        assert(db.event_handler().block_count_int32 == 1);
        assert(db.event_handler().block_count_numeric == 1);

        std::vector<double> vals = {1.1, 1.2, 1.3}; // same type as the top block.
        db.set(0, vals.begin(), vals.end()); // overwrite multiple blocks.
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_numeric == 1);
    }

    {
        mtv_type db(1, std::string("foo"));
        db.push_back(int16_t(1));
        db.push_back(int32_t(20));
        assert(db.event_handler().block_count == 3);
        assert(db.event_handler().block_count_int16 == 1);
        assert(db.event_handler().block_count_int32 == 1);
        assert(db.event_handler().block_count_string == 1);

        std::vector<double> vals = {1.1, 1.2, 1.3}; // differene type from that of the top block.
        db.set(0, vals.begin(), vals.end()); // overwrite multiple blocks.
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_numeric == 1);
    }

    {
        mtv_type db(6);
        db.set(2, 1.1);
        db.set(3, int32_t(22));
        assert(db.event_handler().block_count == 2);
        assert(db.event_handler().block_count_int32 == 1);
        assert(db.event_handler().block_count_numeric == 1);
        db.erase(2, 3);
        assert(db.event_handler().block_count == 0);
    }

    {
        mtv_type db(6, int8_t('a'));
        db.set(2, 1.1);
        db.set(3, int32_t(22));
        assert(db.event_handler().block_count == 4);
        assert(db.event_handler().block_count_int32 == 1);
        assert(db.event_handler().block_count_numeric == 1);
        assert(db.event_handler().block_count_int8 == 2);
        db.erase(2, 3);
        assert(db.event_handler().block_count == 1);
        assert(db.event_handler().block_count_int8 == 1);
    }

    {
        mtv_type src(6, int8_t('a')), dst(6);
        assert(src.event_handler().block_count == 1);
        assert(dst.event_handler().block_count == 0);
        src.transfer(0, 2, dst, 0);
        assert(src.event_handler().block_count == 1);
        assert(dst.event_handler().block_count == 1);

        src.transfer(3, 5, dst, 3);
        assert(src.event_handler().block_count == 0);
        assert(dst.event_handler().block_count == 1);
    }

    {
        mtv_type src(6), dst(6);
        src.set(0, int8_t('z'));
        src.set(1, int32_t(10));
        src.set(2, int16_t(5));
        dst.set(3, 1.1);
        assert(src.event_handler().block_count == 3);
        assert(src.event_handler().block_count_int8 == 1);
        assert(src.event_handler().block_count_int32 == 1);
        assert(src.event_handler().block_count_int16 == 1);
        assert(dst.event_handler().block_count == 1);
        assert(dst.event_handler().block_count_numeric == 1);

        src.transfer(0, 2, dst, 0);
        assert(src.event_handler().block_count == 0);
        assert(dst.event_handler().block_count == 4);
        assert(dst.event_handler().block_count_numeric == 1);
        assert(dst.event_handler().block_count_int8 == 1);
        assert(dst.event_handler().block_count_int32 == 1);
        assert(dst.event_handler().block_count_int16 == 1);
    }

    {
        mtv_type src(6), dst(6);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);
        assert(src.event_handler().block_count == 1);
        assert(dst.event_handler().block_count == 0);

        src.transfer(1, 3, dst, 1);
        assert(src.event_handler().block_count == 1);
        assert(dst.event_handler().block_count == 1);
    }

    {
        mtv_type src(6), dst(6);
        src.set(3, 1.1);
        src.set(4, 1.2);
        src.set(5, 1.3);
        assert(src.event_handler().block_count == 1);
        assert(dst.event_handler().block_count == 0);

        src.transfer(1, 3, dst, 1);
        assert(src.event_handler().block_count == 1);
        assert(dst.event_handler().block_count == 1);
    }

    {
        mtv_type src(3), dst(3);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);

        dst.set(0, std::string("2.1"));
        dst.set(1, std::string("2.2"));
        dst.set(2, std::string("2.3"));

        assert(src.event_handler().block_count == 1);
        assert(dst.event_handler().block_count == 1);

        src.swap(0, 2, dst, 0);
        assert(src.event_handler().block_count == 1);
        assert(dst.event_handler().block_count == 1);
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

        assert(src.event_handler().block_count == 1);
        assert(dst.event_handler().block_count == 1);

        src.swap(0, 2, dst, 0);
        assert(src.event_handler().block_count == 2);
        assert(dst.event_handler().block_count == 1);
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

        assert(src.event_handler().block_count == 2);
        assert(dst.event_handler().block_count == 1);

        src.swap(1, 3, dst, 0);
        assert(src.event_handler().block_count == 2);
        assert(dst.event_handler().block_count == 1);
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

        assert(src.event_handler().block_count == 1);
        assert(dst.event_handler().block_count == 1);

        src.swap(1, 3, dst, 0);
        assert(src.event_handler().block_count == 2);
        assert(dst.event_handler().block_count == 1);
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

        assert(src.event_handler().block_count == 2);
        assert(dst.event_handler().block_count == 1);

        src.swap(1, 3, dst, 0);
        assert(src.event_handler().block_count == 2);
        assert(dst.event_handler().block_count == 1);
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

        assert(src.event_handler().block_count == 1);
        assert(dst.event_handler().block_count == 1);

        src.swap(1, 3, dst, 0);
        assert(src.event_handler().block_count == 3);
        assert(dst.event_handler().block_count == 1);
    }

    {
        mtv_type src(3), dst(3);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);

        dst.set(0, std::string("2.1"));
        dst.set(1, int32_t(100));
        dst.set(2, std::string("2.3"));

        assert(src.event_handler().block_count == 1);
        assert(src.event_handler().block_count_numeric == 1);
        assert(dst.event_handler().block_count == 3);
        assert(dst.event_handler().block_count_string == 2);
        assert(dst.event_handler().block_count_int32 == 1);

        src.swap(0, 2, dst, 0);
        assert(src.event_handler().block_count == 3);
        assert(src.event_handler().block_count_string == 2);
        assert(src.event_handler().block_count_int32 == 1);
        assert(dst.event_handler().block_count == 1);
        assert(dst.event_handler().block_count_numeric == 1);
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

        assert(src.event_handler().block_count == 1);
        assert(src.event_handler().block_count_numeric == 1);
        assert(dst.event_handler().block_count == 3);
        assert(dst.event_handler().block_count_string == 2);
        assert(dst.event_handler().block_count_int32 == 1);

        src.swap(1, 3, dst, 1);
        assert(src.event_handler().block_count == 5);
        assert(src.event_handler().block_count_numeric == 2);
        assert(src.event_handler().block_count_string == 2);
        assert(src.event_handler().block_count_int32 == 1);
        assert(dst.event_handler().block_count == 3);
        assert(dst.event_handler().block_count_string == 2);
        assert(dst.event_handler().block_count_numeric == 1);
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

        assert(src.event_handler().block_count == 1);
        assert(src.event_handler().block_count_numeric == 1);
        assert(dst.event_handler().block_count == 2);
        assert(dst.event_handler().block_count_numeric == 1);
        assert(dst.event_handler().block_count_int32 == 1);

        src.swap(0, 2, dst, 1);
        assert(src.event_handler().block_count == 1);
        assert(src.event_handler().block_count_int32 == 1);
        assert(dst.event_handler().block_count == 1);
        assert(dst.event_handler().block_count_numeric == 1);
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

        assert(src.event_handler().block_count == 1);
        assert(src.event_handler().block_count_numeric == 1);
        assert(dst.event_handler().block_count == 3);
        assert(dst.event_handler().block_count_numeric == 2);
        assert(dst.event_handler().block_count_int32 == 1);

        src.swap(0, 2, dst, 1);
        assert(src.event_handler().block_count == 1);
        assert(src.event_handler().block_count_int32 == 1);
        assert(dst.event_handler().block_count == 1);
        assert(dst.event_handler().block_count_numeric == 1);
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

        assert(src.event_handler().block_count == 1);
        assert(src.event_handler().block_count_numeric == 1);
        assert(dst.event_handler().block_count == 2);
        assert(dst.event_handler().block_count_int32 == 1);
        assert(dst.event_handler().block_count_numeric == 1);

        src.swap(0, 2, dst, 0);
        assert(src.event_handler().block_count == 1);
        assert(src.event_handler().block_count_int32 == 1);
        assert(dst.event_handler().block_count == 1);
        assert(dst.event_handler().block_count_numeric == 1);
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

        assert(src.event_handler().block_count == 1);
        assert(dst.event_handler().block_count == 1);

        src.swap(0, 2, dst, 0);
        assert(src.event_handler().block_count == 1);
        assert(dst.event_handler().block_count == 2);
        assert(dst.event_handler().block_count_int32 == 1);
        assert(dst.event_handler().block_count_numeric == 1);
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

        assert(src.event_handler().block_count == 1);
        assert(src.event_handler().block_count_numeric == 1);
        assert(dst.event_handler().block_count == 2);
        assert(dst.event_handler().block_count_numeric == 1);
        assert(dst.event_handler().block_count_int32 == 1);

        src.swap(0, 2, dst, 1);
        assert(src.event_handler().block_count == 1);
        assert(src.event_handler().block_count_int32 == 1);
        assert(dst.event_handler().block_count == 2);
        assert(dst.event_handler().block_count_numeric == 1);
        assert(dst.event_handler().block_count_int32 == 1);
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

        assert(src.event_handler().block_count == 1);
        assert(dst.event_handler().block_count == 1);

        src.swap(0, 2, dst, 1);
        assert(src.event_handler().block_count == 1);
        assert(src.event_handler().block_count_int32 == 1);
        assert(dst.event_handler().block_count == 3);
        assert(dst.event_handler().block_count_int32 == 2);
        assert(dst.event_handler().block_count_numeric == 1);
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

        assert(src.event_handler().block_count == 1);
        assert(src.event_handler().block_count_numeric == 1);
        assert(dst.event_handler().block_count == 1);
        assert(dst.event_handler().block_count_int32 == 1);

        src.swap(0, 2, dst, 1);
        assert(src.event_handler().block_count == 1);
        assert(src.event_handler().block_count_int32 == 1);
        assert(dst.event_handler().block_count == 2);
        assert(dst.event_handler().block_count_int32 == 1);
        assert(dst.event_handler().block_count_numeric == 1);
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

        assert(src.event_handler().block_count == 1);
        assert(src.event_handler().block_count_numeric == 1);
        assert(dst.event_handler().block_count == 2);
        assert(dst.event_handler().block_count_numeric == 1);
        assert(dst.event_handler().block_count_int32 == 1);

        src.swap(0, 2, dst, 1);
        assert(src.event_handler().block_count == 1);
        assert(src.event_handler().block_count_int32 == 1);
        assert(dst.event_handler().block_count == 2);
        assert(dst.event_handler().block_count_int32 == 1);
        assert(dst.event_handler().block_count_numeric == 1);
    }

    {
        mtv_type src(3), dst(5);
        src.set(0, 1.1);
        src.set(1, 1.2);
        src.set(2, 1.3);

        dst.set(0, 2.1);
        dst.set(4, 2.2);

        assert(src.event_handler().block_count == 1);
        assert(dst.event_handler().block_count == 2);

        src.transfer(0, 2, dst, 1);
        assert(src.event_handler().block_count == 0);
        assert(dst.event_handler().block_count == 1);
    }

    {
        mtv_type src(20), dst(20);
        src.set(9, 1.2);

        dst.set(10, 2.1);
        dst.set(11, 2.2);

        assert(src.event_handler().block_count == 1);
        assert(dst.event_handler().block_count == 1);

        src.transfer(9, 9, dst, 9);
        assert(src.event_handler().block_count == 0);
        assert(dst.event_handler().block_count == 1);
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

        assert(db1.event_handler().block_count == 2);
        assert(db1.event_handler().block_count_int32 == 1);
        assert(db1.event_handler().block_count_string == 1);

        assert(db2.event_handler().block_count == 3);
        assert(db2.event_handler().block_count_int32 == 2);
        assert(db2.event_handler().block_count_int8 == 1);

        db1.swap(1, 7, db2, 2);

        assert(db1.event_handler().block_count == 3);
        assert(db1.event_handler().block_count_int32 == 2);
        assert(db1.event_handler().block_count_string == 0);
        assert(db1.event_handler().block_count_int8 == 1);

        assert(db2.event_handler().block_count == 3);
        assert(db2.event_handler().block_count_int32 == 2);
        assert(db2.event_handler().block_count_int8 == 0);
        assert(db2.event_handler().block_count_string == 1);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
