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

namespace mdds {
namespace mtv {

using boolean_element_block = default_element_block<mtv::element_type_boolean, bool>;
using int8_element_block = default_element_block<mtv::element_type_int8, int8_t>;
using uint8_element_block = default_element_block<mtv::element_type_uint8, uint8_t>;
using int16_element_block = default_element_block<mtv::element_type_int16, int16_t>;
using uint16_element_block = default_element_block<mtv::element_type_uint16, uint16_t>;
using int32_element_block = default_element_block<mtv::element_type_int32, int32_t>;
using uint32_element_block = default_element_block<mtv::element_type_uint32, uint32_t>;
using int64_element_block = default_element_block<mtv::element_type_int64, int64_t>;
using uint64_element_block = default_element_block<mtv::element_type_uint64, uint64_t>;
using float_element_block = default_element_block<mtv::element_type_float, float>;
using double_element_block = default_element_block<mtv::element_type_double, double>;
using string_element_block = default_element_block<mtv::element_type_string, std::string>;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(bool, mdds::mtv::element_type_boolean, false, mdds::mtv::boolean_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(int8_t, mdds::mtv::element_type_int8, 0, mdds::mtv::int8_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(uint8_t, mdds::mtv::element_type_uint8, 0, mdds::mtv::uint8_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(int16_t, mdds::mtv::element_type_int16, 0, mdds::mtv::int16_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(uint16_t, mdds::mtv::element_type_uint16, 0, mdds::mtv::uint16_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(int32_t, mdds::mtv::element_type_int32, 0, mdds::mtv::int32_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(uint32_t, mdds::mtv::element_type_uint32, 0, mdds::mtv::uint32_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(int64_t, mdds::mtv::element_type_int64, 0, mdds::mtv::int64_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(uint64_t, mdds::mtv::element_type_uint64, 0, mdds::mtv::uint64_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(float, mdds::mtv::element_type_float, 0.0, mdds::mtv::float_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(double, mdds::mtv::element_type_double, 0.0, mdds::mtv::double_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(
    std::string, mdds::mtv::element_type_string, std::string(), mdds::mtv::string_element_block)

} // namespace mtv

namespace detail { namespace mtv {

#if !defined(MDDS_MULTI_TYPE_VECTOR_USE_DEQUE)

template<>
inline bool get_block_element_at<mdds::mtv::boolean_element_block>(
    const mdds::mtv::base_element_block& data, size_t offset)
{
    auto it = mdds::mtv::boolean_element_block::cbegin(data);
    std::advance(it, offset);
    return *it;
}

#endif

}} // namespace detail::mtv

} // namespace mdds

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
