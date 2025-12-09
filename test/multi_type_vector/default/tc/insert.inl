/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

void mtv_test_insert_empty()
{
    MDDS_TEST_FUNC_SCOPE;
    {
        mtv_type db(5);
        db.insert_empty(0, 5);
        TEST_ASSERT(db.size() == 10);
        TEST_ASSERT(db.block_size() == 1);

        // Insert data from row 0 to 4.
        for (long i = 0; i < 5; ++i)
            db.set(i, static_cast<double>(i + 1));

        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 10);

        // Now, insert an empty block of size 2 at the top.
        db.insert_empty(0, 2);
        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.size() == 12);

        double test;
        db.get(2, test);
        TEST_ASSERT(test == 1.0);

        // Insert an empty cell into an empty block.  This should shift the
        // data block down by one.
        db.insert_empty(1, 1);
        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.size() == 13);
        db.get(4, test);
        TEST_ASSERT(test == 2.0);
    }

    {
        mtv_type db(5);
        for (long i = 0; i < 5; ++i)
            db.set(i, static_cast<double>(i + 1));

        TEST_ASSERT(db.block_size() == 1);
        TEST_ASSERT(db.size() == 5);

        // Insert an empty block into the middle of a non-empty block.
        db.insert_empty(2, 2);

        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.size() == 7);
        TEST_ASSERT(db.is_empty(2));
        TEST_ASSERT(db.is_empty(3));

        double test;
        db.get(0, test);
        TEST_ASSERT(test == 1.0);
        db.get(1, test);
        TEST_ASSERT(test == 2.0);

        db.get(4, test);
        TEST_ASSERT(test == 3.0);
        db.get(5, test);
        TEST_ASSERT(test == 4.0);
        db.get(6, test);
        TEST_ASSERT(test == 5.0);
    }

    {
        mtv_type db(1);
        db.set(0, 2.5);
        db.insert_empty(0, 2);
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 3);
        TEST_ASSERT(db.is_empty(1));
        TEST_ASSERT(!db.is_empty(2));

        double test;
        db.get(2, test);
        TEST_ASSERT(test == 2.5);
    }

    {
        mtv_type db(2);
        db.set(0, 1.2);
        db.set(1, 2.3);
        db.insert_empty(1, 1);

        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.size() == 3);
        double test;
        db.get(0, test);
        TEST_ASSERT(test == 1.2);
        db.get(2, test);
        TEST_ASSERT(test == 2.3);
    }

    {
        mtv_type db(2);
        db.push_back<double>(12.0);
        db.push_back<float>(13.0);
        db.insert_empty(2, 2);

        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.size() == 6);
        TEST_ASSERT(db.is_empty(0));
        TEST_ASSERT(db.is_empty(1));
        TEST_ASSERT(db.is_empty(2));
        TEST_ASSERT(db.is_empty(3));
        TEST_ASSERT(db.get<double>(4) == 12.0);
        TEST_ASSERT(db.get<float>(5) == 13.0);
    }

    {
        mtv_type db;
        for (int32_t i = 0; i < 9; ++i)
            db.push_back(i);

        db.push_back<int16_t>(123);
        db.insert_empty(7, 3);

        TEST_ASSERT(db.block_size() == 4);
        TEST_ASSERT(db.size() == 13);
    }
}

