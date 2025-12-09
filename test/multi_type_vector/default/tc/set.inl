/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

void mtv_test_set_cells()
{
    MDDS_TEST_FUNC_SCOPE;

    {
        mtv_type db(5);

        // Replace the whole block.

        {
            std::vector<double> vals;
            vals.reserve(5);
            for (size_t i = 0; i < db.size(); ++i)
                vals.push_back(double(i + 1));

            db.set(0, vals.begin(), vals.end());

            double test;
            db.get(0, test);
            TEST_ASSERT(test == 1.0);
            db.get(4, test);
            TEST_ASSERT(test == 5.0);
        }

        {
            std::vector<std::string> vals;
            vals.reserve(5);
            for (size_t i = 0; i < db.size(); ++i)
            {
                std::ostringstream os;
                os << (i + 1);
                vals.push_back(os.str());
            }

            db.set(0, vals.begin(), vals.end());

            std::string test;
            db.get(0, test);
            TEST_ASSERT(test == "1");
            db.get(4, test);
            TEST_ASSERT(test == "5");
        }

        {
            double vals[] = {5.0, 6.0, 7.0, 8.0, 9.0};
            double* p = &vals[0];
            double* p_end = p + 5;
            db.set(0, p, p_end);
            double test;
            db.get(0, test);
            TEST_ASSERT(test == 5.0);
            db.get(4, test);
            TEST_ASSERT(test == 9.0);
        }

        {
            // Replace the whole block of the same type, which shouldn't
            // delete the old data array.
            double vals[] = {5.1, 6.1, 7.1, 8.1, 9.1};
            double* p = &vals[0];
            double* p_end = p + 5;
            db.set(0, p, p_end);
            double test;
            db.get(0, test);
            TEST_ASSERT(test == 5.1);
            db.get(4, test);
            TEST_ASSERT(test == 9.1);

            double vals2[] = {8.2, 9.2};
            p = &vals2[0];
            p_end = p + 2;
            db.set(3, p, p_end);
            db.get(3, test);
            TEST_ASSERT(test == 8.2);
            db.get(4, test);
            TEST_ASSERT(test == 9.2);
        }

        {
            // Replace the upper part of a single block.
            uint64_t vals[] = {1, 2, 3};
            uint64_t* p = &vals[0];
            uint64_t* p_end = p + 3;
            db.set(0, p, p_end);
            TEST_ASSERT(db.block_size() == 2);
            TEST_ASSERT(db.size() == 5);
            uint64_t test;
            db.get(0, test);
            TEST_ASSERT(test == 1);
            db.get(2, test);
            TEST_ASSERT(test == 3);
            double test2;
            db.get(3, test2);
            TEST_ASSERT(test2 == 8.2);
        }

        {
            // Merge with the previos block and erase the whole block.
            uint64_t vals[] = {4, 5};
            uint64_t* p = &vals[0];
            uint64_t* p_end = p + 2;
            db.set(3, p, p_end);
            TEST_ASSERT(db.block_size() == 1);
            TEST_ASSERT(db.size() == 5);
            uint64_t test;
            db.get(2, test);
            TEST_ASSERT(test == 3);
            db.get(3, test);
            TEST_ASSERT(test == 4);
        }

        {
            // Merge with the previous block while keeping the lower part of
            // the block.
            uint64_t prev_value;
            db.get(2, prev_value);

            double val = 2.3;
            db.set(0, val);
            TEST_ASSERT(db.block_size() == 2);
            val = 4.5;
            double* p = &val;
            double* p_end = p + 1;
            db.set(1, p, p_end);
            TEST_ASSERT(db.block_size() == 2);
            TEST_ASSERT(db.size() == 5);
            {
                double test;
                db.get(0, test);
                TEST_ASSERT(test == 2.3);
                db.get(1, test);
                TEST_ASSERT(test == 4.5);
            }

            uint64_t test;
            db.get(2, test);
            TEST_ASSERT(test == prev_value);
        }
    }

    {
        mtv_type db(5);
        for (size_t i = 0; i < 5; ++i)
            db.set(i, static_cast<double>(i + 1));

        TEST_ASSERT(db.block_size() == 1);
        TEST_ASSERT(db.size() == 5);

        {
            uint64_t vals[] = {10, 11};
            uint64_t* p = &vals[0];
            uint64_t* p_end = p + 2;
            db.set(3, p, p_end);

            TEST_ASSERT(db.block_size() == 2);
            TEST_ASSERT(db.size() == 5);

            double test;
            db.get(2, test);
            TEST_ASSERT(test == 3.0);
            uint64_t test2;
            db.get(3, test2);
            TEST_ASSERT(test2 == 10);
            db.get(4, test2);
            TEST_ASSERT(test2 == 11);

            // Insertion into a single block but this time it needs to be
            // merged with the subsequent block.
            db.set(1, p, p_end);

            TEST_ASSERT(db.block_size() == 2);
            TEST_ASSERT(db.size() == 5);

            db.get(1, test2);
            TEST_ASSERT(test2 == 10);
            db.get(2, test2);
            TEST_ASSERT(test2 == 11);
            db.get(3, test2);
            TEST_ASSERT(test2 == 10);
            db.get(4, test2);
            TEST_ASSERT(test2 == 11);
        }
    }

    {
        mtv_type db(6);
        double vals_d[] = {1.0, 1.1, 1.2, 1.3, 1.4, 1.5};
        uint64_t vals_i[] = {12, 13, 14, 15};
        std::string vals_s[] = {"a", "b"};

        {
            double* p = &vals_d[0];
            double* p_end = p + 6;
            db.set(0, p, p_end);
            TEST_ASSERT(db.block_size() == 1);
            TEST_ASSERT(db.size() == 6);
            double test;
            db.get(0, test);
            TEST_ASSERT(test == 1.0);
            db.get(5, test);
            TEST_ASSERT(test == 1.5);
        }

        {
            uint64_t* p = &vals_i[0];
            uint64_t* p_end = p + 4;
            db.set(0, p, p_end);
            TEST_ASSERT(db.block_size() == 2);
            uint64_t test;
            db.get(0, test);
            TEST_ASSERT(test == 12);
            db.get(3, test);
            TEST_ASSERT(test == 15);
        }

        {
            std::string* p = &vals_s[0];
            std::string* p_end = p + 2;
            db.set(2, p, p_end);
            TEST_ASSERT(db.block_size() == 3);
            std::string test;
            db.get(2, test);
            TEST_ASSERT(test == "a");
            db.get(3, test);
            TEST_ASSERT(test == "b");
            double test_d;
            db.get(4, test_d);
            TEST_ASSERT(test_d == 1.4);
            uint64_t test_i;
            db.get(1, test_i);
            TEST_ASSERT(test_i == 13);
        }
    }

    {
        mtv_type db(3);
        {
            double vals[] = {2.1, 2.2, 2.3};
            double* p = &vals[0];
            double* p_end = p + 3;
            db.set(0, p, p_end);
            TEST_ASSERT(db.block_size() == 1);
        }

        {
            uint64_t val_i = 23;
            uint64_t* p = &val_i;
            uint64_t* p_end = p + 1;
            db.set(1, p, p_end);
            TEST_ASSERT(db.block_size() == 3);
            uint64_t test;
            db.get(1, test);
            TEST_ASSERT(test == 23);
            double test_d;
            db.get(0, test_d);
            TEST_ASSERT(test_d == 2.1);
            db.get(2, test_d);
            TEST_ASSERT(test_d == 2.3);
        }
    }

    {
        // Set cells over multiple blocks. Very simple case.

        mtv_type db(2);
        db.set(0, static_cast<double>(1.1));
        db.set(1, std::string("foo"));
        TEST_ASSERT(db.block_size() == 2);

        double vals[] = {2.1, 2.2};
        double* p = &vals[0];
        double* p_end = p + 2;
        db.set(0, p, p_end);
        TEST_ASSERT(db.block_size() == 1);
        TEST_ASSERT(db.size() == 2);

        double test;
        db.get(0, test);
        TEST_ASSERT(test == 2.1);
        db.get(1, test);
        TEST_ASSERT(test == 2.2);
    }

    {
        // Same as above, except that the last block is only partially replaced.

        mtv_type db(3);
        db.set(0, static_cast<double>(1.1));
        db.set(1, std::string("foo"));
        db.set(2, std::string("baa"));

        double vals[] = {2.1, 2.2};
        double* p = &vals[0];
        double* p_end = p + 2;
        db.set(0, p, p_end);
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 3);

        double test_val;
        db.get(0, test_val);
        TEST_ASSERT(test_val == 2.1);
        db.get(1, test_val);
        TEST_ASSERT(test_val == 2.2);

        std::string test_s;
        db.get(2, test_s);
        TEST_ASSERT(test_s == "baa");
    }

    {
        mtv_type db(3);
        db.set(0, static_cast<double>(3.1));
        db.set(1, static_cast<double>(3.2));
        db.set(2, std::string("foo"));
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 3);

        double vals[] = {2.1, 2.2};
        double* p = &vals[0];
        double* p_end = p + 2;
        db.set(1, p, p_end);
        TEST_ASSERT(db.block_size() == 1);
        double test;
        db.get(0, test);
        TEST_ASSERT(test == 3.1);
        db.get(1, test);
        TEST_ASSERT(test == 2.1);
        db.get(2, test);
        TEST_ASSERT(test == 2.2);
    }

    {
        mtv_type db(5);
        db.set(0, 1.1);
        db.set(1, 1.2);
        db.set(2, std::string("foo"));
        db.set(3, 1.3);
        db.set(4, 1.4);
        TEST_ASSERT(db.block_size() == 3);

        double vals[] = {2.1, 2.2, 2.3};
        double* p = &vals[0];
        double* p_end = p + 3;
        db.set(1, p, p_end);
        TEST_ASSERT(db.block_size() == 1);
        TEST_ASSERT(db.size() == 5);
        TEST_ASSERT(db.get<double>(0) == 1.1);
        TEST_ASSERT(db.get<double>(1) == 2.1);
        TEST_ASSERT(db.get<double>(2) == 2.2);
        TEST_ASSERT(db.get<double>(3) == 2.3);
        TEST_ASSERT(db.get<double>(4) == 1.4);
    }

    {
        mtv_type db(4);
        db.set(0, std::string("A"));
        db.set(1, std::string("B"));
        db.set(2, 1.1);
        db.set(3, 1.2);
        TEST_ASSERT(db.block_size() == 2);

        double vals[] = {2.1, 2.2, 2.3};
        double* p = &vals[0];
        db.set(1, p, p + 3);
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.get<std::string>(0) == std::string("A"));
        TEST_ASSERT(db.get<double>(1) == 2.1);
        TEST_ASSERT(db.get<double>(2) == 2.2);
        TEST_ASSERT(db.get<double>(3) == 2.3);
    }

    {
        mtv_type db(4);
        db.set(0, std::string("A"));
        db.set(1, std::string("B"));
        db.set(2, 1.1);
        db.set(3, 1.2);
        TEST_ASSERT(db.block_size() == 2);

        double vals[] = {2.1, 2.2};
        double* p = &vals[0];
        db.set(1, p, p + 2);
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.get<std::string>(0) == std::string("A"));
        TEST_ASSERT(db.get<double>(1) == 2.1);
        TEST_ASSERT(db.get<double>(2) == 2.2);
        TEST_ASSERT(db.get<double>(3) == 1.2);
    }

    {
        mtv_type db(5);
        db.set(0, std::string("A"));
        db.set(1, std::string("B"));
        db.set(2, 1.1);
        db.set(3, 1.2);
        db.set(4, static_cast<uint64_t>(12));
        TEST_ASSERT(db.block_size() == 3);

        uint64_t vals[] = {21, 22, 23};
        uint64_t* p = &vals[0];
        db.set(1, p, p + 3);
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.get<std::string>(0) == std::string("A"));
        TEST_ASSERT(db.get<uint64_t>(1) == 21);
        TEST_ASSERT(db.get<uint64_t>(2) == 22);
        TEST_ASSERT(db.get<uint64_t>(3) == 23);
        TEST_ASSERT(db.get<uint64_t>(4) == 12);
    }

    {
        mtv_type db(3);
        db.set(0, std::string("A"));
        db.set(1, 1.1);
        db.set(2, 1.2);
        TEST_ASSERT(db.block_size() == 2);

        uint64_t vals[] = {11, 12};
        uint64_t* p = &vals[0];
        db.set(0, p, p + 2);
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.get<uint64_t>(0) == 11);
        TEST_ASSERT(db.get<uint64_t>(1) == 12);
        TEST_ASSERT(db.get<double>(2) == 1.2);
    }

    {
        mtv_type db(4);
        db.set(0, static_cast<uint64_t>(35));
        db.set(1, std::string("A"));
        db.set(2, 1.1);
        db.set(3, 1.2);
        TEST_ASSERT(db.block_size() == 3);

        uint64_t vals[] = {11, 12};
        uint64_t* p = &vals[0];
        db.set(1, p, p + 2);
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.get<uint64_t>(0) == 35);
        TEST_ASSERT(db.get<uint64_t>(1) == 11);
        TEST_ASSERT(db.get<uint64_t>(2) == 12);
        TEST_ASSERT(db.get<double>(3) == 1.2);
    }

    {
        // Block 1 is empty.

        mtv_type db(2);
        db.set(1, 1.2);
        TEST_ASSERT(db.block_size() == 2);

        double vals[] = {2.1, 2.2};
        double* p = &vals[0];
        db.set(0, p, p + 2);
        TEST_ASSERT(db.block_size() == 1);
        TEST_ASSERT(db.get<double>(0) == 2.1);
        TEST_ASSERT(db.get<double>(1) == 2.2);
    }

    {
        mtv_type db(3);
        db.set(0, 1.1);
        db.set(2, 1.2);
        TEST_ASSERT(db.block_size() == 3);

        double vals[] = {2.1, 2.2};
        double* p = &vals[0];
        db.set(1, p, p + 2);
        TEST_ASSERT(db.block_size() == 1);
        TEST_ASSERT(db.get<double>(0) == 1.1);
        TEST_ASSERT(db.get<double>(1) == 2.1);
        TEST_ASSERT(db.get<double>(2) == 2.2);
    }

    {
        mtv_type db(5);
        db.set(2, std::string("A"));
        db.set(3, std::string("B"));
        db.set(4, std::string("C"));
        TEST_ASSERT(db.block_size() == 2);

        double vals[] = {1.1, 1.2, 1.3};
        double* p = &vals[0];
        db.set(1, p, p + 3);
        TEST_ASSERT(db.block_size() == 3);

        TEST_ASSERT(db.is_empty(0));
        TEST_ASSERT(db.get<double>(1) == 1.1);
        TEST_ASSERT(db.get<double>(2) == 1.2);
        TEST_ASSERT(db.get<double>(3) == 1.3);
        TEST_ASSERT(db.get<std::string>(4) == std::string("C"));
    }

    {
        mtv_type db(10, true);
        std::vector<bool> bools(3, false);
        db.set(3, 1.1);
        db.set(4, 1.2);
        db.set(5, 1.3);
        TEST_ASSERT(db.block_size() == 3);

        // This should replace the middle numeric block and merge with the top
        // and bottom ones.
        db.set(3, bools.begin(), bools.end());
        TEST_ASSERT(db.block_size() == 1);
        TEST_ASSERT(db.size() == 10);
        TEST_ASSERT(db.get<bool>(0) == true);
        TEST_ASSERT(db.get<bool>(1) == true);
        TEST_ASSERT(db.get<bool>(2) == true);
        TEST_ASSERT(db.get<bool>(3) == false);
        TEST_ASSERT(db.get<bool>(4) == false);
        TEST_ASSERT(db.get<bool>(5) == false);
        TEST_ASSERT(db.get<bool>(6) == true);
        TEST_ASSERT(db.get<bool>(7) == true);
        TEST_ASSERT(db.get<bool>(8) == true);
        TEST_ASSERT(db.get<bool>(9) == true);
    }

    {
        mtv_type db(9);
        db.set(0, 1.1);
        db.set(1, 1.2);
        db.set(2, true);
        db.set(3, false);
        db.set(4, true);
        db.set(5, std::string("a"));
        db.set(6, std::string("b"));
        db.set(7, std::string("c"));
        db.set(8, std::string("d"));
        TEST_ASSERT(db.block_size() == 3);

        std::vector<std::string> strings(3, std::string("foo"));
        db.set(2, strings.begin(), strings.end()); // Merge with the next block.
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 9);
        TEST_ASSERT(db.get<double>(0) == 1.1);
        TEST_ASSERT(db.get<double>(1) == 1.2);
        TEST_ASSERT(db.get<std::string>(2) == "foo");
        TEST_ASSERT(db.get<std::string>(3) == "foo");
        TEST_ASSERT(db.get<std::string>(4) == "foo");
        TEST_ASSERT(db.get<std::string>(5) == "a");
        TEST_ASSERT(db.get<std::string>(6) == "b");
        TEST_ASSERT(db.get<std::string>(7) == "c");
        TEST_ASSERT(db.get<std::string>(8) == "d");
    }

    {
        mtv_type db;

        for (int i = 0; i < 3; ++i)
            db.push_back<int16_t>(i);

        for (int i = 0; i < 3; ++i)
            db.push_back<double>(i);

        for (int i = 0; i < 3; ++i)
            db.push_back_empty();

        std::vector<int16_t> data(5, 11);
        db.set(2, data.begin(), data.end());

        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 9);
        TEST_ASSERT(db.get<int16_t>(0) == 0);
        TEST_ASSERT(db.get<int16_t>(1) == 1);
        TEST_ASSERT(db.get<int16_t>(2) == 11);
        TEST_ASSERT(db.get<int16_t>(3) == 11);
        TEST_ASSERT(db.get<int16_t>(4) == 11);
        TEST_ASSERT(db.get<int16_t>(5) == 11);
        TEST_ASSERT(db.get<int16_t>(6) == 11);
        TEST_ASSERT(db.is_empty(7));
        TEST_ASSERT(db.is_empty(8));
    }

    {
        mtv_type db;

        for (int i = 0; i < 2; ++i)
            db.push_back<int16_t>(i);

        for (int i = 0; i < 2; ++i)
            db.push_back<double>(i);

        for (int i = 0; i < 2; ++i)
            db.push_back<int64_t>(i + 20);

        std::vector<int32_t> data(3, 15);
        db.set(2, data.begin(), data.end());

        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.size() == 6);
        TEST_ASSERT(db.get<int16_t>(0) == 0);
        TEST_ASSERT(db.get<int16_t>(1) == 1);
        TEST_ASSERT(db.get<int32_t>(2) == 15);
        TEST_ASSERT(db.get<int32_t>(3) == 15);
        TEST_ASSERT(db.get<int32_t>(4) == 15);
        TEST_ASSERT(db.get<int64_t>(5) == 21);
    }

    {
        mtv_type db;

        for (int i = 0; i < 2; ++i)
            db.push_back<int16_t>(i);

        for (int i = 0; i < 2; ++i)
            db.push_back<double>(i);

        for (int i = 0; i < 2; ++i)
            db.push_back<int64_t>(i + 20);

        std::vector<int32_t> data(3, 15);
        db.set(1, data.begin(), data.end());

        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.size() == 6);
        TEST_ASSERT(db.get<int16_t>(0) == 0);
        TEST_ASSERT(db.get<int32_t>(1) == 15);
        TEST_ASSERT(db.get<int32_t>(2) == 15);
        TEST_ASSERT(db.get<int32_t>(3) == 15);
        TEST_ASSERT(db.get<int64_t>(4) == 20);
        TEST_ASSERT(db.get<int64_t>(5) == 21);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
