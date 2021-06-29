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

#ifndef INCLUDED_MDDS_MULTI_TYPE_VECTOR_DIR_UTIL_HPP
#define INCLUDED_MDDS_MULTI_TYPE_VECTOR_DIR_UTIL_HPP

#include <sstream>

namespace mdds {

namespace detail { namespace mtv {

inline void throw_block_position_not_found(
    const char* method_sig, int line, size_t pos, size_t block_size, size_t container_size)
{
    std::ostringstream os;
    os << method_sig << "#" << line << ": block position not found! (logical pos="
        << pos << ", block size=" << block_size << ", logical size=" << container_size << ")";
    throw std::out_of_range(os.str());
}

/**
 * Given a pair of iterators comprising the input value sequence, and a
 * desired logical insertion position, calculate the position of the last
 * input value. Also check if the input value sequence is empty.
 *
 * @exception std::out_of_range if the end position is greater than the last
 *               allowed position of the destination storage.
 *
 * @param it_begin iterator pointing to the first element of the input value
 *                 sequence.
 * @param it_end iterator point to the position past the last element of the
 *               input value sequence.
 * @param pos desired insertion position.
 * @param total_size total logical size of the destination storage.
 *
 * @return position of the last input value (first) and a flag on whether or
 *         not the input value sequence is empty (second).
 */
template<typename _T, typename _SizeT>
std::pair<_SizeT, bool> calc_input_end_position(
    const _T& it_begin, const _T& it_end, _SizeT pos, _SizeT total_size)
{
    using ret_type = std::pair<_SizeT, bool>;

    _SizeT length = std::distance(it_begin, it_end);
    if (!length)
        // empty data array.  nothing to do.
        return ret_type(0, false);

    _SizeT end_pos = pos + length - 1;
    if (end_pos >= total_size)
        throw std::out_of_range("Data array is too long.");

    return ret_type(end_pos, true);
}

}} // namespace detail::mtv

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

