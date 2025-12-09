/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2024 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_main.hpp"

namespace {

/**
 * This value is not copyable; only moveable.
 */
struct move_data
{
    std::string value;

    move_data() = default;
    move_data(const move_data&) = delete;
    move_data(move_data&&) = default;
    move_data& operator=(const move_data&) = delete;
    move_data& operator=(move_data&&) = default;

    move_data(std::string _value) : value(std::move(_value))
    {}
    move_data(const char* _value) : value(_value)
    {}

    bool operator==(const move_data& r) const noexcept
    {
        return value == r.value;
    }

    bool operator<(const move_data& r) const noexcept
    {
        return value < r.value;
    }
};

} // anonymous namespace

void st_test_move_constructor()
{
    MDDS_TEST_FUNC_SCOPE;

    using db_type = mdds::segment_tree<float, move_data>;

    db_type db;
    db.insert(-2, 10, "-2:10");
    db.insert(5, 20, "5:20");
    db.insert(6, 15, "6:15");
    db.build_tree();

    // Since the value type is only moveable, this must trigger the move
    // constructor, not the copy constructor.
    db_type db_moved(std::move(db));

    {
        TEST_ASSERT(db_moved.valid_tree());
        TEST_ASSERT(db_moved.size() == 3);

        auto results = db_moved.search(19);
        TEST_ASSERT(results.size() == 1);
        const auto& v = *results.begin();
        TEST_ASSERT(v.start == 5);
        TEST_ASSERT(v.end == 20);
        TEST_ASSERT(v.value == "5:20");
        TEST_ASSERT(std::next(results.begin()) == results.end());
    }

    db_type db_assigned;
    db_assigned = std::move(db_moved); // move assignment

    {
        TEST_ASSERT(db_assigned.valid_tree());
        TEST_ASSERT(db_assigned.size() == 3);

        auto results = db_assigned.search(19);
        TEST_ASSERT(results.size() == 1);
        const auto& v = *results.begin();
        TEST_ASSERT(v.start == 5);
        TEST_ASSERT(v.end == 20);
        TEST_ASSERT(v.value == "5:20");
        TEST_ASSERT(std::next(results.begin()) == results.end());
    }
}

void st_test_move_equality()
{
    MDDS_TEST_FUNC_SCOPE;

    using db_type = mdds::segment_tree<float, move_data>;

    db_type db1;
    db1.insert(-2, 10, "-2:10");
    db1.insert(5, 20, "5:20");
    db1.insert(6, 15, "6:15");

    db_type db2;
    db2.insert(-2, 10, "-2:10");
    db2.insert(5, 20, "5:20");
    db2.insert(6, 15, "6:15");

    TEST_ASSERT(db1 == db2);

    db2.insert(8, 22, "8:22");
    TEST_ASSERT(db1 != db2);

    db1.insert(8, 22, "8:22");
    TEST_ASSERT(db1 == db2);

    db1.erase_if([](float, float, const move_data& v) { return v.value == "-2:10"; });
    TEST_ASSERT(db1 != db2);

    db2.erase_if([](float, float, const move_data& v) { return v.value == "-2:10"; });
    TEST_ASSERT(db1 == db2);

    db1.build_tree();
    TEST_ASSERT(db1 != db2);

    db2.build_tree();
    TEST_ASSERT(db1 == db2);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
