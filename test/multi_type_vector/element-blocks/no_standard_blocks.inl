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

// NB: code in this file must be soa vs aos neutral.

#include <mdds/global.hpp>

#include <deque>
#include <vector>
#include <type_traits>

namespace mdds { namespace mtv {

// These variables are defined in the header, so if it's included it should
// cause a compiler error.
constexpr element_t element_type_boolean = element_type_user_start;
constexpr element_t element_type_int32 = element_type_user_start + 1;
constexpr element_t element_type_uint32 = element_type_user_start + 2;

using boolean_element_block = default_element_block<element_type_boolean, bool, std::deque>;
using int32_element_block = default_element_block<element_type_int32, std::int32_t, std::vector>;
using uint32_element_block = default_element_block<element_type_uint32, std::uint32_t, delayed_delete_vector>;

struct standard_element_blocks_trait;
static_assert(
    !mdds::is_complete<standard_element_blocks_trait>::value,
    "The standard_element_blocks_trait struct should not have been defined.");

}} // namespace mdds::mtv

static_assert(std::is_same_v<mdds::mtv::boolean_element_block::store_type, std::deque<bool>>);
static_assert(std::is_same_v<mdds::mtv::int32_element_block::store_type, std::vector<std::int32_t>>);
static_assert(std::is_same_v<mdds::mtv::uint32_element_block::store_type, mdds::mtv::delayed_delete_vector<std::uint32_t>>);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