void mtv_test_insert_cells()
{
    MDDS_TEST_FUNC_SCOPE;
    {
        // Insert into non-empty block of the same type.
        mtv_type db(1);
        db.set(0, 1.1);
        TEST_ASSERT(db.block_size() == 1);
        TEST_ASSERT(db.size() == 1);

        double vals[] = {2.1, 2.2, 2.3};
        double* p = &vals[0];
        db.insert(0, p, p + 3);
        TEST_ASSERT(db.block_size() == 1);
        TEST_ASSERT(db.size() == 4);
        TEST_ASSERT(db.get<double>(0) == 2.1);
        TEST_ASSERT(db.get<double>(1) == 2.2);
        TEST_ASSERT(db.get<double>(2) == 2.3);
        TEST_ASSERT(db.get<double>(3) == 1.1);
    }

    {
        // Insert into an existing empty block.
        mtv_type db(1);
        TEST_ASSERT(db.block_size() == 1);
        TEST_ASSERT(db.size() == 1);

        double vals[] = {2.1, 2.2, 2.3};
        double* p = &vals[0];
        db.insert(0, p, p + 3);
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 4);
        TEST_ASSERT(db.get<double>(0) == 2.1);
        TEST_ASSERT(db.get<double>(1) == 2.2);
        TEST_ASSERT(db.get<double>(2) == 2.3);
        TEST_ASSERT(db.is_empty(3));
    }

    {
        mtv_type db(2);
        db.set(0, 1.1);
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 2);

        double vals[] = {2.1, 2.2, 2.3};
        double* p = &vals[0];
        db.insert(1, p, p + 3);
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 5);
        TEST_ASSERT(db.get<double>(0) == 1.1);
        TEST_ASSERT(db.get<double>(1) == 2.1);
        TEST_ASSERT(db.get<double>(2) == 2.2);
        TEST_ASSERT(db.get<double>(3) == 2.3);
        TEST_ASSERT(db.is_empty(4));
    }

    {
        mtv_type db(2);
        db.set(0, static_cast<uint64_t>(23));
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 2);

        double vals[] = {2.1, 2.2, 2.3};
        double* p = &vals[0];
        db.insert(1, p, p + 3);
        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.size() == 5);
        TEST_ASSERT(db.get<uint64_t>(0) == 23);
        TEST_ASSERT(db.get<double>(1) == 2.1);
        TEST_ASSERT(db.get<double>(2) == 2.2);
        TEST_ASSERT(db.get<double>(3) == 2.3);
        TEST_ASSERT(db.is_empty(4));
    }

    {
        mtv_type db(2);
        double vals[] = {2.1, 2.2, 2.3};
        double* p = &vals[0];
        db.insert(1, p, p + 3);
        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.size() == 5);
        TEST_ASSERT(db.is_empty(0));
        TEST_ASSERT(db.get<double>(1) == 2.1);
        TEST_ASSERT(db.get<double>(2) == 2.2);
        TEST_ASSERT(db.get<double>(3) == 2.3);
        TEST_ASSERT(db.is_empty(4));
    }

    {
        mtv_type db(2);
        db.set(0, 1.1);
        db.set(1, static_cast<uint64_t>(23));
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 2);

        double vals[] = {2.1, 2.2, 2.3};
        double* p = &vals[0];
        db.insert(1, p, p + 3);
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 5);
        TEST_ASSERT(db.get<double>(0) == 1.1);
        TEST_ASSERT(db.get<double>(1) == 2.1);
        TEST_ASSERT(db.get<double>(2) == 2.2);
        TEST_ASSERT(db.get<double>(3) == 2.3);
        TEST_ASSERT(db.get<uint64_t>(4) == 23);
    }

    {
        mtv_type db(2);
        db.set(0, true);
        db.set(1, static_cast<uint64_t>(23));
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 2);

        double vals[] = {2.1, 2.2, 2.3};
        double* p = &vals[0];
        db.insert(1, p, p + 3);
        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.size() == 5);
        TEST_ASSERT(db.get<bool>(0) == true);
        TEST_ASSERT(db.get<double>(1) == 2.1);
        TEST_ASSERT(db.get<double>(2) == 2.2);
        TEST_ASSERT(db.get<double>(3) == 2.3);
        TEST_ASSERT(db.get<uint64_t>(4) == 23);
    }

    {
        mtv_type db(2);
        db.set(0, static_cast<uint64_t>(12));
        db.set(1, static_cast<uint64_t>(23));
        TEST_ASSERT(db.block_size() == 1);
        TEST_ASSERT(db.size() == 2);

        double vals[] = {2.1, 2.2, 2.3};
        double* p = &vals[0];
        db.insert(1, p, p + 3);
        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.size() == 5);
        TEST_ASSERT(db.get<uint64_t>(0) == 12);
        TEST_ASSERT(db.get<double>(1) == 2.1);
        TEST_ASSERT(db.get<double>(2) == 2.2);
        TEST_ASSERT(db.get<double>(3) == 2.3);
        TEST_ASSERT(db.get<uint64_t>(4) == 23);
    }

    {
        mtv_type db(3);
        db.set(0, 1.0);
        db.set(1, std::string("foo"));
        db.set(2, std::string("baa"));
        TEST_ASSERT(db.size() == 3);
        TEST_ASSERT(db.block_size() == 2);
        double vals[] = {2.1};
        const double* p = &vals[0];
        db.insert(2, p, p + 1);
        TEST_ASSERT(db.size() == 4);
        TEST_ASSERT(db.block_size() == 4);
    }

    {
        mtv_type db(2);
        db.set(0, static_cast<uint64_t>(11));
        db.set(1, static_cast<uint64_t>(12));
        double vals[] = {1.2};
        const double* p = &vals[0];
        db.insert(1, p, p + 1);
        TEST_ASSERT(db.block_size() == 3);

        // Append value to the top block.
        uint64_t vals2[] = {22};
        const uint64_t* p2 = &vals2[0];
        db.insert(1, p2, p2 + 1);
        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.get<uint64_t>(0) == 11);
        TEST_ASSERT(db.get<uint64_t>(1) == 22);
        TEST_ASSERT(db.get<double>(2) == 1.2);
        TEST_ASSERT(db.get<uint64_t>(3) == 12);
    }

    {
        mtv_type db(5);
        db.push_back<double>(1.1);
        db.push_back<double>(1.2);

        double vals[] = {2.1, 2.2, 2.3};
        const double* p = vals;
        db.insert(2, p, p + 3);
        TEST_ASSERT(db.block_size() == 4);
        TEST_ASSERT(db.size() == 10);
        TEST_ASSERT(db.is_empty(0));
        TEST_ASSERT(db.is_empty(1));
        TEST_ASSERT(db.get<double>(2) == 2.1);
        TEST_ASSERT(db.get<double>(3) == 2.2);
        TEST_ASSERT(db.get<double>(4) == 2.3);
        TEST_ASSERT(db.is_empty(5));
        TEST_ASSERT(db.is_empty(6));
        TEST_ASSERT(db.is_empty(7));
        TEST_ASSERT(db.get<double>(8) == 1.1);
        TEST_ASSERT(db.get<double>(9) == 1.2);
    }

    {
        mtv_type db(5, int16_t(222));
        db.push_back<int32_t>(12);
        db.push_back<int32_t>(34);

        int16_t vals[] = {5, 6, 7};
        const int16_t* p = vals;
        db.insert(1, p, p + 3);
        TEST_ASSERT(db.block_size() == 2);
        TEST_ASSERT(db.size() == 10);
        TEST_ASSERT(db.get<int16_t>(0) == 222);
        TEST_ASSERT(db.get<int16_t>(1) == 5);
        TEST_ASSERT(db.get<int16_t>(2) == 6);
        TEST_ASSERT(db.get<int16_t>(3) == 7);
        TEST_ASSERT(db.get<int16_t>(4) == 222);
        TEST_ASSERT(db.get<int16_t>(5) == 222);
        TEST_ASSERT(db.get<int16_t>(6) == 222);
        TEST_ASSERT(db.get<int16_t>(7) == 222);
        TEST_ASSERT(db.get<int32_t>(8) == 12);
        TEST_ASSERT(db.get<int32_t>(9) == 34);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
