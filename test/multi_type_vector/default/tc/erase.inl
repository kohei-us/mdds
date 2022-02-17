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
    stack_printer __stack_printer__(__FUNCTION__);
    {
        // Single empty block.
        mtv_type db(5);
        db.erase(0, 2); // erase rows 0-2.
        assert(db.size() == 2);
        db.erase(0, 1);
        assert(db.size() == 0);
        assert(db.empty());
    }

    {
        // Single empty block followed by a non-empty block.
        mtv_type db(5);
        db.push_back<int32_t>(-234);

        db.erase(0, 2); // erase rows 0-2.
        assert(db.size() == 3);
        db.erase(0, 1);
        assert(db.size() == 1);
    }

    {
        // Single non-empty block.
        mtv_type db(5);
        for (long i = 0; i < 5; ++i)
            db.set(i, static_cast<double>(i + 1));

        assert(db.block_size() == 1);
        assert(db.size() == 5);

        db.erase(0, 2); // erase rows 0-2
        assert(db.size() == 2);
        double test;
        db.get(0, test);
        assert(test == 4.0);
        db.get(1, test);
        assert(test == 5.0);

        db.erase(0, 1);
        assert(db.size() == 0);
        assert(db.empty());
    }

    {
        // Two blocks - non-empty to empty blocks.
        mtv_type db(8);
        for (long i = 0; i < 4; ++i)
            db.set(i, static_cast<double>(i + 1));

        assert(db.block_size() == 2);
        assert(db.size() == 8);
        assert(!db.is_empty(3));
        assert(db.is_empty(4));

        // Erase across two blocks.
        db.erase(3, 6); // 4 cells
        assert(db.block_size() == 2);
        assert(db.size() == 4);

        // Check the integrity of the data.
        double test;
        db.get(2, test);
        assert(test == 3.0);
        assert(db.is_empty(3));

        // Empty it.
        db.erase(0, 3);
        assert(db.block_size() == 0);
        assert(db.size() == 0);
        assert(db.empty());
    }

    {
        // Two blocks - non-empty to non-empty blocks.
        mtv_type db(8);
        for (long i = 0; i < 4; ++i)
            db.set(i, static_cast<double>(i + 1));

        for (long i = 4; i < 8; ++i)
            db.set(i, static_cast<uint64_t>(i + 1));

        assert(db.block_size() == 2);
        assert(db.size() == 8);

        // Erase across two blocks.
        db.erase(3, 6); // 4 cells
        assert(db.block_size() == 2);
        assert(db.size() == 4);

        // Check the integrity of the data.
        double test;
        db.get(2, test);
        assert(test == 3.0);

        uint64_t test2;
        db.get(3, test2);
        assert(test2 == 8);

        // Empty it.
        db.erase(0, 3);
        assert(db.block_size() == 0);
        assert(db.size() == 0);
        assert(db.empty());
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

        assert(db.block_size() == 3);
        assert(db.size() == 9);

        db.erase(2, 7);
        assert(db.block_size() == 2);
        assert(db.size() == 3);

        // Check the integrity of the data.
        double test1;
        db.get(1, test1);
        assert(test1 == 2.0);
        std::string test2;
        db.get(2, test2);
        assert(test2 == "9");

        db.erase(2, 2); // Erase only one-block.
        assert(db.block_size() == 1);
        assert(db.size() == 2);
        test1 = -1.0;
        db.get(1, test1);
        assert(test1 == 2.0);

        db.erase(0, 1);
        assert(db.size() == 0);
        assert(db.empty());
    }

    {
        // erase() to merge two blocks.
        mtv_type db(4);
        db.set(0, 1.1);
        db.set(1, std::string("foo"));
        db.set(2, static_cast<uint64_t>(2));
        db.set(3, std::string("baa"));
        assert(db.block_size() == 4);
        assert(db.size() == 4);

        db.erase(2, 2);
        assert(db.block_size() == 2);
        assert(db.size() == 3);

        // Try again, but this time merge two empty blocks.
        db.resize(4);
        db.set_empty(1, 3);
        db.set(2, static_cast<uint64_t>(10));
        assert(db.get<double>(0) == 1.1);
        assert(db.is_empty(1));
        assert(db.get<uint64_t>(2) == 10);
        assert(db.is_empty(3));

        db.erase(2, 2);
        assert(db.block_size() == 2);
        assert(db.size() == 3);
        assert(db.get<double>(0) == 1.1);
        assert(db.is_empty(1));
        assert(db.is_empty(2));
    }

    {
        mtv_type db(6);
        db.set(0, 1.0);
        db.set(1, 2.0);
        db.set(2, std::string("A"));
        db.set(3, std::string("B"));
        db.set(4, 5.0);
        db.set(5, 6.0);
        assert(db.block_size() == 3);
        assert(db.size() == 6);
        assert(db.get<double>(0) == 1.0);
        assert(db.get<double>(1) == 2.0);
        assert(db.get<std::string>(2) == "A");
        assert(db.get<std::string>(3) == "B");
        assert(db.get<double>(4) == 5.0);
        assert(db.get<double>(5) == 6.0);
        db.erase(1, 4);
        assert(db.block_size() == 1);
        assert(db.size() == 2);
        assert(db.get<double>(0) == 1.0);
        assert(db.get<double>(1) == 6.0);
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
        assert(db.block_size() == 3);
        assert(db.size() == 6);
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
        assert(db.block_size() == 2);
        assert(db.size() == 7);
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
        assert(db.block_size() == 2);
        assert(db.size() == 9);
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
        assert(db.block_size() == 3);
        assert(db.size() == 9);
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
        assert(db.block_size() == 3);
        assert(db.size() == 9);
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
        assert(db.block_size() == 2);
        assert(db.size() == 9);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
