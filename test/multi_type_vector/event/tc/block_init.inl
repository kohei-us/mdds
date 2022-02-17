/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2021 Kohei Yoshida
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************/

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

struct eb_init_trait
{
    using event_func = event_block_init;

    constexpr static mdds::mtv::lu_factor_t loop_unrolling = mdds::mtv::lu_factor_t::lu8;
};

void mtv_test_block_init()
{
    stack_printer __stack_printer__(__FUNCTION__);

    using mtv_type = mtv_template_type<mdds::mtv::element_block_func, eb_init_trait>;

    {
        mtv_type db(event_block_init("some name")); // pass an rvalue
        assert(db.event_handler().name == "some name");
        assert(db.event_handler().ctor_type == "move");

        auto db2{db};
        assert(db2.event_handler().name == "some name");
        assert(db2.event_handler().ctor_type == "copy");
    }

    {
        event_block_init ebi("other name");
        assert(ebi.ctor_type == "normal");
        mtv_type db(ebi); // pass an lvalue
        assert(db.event_handler().name == "other name");
        assert(db.event_handler().ctor_type == "copy");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
