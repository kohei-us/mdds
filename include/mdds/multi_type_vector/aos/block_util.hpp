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

#ifndef INCLUDED_MDDS_MULTI_TYPE_VECTOR_DIR_AOS_BLOCK_UTIL_HPP
#define INCLUDED_MDDS_MULTI_TYPE_VECTOR_DIR_AOS_BLOCK_UTIL_HPP

#include "mdds/global.hpp"
#include "../types.hpp"

namespace mdds { namespace mtv { namespace aos { namespace detail {

template<typename Blks, lu_factor_t F>
struct adjust_block_positions
{
    void operator()(Blks& blocks, int64_t start_block_index, int64_t delta) const
    {
        static_assert(
            mdds::detail::invalid_static_int<F>, "The loop-unrolling factor must be one of 0, 4, 8, 16, or 32.");
    }
};

template<typename Blks>
struct adjust_block_positions<Blks, lu_factor_t::none>
{
    void operator()(Blks& blocks, int64_t start_block_index, int64_t delta) const
    {
        int64_t n = blocks.size();

        if (start_block_index >= n)
            return;

#if MDDS_USE_OPENMP
#pragma omp parallel for
#endif
        for (int64_t i = start_block_index; i < n; ++i)
            blocks[i].position += delta;
    }
};

template<typename Blks>
struct adjust_block_positions<Blks, lu_factor_t::lu4>
{
    void operator()(Blks& blocks, int64_t start_block_index, int64_t delta) const
    {
        int64_t n = blocks.size();

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
            blocks[i].position += delta;
            blocks[i + 1].position += delta;
            blocks[i + 2].position += delta;
            blocks[i + 3].position += delta;
        }

        rem += len;
        for (int64_t i = len; i < rem; ++i)
            blocks[i].position += delta;
    }
};

template<typename Blks>
struct adjust_block_positions<Blks, lu_factor_t::lu8>
{
    void operator()(Blks& blocks, int64_t start_block_index, int64_t delta) const
    {
        int64_t n = blocks.size();

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
            blocks[i].position += delta;
            blocks[i + 1].position += delta;
            blocks[i + 2].position += delta;
            blocks[i + 3].position += delta;
            blocks[i + 4].position += delta;
            blocks[i + 5].position += delta;
            blocks[i + 6].position += delta;
            blocks[i + 7].position += delta;
        }

        rem += len;
        for (int64_t i = len; i < rem; ++i)
            blocks[i].position += delta;
    }
};

template<typename Blks>
struct adjust_block_positions<Blks, lu_factor_t::lu16>
{
    void operator()(Blks& blocks, int64_t start_block_index, int64_t delta) const
    {
        int64_t n = blocks.size();

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
            blocks[i].position += delta;
            blocks[i + 1].position += delta;
            blocks[i + 2].position += delta;
            blocks[i + 3].position += delta;
            blocks[i + 4].position += delta;
            blocks[i + 5].position += delta;
            blocks[i + 6].position += delta;
            blocks[i + 7].position += delta;
            blocks[i + 8].position += delta;
            blocks[i + 9].position += delta;
            blocks[i + 10].position += delta;
            blocks[i + 11].position += delta;
            blocks[i + 12].position += delta;
            blocks[i + 13].position += delta;
            blocks[i + 14].position += delta;
            blocks[i + 15].position += delta;
        }

        rem += len;
        for (int64_t i = len; i < rem; ++i)
            blocks[i].position += delta;
    }
};

template<typename Blks>
struct adjust_block_positions<Blks, lu_factor_t::lu32>
{
    void operator()(Blks& blocks, int64_t start_block_index, int64_t delta) const
    {
        int64_t n = blocks.size();

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
            blocks[i].position += delta;
            blocks[i + 1].position += delta;
            blocks[i + 2].position += delta;
            blocks[i + 3].position += delta;
            blocks[i + 4].position += delta;
            blocks[i + 5].position += delta;
            blocks[i + 6].position += delta;
            blocks[i + 7].position += delta;
            blocks[i + 8].position += delta;
            blocks[i + 9].position += delta;
            blocks[i + 10].position += delta;
            blocks[i + 11].position += delta;
            blocks[i + 12].position += delta;
            blocks[i + 13].position += delta;
            blocks[i + 14].position += delta;
            blocks[i + 15].position += delta;
            blocks[i + 16].position += delta;
            blocks[i + 17].position += delta;
            blocks[i + 18].position += delta;
            blocks[i + 19].position += delta;
            blocks[i + 20].position += delta;
            blocks[i + 21].position += delta;
            blocks[i + 22].position += delta;
            blocks[i + 23].position += delta;
            blocks[i + 24].position += delta;
            blocks[i + 25].position += delta;
            blocks[i + 26].position += delta;
            blocks[i + 27].position += delta;
            blocks[i + 28].position += delta;
            blocks[i + 29].position += delta;
            blocks[i + 30].position += delta;
            blocks[i + 31].position += delta;
        }

        rem += len;
        for (int64_t i = len; i < rem; ++i)
            blocks[i].position += delta;
    }
};

}}}} // namespace mdds::mtv::aos::detail

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
