/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

void test_push_back_copy()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type vec;
    user_cell::reset_counter();

    user_cell v;
    const user_cell& vref = v;

    // first push_back() call should trigger 1 copy and 0 moves
    auto it = vec.push_back(vref);
    TEST_ASSERT(it == std::prev(vec.end()));

    user_cell::print_counters();
    TEST_ASSERT(user_cell::copy_count == 1);
    TEST_ASSERT(!user_cell::move_count);

    // second push_back() call should trigger another copy, it also may cause
    // one move due to reallocation of the buffer in the destination storage,
    // so we don't check the move counter.
    it = vec.push_back(vref);
    TEST_ASSERT(it == std::prev(vec.end()));
    user_cell::print_counters();
    TEST_ASSERT(user_cell::copy_count == 2);
}

void test_push_back_move()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type vec;
    user_cell::reset_counter();

    {
        // first push_back() call should trigger 0 copies and 1 move
        user_cell v;
        auto it = vec.push_back(std::move(v));
        TEST_ASSERT(it == std::prev(vec.end()));

        user_cell::print_counters();
        TEST_ASSERT(!user_cell::copy_count);
        TEST_ASSERT(user_cell::move_count == 1);
    }

    {
        // second push_back() call should still trigger 0 copies and may trigger
        // move than 2 moves due to buffer reallocation of the destination
        // storage
        user_cell v;
        auto it = vec.push_back(std::move(v));
        TEST_ASSERT(it == std::prev(vec.end()));
        user_cell::print_counters();
        TEST_ASSERT(!user_cell::copy_count);
        TEST_ASSERT(user_cell::move_count >= 2);
    }
}

void test_emplace_back()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type vec;
    auto it = vec.emplace_back<user_cell>(int(12));
    TEST_ASSERT(it == std::prev(vec.end()));

    it = vec.emplace_back<user_cell>(float(-42));
    TEST_ASSERT(it == std::prev(vec.end()));

    vec.push_back_empty();

    it = vec.emplace_back<user_cell>(short(18), short(12));
    TEST_ASSERT(it == std::prev(vec.end()));

    TEST_ASSERT(vec.get<user_cell>(0).get_value() == "int: 12");
    TEST_ASSERT(vec.get<user_cell>(1).get_value() == "float: -42");
    TEST_ASSERT(vec.get<user_cell>(3).get_value() == "short+short: 30");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
