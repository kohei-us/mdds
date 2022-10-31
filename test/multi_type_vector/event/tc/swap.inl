/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2022 Kohei Yoshida
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
    stack_printer __stack_printer__(__FUNCTION__);

    mtv_type db1(10), db2(2);
    db1.event_handler().name = "db1";
    db2.event_handler().name = "db2";

    assert(db1.size() == 10);
    assert(db2.size() == 2);
    assert(db1.event_handler().name == "db1");
    assert(db2.event_handler().name == "db2");

    // This should also swap the event handlers.
    db1.swap(db2);

    assert(db1.size() == 2);
    assert(db2.size() == 10);
    assert(db1.event_handler().name == "db2");
    assert(db2.event_handler().name == "db1");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
