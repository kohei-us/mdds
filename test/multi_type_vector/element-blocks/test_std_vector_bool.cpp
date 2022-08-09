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

#include "test_main.hpp"

#include <mdds/multi_type_vector/types.hpp>
#include <mdds/multi_type_vector/block_funcs.hpp>

#include <vector>
#include <deque>

void mtv_test_element_blocks_std_vector_bool()
{
    stack_printer __stack_printer__(__func__);

    constexpr mdds::mtv::element_t element_type_bool = mdds::mtv::element_type_user_start + 3;
    using this_block = mdds::mtv::default_element_block<element_type_bool, bool, std::vector>;

    static_assert(this_block::block_type == element_type_bool);
    static_assert(mdds::mtv::detail::has_std_vector_bool_store<this_block>::type::value);

    auto* blk = this_block::create_block(10);

    [[maybe_unused]] auto v = mdds::mtv::detail::get_block_element_at<this_block>(*blk, 2);

    this_block::delete_block(blk);
}

void mtv_test_element_blocks_std_deque_bool()
{
    stack_printer __stack_printer__(__func__);

    constexpr mdds::mtv::element_t element_type_bool = mdds::mtv::element_type_user_start + 2;
    using this_block = mdds::mtv::default_element_block<element_type_bool, bool, std::deque>;

    static_assert(this_block::block_type == element_type_bool);
    static_assert(!mdds::mtv::detail::has_std_vector_bool_store<this_block>::type::value);

    auto* blk = this_block::create_block(10);

    [[maybe_unused]] auto v = mdds::mtv::detail::get_block_element_at<this_block>(*blk, 2);

    this_block::delete_block(blk);
}

void mtv_test_element_blocks_delayed_delete_vector_bool()
{
    stack_printer __stack_printer__(__func__);

    constexpr mdds::mtv::element_t element_type_bool = mdds::mtv::element_type_user_start + 1;
    using this_block = mdds::mtv::default_element_block<element_type_bool, bool, mdds::mtv::delayed_delete_vector>;

    static_assert(this_block::block_type == element_type_bool);
    static_assert(mdds::mtv::detail::has_std_vector_bool_store<this_block>::type::value);

    auto* blk = this_block::create_block(10);

    [[maybe_unused]] auto v = mdds::mtv::detail::get_block_element_at<this_block>(*blk, 2);

    this_block::delete_block(blk);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
