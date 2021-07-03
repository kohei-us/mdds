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

namespace detail {

template<typename _SizeT, typename _VecT>
void erase(_VecT& arr, _SizeT index, _SizeT size)
{
    auto it = arr.begin() + index;
    arr.erase(it, it + size);
}

} // namespace detail

template<typename _CellBlockFunc, typename _EventFunc>
multi_type_vector<_CellBlockFunc, _EventFunc>::multi_type_vector::blocks_type::blocks_type()
{
}

template<typename _CellBlockFunc, typename _EventFunc>
multi_type_vector<_CellBlockFunc, _EventFunc>::multi_type_vector::blocks_type::blocks_type(const blocks_type& other) :
    positions(other.positions),
    sizes(other.sizes),
    element_blocks(other.element_blocks)
{
    for (element_block_type*& data : element_blocks)
    {
        if (data)
            data = element_block_func::clone_block(*data);
    }
}

template<typename _CellBlockFunc, typename _EventFunc>
void multi_type_vector<_CellBlockFunc, _EventFunc>::multi_type_vector::blocks_type::erase(size_type index)
{
    positions.erase(positions.begin() + index);
    sizes.erase(sizes.begin() + index);
    element_blocks.erase(element_blocks.begin() + index);
}

template<typename _CellBlockFunc, typename _EventFunc>
void multi_type_vector<_CellBlockFunc, _EventFunc>::multi_type_vector::blocks_type::erase(
    size_type index, size_type size)
{
    detail::erase(positions, index, size);
    detail::erase(sizes, index, size);
    detail::erase(element_blocks, index, size);
}

template<typename _CellBlockFunc, typename _EventFunc>
void multi_type_vector<_CellBlockFunc, _EventFunc>::multi_type_vector::blocks_type::insert(
    size_type index, size_type size)
{
    positions.insert(positions.begin() + index, size, 0);
    sizes.insert(sizes.begin() + index, size, 0);
    element_blocks.insert(element_blocks.begin() + index, size, nullptr);
}

template<typename _CellBlockFunc, typename _EventFunc>
void multi_type_vector<_CellBlockFunc, _EventFunc>::multi_type_vector::blocks_type::insert(
    size_type index, size_type pos, size_type size, element_block_type* data)
{
    positions.insert(positions.begin() + index, pos);
    sizes.insert(sizes.begin() + index, size);
    element_blocks.insert(element_blocks.begin() + index, data);
}

template<typename _CellBlockFunc, typename _EventFunc>
void multi_type_vector<_CellBlockFunc, _EventFunc>::multi_type_vector::blocks_type::calc_block_position(size_type index)
{
    if (index == 0)
    {
        positions[index] = 0;
        return;
    }

    assert(index < positions.size());
    positions[index] = positions[index-1] + sizes[index-1];
}

template<typename _CellBlockFunc, typename _EventFunc>
void multi_type_vector<_CellBlockFunc, _EventFunc>::multi_type_vector::blocks_type::swap(size_type index1, size_type index2)
{
    std::swap(positions[index1], positions[index2]);
    std::swap(sizes[index1], sizes[index2]);
    std::swap(element_blocks[index1], element_blocks[index2]);
}

template<typename _CellBlockFunc, typename _EventFunc>
void multi_type_vector<_CellBlockFunc, _EventFunc>::multi_type_vector::blocks_type::swap(blocks_type& other)
{
    positions.swap(other.positions);
    sizes.swap(other.sizes);
    element_blocks.swap(other.element_blocks);
}

template<typename _CellBlockFunc, typename _EventFunc>
void multi_type_vector<_CellBlockFunc, _EventFunc>::multi_type_vector::blocks_type::reserve(size_type n)
{
    positions.reserve(n);
    sizes.reserve(n);
    element_blocks.reserve(n);
}

template<typename _CellBlockFunc, typename _EventFunc>
bool multi_type_vector<_CellBlockFunc, _EventFunc>::multi_type_vector::blocks_type::equals(const blocks_type& other) const
{
    if (positions != other.positions)
        return false;

    if (sizes != other.sizes)
        return false;

    if (element_blocks.size() != other.element_blocks.size())
        return false;

    auto it2 = other.element_blocks.cbegin();

    for (const element_block_type* data1 : element_blocks)
    {
        const element_block_type* data2 = *it2++;

        if (data1)
        {
            if (!data2)
                // left is non-empty while right is empty.
                return false;
        }
        else
        {
            if (data2)
                // left is empty while right is non-empty.
                return false;
        }

        if (!data1)
        {
            // Both are empty blocks.
            assert(!data2);
            continue;
        }

        assert(data1 && data2);
        if (!element_block_func::equal_block(*data1, *data2))
            return false;
    }

    return true;
}

