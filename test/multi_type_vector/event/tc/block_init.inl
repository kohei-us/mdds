/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

struct event_block_init
{
    std::string name;
    std::string ctor_type;

    event_block_init() : ctor_type("empty")
    {}
    event_block_init(const std::string& _name) : name(_name), ctor_type("normal")
    {}
    event_block_init(const event_block_init& other) : name(other.name), ctor_type("copy")
    {}
    event_block_init(event_block_init&& other) : name(std::move(other.name)), ctor_type("move")
    {}

    void element_block_acquired(const mdds::mtv::base_element_block* /*block*/)
    {}
    void element_block_released(const mdds::mtv::base_element_block* /*block*/)
    {}
};

struct eb_init_trait : public mdds::mtv::standard_element_blocks_traits
{
    using event_func = event_block_init;

    constexpr static mdds::mtv::lu_factor_t loop_unrolling = mdds::mtv::lu_factor_t::lu8;
};

void mtv_test_block_init()
{
    MDDS_TEST_FUNC_SCOPE;

    using mtv_type = mtv_template_type<eb_init_trait>;

    {
        mtv_type db(event_block_init("some name")); // pass an rvalue
        TEST_ASSERT(db.event_handler().name == "some name");
        TEST_ASSERT(db.event_handler().ctor_type == "move");

        auto db2{db};
        TEST_ASSERT(db2.event_handler().name == "some name");
        TEST_ASSERT(db2.event_handler().ctor_type == "copy");
    }

    {
        event_block_init ebi("other name");
        TEST_ASSERT(ebi.ctor_type == "normal");
        mtv_type db(ebi); // pass an lvalue
        TEST_ASSERT(db.event_handler().name == "other name");
        TEST_ASSERT(db.event_handler().ctor_type == "copy");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
