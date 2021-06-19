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

#include "../macro.hpp"
#include "../util.hpp"

namespace mdds { namespace multi_type_vector { namespace soa {

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
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(std::string, mdds::mtv::element_type_string, std::string(), mdds::mtv::string_element_block)

template<typename _CellBlockFunc, typename _EventFunc>
multi_type_vector<_CellBlockFunc, _EventFunc>::multi_type_vector() : m_cur_size(0) {}

template<typename _CellBlockFunc, typename _EventFunc>
multi_type_vector<_CellBlockFunc, _EventFunc>::multi_type_vector(size_type init_size) :
    m_cur_size(init_size)
{
    if (!init_size)
        return;

    // Initialize with an empty block that spans from 0 to max.
    m_block_store.positions.emplace_back(0);
    m_block_store.sizes.emplace_back(init_size);
    m_block_store.element_blocks.emplace_back(nullptr);
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
multi_type_vector<_CellBlockFunc, _EventFunc>::multi_type_vector(size_type init_size, const _T& value) :
    m_cur_size(init_size)
{
    if (!init_size)
        return;

    element_block_type* data = mdds_mtv_create_new_block(init_size, value);
    m_hdl_event.element_block_acquired(data);
    m_block_store.positions.emplace_back(0);
    m_block_store.sizes.emplace_back(init_size);
    m_block_store.element_blocks.emplace_back(data);
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
multi_type_vector<_CellBlockFunc, _EventFunc>::multi_type_vector(size_type init_size, const _T& it_begin, const _T& it_end) :
    m_cur_size(init_size)
{
    if (!m_cur_size)
        return;

    size_type data_len = std::distance(it_begin, it_end);
    if (m_cur_size != data_len)
        throw mdds::invalid_arg_error("Specified size does not match the size of the initial data array.");

    element_block_type* data = mdds_mtv_create_new_block(*it_begin, it_begin, it_end);
    m_hdl_event.element_block_acquired(data);
    m_block_store.positions.emplace_back(0);
    m_block_store.sizes.emplace_back(m_cur_size);
    m_block_store.element_blocks.emplace_back(data);
}

template<typename _CellBlockFunc, typename _EventFunc>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::size_type
multi_type_vector<_CellBlockFunc, _EventFunc>::size() const
{
    return m_cur_size;
}

template<typename _CellBlockFunc, typename _EventFunc>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::size_type
multi_type_vector<_CellBlockFunc, _EventFunc>::block_size() const
{
    return m_block_store.positions.size();
}

template<typename _CellBlockFunc, typename _EventFunc>
bool multi_type_vector<_CellBlockFunc, _EventFunc>::empty() const
{
    return m_block_store.positions.empty();
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc, _EventFunc>::get(size_type pos, _T& value) const
{
    size_type block_index = get_block_position(pos);
    if (block_index == m_block_store.positions.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::get", __LINE__, pos, block_size(), size());

    assert(block_index < m_block_store.element_blocks.size());
    const element_block_type* data = m_block_store.element_blocks[block_index];
    if (!data)
    {
        // empty cell block.
        mdds_mtv_get_empty_value(value);
        return;
    }

    size_type start_row = m_block_store.positions[block_index];
    assert(pos >= start_row);
    size_type offset = pos - start_row;
    mdds_mtv_get_value(*data, offset, value);
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
_T multi_type_vector<_CellBlockFunc, _EventFunc>::get(size_type pos) const
{
    _T cell;
    get(pos, cell);
    return cell;
}

template<typename _CellBlockFunc, typename _EventFunc>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::size_type
multi_type_vector<_CellBlockFunc, _EventFunc>::get_block_position(size_type row, size_type start_block_index) const
{
    if (row >= m_cur_size || start_block_index >= m_block_store.positions.size())
        return m_block_store.positions.size();

    auto it0 = m_block_store.positions.begin();
    std::advance(it0, start_block_index);

    auto it = std::lower_bound(it0, m_block_store.positions.end(), row);

    if (it == m_block_store.positions.end() || *it != row)
    {
        // Binary search has overshot by one block.  Move back one.
        assert(it != it0);
        --it;
    }

    size_type pos = std::distance(it0, it);
    assert(*it <= row);
    assert(row < *it + m_block_store.sizes[pos]);
    return pos + start_block_index;
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