template<typename _CellBlockFunc, typename _EventFunc>
void multi_type_vector<_CellBlockFunc, _EventFunc>::multi_type_vector::blocks_type::clear()
{
    positions.clear();
    sizes.clear();
    element_blocks.clear();
}

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
multi_type_vector<_CellBlockFunc, _EventFunc>::multi_type_vector(const multi_type_vector& other) :
    m_hdl_event(other.m_hdl_event),
    m_block_store(other.m_block_store),
    m_cur_size(other.m_cur_size)
{
#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    try
    {
        check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        os << "block integrity check failed in copy construction" << std::endl;
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif
}

template<typename _CellBlockFunc, typename _EventFunc>
multi_type_vector<_CellBlockFunc, _EventFunc>::~multi_type_vector()
{
    delete_element_blocks(0, m_block_store.positions.size());
}

template<typename _CellBlockFunc, typename _EventFunc>
void multi_type_vector<_CellBlockFunc, _EventFunc>::delete_element_block(size_type block_index)
{
    element_block_type* data = m_block_store.element_blocks[block_index];
    if (!data)
        // This block is empty.
        return;

    m_hdl_event.element_block_released(data);
    element_block_func::delete_block(data);
    m_block_store.element_blocks[block_index] = nullptr;
}

template<typename _CellBlockFunc, typename _EventFunc>
void multi_type_vector<_CellBlockFunc, _EventFunc>::delete_element_blocks(size_type start, size_type end)
{
    for (size_type i = start; i < end; ++i)
        delete_element_block(i);
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::set(size_type pos, const _T& value)
{
    size_type block_index = get_block_position(pos);
    if (block_index == m_block_store.positions.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::set", __LINE__, pos, block_size(), size());

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_prev_block;
    dump_blocks(os_prev_block);
#endif

    iterator ret = set_impl(pos, block_index, value);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    try
    {
        check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        os << "block integrity check failed in set (pos=" << pos << ")" << std::endl;
        os << "previous block state:" << std::endl;
        os << os_prev_block.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif

    return ret;
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::set(size_type pos, const _T& it_begin, const _T& it_end)
{
    auto res = mdds::detail::mtv::calc_input_end_position(it_begin, it_end, pos, m_cur_size);

    if (!res.second)
        return iterator();
//      return end();

    size_type end_pos = res.first;
    size_type block_index1 = get_block_position(pos);

    if (block_index1 == m_block_store.positions.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::set", __LINE__, pos, block_size(), size());

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_prev_block;
    dump_blocks(os_prev_block);
#endif

    auto ret = set_cells_impl(pos, end_pos, block_index1, it_begin, it_end);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    try
    {
        check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        os << "block integrity check failed in set (pos=" << pos << ")" << endl;
        os << "previous block state:" << endl;
        os << os_prev_block.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif

    return ret;
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::push_back(const _T& value)
{
#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_prev_block;
    dump_blocks(os_prev_block);
#endif

    auto ret = push_back_impl(value);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    try
    {
        check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        os << "block integrity check failed in push_back" << std::endl;
        os << "previous block state:" << std::endl;
        os << os_prev_block.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif

    return ret;
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::push_back_impl(const _T& value)
{
    element_category_type cat = mdds_mtv_get_element_type(value);
    element_block_type* last_data = m_block_store.element_blocks.empty() ? nullptr : m_block_store.element_blocks.back();

    if (!last_data || cat != get_block_type(*last_data))
    {
        // Either there is no block, or the last block is empty or of different
        // type.  Append a new block.
        size_type block_index = m_block_store.positions.size();
        size_type start_pos = m_cur_size;

        m_block_store.push_back(start_pos, 1, nullptr);
        create_new_block_with_new_cell(block_index, value);
        ++m_cur_size;

        return get_iterator(block_index);
    }

    assert(last_data);
    assert(cat == get_block_type(*last_data));

    // Append the new value to the last block.
    size_type block_index = m_block_store.positions.size() - 1;

    mdds_mtv_append_value(*last_data, value);
    ++m_block_store.sizes.back();
    ++m_cur_size;

    return get_iterator(block_index);
}

template<typename _CellBlockFunc, typename _EventFunc>
mtv::element_t multi_type_vector<_CellBlockFunc, _EventFunc>::get_type(size_type pos) const
{
    size_type block_index = get_block_position(pos);
    if (block_index == m_block_store.positions.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::get_type", __LINE__, pos, block_size(), size());

    element_block_type* blk_data = m_block_store.element_blocks[block_index];
    if (!blk_data)
        return mtv::element_type_empty;

    return mtv::get_block_type(*blk_data);
}

template<typename _CellBlockFunc, typename _EventFunc>
bool multi_type_vector<_CellBlockFunc, _EventFunc>::is_empty(size_type pos) const
{
    size_type block_index = get_block_position(pos);
    if (block_index == m_block_store.positions.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::is_empty", __LINE__, pos, block_size(), size());

    return m_block_store.element_blocks[block_index] == nullptr;
}

template<typename _CellBlockFunc, typename _EventFunc>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::set_empty(size_type start_pos, size_type end_pos)
{
    size_type block_index1 = get_block_position(start_pos);
    if (block_index1 == m_block_store.positions.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::set_empty", __LINE__, start_pos, block_size(), size());

    return set_empty_impl(start_pos, end_pos, block_index1, true);
}

template<typename _CellBlockFunc, typename _EventFunc>
void multi_type_vector<_CellBlockFunc, _EventFunc>::clear()
{
    delete_element_blocks(0, m_block_store.element_blocks.size());
    m_block_store.clear();
    m_cur_size = 0;
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
bool multi_type_vector<_CellBlockFunc, _EventFunc>::set_cells_precheck(
    size_type pos, const _T& it_begin, const _T& it_end, size_type& end_pos)
{
    size_type length = std::distance(it_begin, it_end);
    if (!length)
        // empty data array.  nothing to do.
        return false;

    end_pos = pos + length - 1;
    if (end_pos >= m_cur_size)
        throw std::out_of_range("Data array is too long.");

    return true;
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::set_impl(size_type pos, size_type block_index, const _T& value)
{
    size_type start_row = m_block_store.positions[block_index];

    size_type blk_size = m_block_store.sizes[block_index];
    element_block_type* blk_data = m_block_store.element_blocks[block_index];
    assert(blk_size > 0); // block size should never be zero at any time.

    assert(pos >= start_row);
    size_type pos_in_block = pos - start_row;
    assert(pos_in_block < blk_size);

    if (!blk_data)
    {
        // This is an empty block.
        return set_cell_to_empty_block(block_index, pos_in_block, value);
    }

    element_category_type cat = mdds_mtv_get_element_type(value);

    assert(blk_data);
    element_category_type blk_cat = mdds::mtv::get_block_type(*blk_data);

    if (blk_cat == cat)
    {
        // This block is of the same type as the cell being inserted.
        size_type i = pos - start_row;
        element_block_func::overwrite_values(*blk_data, i, 1);
        mdds_mtv_set_value(*blk_data, i, value);
//      return iterator(block_pos, m_blocks.end(), block_index);
        return iterator();
    }

    assert(blk_cat != cat);

    if (pos == start_row)
    {
        // t|???|x--|???|b - Insertion point is at the start of the block.
        if (blk_size == 1)
        {
            // t|???|x|???|b
            return set_cell_to_non_empty_block_of_size_one(block_index, value);
        }

        assert(blk_size > 1);
        bool blk_prev = is_previous_block_of_type(block_index, cat);
        if (blk_prev)
        {
            // t|xxx|x--|???|b - Append to the previous block.
            m_block_store.sizes[block_index] -= 1;
            m_block_store.positions[block_index] += 1;
            element_block_func::overwrite_values(*m_block_store.element_blocks[block_index], 0, 1);
            element_block_func::erase(*m_block_store.element_blocks[block_index], 0);
            m_block_store.sizes[block_index-1] += 1;
            mdds_mtv_append_value(*m_block_store.element_blocks[block_index-1], value);
//          return get_iterator(block_index-1);
            return iterator();
        }

        // t|---|x--|???|b
        set_cell_to_top_of_data_block(block_index, value);
//      return get_iterator(block_index);
        return iterator();
    }

    if (pos < (start_row + blk_size - 1))
    {
        // t|???|-x-|???|b - Insertion point is in the middle of the block.
        return set_cell_to_middle_of_block(block_index, pos_in_block, value);
    }

    // t|???|--x|???|b - Insertion point is at the end of the block.
    assert(pos == (start_row + m_block_store.sizes[block_index] - 1));
    assert(pos > start_row);
    assert(m_block_store.sizes[block_index] > 1);

    if (block_index == 0)
    {
        if (m_block_store.positions.size() == 1)
        {
            // t|--x|b - This is the only block.
            set_cell_to_bottom_of_data_block(0, value);
//          iterator itr = end();
//          --itr;
//          return itr;
            return iterator();
        }

        bool blk_next = is_next_block_of_type(block_index, cat);
        if (!blk_next)
        {
            // t|--x|---|b - Next block is of different type.
            set_cell_to_bottom_of_data_block(0, value);
//          iterator itr = begin();
//          ++itr;
//          return itr;
            return iterator();
        }

        // t|--x|xxx|b - Next block is of the same type as the new value.

        element_block_func::overwrite_values(*blk_data, blk_size-1, 1);
        element_block_func::erase(*blk_data, blk_size-1);
        m_block_store.sizes[block_index] -= 1;
        mdds_mtv_prepend_value(*m_block_store.element_blocks[block_index+1], value);
        m_block_store.sizes[block_index+1] += 1;
        m_block_store.positions[block_index+1] -= 1;

//      return get_iterator(block_index+1);
        return iterator();
    }

    assert(block_index > 0);

    if (block_index == m_block_store.positions.size() - 1)
    {
        // t|???|--x|b - This is the last block.
        set_cell_to_bottom_of_data_block(block_index, value);
//      iterator itr = end();
//      --itr;
//      return itr;
        return iterator();
    }

    bool blk_next = is_next_block_of_type(block_index, cat);
    if (!blk_next)
    {
        // t|???|--x|---|b - Next block is of different type than the new
        // value's.
        set_cell_to_bottom_of_data_block(block_index, value);
//      return get_iterator(block_index+1);
        return iterator();
    }

    // t|???|--x|xxx|b - The next block is of the same type as the new value.
    element_block_func::overwrite_values(*blk_data, blk_size-1, 1);
    element_block_func::erase(*blk_data, blk_size-1);
    m_block_store.sizes[block_index] -= 1;
    mdds_mtv_prepend_value(*m_block_store.element_blocks[block_index+1], value);
    m_block_store.sizes[block_index+1] += 1;
    m_block_store.positions[block_index+1] -= 1;

//  return get_iterator(block_index+1);
    return iterator();
}

template<typename _CellBlockFunc, typename _EventFunc>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::set_empty_impl(
    size_type start_pos, size_type end_pos, size_type block_index1, bool overwrite)
{
    if (start_pos > end_pos)
        throw std::out_of_range("Start row is larger than the end row.");

    size_type block_index2 = get_block_position(end_pos, block_index1);
    if (block_index2 == m_block_store.positions.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::set_empty_impl", __LINE__, end_pos, block_size(), size());

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_prev_block;
    dump_blocks(os_prev_block);
#endif

    iterator ret_it;
    if (block_index1 == block_index2)
        ret_it = set_empty_in_single_block(start_pos, end_pos, block_index1, overwrite);
    else
        ret_it = set_empty_in_multi_blocks(
            start_pos, end_pos, block_index1, block_index2, overwrite);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    try
    {
        check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        os << "block integrity check failed in set_empty(start_pos=" << start_pos << "; end_pos=" << end_pos << ")" << std::endl;
        os << "previous block state:" << std::endl;
        os << os_prev_block.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif
    return ret_it;
}

template<typename _CellBlockFunc, typename _EventFunc>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::set_empty_in_single_block(
    size_type start_row, size_type end_row, size_type block_index, bool overwrite)
{
    // Range is within a single block.
    element_block_type* blk_data = m_block_store.element_blocks[block_index];
    if (!blk_data)
        // This block is already empty.  Do nothing.
        return get_iterator(block_index);

    size_type start_row_in_block = m_block_store.positions[block_index];
    assert(start_row_in_block + m_block_store.sizes[block_index] >= 1);
    size_type end_row_in_block = start_row_in_block + m_block_store.sizes[block_index] - 1;
    size_type empty_block_size = end_row - start_row + 1;

    if (start_row == start_row_in_block)
    {
        // start row coincides with the start of a block.

        if (end_row == end_row_in_block)
            return set_whole_block_empty(block_index, overwrite);

        // Set the upper part of the block empty.
        if (overwrite)
            element_block_func::overwrite_values(*blk_data, 0, empty_block_size);

        element_block_func::erase(*blk_data, 0, empty_block_size);
        m_block_store.sizes[block_index] -= empty_block_size;

        // Check if the preceding block (if exists) is also empty.
        bool blk_prev = is_previous_block_of_type(block_index, mtv::element_type_empty);
        if (blk_prev)
        {
            // Extend the previous empty block.
            m_block_store.sizes[block_index-1] += empty_block_size;
            m_block_store.positions[block_index] += empty_block_size;
            return get_iterator(block_index-1);
        }

        // Insert a new empty block before the current one.
        size_type block_position = m_block_store.positions[block_index];
        m_block_store.positions[block_index] += empty_block_size;
        m_block_store.insert(block_index, block_position, empty_block_size, nullptr);
        return get_iterator(block_index);
    }

    if (end_row == end_row_in_block)
    {
        // end row equals the end of a block.
        assert(start_row > start_row_in_block);

        // Set the lower part of the block empty.
        size_type start_pos = start_row - start_row_in_block;
        if (overwrite)
            element_block_func::overwrite_values(*blk_data, start_pos, empty_block_size);

        element_block_func::erase(*blk_data, start_pos, empty_block_size);
        m_block_store.sizes[block_index] -= empty_block_size;

        // Check if the following block (if exists) is also empty.
        bool blk_next = is_next_block_of_type(block_index, mtv::element_type_empty);
        if (blk_next)
        {
            // Extend the next empty block to cover the new empty segment.
            m_block_store.sizes[block_index+1] += empty_block_size;
            m_block_store.positions[block_index+1] = start_row;
        }
        else
        {
            // Insert a new empty block after the current one.
            m_block_store.insert(block_index+1, start_row, empty_block_size, nullptr);
        }

        return get_iterator(block_index+1);
    }

    // Empty the middle part of a block.
    assert(end_row_in_block - end_row > 0);
    set_new_block_to_middle(block_index, start_row-start_row_in_block, empty_block_size, overwrite);

    return get_iterator(block_index+1);
}

template<typename _CellBlockFunc, typename _EventFunc>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::set_empty_in_multi_blocks(
    size_type start_row, size_type end_row,
    size_type block_index1, size_type block_index2, bool overwrite)
{
    assert(block_index1 < block_index2);
    size_type start_row_in_block1 = m_block_store.positions[block_index1];
    size_type start_row_in_block2 = m_block_store.positions[block_index2];

    {
        // Empty the lower part of the first block.
        element_block_type* blk_data = m_block_store.element_blocks[block_index1];
        if (blk_data)
        {
            if (start_row_in_block1 == start_row)
            {
                // Empty the whole block.

                bool prev_empty = is_previous_block_of_type(block_index1, mtv::element_type_empty);
                if (prev_empty)
                {
                    // Previous block is empty.  Move the start row to the
                    // first row of the previous block, and make the previous
                    // block 'block 1'.
                    start_row -= m_block_store.sizes[block_index1-1];
                    --block_index1;
                }
                else
                {
                    // Make block 1 empty.
                    if (!overwrite)
                        element_block_func::resize_block(*blk_data, 0);

                    delete_element_block(block_index1);
                }
            }
            else
            {
                // Empty the lower part.
                size_type new_size = start_row - start_row_in_block1;
                if (overwrite)
                    element_block_func::overwrite_values(
                        *blk_data, new_size, m_block_store.sizes[block_index1]-new_size);

                element_block_func::resize_block(*blk_data, new_size);
                m_block_store.sizes[block_index1] = new_size;
            }
        }
        else
        {
            // First block is already empty.  Adjust the start row of the new
            // empty range.
            start_row = start_row_in_block1;
        }
    }

    size_type end_block_to_erase = block_index2; // End block position is non-inclusive.

    {
        // Empty the upper part of the last block.
        element_block_type* blk_data = m_block_store.element_blocks[block_index2];
        size_type last_row_in_block = start_row_in_block2 + m_block_store.sizes[block_index2] - 1;

        if (blk_data)
        {
            if (last_row_in_block == end_row)
            {
                // Delete the whole block.
                ++end_block_to_erase;

                // Check if the following block (if exists) is also empty.
                bool next_empty = is_next_block_of_type(block_index2, mtv::element_type_empty);
                if (next_empty)
                {
                    // The following block is also empty.
                    end_row += m_block_store.sizes[block_index2+1];
                    ++end_block_to_erase;
                }
            }
            else
            {
                // Empty the upper part.
                size_type size_to_erase = end_row - start_row_in_block2 + 1;
                if (overwrite)
                    element_block_func::overwrite_values(*blk_data, 0, size_to_erase);

                element_block_func::erase(*blk_data, 0, size_to_erase);
                m_block_store.sizes[block_index2] -= size_to_erase;
                m_block_store.positions[block_index2] = start_row_in_block2 + size_to_erase;
            }
        }
        else
        {
            // Last block is empty.  Delete this block and adjust the end row
            // of the new empty range.
            ++end_block_to_erase;
            end_row = last_row_in_block;
        }
    }

    if (end_block_to_erase - block_index1 > 1)
    {
        // Remove all blocks in-between, from block_index1+1 to end_block_to_erase-1.

        for (size_type i = block_index1 + 1; i < end_block_to_erase; ++i)
        {
            element_block_type *data = m_block_store.element_blocks[i];
            if (!overwrite && data)
                element_block_func::resize_block(*data, 0);

            delete_element_block(i);
        }

        size_type n_erase_blocks = end_block_to_erase - block_index1 - 1;
        m_block_store.erase(block_index1+1, n_erase_blocks);
    }

    element_block_type* blk_data = m_block_store.element_blocks[block_index1];
    size_type empty_block_size = end_row - start_row + 1;
    if (blk_data)
    {
        // Insert a new empty block after the first block.
        m_block_store.insert(block_index1+1, start_row, empty_block_size, nullptr);
        return get_iterator(block_index1+1);
    }

    // Current block is already empty. Just extend its size.
    m_block_store.sizes[block_index1] = empty_block_size;
    m_block_store.positions[block_index1] = start_row;
    return get_iterator(block_index1);
}

template<typename _CellBlockFunc, typename _EventFunc>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::set_whole_block_empty(size_type block_index, bool overwrite)
{
    element_block_type* blk_data = m_block_store.element_blocks[block_index];
    if (!overwrite)
        // Resize block to 0 before deleting, to prevent its elements from getting deleted.
        element_block_func::resize_block(*blk_data, 0);

    delete_element_block(block_index);

    bool blk_prev = is_previous_block_of_type(block_index, mtv::element_type_empty);
    bool blk_next = is_next_block_of_type(block_index, mtv::element_type_empty);

    // Merge with adjacent block(s) if necessary.
    if (blk_prev)
    {
        element_block_type* blk_prev_data = m_block_store.element_blocks[block_index-1];
        assert(!blk_prev_data);

        if (blk_next)
        {
            // Both preceding and next blocks are empty.
            element_block_type* blk_next_data = m_block_store.element_blocks[block_index+1];
            assert(!blk_next_data);

            m_block_store.sizes[block_index-1] +=
                m_block_store.sizes[block_index] + m_block_store.sizes[block_index+1];

            // No need delete the current and next element blocks since they are both empty.
            m_block_store.erase(block_index, 2);

            return get_iterator(block_index-1);
        }

        // Only the preceding block is empty. Merge the current block with the previous.
        m_block_store.sizes[block_index-1] += m_block_store.sizes[block_index];
        m_block_store.erase(block_index);

        return get_iterator(block_index-1);
    }
    else if (blk_next)
    {
        { std::ostringstream os; os << __FILE__ << "#" << __LINE__ << " (multi_type_vector:set_whole_block_empty): WIP"; throw std::runtime_error(os.str()); }
#if 0
        assert(!blk_next->mp_data);

        // Only the next block is empty. Merge the next block with the current.
        blk->m_size += blk_next->m_size;
        typename blocks_type::iterator it = m_blocks.begin();
        std::advance(it, block_index+1);
        m_blocks.erase(it);

#endif
        return get_iterator(block_index);
    }

    return get_iterator(block_index);
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::set_cells_impl(
    size_type row, size_type end_row, size_type block_index1, const _T& it_begin, const _T& it_end)
{
    size_type block_index2 = get_block_position(end_row, block_index1);
    if (block_index2 == m_block_store.positions.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::set_cells_impl", __LINE__, end_row, block_size(), size());

    if (block_index1 == block_index2)
    {
        // The whole data array will fit in a single block.
        return set_cells_to_single_block(row, end_row, block_index1, it_begin, it_end);
    }

    { std::ostringstream os; os << __FILE__ << "#" << __LINE__ << " (multi_type_vector:set_cells_impl): WIP"; throw std::runtime_error(os.str()); }
#if 0
    return set_cells_to_multi_blocks(
        row, end_row, block_index1, block_index2, it_begin, it_end);
#endif
    return iterator();
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::set_cells_to_single_block(
    size_type start_row, size_type end_row, size_type block_index,
    const _T& it_begin, const _T& it_end)
{
    assert(it_begin != it_end);
    assert(!m_block_store.positions.empty());

    element_category_type cat = mdds_mtv_get_element_type(*it_begin);
    size_type start_row_in_block = m_block_store.positions[block_index];
    size_type data_length = std::distance(it_begin, it_end);
    element_block_type* blk_data = m_block_store.element_blocks[block_index];

    if (blk_data && mdds::mtv::get_block_type(*blk_data) == cat)
    {
#if 0
        // simple overwrite.
        size_type offset = start_row - start_row_in_block;
        element_block_func::overwrite_values(*blk->mp_data, offset, data_length);
        if (!offset && data_length == blk->m_size)
            // Overwrite the whole block.  It's faster to use assign_values.
            mdds_mtv_assign_values(*blk->mp_data, *it_begin, it_begin, it_end);
        else
            mdds_mtv_set_values(*blk->mp_data, offset, *it_begin, it_begin, it_end);

#endif
//      return get_iterator(block_index);
        return iterator();
    }

    size_type end_row_in_block = start_row_in_block + m_block_store.sizes[block_index] - 1;
    if (start_row == start_row_in_block)
    {
        if (end_row == end_row_in_block)
        {
#if 0
            // Check if we could append it to the previous block.
            if (append_to_prev_block(block_index, cat, end_row-start_row+1, it_begin, it_end))
            {
                delete_element_block(*blk);
                m_blocks.erase(m_blocks.begin()+block_index);

                // Check if we need to merge it with the next block.
                --block_index;
                merge_with_next_block(block_index);
//              return get_iterator(block_index);
                return iterator();
            }

            // Replace the whole block.
            if (blk->mp_data)
            {
                m_hdl_event.element_block_released(blk->mp_data);
                element_block_func::delete_block(blk->mp_data);
            }

            blk->mp_data = element_block_func::create_new_block(cat, 0);
            m_hdl_event.element_block_acquired(blk->mp_data);
            mdds_mtv_assign_values(*blk->mp_data, *it_begin, it_begin, it_end);
            merge_with_next_block(block_index);
#endif
//          return get_iterator(block_index);
            return iterator();
        }

        { std::ostringstream os; os << __FILE__ << "#" << __LINE__ << " (multi_type_vector:set_cells_to_single_block): WIP"; throw std::runtime_error(os.str()); }
        // Replace the upper part of the block.
#if 0
        // Shrink the current block first.
        size_type length = end_row_in_block - end_row;
        blk->m_size = length;
#endif

        if (blk_data)
        {
#if 0
            // Erase the upper part of the data from the current data array.
            std::unique_ptr<element_block_type, element_block_deleter> new_data(
                element_block_func::create_new_block(mdds::mtv::get_block_type(*blk->mp_data), 0));

            if (!new_data)
                throw std::logic_error("failed to instantiate a new data array.");

            size_type pos = end_row - start_row_in_block + 1;
            element_block_func::assign_values_from_block(*new_data, *blk->mp_data, pos, length);
            element_block_func::overwrite_values(*blk->mp_data, 0, pos);

            // Resize the block to zero before deleting, to prevent the
            // managed cells from being deleted when the block is deleted.
            element_block_func::resize_block(*blk->mp_data, 0);
            element_block_func::delete_block(blk->mp_data);
            blk->mp_data = new_data.release();

            // We don't call element block event listeners here.
#endif
        }

        { std::ostringstream os; os << __FILE__ << "#" << __LINE__ << " (multi_type_vector:set_cells_to_single_block): WIP"; throw std::runtime_error(os.str()); }
#if 0
        length = end_row - start_row + 1;
        if (append_to_prev_block(block_index, cat, length, it_begin, it_end))
        {
            // The new values have been successfully appended to the previous block.
            blk->m_position += length;
            return get_iterator(block_index-1);
        }

        // Insert a new block before the current block, and populate it with
        // the new data.
        size_type position = blk->m_position;
        blk->m_position += length;
        m_blocks.emplace(m_blocks.begin()+block_index, position, length);
        blk = &m_blocks[block_index];
        blk->mp_data = element_block_func::create_new_block(cat, 0);
        m_hdl_event.element_block_acquired(blk->mp_data);
        blk->m_size = length;
        mdds_mtv_assign_values(*blk->mp_data, *it_begin, it_begin, it_end);
#endif
//      return get_iterator(block_index);
        return iterator();
    }

    assert(start_row > start_row_in_block);
    if (end_row == end_row_in_block)
    {
        { std::ostringstream os; os << __FILE__ << "#" << __LINE__ << " (multi_type_vector:set_cells_to_single_block): WIP"; throw std::runtime_error(os.str()); }
#if 0
        // Shrink the end of the current block and insert a new block for the new data series after the current block.
        size_type new_size = start_row - start_row_in_block;
        blk->m_size = new_size;
        if (blk->mp_data)
        {
            element_block_func::overwrite_values(*blk->mp_data, new_size, data_length);
            element_block_func::resize_block(*blk->mp_data, new_size);
        }

        new_size = end_row - start_row + 1; // size of the data array being inserted.

        if (block_index < m_blocks.size() - 1)
        {
            // There is a block (or more) after the current block. Check the next block.
            block* blk_next = get_next_block_of_type(block_index, cat);
            if (blk_next)
            {
                // Prepend it to the next block.
                mdds_mtv_prepend_values(*blk_next->mp_data, *it_begin, it_begin, it_end);
                blk_next->m_size += new_size;
                blk_next->m_position -= new_size;
                return get_iterator(block_index+1);
            }

            // Next block has a different data type. Do the normal insertion.
            size_type position = detail::mtv::calc_next_block_position(*blk);
            m_blocks.emplace(m_blocks.begin()+block_index+1, position, new_size);
            blk = &m_blocks[block_index+1];
            blk->mp_data = element_block_func::create_new_block(cat, 0);
            m_hdl_event.element_block_acquired(blk->mp_data);
            mdds_mtv_assign_values(*blk->mp_data, *it_begin, it_begin, it_end);
            return get_iterator(block_index+1);
        }

        // Last block.
        assert(block_index == m_blocks.size() - 1);

        m_blocks.emplace_back(m_cur_size-new_size, new_size);
        blk = &m_blocks.back();
        blk->mp_data = element_block_func::create_new_block(cat, 0);
        m_hdl_event.element_block_acquired(blk->mp_data);
        mdds_mtv_assign_values(*blk->mp_data, *it_begin, it_begin, it_end);
#endif
//      return get_iterator(block_index+1);
        return iterator();
    }

    // t|???|-xx-|???|b - New values will be in the middle of the current block.
    assert(start_row_in_block < start_row && end_row < end_row_in_block);

    block_index = set_new_block_to_middle(
        block_index, start_row-start_row_in_block, end_row-start_row+1, true);

    m_block_store.element_blocks[block_index] = element_block_func::create_new_block(cat, 0);
    blk_data = m_block_store.element_blocks[block_index];
    m_hdl_event.element_block_acquired(blk_data);
    mdds_mtv_assign_values(*blk_data, *it_begin, it_begin, it_end);

//  return get_iterator(block_index+1);
    return iterator();
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::set_cell_to_empty_block(
    size_type block_index, size_type pos_in_block, const _T& cell)
{
    assert(!m_block_store.element_blocks[block_index]); // In this call, the current block is an empty block.

    if (block_index == 0)
    {
        // Topmost block.
        if (m_block_store.positions.size() == 1)
        {
            // this is the only block.
            assert(m_block_store.sizes[block_index] == m_cur_size);
            if (m_cur_size == 1)
            {
                // This column is allowed to have only one row!
                assert(pos_in_block == 0);
                create_new_block_with_new_cell(block_index, cell);
                return iterator();
//              return begin();
            }

            // block has multiple rows.
            if (pos_in_block == 0)
            {
                // Insert into the first cell in this block.
                m_block_store.sizes[block_index] -= 1;
                assert(m_block_store.sizes[block_index] > 0);

                m_block_store.positions.emplace(m_block_store.positions.begin(), 0);
                m_block_store.sizes.emplace(m_block_store.sizes.begin(), 1);
                m_block_store.element_blocks.emplace(m_block_store.element_blocks.begin(), nullptr);

                create_new_block_with_new_cell(0, cell);

                m_block_store.positions[1] = 1;
                return iterator();
//              return begin();
            }

            if (size_type& blk_size = m_block_store.sizes[block_index]; pos_in_block == blk_size - 1)
            {
                // Insert into the last cell in block.
                blk_size -= 1;
                assert(blk_size > 0);

                m_block_store.positions.push_back(blk_size);
                m_block_store.sizes.push_back(1);
                m_block_store.element_blocks.push_back(nullptr);

                create_new_block_with_new_cell(block_index+1, cell);
                return iterator();
//              iterator ret = end();
//              --ret;
//              return ret;
            }

            // Insert into the middle of the block.
            return set_cell_to_middle_of_block(block_index, pos_in_block, cell);
        }

        // This topmost empty block is followed by a non-empty block.

        if (pos_in_block == 0)
        {
            // t|x  |???|b
            assert(block_index < m_block_store.positions.size() - 1);

            if (m_block_store.sizes[block_index] == 1)
            {
                // t|x|???| - Top empty block with only one cell size.
                element_category_type cat = mdds_mtv_get_element_type(cell);
                bool blk_next = is_next_block_of_type(block_index, cat);
                if (blk_next)
                {
                    // t|x|xxx|b - Remove this top empty block of size 1, and
                    // prepend the cell to the next block.
                    delete_element_block(block_index);
                    m_block_store.erase(block_index);
                    m_block_store.sizes[block_index] += 1;
                    m_block_store.positions[block_index] -= 1;
                    mdds_mtv_prepend_value(*m_block_store.element_blocks[block_index], cell);
                }
                else
                {
                    // t|x|---|b
                    create_new_block_with_new_cell(block_index, cell);
                }
            }
            else
            {
                // Shrink this topmost block by one and set the new value above it as a new block of size 1.
                assert(block_index == 0);
                assert(m_block_store.sizes[0] > 1);
                m_block_store.sizes[0] -= 1;
                m_block_store.positions[0] = 1;

                m_block_store.positions.emplace(m_block_store.positions.begin(), 0);
                m_block_store.sizes.emplace(m_block_store.sizes.begin(), 1);
                m_block_store.element_blocks.emplace(m_block_store.element_blocks.begin(), nullptr);
                create_new_block_with_new_cell(0, cell);
            }

            return iterator();
//          return begin();
        }

        if (pos_in_block == m_block_store.sizes[block_index] - 1)
        {
            // Set the cell to the last position of the block, immediately above
            // a non-empty block.
            element_category_type cat = mdds_mtv_get_element_type(cell);
            bool blk_next = is_next_block_of_type(block_index, cat);
            if (blk_next)
            {
                assert(m_block_store.sizes[block_index] > 1);

                // Shrink this empty block by one, and prepend the cell to the next block.
                m_block_store.sizes[block_index] -= 1;
                m_block_store.sizes[block_index+1] += 1;
                m_block_store.positions[block_index+1] -= 1;
                mdds_mtv_prepend_value(*m_block_store.element_blocks[block_index+1], cell);
            }
            else
            {
                // Shrink the current empty block by one, and create a new block of size 1 to store the new value.
                m_block_store.sizes[block_index] -= 1;
                m_block_store.insert(block_index+1, 1);
                m_block_store.calc_block_position(block_index+1);
                m_block_store.sizes[block_index+1] = 1;

                create_new_block_with_new_cell(block_index+1, cell);
            }

            return iterator();
//          return get_iterator(block_index+1);
        }

        // Inserting into the middle of an empty block.
        return set_cell_to_middle_of_block(block_index, pos_in_block, cell);
    }

    // This empty block is right below a non-empty block.
    assert(block_index > 0 && m_block_store.element_blocks[block_index-1] != nullptr);

    if (pos_in_block == 0)
    {
        // Set the value to the top of the block, right below a non-empty block.
        element_category_type blk_cat_prev =
            mdds::mtv::get_block_type(*m_block_store.element_blocks[block_index-1]);
        element_category_type cat = mdds_mtv_get_element_type(cell);

        if (blk_cat_prev == cat)
        {
            // Extend the previous block by one to insert this cell.
            if (m_block_store.sizes[block_index] == 1)
            {
                // Check if we need to merge with the following block.
                if (block_index == m_block_store.positions.size() - 1)
                {
                    // Last block.  Delete this block and extend the previous
                    // block by one.
                    delete_element_block(block_index);
                    m_block_store.pop_back();
                    append_cell_to_block(block_index-1, cell);
                }
                else
                {
                    // Block exists below.
                    bool blk_next = is_next_block_of_type(block_index, blk_cat_prev);
                    if (blk_next)
                    {
                        // Empty block must not be followed by another empty block.
                        assert(m_block_store.element_blocks[block_index+1]);

                        // We need to merge the previous and next blocks, then 
                        // delete the current and next blocks.  Be sure to 
                        // resize the next block to zero to prevent the 
                        // transferred cells to be deleted. 

                        // Check if the next block is bigger.
                        if (m_block_store.sizes[block_index-1] < m_block_store.sizes[block_index+1])
                        {
                            // Prepend the new item to the next block, then
                            // prepend the content of the previous block and
                            // release both previous and current blocks.

                            size_type position = m_block_store.positions[block_index-1];

                            element_block_type* data = m_block_store.element_blocks[block_index];
                            element_block_type* prev_data = m_block_store.element_blocks[block_index-1];
                            element_block_type* next_data = m_block_store.element_blocks[block_index+1];

                            // Increase the size of block and prepend the new cell
                            m_block_store.sizes[block_index+1] += 1;
                            mdds_mtv_prepend_value(*next_data, cell);

                            // Preprend the content of previous block to the next block.
                            size_type prev_size = m_block_store.sizes[block_index-1];
                            element_block_func::prepend_values_from_block(*next_data, *prev_data, 0, prev_size);
                            m_block_store.sizes[block_index+1] += prev_size;
                            m_block_store.positions[block_index+1] = position;

                            // Resize the previous block to zero
                            element_block_func::resize_block(*prev_data, 0);
                            m_hdl_event.element_block_released(prev_data);

                            // Release both blocks which are no longer used
                            element_block_func::delete_block(data);
                            element_block_func::delete_block(prev_data);

                            // Remove the previous and current blocks.
                            m_block_store.erase(block_index-1, 2);
                        }
                        else
                        {
                            // Be sure to resize the next block to zero to prevent the
                            // transferred cells to be deleted.
                            m_block_store.sizes[block_index-1] += 1 + m_block_store.sizes[block_index+1];
                            element_block_type* data = m_block_store.element_blocks[block_index];
                            element_block_type* data_prev = m_block_store.element_blocks[block_index-1];
                            element_block_type* data_next = m_block_store.element_blocks[block_index+1];
                            mdds_mtv_append_value(*data_prev, cell);
                            element_block_func::append_values_from_block(*data_prev, *data_next);
                            element_block_func::resize_block(*data_next, 0);
                            m_hdl_event.element_block_released(data_next);
                            element_block_func::delete_block(data);
                            element_block_func::delete_block(data_next);
                            m_block_store.erase(block_index, 2);
                        }
                    }
                    else
                    {
                        // Ignore the next block. Just extend the previous block.
                        delete_element_block(block_index);
                        m_block_store.erase(block_index);
                        append_cell_to_block(block_index-1, cell);
                    }
                }
            }
            else
            {
                // Extend the previous block to append the cell.
                assert(m_block_store.sizes[block_index] > 1);
                m_block_store.sizes[block_index] -= 1;
                m_block_store.positions[block_index] += 1;
                append_cell_to_block(block_index-1, cell);
            }

            return iterator();
//          return get_iterator(block_index-1);
        }
        else
        {
            // t|---|x  |???|b - Cell type is different from the previous block's.
            if (m_block_store.sizes[block_index] == 1)
            {
                // t|---|x|???|b
                if (block_index == m_block_store.positions.size() - 1)
                {
                    // t|---|x|b - There is no more block below.  Simply turn this empty block into a non-empty one.
                    create_new_block_with_new_cell(block_index, cell);
                }
                else
                {
                    // Check the type of the following non-empty block.
                    assert(block_index < m_block_store.positions.size() - 1);
                    bool blk_next = is_next_block_of_type(block_index, cat);
                    if (blk_next)
                    {
                        // t|---|x|xxx|b - Remove this empty block, and prepend the cell to the next block.
                        m_block_store.sizes[block_index+1] += 1;
                        m_block_store.positions[block_index+1] -= 1;
                        mdds_mtv_prepend_value(*m_block_store.element_blocks[block_index+1], cell);
                        delete_element_block(block_index);
                        m_block_store.erase(block_index);
                    }
                    else
                    {
                        // t|---|x|---|b - Simply turn this empty block into a
                        // non-empty one.
                        create_new_block_with_new_cell(block_index, cell);
                    }
                }
            }
            else
            {
                // t|---|x  |???|b

                size_type new_block_size = m_block_store.sizes[block_index] - 1;
                size_type new_block_position = m_block_store.positions[block_index] + 1;
                m_block_store.sizes[block_index] = 1;
                create_new_block_with_new_cell(block_index, cell);
                m_block_store.insert(block_index+1, new_block_position, new_block_size, nullptr);
            }

//          return get_iterator(block_index);
            return iterator();
        }
    }
    else if (pos_in_block == m_block_store.sizes[block_index] - 1)
    {
        // t|???|  x|???|b - New cell is set to the end of the current block.
        assert(m_block_store.sizes[block_index] > 1);
        if (block_index == m_block_store.positions.size() - 1)
        {
            // t|???|  x|b - The current block is the last block.
            m_block_store.sizes[block_index] -= 1;
            m_block_store.push_back(0, 1, nullptr);
            m_block_store.calc_block_position(block_index+1);
            create_new_block_with_new_cell(block_index+1, cell);

            return iterator();
//          iterator it = end();
//          --it;
//          return it;
        }
        else
        {
            // t|???|  x|???|b - A non-empty block exists below.
            element_category_type cat = mdds_mtv_get_element_type(cell);
            bool blk_next = is_next_block_of_type(block_index, cat);
            if (blk_next)
            {
                // t|???|  x|xxx|b - Shrink this empty block and extend the next block.
                m_block_store.sizes[block_index] -= 1;
                m_block_store.sizes[block_index+1] += 1;
                m_block_store.positions[block_index+1] -= 1;
                mdds_mtv_prepend_value(*m_block_store.element_blocks[block_index+1], cell);
            }
            else
            {
                // t|???|  x|---|b - Shrink this block by one and insert a new block for the new cell.
                m_block_store.sizes[block_index] -= 1;
                m_block_store.insert(block_index+1, 0, 1, nullptr);
                m_block_store.calc_block_position(block_index+1);
                create_new_block_with_new_cell(block_index+1, cell);
            }

//          return get_iterator(block_index+1);
            return iterator();
        }
    }

    // New cell is somewhere in the middle of an empty block.
    return set_cell_to_middle_of_block(block_index, pos_in_block, cell);
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::set_cell_to_non_empty_block_of_size_one(size_type block_index, const _T& cell)
{
    assert(m_block_store.sizes[block_index] == 1);
    assert(m_block_store.element_blocks[block_index]);
    element_category_type cat = mdds_mtv_get_element_type(cell);
    assert(mdds::mtv::get_block_type(*m_block_store.element_blocks[block_index]) != cat);

    if (block_index == 0)
    {
        // t|x|???|b - topmost block of size 1.
        if (block_index == m_block_store.positions.size() - 1)
        {
            // t|x|b - This is the only block.
            create_new_block_with_new_cell(block_index, cell);
//          return begin();
            return iterator();
        }

        // There is a block below.
        bool blk_next = is_next_block_of_type(block_index, cat);
        if (!blk_next)
        {
            // t|x|---|b - Next block is of different type.
            create_new_block_with_new_cell(block_index, cell);
//          return begin();
            return iterator();
        }

        // t|x|xxx|b - Delete this block and prepend the cell to the next block.
        m_block_store.sizes[block_index+1] += 1;
        m_block_store.positions[block_index+1] -= 1;
        mdds_mtv_prepend_value(*m_block_store.element_blocks[block_index+1], cell);
        delete_element_block(block_index);
        m_block_store.erase(block_index);

//      return begin();
        return iterator();
    }

    assert(block_index > 0);

    if (block_index == m_block_store.positions.size() - 1)
    {
        // t|???|x|b - This is the last block and another block exists above.
        element_block_type* prev_data = m_block_store.element_blocks[block_index-1];
        if (!prev_data || mdds::mtv::get_block_type(*prev_data) != cat)
        {
            // t|---|x|b - The previous block is of different type.
            create_new_block_with_new_cell(block_index, cell);
        }
        else
        {
            // t|xxx|x|b - Append the cell to the previous block and remove the current one.
            mdds_mtv_append_value(*m_block_store.element_blocks[block_index-1], cell);
            m_block_store.sizes[block_index-1] += 1;
            delete_element_block(block_index);
            m_block_store.erase(block_index);
        }

//      iterator itr = end();
//      --itr;
//      return itr;
        return iterator();
    }

    // Remove the current block, and check if the cell can be append to the
    // previous block, or prepended to the following block. Also check if the
    // blocks above and below need to be combined.

    if (!m_block_store.element_blocks[block_index-1])
    {
        // t|   |x|???|b - Previous block is empty.
        if (!m_block_store.element_blocks[block_index+1])
        {
            // t|   |x|   |b - Next block is empty too.
            create_new_block_with_new_cell(block_index, cell);
//          return get_iterator(block_index);
            return iterator();
        }

        // Previous block is empty, but the next block is not.
        element_category_type blk_cat_next = mdds::mtv::get_block_type(
            *m_block_store.element_blocks[block_index+1]);

        if (blk_cat_next == cat)
        {
            // t|   |x|xxx|b - Next block is of the same type as the new value.
            delete_element_block(block_index);
            m_block_store.erase(block_index);
            m_block_store.sizes[block_index] += 1;
            m_block_store.positions[block_index] -= 1;
            mdds_mtv_prepend_value(*m_block_store.element_blocks[block_index], cell);
//          return get_iterator(block_index);
            return iterator();
        }

        // t|   |x|---|b
        assert(blk_cat_next != cat);
        create_new_block_with_new_cell(block_index, cell);
//      return get_iterator(block_index);
        return iterator();
    }

    if (!m_block_store.element_blocks[block_index+1])
    {
        // t|---|x|   |b - Next block is empty and the previous block is not.
        element_block_type *prev_data = m_block_store.element_blocks[block_index - 1];
        assert(prev_data);
        element_category_type prev_cat = mdds::mtv::get_block_type(*prev_data);

        if (prev_cat == cat)
        {
            // t|xxx|x|   |b - Append to the previous block.
            m_block_store.sizes[block_index-1] += 1;
            mdds_mtv_append_value(*prev_data, cell);
            delete_element_block(block_index);
            m_block_store.erase(block_index);
//          return get_iterator(block_index-1);
            return iterator();
        }

        { std::ostringstream os; os << __FILE__ << "#" << __LINE__ << " (multi_type_vector:set_impl): WIP"; throw std::runtime_error(os.str()); }
#if 0
        // Just overwrite the current block.
        create_new_block_with_new_cell(blk->mp_data, cell);
#endif
//      return get_iterator(block_index);
        return iterator();
    }

    // t|???|x|???|b - Neither previous nor next blocks are empty.
    element_block_type* prev_data = m_block_store.element_blocks[block_index-1];
    element_block_type* next_data = m_block_store.element_blocks[block_index+1];
    assert(prev_data);
    assert(next_data);
    element_category_type prev_cat = mdds::mtv::get_block_type(*prev_data);
    element_category_type next_cat = mdds::mtv::get_block_type(*next_data);

    if (prev_cat == next_cat)
    {
        if (prev_cat == cat)
        {
            // t|xxx|x|xxx|b - All three blocks are of the same type. Merge all
            // three blocks.
            m_block_store.sizes[block_index-1] += 1 + m_block_store.sizes[block_index+1];
            mdds_mtv_append_value(*prev_data, cell);
            element_block_func::append_values_from_block(*prev_data, *next_data);
            element_block_func::resize_block(*next_data, 0); // to prevent deletion of managed cells on block deletion

            // Delete the current and next blocks.
            delete_element_block(block_index);
            delete_element_block(block_index+1);
            m_block_store.erase(block_index, 2);

//          return get_iterator(block_index-1);
            return iterator();
        }

        // t|---|x|---|b - Just overwrite the current block.
        create_new_block_with_new_cell(block_index, cell);
//      return get_iterator(block_index);
        return iterator();
    }

    assert(prev_cat != next_cat);

    if (prev_cat == cat)
    {
        // t|xxx|x|---|b - Append to the previous block.
        m_block_store.sizes[block_index-1] += 1;
        mdds_mtv_append_value(*m_block_store.element_blocks[block_index-1], cell);
        delete_element_block(block_index);
        m_block_store.erase(block_index);
//      return get_iterator(block_index-1);
        return iterator();
    }

    if (next_cat == cat)
    {
        // t|---|x|xxx|b - Prepend to the next block.
        m_block_store.sizes[block_index+1] += 1;
        m_block_store.positions[block_index+1] -= 1;
        mdds_mtv_prepend_value(*m_block_store.element_blocks[block_index+1], cell);
        delete_element_block(block_index);
        m_block_store.erase(block_index);
//      return get_iterator(block_index);
        return iterator();
    }

    // t|---|x|+++|b - Just overwrite the current block.
    create_new_block_with_new_cell(block_index, cell);
//  return get_iterator(block_index);
    return iterator();
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
typename multi_type_vector<_CellBlockFunc, _EventFunc>::iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::begin()
{
    return iterator(
        { m_block_store.positions.begin(), m_block_store.sizes.begin(), m_block_store.element_blocks.begin() },
        { m_block_store.positions.end(), m_block_store.sizes.end(), m_block_store.element_blocks.end() },
        0
    );
}

template<typename _CellBlockFunc, typename _EventFunc>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::end()
{
    return iterator(
        { m_block_store.positions.end(), m_block_store.sizes.end(), m_block_store.element_blocks.end() },
        { m_block_store.positions.end(), m_block_store.sizes.end(), m_block_store.element_blocks.end() },
        m_block_store.positions.size()
    );
}

template<typename _CellBlockFunc, typename _EventFunc>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::const_iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::begin() const
{
    return cbegin();
}

template<typename _CellBlockFunc, typename _EventFunc>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::const_iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::end() const
{
    return cend();
}

template<typename _CellBlockFunc, typename _EventFunc>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::const_iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::cbegin() const
{
    return const_iterator(
        { m_block_store.positions.cbegin(), m_block_store.sizes.cbegin(), m_block_store.element_blocks.cbegin() },
        { m_block_store.positions.cend(), m_block_store.sizes.cend(), m_block_store.element_blocks.cend() },
        0
    );
}

template<typename _CellBlockFunc, typename _EventFunc>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::const_iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::cend() const
{
    return const_iterator(
        { m_block_store.positions.cend(), m_block_store.sizes.cend(), m_block_store.element_blocks.cend() },
        { m_block_store.positions.cend(), m_block_store.sizes.cend(), m_block_store.element_blocks.cend() },
        m_block_store.positions.size()
    );
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

    size_type pos = std::distance(it0, it) + start_block_index;
    assert(*it <= row);
    assert(row < *it + m_block_store.sizes[pos]);
    return pos;
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc, _EventFunc>::create_new_block_with_new_cell(
    size_type block_index, const _T& cell)
{
    element_block_type* data = m_block_store.element_blocks[block_index];
    if (data)
    {
        m_hdl_event.element_block_released(data);
        element_block_func::delete_block(data);
    }

    // New cell block with size 1.
    data = mdds_mtv_create_new_block(1, cell);
    if (!data)
        throw general_error("Failed to create new block.");

    m_hdl_event.element_block_acquired(data);

    m_block_store.element_blocks[block_index] = data;
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc, _EventFunc>::append_cell_to_block(size_type block_index, const _T& cell)
{
    m_block_store.sizes[block_index] += 1;
    mdds_mtv_append_value(*m_block_store.element_blocks[block_index], cell);
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::iterator
multi_type_vector<_CellBlockFunc, _EventFunc>::set_cell_to_middle_of_block(
    size_type block_index, size_type pos_in_block, const _T& cell)
{
    block_index = set_new_block_to_middle(block_index, pos_in_block, 1, true);
    create_new_block_with_new_cell(block_index, cell);

    // Return the iterator referencing the inserted block.
//  return get_iterator(block_index+1);
    return iterator();
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc, _EventFunc>::set_cell_to_top_of_data_block(size_type block_index, const _T& cell)
{
    // t|---|x--|???|b

    m_block_store.sizes[block_index] -= 1;
    size_type position = m_block_store.positions[block_index];
    m_block_store.positions[block_index] += 1;

    element_block_type* data = m_block_store.element_blocks[block_index];
    if (data)
    {
        element_block_func::overwrite_values(*data, 0, 1);
        element_block_func::erase(*data, 0);
    }

    m_block_store.insert(block_index, position, 1, nullptr);
    create_new_block_with_new_cell(block_index, cell);
}

template<typename _CellBlockFunc, typename _EventFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc, _EventFunc>::set_cell_to_bottom_of_data_block(size_type block_index, const _T& cell)
{
    // Erase the last value of the block.
    assert(block_index < m_block_store.positions.size());
    element_block_type* blk_data = m_block_store.element_blocks[block_index];
    size_type& blk_size = m_block_store.sizes[block_index];
    if (blk_data)
    {
        element_block_func::overwrite_values(*blk_data, blk_size-1, 1);
        element_block_func::erase(*blk_data, blk_size-1);
    }
    blk_size -= 1;

    // Insert a new block of size one with the new value.
    m_block_store.insert(block_index+1, 0, 1, nullptr);
    m_block_store.calc_block_position(block_index+1);
    create_new_block_with_new_cell(block_index+1, cell);
}

template<typename _CellBlockFunc, typename _EventFunc>
typename multi_type_vector<_CellBlockFunc, _EventFunc>::size_type
multi_type_vector<_CellBlockFunc, _EventFunc>::set_new_block_to_middle(
    size_type block_index, size_type offset, size_type new_block_size, bool overwrite)
{
    assert(block_index < m_block_store.positions.size());

    // First, insert two new blocks after the current block.
    size_type lower_block_size = m_block_store.sizes[block_index] - offset - new_block_size;
    m_block_store.insert(block_index+1, 2);
    m_block_store.sizes[block_index+1] = new_block_size; // empty block.
    m_block_store.sizes[block_index+2] = lower_block_size;

    element_block_type* blk_data = m_block_store.element_blocks[block_index];
    if (blk_data)
    {
        size_type lower_data_start = offset + new_block_size;
        assert(m_block_store.sizes[block_index+2] == lower_block_size);
        element_category_type cat = mtv::get_block_type(*blk_data);
        m_block_store.element_blocks[block_index+2] = element_block_func::create_new_block(cat, 0);
        m_hdl_event.element_block_acquired(m_block_store.element_blocks[block_index+2]);

        // Try to copy the fewer amount of data to the new non-empty block.
        if (offset > lower_block_size)
        {
            // Keep the upper values in the current block and copy the lower
            // values to the new non-empty block.
            element_block_func::assign_values_from_block(
                *m_block_store.element_blocks[block_index+2],
                *blk_data, lower_data_start, lower_block_size);

            if (overwrite)
            {
                // Overwrite cells that will become empty.
                element_block_func::overwrite_values(*blk_data, offset, new_block_size);
            }

            // Shrink the current data block.
            element_block_func::resize_block(*blk_data, offset);
            m_block_store.sizes[block_index] = offset;
            m_block_store.sizes[block_index+2] = lower_block_size;
        }
        else
        {
            // Keep the lower values in the current block and copy the upper
            // values to the new non-empty block (blk_lower), and swap the two
            // later.
            element_block_type* blk_lower_data = m_block_store.element_blocks[block_index+2];
            element_block_func::assign_values_from_block(*blk_lower_data, *blk_data, 0, offset);
            m_block_store.sizes[block_index+2] = offset;

            if (overwrite)
            {
                // Overwrite cells that will become empty.
                element_block_func::overwrite_values(*blk_data, offset, new_block_size);
            }

            // Remove the upper and middle values and push the rest to the top.
            element_block_func::erase(*blk_data, 0, lower_data_start);

            // Set the size of the current block to its new size ( what is after the new block )
            m_block_store.sizes[block_index] = lower_block_size;
            m_block_store.sizes[block_index+2] = offset;

            // And now let's swap the blocks, while preserving the position of the original block.
            size_type position = m_block_store.positions[block_index];
            m_block_store.swap(block_index, block_index+2);
            m_block_store.positions[block_index] = position;
        }
    }
    else
    {
        // There is no data, we just need to update the size of the block
        m_block_store.sizes[block_index] = offset;
    }

    // Re-calculate the block positions.
    m_block_store.calc_block_position(block_index+1);
    m_block_store.calc_block_position(block_index+2);

    return block_index+1;
}

template<typename _CellBlockFunc, typename _EventFunc>
bool multi_type_vector<_CellBlockFunc, _EventFunc>::is_previous_block_of_type(
    size_type block_index, element_category_type cat) const
{
    if (block_index == 0)
        // No previous block.
        return false;

    const element_block_type* data = m_block_store.element_blocks[block_index-1];
    if (data)
        return cat == mdds::mtv::get_block_type(*data);

    return cat == mtv::element_type_empty;
}

template<typename _CellBlockFunc, typename _EventFunc>
bool multi_type_vector<_CellBlockFunc, _EventFunc>::is_next_block_of_type(
    size_type block_index, element_category_type cat) const
{
    if (block_index == m_block_store.positions.size() - 1)
        // No next block.
        return false;

    const element_block_type* data = m_block_store.element_blocks[block_index+1];
    if (data)
        return cat == mdds::mtv::get_block_type(*data);

    return cat == mtv::element_type_empty;
}

template<typename _CellBlockFunc, typename _EventFunc>
bool multi_type_vector<_CellBlockFunc, _EventFunc>::append_empty(size_type len)
{
    // Append empty cells.
    if (m_block_store.positions.empty())
    {
        // No existing block. Create a new one.
        assert(m_cur_size == 0);
        m_block_store.push_back(0, len, nullptr);
        m_cur_size = len;
        return true;
    }

    bool new_block_added = false;

    element_block_type* last_data = m_block_store.element_blocks.back();

    if (!last_data)
    {
        // Last block is empty.  Just increase its size.
        m_block_store.sizes.back() += len;
    }
    else
    {
        // Append a new empty block.
        m_block_store.push_back(m_cur_size, len, nullptr);
        new_block_added = true;
    }

    m_cur_size += len;

    return new_block_added;
}

template<typename _CellBlockFunc, typename _EventFunc>
void multi_type_vector<_CellBlockFunc, _EventFunc>::resize(size_type new_size)
{
#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_prev_block;
    dump_blocks(os_prev_block);
#endif

    resize_impl(new_size);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    try
    {
        check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        os << "block integrity check failed in resize (new-size=" << new_size << ")" << std::endl;
        os << "previous block state:" << std::endl;
        os << os_prev_block.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif
}

template<typename _CellBlockFunc, typename _EventFunc>
void multi_type_vector<_CellBlockFunc, _EventFunc>::resize_impl(size_type new_size)
{
    if (new_size == m_cur_size)
        return;

    if (!new_size)
    {
        clear();
        return;
    }

    if (new_size > m_cur_size)
    {
        // Append empty cells.
        append_empty(new_size - m_cur_size);
        return;
    }

    assert(new_size < m_cur_size && new_size > 0);

    // Find out in which block the new end row will be.
    size_type new_end_row = new_size - 1;
    size_type block_index = get_block_position(new_end_row);
    if (block_index == m_block_store.positions.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::resize", __LINE__, new_end_row, block_size(), size());

    element_block_type* data = m_block_store.element_blocks[block_index];
    size_type start_row_in_block = m_block_store.positions[block_index];
    size_type end_row_in_block = start_row_in_block + m_block_store.sizes[block_index] - 1;

    if (new_end_row < end_row_in_block)
    {
        // Shrink the size of the current block.
        size_type new_block_size = new_end_row - start_row_in_block + 1;
        if (data)
        {
            element_block_func::overwrite_values(*data, new_end_row+1, end_row_in_block-new_end_row);
            element_block_func::resize_block(*data, new_block_size);
        }
        m_block_store.sizes[block_index] = new_block_size;
    }

    // Remove all blocks below the current one.
    delete_element_blocks(block_index+1, m_block_store.element_blocks.size()-1);
    size_type len = m_block_store.element_blocks.size() - block_index - 1;
    m_block_store.erase(block_index+1, len);
    m_cur_size = new_size;
}

template<typename _CellBlockFunc, typename _EventFunc>
void multi_type_vector<_CellBlockFunc, _EventFunc>::swap(multi_type_vector& other)
{
    std::swap(m_hdl_event, other.m_hdl_event);
    std::swap(m_cur_size, other.m_cur_size);
    m_block_store.swap(other.m_block_store);
}

template<typename _CellBlockFunc, typename _EventFunc>
bool multi_type_vector<_CellBlockFunc, _EventFunc>::operator== (const multi_type_vector& other) const
{
    if (this == &other)
        // Comparing to self is always equal.
        return true;

    if (m_cur_size != other.m_cur_size)
        // Row sizes differ.
        return false;

    return m_block_store.equals(other.m_block_store);
}

template<typename _CellBlockFunc, typename _EventFunc>
bool multi_type_vector<_CellBlockFunc, _EventFunc>::operator!= (const multi_type_vector& other) const
{
    return !operator== (other);
}

template<typename _CellBlockFunc, typename _EventFunc>
multi_type_vector<_CellBlockFunc, _EventFunc>& multi_type_vector<_CellBlockFunc, _EventFunc>::operator= (const multi_type_vector& other)
{
    multi_type_vector assigned(other);
    swap(assigned);
    return *this;
}

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG

template<typename _CellBlockFunc, typename _EventFunc>
void multi_type_vector<_CellBlockFunc, _EventFunc>::dump_blocks(std::ostream& os) const
{
    os << "--- blocks" << endl;
    for (size_type i = 0, n = m_block_store.positions.size(); i < n; ++i)
    {
        size_type pos = m_block_store.positions[i];
        size_type size = m_block_store.sizes[i];
        const element_block_type* data = m_block_store.element_blocks[i];
        element_category_type cat = mtv::element_type_empty;
        if (data)
            cat = mtv::get_block_type(*data);
        os << "  block " << i << ": position=" << pos << " size=" << size << " type=" << cat << endl;
    }
}

template<typename _CellBlockFunc, typename _EventFunc>
void multi_type_vector<_CellBlockFunc, _EventFunc>::check_block_integrity() const
{
    if (m_block_store.positions.size() != m_block_store.sizes.size())
        throw mdds::integrity_error("position and size arrays are of different sizes!");

    if (m_block_store.positions.size() != m_block_store.element_blocks.size())
        throw mdds::integrity_error("position and element-block arrays are of different sizes!");

    if (m_block_store.positions.empty())
        // Nothing to check.
        return;

    if (m_block_store.sizes.size() == 1 && m_block_store.sizes[0] == 0)
        throw mdds::integrity_error("block should never be zero sized!");

    if (m_block_store.positions[0] != 0u)
    {
        std::ostringstream os;
        os << "position of the first block should be zero!" << std::endl;

        dump_blocks(os);
        mdds::integrity_error(os.str());
    }

    const element_block_type* data_prev = m_block_store.element_blocks[0];
    element_category_type cat_prev =
        data_prev ? mtv::get_block_type(*data_prev) : mtv::element_type_empty;

    size_type cur_position = m_block_store.sizes[0];
    size_type total_size = m_block_store.sizes[0];

    for (size_type i = 1, n = m_block_store.positions.size(); i < n; ++i)
    {
        size_type this_size = m_block_store.sizes[i];
        if (this_size == 0)
            throw mdds::integrity_error("block should never be zero sized!");

        if (m_block_store.positions[i] != cur_position)
        {
            std::ostringstream os;
            os << "position of the current block is wrong! (expected="
                << cur_position << "; actual=" << m_block_store.positions[i] << ")"
                << std::endl;

            dump_blocks(os);
            mdds::integrity_error(os.str());
        }

        element_category_type cat = mtv::element_type_empty;

        const element_block_type* data = m_block_store.element_blocks[i];

        if (data)
        {
            cat = mtv::get_block_type(*data);

            if (element_block_func::size(*data) != this_size)
                throw mdds::integrity_error(
                    "block size cache and the actual element block size differ!");
        }

        if (cat_prev == cat)
        {
            std::ostringstream os;
            os << "Two adjacent blocks should never be of the same type." << std::endl;
            dump_blocks(os);
            mdds::integrity_error(os.str());
        }

        data_prev = data;
        cat_prev = cat;

        total_size += this_size;
        cur_position += this_size;
    }

    if (total_size != m_cur_size)
    {
        std::ostringstream os;
        os << "Current size does not equal the total sizes of all blocks." << std::endl;
        os << "current size=" << m_cur_size << " total block size=" << total_size << std::endl;
        dump_blocks(os);
        mdds::integrity_error(os.str());
    }
}

#endif

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

