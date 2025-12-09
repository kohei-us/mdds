/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

void mtv_test_basic()
{
    MDDS_TEST_FUNC_SCOPE;

    // mdds::multi_type_vector does not manage the life cycle of individual cells;
    // the client code needs to manage them when storing pointers.

    user_cell_pool pool;

    {
        // set_cell()
        mtv_type db(4);
        user_cell* p = pool.construct(1.2);
        db.set(0, p);
        db.set(1, p);
        db.set(3, p);
        db.set(2, p);

        user_cell* p2 = db.get<user_cell*>(0);
        TEST_ASSERT(p->value == p2->value);

        p = pool.construct(3.4);
        db.set(0, p);
        p2 = db.get<user_cell*>(0);
        TEST_ASSERT(p->value == p2->value);
        pool.clear();
    }

    {
        // set_cells(), resize(), insert_cells().
        mtv_type db(3);
        user_cell* p1 = pool.construct(1.1);
        user_cell* p2 = pool.construct(2.2);
        user_cell* p3 = pool.construct(3.3);
        std::vector<user_cell*> vals;
        vals.reserve(3);
        vals.push_back(p1);
        vals.push_back(p2);
        vals.push_back(p3);
        db.set(0, vals.begin(), vals.end());

        user_cell* ptest;
        ptest = db.get<user_cell*>(0);
        TEST_ASSERT(ptest && ptest->value == 1.1);
        ptest = db.get<user_cell*>(1);
        TEST_ASSERT(ptest && ptest->value == 2.2);
        ptest = db.get<user_cell*>(2);
        TEST_ASSERT(ptest && ptest->value == 3.3);

        db.resize(6);
        user_cell* p4 = pool.construct(11);
        user_cell* p5 = pool.construct(22);
        user_cell* p6 = pool.construct(33);
        vals.clear();
        vals.push_back(p4);
        vals.push_back(p5);
        vals.push_back(p6);
        db.set(3, vals.begin(), vals.end());

        ptest = db.get<user_cell*>(0);
        TEST_ASSERT(ptest && ptest->value == 1.1);
        ptest = db.get<user_cell*>(1);
        TEST_ASSERT(ptest && ptest->value == 2.2);
        ptest = db.get<user_cell*>(2);
        TEST_ASSERT(ptest && ptest->value == 3.3);
        ptest = db.get<user_cell*>(3);
        TEST_ASSERT(ptest && ptest->value == 11);
        ptest = db.get<user_cell*>(4);
        TEST_ASSERT(ptest && ptest->value == 22);
        ptest = db.get<user_cell*>(5);
        TEST_ASSERT(ptest && ptest->value == 33);

        // Shrink the block to erase the bottom 3 cells.
        db.resize(3);
        TEST_ASSERT(db.size() == 3);
        ptest = db.get<user_cell*>(2);
        TEST_ASSERT(ptest && ptest->value == 3.3);

        // Re-insert the values at the front.
        db.insert(0, vals.begin(), vals.end());
        TEST_ASSERT(db.size() == 6);

        ptest = db.get<user_cell*>(0);
        TEST_ASSERT(ptest && ptest->value == 11);
        ptest = db.get<user_cell*>(1);
        TEST_ASSERT(ptest && ptest->value == 22);
        ptest = db.get<user_cell*>(2);
        TEST_ASSERT(ptest && ptest->value == 33);
        ptest = db.get<user_cell*>(3);
        TEST_ASSERT(ptest && ptest->value == 1.1);
        ptest = db.get<user_cell*>(4);
        TEST_ASSERT(ptest && ptest->value == 2.2);
        ptest = db.get<user_cell*>(5);
        TEST_ASSERT(ptest && ptest->value == 3.3);

        // set_empty(), is_empty().
        db.set_empty(2, 4);
        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.get<user_cell*>(1)->value == 22);
        TEST_ASSERT(db.is_empty(2));
        TEST_ASSERT(db.is_empty(3));
        TEST_ASSERT(db.is_empty(4));
        TEST_ASSERT(db.get<user_cell*>(5)->value == 3.3);

        // erase()
        db.erase(3, 5);
        TEST_ASSERT(db.size() == 3);
        TEST_ASSERT(db.get<user_cell*>(1)->value == 22);
        TEST_ASSERT(db.is_empty(2));

        // insert_empty().
        db.insert_empty(1, 2);
        TEST_ASSERT(db.size() == 5);
        TEST_ASSERT(db.get<user_cell*>(0)->value == 11);
        TEST_ASSERT(db.is_empty(1));
        TEST_ASSERT(db.is_empty(2));
        TEST_ASSERT(db.get<user_cell*>(3)->value == 22);
        TEST_ASSERT(db.is_empty(4));

        pool.clear();
    }

    {
        // set_cells() to overwrite existing values of type user_cell*.
        mtv_type db(2);
        user_cell* p0 = pool.construct(1.2);
        db.set(1, p0);
        db.set(0, p0);

        std::vector<user_cell*> vals;
        vals.push_back(pool.construct(2.3));
        vals.push_back(pool.construct(2.4));
        db.set(0, vals.begin(), vals.end());
        pool.clear();
    }

    {
        mtv_type db(4);
        user_cell* p0 = pool.construct(1.1);
        db.set(3, p0);

        std::vector<user_cell*> vals;
        vals.push_back(pool.construct(2.3));
        vals.push_back(pool.construct(2.4));
        db.set(1, vals.begin(), vals.end());
        TEST_ASSERT(db.is_empty(0));
        TEST_ASSERT(db.get<user_cell*>(1)->value == 2.3);
        TEST_ASSERT(db.get<user_cell*>(2)->value == 2.4);
        TEST_ASSERT(db.get<user_cell*>(3)->value == 1.1);

        pool.clear();
    }

    {
        // Get empty value.
        mtv_type db(1);
        user_cell* p = db.get<user_cell*>(0);
        TEST_ASSERT(p == nullptr);
    }
}

void mtv_test_basic_equality()
{
    MDDS_TEST_FUNC_SCOPE;

    user_cell_pool pool;

    mtv_type db1(3);
    mtv_type db2 = db1;
    TEST_ASSERT(db2 == db1);
    user_cell* p0 = pool.construct(1.1);
    db1.set(0, p0);
    TEST_ASSERT(db1 != db2);
    db2.set(0, p0);
    TEST_ASSERT(db1 == db2);
    db1.set(2, std::string("foo"));
    db2.set(2, std::string("foo"));
    TEST_ASSERT(db1 == db2);

    // same value but different memory addresses.
    user_cell* p1 = pool.construct(1.2);
    user_cell* p2 = pool.construct(1.2);
    db1.set(1, p1);
    db2.set(1, p2);
    TEST_ASSERT(db1 != db2); // equality is by the pointer value.
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
