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

#ifndef INCLUDED_MDDS_MULTI_TYPE_VECTOR_DIR_SOA_BLOCK_UTIL_HPP
#define INCLUDED_MDDS_MULTI_TYPE_VECTOR_DIR_SOA_BLOCK_UTIL_HPP

#include "mdds/global.hpp"
#include "../types.hpp"

#if defined(__SSE2__)
#include <emmintrin.h>
#endif
#if defined(__AVX2__)
#include <immintrin.h>
#endif

namespace mdds { namespace mtv { namespace soa { namespace detail {

template<typename Blks, lu_factor_t F>
struct adjust_block_positions
{
    void operator()(Blks& /*block_store*/, int64_t /*start_block_index*/, int64_t /*delta*/) const
    {
        static_assert(
            mdds::detail::invalid_static_int<F>, "The loop-unrolling factor must be one of 0, 4, 8, 16, or 32.");
    }
};

template<typename Blks>
struct adjust_block_positions<Blks, lu_factor_t::none>
{
    void operator()(Blks& block_store, int64_t start_block_index, int64_t delta) const
    {
        int64_t n = block_store.positions.size();

        if (start_block_index >= n)
            return;

#if MDDS_USE_OPENMP
#pragma omp parallel for
#endif
        for (int64_t i = start_block_index; i < n; ++i)
            block_store.positions[i] += delta;
    }
};

template<typename Blks>
struct adjust_block_positions<Blks, lu_factor_t::lu4>
{
    void operator()(Blks& block_store, int64_t start_block_index, int64_t delta) const
    {
        int64_t n = block_store.positions.size();

        if (start_block_index >= n)
            return;

        // Ensure that the section length is divisible by 4.
        int64_t len = n - start_block_index;
        int64_t rem = len & 3; // % 4
        len -= rem;
        len += start_block_index;
#if MDDS_USE_OPENMP
#pragma omp parallel for
#endif
        for (int64_t i = start_block_index; i < len; i += 4)
        {
            block_store.positions[i + 0] += delta;
            block_store.positions[i + 1] += delta;
            block_store.positions[i + 2] += delta;
            block_store.positions[i + 3] += delta;
        }

        rem += len;
        for (int64_t i = len; i < rem; ++i)
            block_store.positions[i] += delta;
    }
};

template<typename Blks>
struct adjust_block_positions<Blks, lu_factor_t::lu8>
{
    void operator()(Blks& block_store, int64_t start_block_index, int64_t delta) const
    {
        int64_t n = block_store.positions.size();

        if (start_block_index >= n)
            return;

        // Ensure that the section length is divisible by 8.
        int64_t len = n - start_block_index;
        int64_t rem = len & 7; // % 8
        len -= rem;
        len += start_block_index;
#if MDDS_USE_OPENMP
#pragma omp parallel for
#endif
        for (int64_t i = start_block_index; i < len; i += 8)
        {
            block_store.positions[i + 0] += delta;
            block_store.positions[i + 1] += delta;
            block_store.positions[i + 2] += delta;
            block_store.positions[i + 3] += delta;
            block_store.positions[i + 4] += delta;
            block_store.positions[i + 5] += delta;
            block_store.positions[i + 6] += delta;
            block_store.positions[i + 7] += delta;
        }

        rem += len;
        for (int64_t i = len; i < rem; ++i)
            block_store.positions[i] += delta;
    }
};

template<typename Blks>
struct adjust_block_positions<Blks, lu_factor_t::lu16>
{
    void operator()(Blks& block_store, int64_t start_block_index, int64_t delta) const
    {
        int64_t n = block_store.positions.size();

        if (start_block_index >= n)
            return;

        // Ensure that the section length is divisible by 16.
        int64_t len = n - start_block_index;
        int64_t rem = len & 15; // % 16
        len -= rem;
        len += start_block_index;
#if MDDS_USE_OPENMP
#pragma omp parallel for
#endif
        for (int64_t i = start_block_index; i < len; i += 16)
        {
            block_store.positions[i + 0] += delta;
            block_store.positions[i + 1] += delta;
            block_store.positions[i + 2] += delta;
            block_store.positions[i + 3] += delta;
            block_store.positions[i + 4] += delta;
            block_store.positions[i + 5] += delta;
            block_store.positions[i + 6] += delta;
            block_store.positions[i + 7] += delta;
            block_store.positions[i + 8] += delta;
            block_store.positions[i + 9] += delta;
            block_store.positions[i + 10] += delta;
            block_store.positions[i + 11] += delta;
            block_store.positions[i + 12] += delta;
            block_store.positions[i + 13] += delta;
            block_store.positions[i + 14] += delta;
            block_store.positions[i + 15] += delta;
        }

        rem += len;
        for (int64_t i = len; i < rem; ++i)
            block_store.positions[i] += delta;
    }
};

template<typename Blks>
struct adjust_block_positions<Blks, lu_factor_t::lu32>
{
    void operator()(Blks& block_store, int64_t start_block_index, int64_t delta) const
    {
        int64_t n = block_store.positions.size();

        if (start_block_index >= n)
            return;

        // Ensure that the section length is divisible by 32.
        int64_t len = n - start_block_index;
        int64_t rem = len & 31; // % 32
        len -= rem;
        len += start_block_index;
#if MDDS_USE_OPENMP
#pragma omp parallel for
#endif
        for (int64_t i = start_block_index; i < len; i += 32)
        {
            block_store.positions[i + 0] += delta;
            block_store.positions[i + 1] += delta;
            block_store.positions[i + 2] += delta;
            block_store.positions[i + 3] += delta;
            block_store.positions[i + 4] += delta;
            block_store.positions[i + 5] += delta;
            block_store.positions[i + 6] += delta;
            block_store.positions[i + 7] += delta;
            block_store.positions[i + 8] += delta;
            block_store.positions[i + 9] += delta;
            block_store.positions[i + 10] += delta;
            block_store.positions[i + 11] += delta;
            block_store.positions[i + 12] += delta;
            block_store.positions[i + 13] += delta;
            block_store.positions[i + 14] += delta;
            block_store.positions[i + 15] += delta;
            block_store.positions[i + 16] += delta;
            block_store.positions[i + 17] += delta;
            block_store.positions[i + 18] += delta;
            block_store.positions[i + 19] += delta;
            block_store.positions[i + 20] += delta;
            block_store.positions[i + 21] += delta;
            block_store.positions[i + 22] += delta;
            block_store.positions[i + 23] += delta;
            block_store.positions[i + 24] += delta;
            block_store.positions[i + 25] += delta;
            block_store.positions[i + 26] += delta;
            block_store.positions[i + 27] += delta;
            block_store.positions[i + 28] += delta;
            block_store.positions[i + 29] += delta;
            block_store.positions[i + 30] += delta;
            block_store.positions[i + 31] += delta;
        }

        rem += len;
        for (int64_t i = len; i < rem; ++i)
            block_store.positions[i] += delta;
    }
};

#if defined(__SSE2__)

template<typename Blks>
struct adjust_block_positions<Blks, lu_factor_t::sse2_x64>
{
    void operator()(Blks& block_store, int64_t start_block_index, int64_t delta) const
    {
        static_assert(
            sizeof(typename decltype(block_store.positions)::value_type) == 8,
            "This code works only when the position values are 64-bit wide.");

        int64_t n = block_store.positions.size();

        if (start_block_index >= n)
            return;

        // Ensure that the section length is divisible by 2.
        int64_t len = n - start_block_index;
        bool odd = len & 1;
        if (odd)
            len -= 1;

        len += start_block_index;

        __m128i right = _mm_set_epi64x(delta, delta);

#if MDDS_USE_OPENMP
#pragma omp parallel for
#endif
        for (int64_t i = start_block_index; i < len; i += 2)
        {
            __m128i* dst = (__m128i*)&block_store.positions[i];
            _mm_storeu_si128(dst, _mm_add_epi64(_mm_loadu_si128(dst), right));
        }

        if (odd)
            block_store.positions[len] += delta;
    }
};

template<typename Blks>
struct adjust_block_positions<Blks, lu_factor_t::sse2_x64_lu4>
{
    void operator()(Blks& block_store, int64_t start_block_index, int64_t delta) const
    {
        static_assert(
            sizeof(typename decltype(block_store.positions)::value_type) == 8,
            "This code works only when the position values are 64-bit wide.");

        int64_t n = block_store.positions.size();

        if (start_block_index >= n)
            return;

        // Ensure that the section length is divisible by 8.
        int64_t len = n - start_block_index;
        int64_t rem = len & 7; // % 8
        len -= rem;
        len += start_block_index;

        __m128i right = _mm_set_epi64x(delta, delta);

#if MDDS_USE_OPENMP
#pragma omp parallel for
#endif
        for (int64_t i = start_block_index; i < len; i += 8)
        {
            __m128i* dst0 = (__m128i*)&block_store.positions[i];
            _mm_storeu_si128(dst0, _mm_add_epi64(_mm_loadu_si128(dst0), right));

            __m128i* dst2 = (__m128i*)&block_store.positions[i + 2];
            _mm_storeu_si128(dst2, _mm_add_epi64(_mm_loadu_si128(dst2), right));

            __m128i* dst4 = (__m128i*)&block_store.positions[i + 4];
            _mm_storeu_si128(dst4, _mm_add_epi64(_mm_loadu_si128(dst4), right));

            __m128i* dst6 = (__m128i*)&block_store.positions[i + 6];
            _mm_storeu_si128(dst6, _mm_add_epi64(_mm_loadu_si128(dst6), right));
        }

        rem += len;
        for (int64_t i = len; i < rem; ++i)
            block_store.positions[i] += delta;
    }
};

template<typename Blks>
struct adjust_block_positions<Blks, lu_factor_t::sse2_x64_lu8>
{
    void operator()(Blks& block_store, int64_t start_block_index, int64_t delta) const
    {
        static_assert(
            sizeof(typename decltype(block_store.positions)::value_type) == 8,
            "This code works only when the position values are 64-bit wide.");

        int64_t n = block_store.positions.size();

        if (start_block_index >= n)
            return;

        // Ensure that the section length is divisible by 16.
        int64_t len = n - start_block_index;
        int64_t rem = len & 15; // % 16
        len -= rem;
        len += start_block_index;

        __m128i right = _mm_set_epi64x(delta, delta);

#if MDDS_USE_OPENMP
#pragma omp parallel for
#endif
        for (int64_t i = start_block_index; i < len; i += 16)
        {
            __m128i* dst0 = (__m128i*)&block_store.positions[i];
            _mm_storeu_si128(dst0, _mm_add_epi64(_mm_loadu_si128(dst0), right));

            __m128i* dst2 = (__m128i*)&block_store.positions[i + 2];
            _mm_storeu_si128(dst2, _mm_add_epi64(_mm_loadu_si128(dst2), right));

            __m128i* dst4 = (__m128i*)&block_store.positions[i + 4];
            _mm_storeu_si128(dst4, _mm_add_epi64(_mm_loadu_si128(dst4), right));

            __m128i* dst6 = (__m128i*)&block_store.positions[i + 6];
            _mm_storeu_si128(dst6, _mm_add_epi64(_mm_loadu_si128(dst6), right));

            __m128i* dst8 = (__m128i*)&block_store.positions[i + 8];
            _mm_storeu_si128(dst8, _mm_add_epi64(_mm_loadu_si128(dst8), right));

            __m128i* dst10 = (__m128i*)&block_store.positions[i + 10];
            _mm_storeu_si128(dst10, _mm_add_epi64(_mm_loadu_si128(dst10), right));

            __m128i* dst12 = (__m128i*)&block_store.positions[i + 12];
            _mm_storeu_si128(dst12, _mm_add_epi64(_mm_loadu_si128(dst12), right));

            __m128i* dst14 = (__m128i*)&block_store.positions[i + 14];
            _mm_storeu_si128(dst14, _mm_add_epi64(_mm_loadu_si128(dst14), right));
        }

        rem += len;
        for (int64_t i = len; i < rem; ++i)
            block_store.positions[i] += delta;
    }
};

template<typename Blks>
struct adjust_block_positions<Blks, lu_factor_t::sse2_x64_lu16>
{
    void operator()(Blks& block_store, int64_t start_block_index, int64_t delta) const
    {
        static_assert(
            sizeof(typename decltype(block_store.positions)::value_type) == 8,
            "This code works only when the position values are 64-bit wide.");

        int64_t n = block_store.positions.size();

        if (start_block_index >= n)
            return;

        // Ensure that the section length is divisible by 32.
        int64_t len = n - start_block_index;
        int64_t rem = len & 31; // % 32
        len -= rem;
        len += start_block_index;

        __m128i right = _mm_set_epi64x(delta, delta);

#if MDDS_USE_OPENMP
#pragma omp parallel for
#endif
        for (int64_t i = start_block_index; i < len; i += 32)
        {
            __m128i* dst0 = (__m128i*)&block_store.positions[i];
            _mm_storeu_si128(dst0, _mm_add_epi64(_mm_loadu_si128(dst0), right));

            __m128i* dst2 = (__m128i*)&block_store.positions[i + 2];
            _mm_storeu_si128(dst2, _mm_add_epi64(_mm_loadu_si128(dst2), right));

            __m128i* dst4 = (__m128i*)&block_store.positions[i + 4];
            _mm_storeu_si128(dst4, _mm_add_epi64(_mm_loadu_si128(dst4), right));

            __m128i* dst6 = (__m128i*)&block_store.positions[i + 6];
            _mm_storeu_si128(dst6, _mm_add_epi64(_mm_loadu_si128(dst6), right));

            __m128i* dst8 = (__m128i*)&block_store.positions[i + 8];
            _mm_storeu_si128(dst8, _mm_add_epi64(_mm_loadu_si128(dst8), right));

            __m128i* dst10 = (__m128i*)&block_store.positions[i + 10];
            _mm_storeu_si128(dst10, _mm_add_epi64(_mm_loadu_si128(dst10), right));

            __m128i* dst12 = (__m128i*)&block_store.positions[i + 12];
            _mm_storeu_si128(dst12, _mm_add_epi64(_mm_loadu_si128(dst12), right));

            __m128i* dst14 = (__m128i*)&block_store.positions[i + 14];
            _mm_storeu_si128(dst14, _mm_add_epi64(_mm_loadu_si128(dst14), right));

            __m128i* dst16 = (__m128i*)&block_store.positions[i + 16];
            _mm_storeu_si128(dst16, _mm_add_epi64(_mm_loadu_si128(dst16), right));

            __m128i* dst18 = (__m128i*)&block_store.positions[i + 18];
            _mm_storeu_si128(dst18, _mm_add_epi64(_mm_loadu_si128(dst18), right));

            __m128i* dst20 = (__m128i*)&block_store.positions[i + 20];
            _mm_storeu_si128(dst20, _mm_add_epi64(_mm_loadu_si128(dst20), right));

            __m128i* dst22 = (__m128i*)&block_store.positions[i + 22];
            _mm_storeu_si128(dst22, _mm_add_epi64(_mm_loadu_si128(dst22), right));

            __m128i* dst24 = (__m128i*)&block_store.positions[i + 24];
            _mm_storeu_si128(dst24, _mm_add_epi64(_mm_loadu_si128(dst24), right));

            __m128i* dst26 = (__m128i*)&block_store.positions[i + 26];
            _mm_storeu_si128(dst26, _mm_add_epi64(_mm_loadu_si128(dst26), right));

            __m128i* dst28 = (__m128i*)&block_store.positions[i + 28];
            _mm_storeu_si128(dst28, _mm_add_epi64(_mm_loadu_si128(dst28), right));

            __m128i* dst30 = (__m128i*)&block_store.positions[i + 30];
            _mm_storeu_si128(dst30, _mm_add_epi64(_mm_loadu_si128(dst30), right));
        }

        rem += len;
        for (int64_t i = len; i < rem; ++i)
            block_store.positions[i] += delta;
    }
};

#endif // __SSE2__

#if defined(__AVX2__)

template<typename Blks>
struct adjust_block_positions<Blks, lu_factor_t::avx2_x64>
{
    void operator()(Blks& block_store, int64_t start_block_index, int64_t delta) const
    {
        static_assert(
            sizeof(typename decltype(block_store.positions)::value_type) == 8,
            "This code works only when the position values are 64-bit wide.");

        int64_t n = block_store.positions.size();

        if (start_block_index >= n)
            return;

        // Ensure that the section length is divisible by 4.
        int64_t len = n - start_block_index;
        int64_t rem = len & 3; // % 4
        len -= rem;
        len += start_block_index;

        __m256i right = _mm256_set1_epi64x(delta);

#if MDDS_USE_OPENMP
#pragma omp parallel for
#endif
        for (int64_t i = start_block_index; i < len; i += 4)
        {
            __m256i* dst = (__m256i*)&block_store.positions[i];
            _mm256_storeu_si256(dst, _mm256_add_epi64(_mm256_loadu_si256(dst), right));
        }

        rem += len;
        for (int64_t i = len; i < rem; ++i)
            block_store.positions[i] += delta;
    }
};

template<typename Blks>
struct adjust_block_positions<Blks, lu_factor_t::avx2_x64_lu4>
{
    void operator()(Blks& block_store, int64_t start_block_index, int64_t delta) const
    {
        static_assert(
            sizeof(typename decltype(block_store.positions)::value_type) == 8,
            "This code works only when the position values are 64-bit wide.");

        int64_t n = block_store.positions.size();

        if (start_block_index >= n)
            return;

        // Ensure that the section length is divisible by 16.
        int64_t len = n - start_block_index;
        int64_t rem = len & 15; // % 16
        len -= rem;
        len += start_block_index;

        __m256i right = _mm256_set1_epi64x(delta);

#if MDDS_USE_OPENMP
#pragma omp parallel for
#endif
        for (int64_t i = start_block_index; i < len; i += 16)
        {
            __m256i* dst = (__m256i*)&block_store.positions[i];
            _mm256_storeu_si256(dst, _mm256_add_epi64(_mm256_loadu_si256(dst), right));

            __m256i* dst4 = (__m256i*)&block_store.positions[i + 4];
            _mm256_storeu_si256(dst4, _mm256_add_epi64(_mm256_loadu_si256(dst4), right));

            __m256i* dst8 = (__m256i*)&block_store.positions[i + 8];
            _mm256_storeu_si256(dst8, _mm256_add_epi64(_mm256_loadu_si256(dst8), right));

            __m256i* dst12 = (__m256i*)&block_store.positions[i + 12];
            _mm256_storeu_si256(dst12, _mm256_add_epi64(_mm256_loadu_si256(dst12), right));
        }

        rem += len;
        for (int64_t i = len; i < rem; ++i)
            block_store.positions[i] += delta;
    }
};

template<typename Blks>
struct adjust_block_positions<Blks, lu_factor_t::avx2_x64_lu8>
{
    void operator()(Blks& block_store, int64_t start_block_index, int64_t delta) const
    {
        static_assert(
            sizeof(typename decltype(block_store.positions)::value_type) == 8,
            "This code works only when the position values are 64-bit wide.");

        int64_t n = block_store.positions.size();

        if (start_block_index >= n)
            return;

        // Ensure that the section length is divisible by 16.
        int64_t len = n - start_block_index;
        int64_t rem = len & 31; // % 32
        len -= rem;
        len += start_block_index;

        __m256i right = _mm256_set1_epi64x(delta);

#if MDDS_USE_OPENMP
#pragma omp parallel for
#endif
        for (int64_t i = start_block_index; i < len; i += 32)
        {
            __m256i* dst = (__m256i*)&block_store.positions[i];
            _mm256_storeu_si256(dst, _mm256_add_epi64(_mm256_loadu_si256(dst), right));

            __m256i* dst4 = (__m256i*)&block_store.positions[i + 4];
            _mm256_storeu_si256(dst4, _mm256_add_epi64(_mm256_loadu_si256(dst4), right));

            __m256i* dst8 = (__m256i*)&block_store.positions[i + 8];
            _mm256_storeu_si256(dst8, _mm256_add_epi64(_mm256_loadu_si256(dst8), right));

            __m256i* dst12 = (__m256i*)&block_store.positions[i + 12];
            _mm256_storeu_si256(dst12, _mm256_add_epi64(_mm256_loadu_si256(dst12), right));

            __m256i* dst16 = (__m256i*)&block_store.positions[i + 16];
            _mm256_storeu_si256(dst16, _mm256_add_epi64(_mm256_loadu_si256(dst16), right));

            __m256i* dst20 = (__m256i*)&block_store.positions[i + 20];
            _mm256_storeu_si256(dst20, _mm256_add_epi64(_mm256_loadu_si256(dst20), right));

            __m256i* dst24 = (__m256i*)&block_store.positions[i + 24];
            _mm256_storeu_si256(dst24, _mm256_add_epi64(_mm256_loadu_si256(dst24), right));

            __m256i* dst28 = (__m256i*)&block_store.positions[i + 28];
            _mm256_storeu_si256(dst28, _mm256_add_epi64(_mm256_loadu_si256(dst28), right));
        }

        rem += len;
        for (int64_t i = len; i < rem; ++i)
            block_store.positions[i] += delta;
    }
};

#endif // __AVX2__

}}}} // namespace mdds::mtv::soa::detail

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
