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

#include "./block_funcs.hpp"

#include <sstream>

namespace mdds { namespace mtv {

/**
 * Empty event function handler structure, used when no custom function
 * handler is specified.
 */
struct empty_event_func
{
    /**
     * Callback function for element block acquisition events.  This gets called
     * whenever the container acquires a new element block either as a result of
     * a new element block creation or a transfer of an existing element block
     * from another container.
     *
     * @param block pointer to the acquired element block instance.
     */
    void element_block_acquired(const base_element_block* block)
    {
        (void)block;
    }

    /**
     * Callback function for element block release events.  This gets called
     * whenever the container releases an existing element block either because
     * the block is about to be deleted or to be transferred to another
     * container.
     *
     * @param block pointer to the element block instance being released.
     */
    void element_block_released(const base_element_block* block)
    {
        (void)block;
    }
};

/**
 * Default trait to be used when no custom trait is specified.
 */
struct default_traits
{
    /**
     * Class or struct type that contains callback functions for element block
     * events as its member functions.
     */
    using event_func = empty_event_func;

    /**
     * Static value specifying the loop-unrolling factor to use for the block
     * position adjustment function.  This must be a const expression.
     */
    static constexpr lu_factor_t loop_unrolling = lu_factor_t::lu16;

    /**
     * Type that contains block functions used throughout the multi_type_vector
     * implementation.  The user must overwrite this type to specify one or more
     * block types as template arguments to element_block_funcs.  Alternatively,
     * you may be interested in using standard_element_blocks_traits which
     * already supports the pre-defined block types for the optional standard
     * data types.
     */
    using block_funcs = element_block_funcs<>;
};

namespace detail {

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG

template<typename T, typename = void>
struct has_trace : std::false_type
{
};

template<typename T>
struct has_trace<T, decltype((void)T::trace)> : std::true_type
{
};

template<typename Traits>
struct call_trace
{
    int& call_depth;

    call_trace(int& _call_depth) : call_depth(_call_depth)
    {
        ++call_depth;
    }
    ~call_trace() noexcept
    {
        --call_depth;
    }

    void call(std::false_type, const ::mdds::mtv::trace_method_properties_t&) const
    {
        // sink
    }

    void call(std::true_type, const ::mdds::mtv::trace_method_properties_t& props) const
    {
        // In case of recursive calls, only trace the first encountered method.
        if (call_depth <= 1)
            Traits::trace(props);
    }

    void operator()(const ::mdds::mtv::trace_method_properties_t& props) const
    {
        call(has_trace<Traits>{}, props);
    }
};

#endif

inline void throw_block_position_not_found(
    const char* method_sig, int line, size_t pos, size_t block_size, size_t container_size)
{
    std::ostringstream os;
    os << method_sig << "#" << line << ": block position not found! (logical pos=" << pos
       << ", block size=" << block_size << ", logical size=" << container_size << ")";
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
std::pair<_SizeT, bool> calc_input_end_position(const _T& it_begin, const _T& it_end, _SizeT pos, _SizeT total_size)
{
    using ret_type = std::pair<_SizeT, bool>;

    _SizeT length = std::distance(it_begin, it_end);
    if (!length)
        // empty data array.  nothing to do.
        return ret_type(0, false);

    _SizeT end_pos = pos + length - 1;
    if (end_pos >= total_size)
        throw std::out_of_range("Input data sequence is too long.");

    return ret_type(end_pos, true);
}

template<typename T>
T advance_position(const T& pos, int steps)
{
    T ret = pos;

    if (steps > 0)
    {
        while (steps > 0)
        {
            if (ret.second + steps < ret.first->size)
            {
                // element is still in the same block.
                ret.second += steps;
                break;
            }
            else
            {
                steps -= static_cast<int>(ret.first->size - ret.second);
                ++ret.first;
                ret.second = 0;
            }
        }
    }
    else
    {
        while (steps < 0)
        {
            if (static_cast<int>(ret.second) >= -steps)
            {
                ret.second += steps;
                break;
            }
            else
            {
                steps += static_cast<int>(ret.second + 1);
                --ret.first;
                ret.second = ret.first->size - 1;
            }
        }
    }

    return ret;
}

} // namespace detail

}} // namespace mdds::mtv

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG

#define MDDS_MTV_TRACE(method_type) \
    ::mdds::mtv::detail::call_trace<Traits> mdds_mtv_ct(m_trace_call_depth); \
    mdds_mtv_ct({trace_method_t::method_type, this, __func__, "", __FILE__, __LINE__})

#define MDDS_MTV_TRACE_ARGS(method_type, stream) \
    ::mdds::mtv::detail::call_trace<Traits> mdds_mtv_ct(m_trace_call_depth); \
    do \
    { \
        std::ostringstream _os_; \
        _os_ << stream; \
        mdds_mtv_ct({trace_method_t::method_type, this, __func__, _os_.str(), __FILE__, __LINE__}); \
    } while (false)

#else

#define MDDS_MTV_TRACE(...)

#define MDDS_MTV_TRACE_ARGS(...)

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
