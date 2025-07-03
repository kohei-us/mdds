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

void mtv_test_erase()
{
    MDDS_TEST_FUNC_SCOPE;
    {
        // Single empty block.
        mtv_type db(5);
        db.erase(0, 2); // erase rows 0-2.
        TEST_ASSERT(db.size() == 2);
        db.erase(0, 1);
        TEST_ASSERT(db.size() == 0);
        TEST_ASSERT(db.empty());
    }

    {
        // Single empty block followed by a non-empty block.
        mtv_type db(5);
        db.push_back<int32_t>(-234);

        db.erase(0, 2); // erase rows 0-2.
        TEST_ASSERT(db.size() == 3);
        db.erase(0, 1);
        TEST_ASSERT(db.size() == 1);
    }

    {
        // Single non-empty block.
        mtv_type db(5);
        for (long i = 0; i < 5; ++i)
            db.set(i, static_cast<double>(i + 1));

        TEST_ASSERT(db.block_size() == 1);
        TEST_ASSERT(db.size() == 5);

        db.erase(0, 2); // erase rows 0-2
        TEST_ASSERT(db.size() == 2);
        double test;
        db.get(0, test);
        TEST_ASSERT(test == 4.0);
        db.get(1, test);
        TEST_ASSERT(test == 5.0);

        db.erase(0, 1);
        TEST_ASSERT(db.size() == 0);
        TEST_ASSERT(db.empty());
    }

    {
        // Two blocks - non-empty to empty blocks.
        mtv_type db(8);
        for (long i = 0; i < 4; ++i)
            db.set(i, static_cast<double>(i + 1));

        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 8);
        TEST_ASSERT(!db.is_empty(3));
        TEST_ASSERT(db.is_empty(4));

        // Erase across two blocks.
        db.erase(3, 6); // 4 cells
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 4);

        // Check the integrity of the data.
        double test;
        db.get(2, test);
        TEST_ASSERT(test == 3.0);
        TEST_ASSERT(db.is_empty(3));

        // Empty it.
        db.erase(0, 3);
        TEST_ASSERT(db.block_size() == 0);
        TEST_ASSERT(db.size() == 0);
        TEST_ASSERT(db.empty());
    }

    {
        // Two blocks - non-empty to non-empty blocks.
        mtv_type db(8);
        for (long i = 0; i < 4; ++i)
            db.set(i, static_cast<double>(i + 1));

        for (long i = 4; i < 8; ++i)
            db.set(i, static_cast<uint64_t>(i + 1));

        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 8);

        // Erase across two blocks.
        db.erase(3, 6); // 4 cells
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 4);

        // Check the integrity of the data.
        double test;
        db.get(2, test);
        TEST_ASSERT(test == 3.0);

        uint64_t test2;
        db.get(3, test2);
        TEST_ASSERT(test2 == 8);

        // Empty it.
        db.erase(0, 3);
        TEST_ASSERT(db.block_size() == 0);
        TEST_ASSERT(db.size() == 0);
        TEST_ASSERT(db.empty());
    }

    {
        // 3 blocks, all non-empty.
        mtv_type db(9);
        for (long i = 0; i < 3; ++i)
            db.set(i, static_cast<double>(i + 1));

        for (long i = 3; i < 6; ++i)
            db.set(i, static_cast<uint64_t>(i + 1));

        for (long i = 6; i < 9; ++i)
        {
            std::ostringstream os;
            os << i + 1;
            db.set(i, os.str());
        }

        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.size() == 9);

        db.erase(2, 7);
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 3);

        // Check the integrity of the data.
        double test1;
        db.get(1, test1);
        TEST_ASSERT(test1 == 2.0);
        std::string test2;
        db.get(2, test2);
        TEST_ASSERT(test2 == "9");

        db.erase(2, 2); // Erase only one-block.
        TEST_ASSERT(db.block_size() == 1);
        TEST_ASSERT(db.size() == 2);
        test1 = -1.0;
        db.get(1, test1);
        TEST_ASSERT(test1 == 2.0);

        db.erase(0, 1);
        TEST_ASSERT(db.size() == 0);
        TEST_ASSERT(db.empty());
    }

    {
        // erase() to merge two blocks.
        mtv_type db(4);
        db.set(0, 1.1);
        db.set(1, std::string("foo"));
        db.set(2, static_cast<uint64_t>(2));
        db.set(3, std::string("baa"));
        TEST_ASSERT(db.block_size() == 4);
        TEST_ASSERT(db.size() == 4);

        db.erase(2, 2);
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 3);

        // Try again, but this time merge two empty blocks.
        db.resize(4);
        db.set_empty(1, 3);
        db.set(2, static_cast<uint64_t>(10));
        TEST_ASSERT(db.get<double>(0) == 1.1);
        TEST_ASSERT(db.is_empty(1));
        TEST_ASSERT(db.get<uint64_t>(2) == 10);
        TEST_ASSERT(db.is_empty(3));

        db.erase(2, 2);
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 3);
        TEST_ASSERT(db.get<double>(0) == 1.1);
        TEST_ASSERT(db.is_empty(1));
        TEST_ASSERT(db.is_empty(2));
    }

    {
        mtv_type db(6);
        db.set(0, 1.0);
        db.set(1, 2.0);
        db.set(2, std::string("A"));
        db.set(3, std::string("B"));
        db.set(4, 5.0);
        db.set(5, 6.0);
        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.size() == 6);
        TEST_ASSERT(db.get<double>(0) == 1.0);
        TEST_ASSERT(db.get<double>(1) == 2.0);
        TEST_ASSERT(db.get<std::string>(2) == "A");
        TEST_ASSERT(db.get<std::string>(3) == "B");
        TEST_ASSERT(db.get<double>(4) == 5.0);
        TEST_ASSERT(db.get<double>(5) == 6.0);
        db.erase(1, 4);
        TEST_ASSERT(db.block_size() == 1);
        TEST_ASSERT(db.size() == 2);
        TEST_ASSERT(db.get<double>(0) == 1.0);
        TEST_ASSERT(db.get<double>(1) == 6.0);
    }

    {
        mtv_type db(6);
        db.set(0, 1.0);
        db.set<int8_t>(4, 2);
        db.set<int16_t>(5, 3);
        db.erase(1, 3);
    }

    {
        mtv_type db;

        for (int i = 0; i < 3; ++i)
            db.push_back<double>(1.0);

        for (int i = 0; i < 3; ++i)
            db.push_back<int16_t>(3);

        for (int i = 0; i < 3; ++i)
            db.push_back<int32_t>(9);

        db.erase(1, 3);
        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.size() == 6);
    }

    {
        mtv_type db;

        for (int i = 0; i < 3; ++i)
            db.push_back<double>(1.0);

        for (int i = 0; i < 3; ++i)
            db.push_back<int16_t>(3);

        for (int i = 0; i < 3; ++i)
            db.push_back<double>(9);

        for (int i = 0; i < 3; ++i)
            db.push_back<int32_t>(9);

        db.erase(1, 5);
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 7);
    }

    {
        mtv_type db;

        for (int i = 0; i < 3; ++i)
            db.push_back<double>(1.0);

        for (int i = 0; i < 3; ++i)
            db.push_back<int16_t>(3);

        for (int i = 0; i < 3; ++i)
            db.push_back<double>(9);

        for (int i = 0; i < 3; ++i)
            db.push_back<int32_t>(9);

        db.erase(3, 5);
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 9);
    }

    {
        mtv_type db;

        for (int i = 0; i < 3; ++i)
            db.push_back<double>(1.0);

        for (int i = 0; i < 3; ++i)
            db.push_back<int16_t>(3);

        for (int i = 0; i < 3; ++i)
            db.push_back_empty();

        for (int i = 0; i < 3; ++i)
            db.push_back<int32_t>(9);

        db.erase(3, 5);
        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.size() == 9);
    }

    {
        mtv_type db;

        for (int i = 0; i < 3; ++i)
            db.push_back_empty();

        for (int i = 0; i < 3; ++i)
            db.push_back<int16_t>(3);

        for (int i = 0; i < 3; ++i)
            db.push_back<double>(1.23);

        for (int i = 0; i < 3; ++i)
            db.push_back<int32_t>(9);

        db.erase(3, 5);
        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.size() == 9);
    }

    {
        mtv_type db;

        for (int i = 0; i < 3; ++i)
            db.push_back_empty();

        for (int i = 0; i < 3; ++i)
            db.push_back<int16_t>(3);

        for (int i = 0; i < 3; ++i)
            db.push_back_empty();

        for (int i = 0; i < 3; ++i)
            db.push_back<int32_t>(9);

        db.erase(3, 5);
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 9);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
