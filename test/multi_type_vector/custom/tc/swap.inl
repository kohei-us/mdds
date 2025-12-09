/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

void mtv_test_swap()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv3_type db1(5), db2(2);
    db1.set(0, new muser_cell(1.1));
    db1.set(1, new muser_cell(1.2));
    db1.set(2, new muser_cell(1.3));
    db1.set(3, new muser_cell(1.4));
    db1.set(4, new muser_cell(1.5));

    db2.set(0, std::string("A"));
    db2.set(1, std::string("B"));

    db1.swap(2, 3, db2, 0);

    // swap blocks of equal size, one managed, and one default.

    db1.clear();
    db1.resize(10);
    db2.clear();
    db2.resize(10);

    db1.set(3, 2.1);
    db1.set(4, 2.2);
    db1.set(5, 2.3);

    db2.set(3, new muser_cell(3.1));
    db2.set(4, new muser_cell(3.2));
    db2.set(5, new muser_cell(3.3));

    db2.swap(3, 5, db1, 3);

    TEST_ASSERT(db1.size() == 10);
    TEST_ASSERT(db1.block_size() == 3);
    TEST_ASSERT(db2.size() == 10);
    TEST_ASSERT(db2.block_size() == 3);

    TEST_ASSERT(db1.get<muser_cell*>(3)->value == 3.1);
    TEST_ASSERT(db1.get<muser_cell*>(4)->value == 3.2);
    TEST_ASSERT(db1.get<muser_cell*>(5)->value == 3.3);
    TEST_ASSERT(db2.get<double>(3) == 2.1);
    TEST_ASSERT(db2.get<double>(4) == 2.2);
    TEST_ASSERT(db2.get<double>(5) == 2.3);

    db2.swap(3, 5, db1, 3);

    TEST_ASSERT(db1.get<double>(3) == 2.1);
    TEST_ASSERT(db1.get<double>(4) == 2.2);
    TEST_ASSERT(db1.get<double>(5) == 2.3);
    TEST_ASSERT(db2.get<muser_cell*>(3)->value == 3.1);
    TEST_ASSERT(db2.get<muser_cell*>(4)->value == 3.2);
    TEST_ASSERT(db2.get<muser_cell*>(5)->value == 3.3);

    // Same as above, except that the source segment splits the block into 2.

    db1.clear();
    db1.resize(10);
    db2.clear();
    db2.resize(10);

    db1.set(3, 2.1);
    db1.set(4, 2.2);

    db2.set(3, new muser_cell(3.1));
    db2.set(4, new muser_cell(3.2));
    db2.set(5, new muser_cell(3.3));

    db2.swap(3, 4, db1, 3);

    // Another scenario that used to crash on double delete.

    db1.clear();
    db1.resize(10);
    db2.clear();
    db2.resize(10);

    db1.set(2, new muser_cell(4.1));
    db1.set(3, 4.2);
    db1.set(4, new muser_cell(4.3));

    db2.set(3, new muser_cell(6.1));
    db2.set(4, 6.2);
    db2.set(5, 6.3);

    TEST_ASSERT(db1.get<muser_cell*>(2)->value == 4.1);
    TEST_ASSERT(db1.get<double>(3) == 4.2);
    TEST_ASSERT(db1.get<muser_cell*>(4)->value == 4.3);

    TEST_ASSERT(db2.get<muser_cell*>(3)->value == 6.1);
    TEST_ASSERT(db2.get<double>(4) == 6.2);
    TEST_ASSERT(db2.get<double>(5) == 6.3);

    db2.swap(4, 4, db1, 4);

    TEST_ASSERT(db1.get<muser_cell*>(2)->value == 4.1);
    TEST_ASSERT(db1.get<double>(3) == 4.2);
    TEST_ASSERT(db1.get<double>(4) == 6.2);

    TEST_ASSERT(db2.get<muser_cell*>(3)->value == 6.1);
    TEST_ASSERT(db2.get<muser_cell*>(4)->value == 4.3);
    TEST_ASSERT(db2.get<double>(5) == 6.3);

    // One more on double deletion...

    db1.clear();
    db1.resize(10);
    db2.clear();
    db2.resize(10);

    db1.set(0, 2.1);
    db1.set(1, 2.2);
    db1.set(2, 2.3);
    db1.set(3, new muser_cell(4.5));

    db2.set(2, new muser_cell(3.1));
    db2.set(3, new muser_cell(3.2));
    db2.set(4, new muser_cell(3.3));

    db1.swap(2, 2, db2, 3);

    TEST_ASSERT(db1.get<double>(0) == 2.1);
    TEST_ASSERT(db1.get<double>(1) == 2.2);
    TEST_ASSERT(db1.get<muser_cell*>(2)->value == 3.2);
    TEST_ASSERT(db1.get<muser_cell*>(3)->value == 4.5);

    TEST_ASSERT(db2.get<muser_cell*>(2)->value == 3.1);
    TEST_ASSERT(db2.get<double>(3) == 2.3);
    TEST_ASSERT(db2.get<muser_cell*>(4)->value == 3.3);
}

void mtv_test_swap_2()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv3_type db1(3), db2(3);

    db1.set(0, new muser_cell(1.1));
    db1.set(1, new muser_cell(1.2));

    db2.set(0, 1.2);
    db2.set(1, std::string("foo"));

    // Repeat the same swap twice.
    db1.swap(0, 1, db2, 0);
    TEST_ASSERT(db2.get<muser_cell*>(0)->value == 1.1);
    TEST_ASSERT(db2.get<muser_cell*>(1)->value == 1.2);
    TEST_ASSERT(db1.get<double>(0) == 1.2);
    TEST_ASSERT(db1.get<std::string>(1) == "foo");

    db1.swap(0, 1, db2, 0);
    TEST_ASSERT(db1.get<muser_cell*>(0)->value == 1.1);
    TEST_ASSERT(db1.get<muser_cell*>(1)->value == 1.2);
    TEST_ASSERT(db2.get<double>(0) == 1.2);
    TEST_ASSERT(db2.get<std::string>(1) == "foo");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
