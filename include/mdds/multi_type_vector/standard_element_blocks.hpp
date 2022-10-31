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
#pragma once

#include "types.hpp"
#include "util.hpp"
#include "block_funcs.hpp"
#include "macro.hpp"

namespace mdds { namespace mtv {

constexpr element_t element_type_boolean = element_type_reserved_start;
constexpr element_t element_type_int8 = element_type_reserved_start + 1;
constexpr element_t element_type_uint8 = element_type_reserved_start + 2;
constexpr element_t element_type_int16 = element_type_reserved_start + 3;
constexpr element_t element_type_uint16 = element_type_reserved_start + 4;
constexpr element_t element_type_int32 = element_type_reserved_start + 5;
constexpr element_t element_type_uint32 = element_type_reserved_start + 6;
constexpr element_t element_type_int64 = element_type_reserved_start + 7;
constexpr element_t element_type_uint64 = element_type_reserved_start + 8;
constexpr element_t element_type_float = element_type_reserved_start + 9;
constexpr element_t element_type_double = element_type_reserved_start + 10;
constexpr element_t element_type_string = element_type_reserved_start + 11;

using boolean_element_block = default_element_block<element_type_boolean, bool>;
using int8_element_block = default_element_block<element_type_int8, int8_t>;
using uint8_element_block = default_element_block<element_type_uint8, uint8_t>;
using int16_element_block = default_element_block<element_type_int16, int16_t>;
using uint16_element_block = default_element_block<element_type_uint16, uint16_t>;
using int32_element_block = default_element_block<element_type_int32, int32_t>;
using uint32_element_block = default_element_block<element_type_uint32, uint32_t>;
using int64_element_block = default_element_block<element_type_int64, int64_t>;
using uint64_element_block = default_element_block<element_type_uint64, uint64_t>;
using float_element_block = default_element_block<element_type_float, float>;
using double_element_block = default_element_block<element_type_double, double>;
using string_element_block = default_element_block<element_type_string, std::string>;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(bool, element_type_boolean, false, boolean_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(int8_t, element_type_int8, 0, int8_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(uint8_t, element_type_uint8, 0, uint8_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(int16_t, element_type_int16, 0, int16_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(uint16_t, element_type_uint16, 0, uint16_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(int32_t, element_type_int32, 0, int32_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(uint32_t, element_type_uint32, 0, uint32_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(int64_t, element_type_int64, 0, int64_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(uint64_t, element_type_uint64, 0, uint64_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(float, element_type_float, 0.0, float_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(double, element_type_double, 0.0, double_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(std::string, element_type_string, std::string(), string_element_block)

struct standard_element_blocks_traits : public default_traits
{
    using block_funcs = element_block_funcs<
        boolean_element_block, int8_element_block, uint8_element_block, int16_element_block, uint16_element_block,
        int32_element_block, uint32_element_block, int64_element_block, uint64_element_block, float_element_block,
        double_element_block, string_element_block>;
};

}} // namespace mdds::mtv

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
