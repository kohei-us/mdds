/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2022 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

struct event_handler : public mdds::mtv::empty_event_func
{
    std::string name;

    event_handler()
    {}
};

struct trait : public mdds::mtv::default_traits
{
    using event_func = event_handler;
};

using mtv_type = mtv_template_type<trait>;

void mtv_test_swap()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type db1(10), db2(2);
    db1.event_handler().name = "db1";
    db2.event_handler().name = "db2";

    TEST_ASSERT(db1.size() == 10);
    TEST_ASSERT(db2.size() == 2);
    TEST_ASSERT(db1.event_handler().name == "db1");
    TEST_ASSERT(db2.event_handler().name == "db2");

    // This should also swap the event handlers.
    db1.swap(db2);

    TEST_ASSERT(db1.size() == 2);
    TEST_ASSERT(db2.size() == 10);
    TEST_ASSERT(db1.event_handler().name == "db2");
    TEST_ASSERT(db2.event_handler().name == "db1");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
