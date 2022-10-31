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

namespace {

using mdds::mtv::lu_factor_t;

template<lu_factor_t F>
struct trait_lu : public mdds::mtv::standard_element_blocks_traits
{
    using event_func = mdds::mtv::empty_event_func;

    constexpr static lu_factor_t loop_unrolling = F;
};

} // namespace

template<lu_factor_t F>
void mtv_test_loop_unrolling()
{
    cout << "loop unrolling factor = " << int(F) << endl;
    using mtv_type = mtv_alias_type<trait_lu<F>>;

    mtv_type db(5, std::string("test"));

    // keep inserting blocks of alternating types at position 0 to force block
    // position adjustments.

    for (int i = 0; i < 50; ++i)
    {
        db.insert_empty(0, 2);

        std::vector<int8_t> values(2, 89);
        db.insert(0, values.begin(), values.end());
    }
}

void mtv_test_loop_unrolling_0()
{
    stack_printer __stack_printer__(__FUNCTION__);

    mtv_test_loop_unrolling<lu_factor_t::none>();
}

void mtv_test_loop_unrolling_4()
{
    stack_printer __stack_printer__(__FUNCTION__);

    mtv_test_loop_unrolling<lu_factor_t::lu4>();
}

void mtv_test_loop_unrolling_8()
{
    stack_printer __stack_printer__(__FUNCTION__);

    mtv_test_loop_unrolling<lu_factor_t::lu8>();
}

void mtv_test_loop_unrolling_16()
{
    stack_printer __stack_printer__(__FUNCTION__);

    mtv_test_loop_unrolling<lu_factor_t::lu16>();
}

void mtv_test_loop_unrolling_32()
{
    stack_printer __stack_printer__(__FUNCTION__);

    mtv_test_loop_unrolling<lu_factor_t::lu32>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
