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

#include "test_global.hpp" // This must be the first header to be included.
#include "test_main.hpp"

namespace {

using mdds::mtv::lu_factor_t;

template<lu_factor_t F>
struct trait_lu
{
    using event_func = mdds::mtv::empty_event_func;

    constexpr static lu_factor_t loop_unrolling = F;
};

template<lu_factor_t F>
void run_test()
{
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

} // namespace

#if SIZEOF_VOID_P == 8 && defined(__SSE2__)

void mtv_test_loop_unrolling_sse2_x64()
{
    stack_printer __stack_printer__(__FUNCTION__);

    run_test<lu_factor_t::sse2_x64>();
}

void mtv_test_loop_unrolling_sse2_x64_4()
{
    stack_printer __stack_printer__(__FUNCTION__);

    run_test<lu_factor_t::sse2_x64_lu4>();
}

void mtv_test_loop_unrolling_sse2_x64_8()
{
    stack_printer __stack_printer__(__FUNCTION__);

    run_test<lu_factor_t::sse2_x64_lu8>();
}

void mtv_test_loop_unrolling_sse2_x64_16()
{
    stack_printer __stack_printer__(__FUNCTION__);

    run_test<lu_factor_t::sse2_x64_lu16>();
}

#else

void print_disabled_reasons_sse2()
{
    cout << "Test disabled for the following reasons:" << endl;

#if SIZEOF_VOID_P != 8
    cout << "  * not a 64-bit build" << endl;
#endif

#ifndef __SSE2__
    cout << "  * __SSE2__ not defined" << endl;
#endif
}

void mtv_test_loop_unrolling_sse2_x64()
{
    stack_printer __stack_printer__(__FUNCTION__);

    print_disabled_reasons_sse2();
}

void mtv_test_loop_unrolling_sse2_x64_4()
{
    stack_printer __stack_printer__(__FUNCTION__);

    print_disabled_reasons_sse2();
}

void mtv_test_loop_unrolling_sse2_x64_8()
{
    stack_printer __stack_printer__(__FUNCTION__);

    print_disabled_reasons_sse2();
}

void mtv_test_loop_unrolling_sse2_x64_16()
{
    stack_printer __stack_printer__(__FUNCTION__);

    print_disabled_reasons_sse2();
}

#endif

#if SIZEOF_VOID_P == 8 && defined(__AVX2__)

void mtv_test_loop_unrolling_avx2_x64()
{
    stack_printer __stack_printer__(__FUNCTION__);

    run_test<lu_factor_t::avx2_x64>();
}

void mtv_test_loop_unrolling_avx2_x64_4()
{
    stack_printer __stack_printer__(__FUNCTION__);

    run_test<lu_factor_t::avx2_x64_lu4>();
}

void mtv_test_loop_unrolling_avx2_x64_8()
{
    stack_printer __stack_printer__(__FUNCTION__);

    run_test<lu_factor_t::avx2_x64_lu8>();
}

#else

void print_disabled_reasons_avx2()
{
    cout << "Test disabled for the following reasons:" << endl;

#if SIZEOF_VOID_P != 8
    cout << "  * not a 64-bit build" << endl;
#endif

#ifndef __AVX2__
    cout << "  * __AVX2__ not defined" << endl;
#endif
}

void mtv_test_loop_unrolling_avx2_x64()
{
    stack_printer __stack_printer__(__FUNCTION__);

    print_disabled_reasons_avx2();
}

void mtv_test_loop_unrolling_avx2_x64_4()
{
    stack_printer __stack_printer__(__FUNCTION__);

    print_disabled_reasons_avx2();
}

void mtv_test_loop_unrolling_avx2_x64_8()
{
    stack_printer __stack_printer__(__FUNCTION__);

    print_disabled_reasons_avx2();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
