/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

void mtv_test_construction()
{
    MDDS_TEST_FUNC_SCOPE;

    {
        mtv_type db; // default constructor.
        TEST_ASSERT(db.size() == 0);
        TEST_ASSERT(db.empty());
        TEST_ASSERT(db.block_size() == 0);
    }

    {
        // Create an empty segment of size 7.
        mtv_type db(7);
        TEST_ASSERT(db.size() == 7);
        TEST_ASSERT(db.block_size() == 1);
    }

    {
        // Create with initial value and size.
        mtv_type db(10, 1.0);
        TEST_ASSERT(db.size() == 10);
        TEST_ASSERT(db.block_size() == 1);
        TEST_ASSERT(db.get<double>(0) == 1.0);
        TEST_ASSERT(db.get<double>(9) == 1.0);
    }

    {
        // Create with initial value and size.
        mtv_type db(10, std::string("foo"));
        TEST_ASSERT(db.size() == 10);
        TEST_ASSERT(db.block_size() == 1);
        TEST_ASSERT(db.get<std::string>(0) == "foo");
        TEST_ASSERT(db.get<std::string>(9) == "foo");
    }

    {
        // Create with an array of values.
        std::vector<double> vals;
        vals.push_back(1.1);
        vals.push_back(1.2);
        vals.push_back(1.3);
        mtv_type db(vals.size(), vals.begin(), vals.end());
        TEST_ASSERT(db.size() == 3);
        TEST_ASSERT(db.block_size() == 1);
        TEST_ASSERT(db.get<double>(0) == 1.1);
        TEST_ASSERT(db.get<double>(1) == 1.2);
        TEST_ASSERT(db.get<double>(2) == 1.3);
    }

    {
        std::vector<std::string> vals;
        mtv_type db_empty(0, vals.begin(), vals.end());
        TEST_ASSERT(db_empty.size() == 0);
        TEST_ASSERT(db_empty.block_size() == 0);

        vals.push_back("Andy");
        vals.push_back("Bruce");

        mtv_type db(2, vals.begin(), vals.end());
        TEST_ASSERT(db.size() == 2);
        TEST_ASSERT(db.block_size() == 1);
        TEST_ASSERT(db.get<std::string>(0) == "Andy");
        TEST_ASSERT(db.get<std::string>(1) == "Bruce");
    }

    {
        std::vector<int32_t> vals(10, 1);
        try
        {
            mtv_type db(20, vals.begin(), vals.end());
            TEST_ASSERT(!"This construction should have failed due to incorrect initial array size.");
        }
        catch (const mdds::invalid_arg_error&)
        {
            // good.
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
