/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2022 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

void test_mtv_basic()
{
    MDDS_TEST_FUNC_SCOPE;

    using this_mtv_type = mtv_type<my_trait>;

    this_mtv_type db{};

    db.push_back(true);
    db.push_back(false);
    db.push_back<std::int32_t>(123);
    db.push_back<std::uint32_t>(456u);

    TEST_ASSERT(db.size() == 4);
    TEST_ASSERT(db.block_size() == 3);
    TEST_ASSERT(db.get<bool>(0) == true);
    TEST_ASSERT(db.get<bool>(1) == false);
    TEST_ASSERT(db.get<std::int32_t>(2) == 123);
    TEST_ASSERT(db.get<std::uint32_t>(3) == 456u);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
