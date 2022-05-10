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

#include <stdexcept>
#include <memory>

namespace mdds { namespace mtv { namespace aos {

namespace detail {

template<typename Blk, typename SizeT>
inline SizeT calc_next_block_position(const std::vector<Blk>& blocks, SizeT block_index)
{
    assert(block_index < blocks.size());
    const Blk& blk = blocks[block_index];
    return blk.position + blk.size;
}

template<typename Blk>
inline auto calc_next_block_position(const Blk& blk) -> decltype(blk.position)
{
    return blk.position + blk.size;
}

template<typename Blk>
bool compare_blocks(const Blk& left, const Blk& right)
{
    return left.position < right.position;
}

} // namespace detail

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

template<typename ElemBlockFunc, typename Trait>
multi_type_vector<ElemBlockFunc, Trait>::block::block() : position(0), size(0), data(nullptr)
{}

template<typename ElemBlockFunc, typename Trait>
multi_type_vector<ElemBlockFunc, Trait>::block::block(size_type _position, size_type _size)
    : position(_position), size(_size), data(nullptr)
{}

template<typename ElemBlockFunc, typename Trait>
multi_type_vector<ElemBlockFunc, Trait>::block::block(size_type _position, size_type _size, element_block_type* _data)
    : position(_position), size(_size), data(_data)
{}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::block::swap(block& other)
{
    std::swap(position, other.position);
    std::swap(size, other.size);
    std::swap(data, other.data);
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::block::clone_to(block& other) const
{
    other.position = position;
    other.size = size;
    if (data)
        other.data = element_block_func::clone_block(*data);
    else
        other.data = nullptr;
}

template<typename ElemBlockFunc, typename Trait>
multi_type_vector<ElemBlockFunc, Trait>::blocks_to_transfer::blocks_to_transfer() : insert_index(0)
{}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::position_type multi_type_vector<ElemBlockFunc, Trait>::next_position(
    const position_type& pos)
{
    position_type ret = pos;
    if (pos.second + 1 < pos.first->size)
    {
        // Next element is still in the same block.
        ++ret.second;
    }
    else
    {
        ++ret.first;
        ret.second = 0;
    }

    return ret;
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::position_type multi_type_vector<
    ElemBlockFunc, Trait>::advance_position(const position_type& pos, int steps)
{
    return mdds::detail::mtv::advance_position<position_type>(pos, steps);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::const_position_type multi_type_vector<
    ElemBlockFunc, Trait>::next_position(const const_position_type& pos)
{
    const_position_type ret = pos;
    if (pos.second + 1 < pos.first->size)
    {
        // Next element is still in the same block.
        ++ret.second;
    }
    else
    {
        ++ret.first;
        ret.second = 0;
    }

    return ret;
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::const_position_type multi_type_vector<
    ElemBlockFunc, Trait>::advance_position(const const_position_type& pos, int steps)
{
    return mdds::detail::mtv::advance_position<const_position_type>(pos, steps);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::size_type multi_type_vector<ElemBlockFunc, Trait>::logical_position(
    const const_position_type& pos)
{
    return pos.first->position + pos.second;
}

template<typename ElemBlockFunc, typename Trait>
template<typename Blk>
typename Blk::value_type multi_type_vector<ElemBlockFunc, Trait>::get(const const_position_type& pos)
{
    return mdds::detail::mtv::get_block_element_at<Blk>(*pos.first->data, pos.second);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::begin()
{
    return iterator(m_blocks.begin(), m_blocks.end(), this, 0);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::end()
{
    return iterator(m_blocks.end(), m_blocks.end(), this, m_blocks.size());
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::const_iterator multi_type_vector<ElemBlockFunc, Trait>::begin() const
{
    return cbegin();
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::const_iterator multi_type_vector<ElemBlockFunc, Trait>::end() const
{
    return cend();
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::const_iterator multi_type_vector<ElemBlockFunc, Trait>::cbegin() const
{
    return const_iterator(m_blocks.cbegin(), m_blocks.cend(), this, 0);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::const_iterator multi_type_vector<ElemBlockFunc, Trait>::cend() const
{
    return const_iterator(m_blocks.end(), m_blocks.end(), this, m_blocks.size());
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::reverse_iterator multi_type_vector<ElemBlockFunc, Trait>::rbegin()
{
    return reverse_iterator(m_blocks.rbegin(), m_blocks.rend(), this, 0);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::reverse_iterator multi_type_vector<ElemBlockFunc, Trait>::rend()
{
    return reverse_iterator(m_blocks.rend(), m_blocks.rend(), this, 0);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::const_reverse_iterator multi_type_vector<
    ElemBlockFunc, Trait>::rbegin() const
{
    return const_reverse_iterator(m_blocks.rbegin(), m_blocks.rend(), this, 0);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::const_reverse_iterator multi_type_vector<ElemBlockFunc, Trait>::rend()
    const
{
    return const_reverse_iterator(m_blocks.rend(), m_blocks.rend(), this, 0);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::const_reverse_iterator multi_type_vector<
    ElemBlockFunc, Trait>::crbegin() const
{
    return const_reverse_iterator(m_blocks.crbegin(), m_blocks.crend(), this, 0);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::const_reverse_iterator multi_type_vector<
    ElemBlockFunc, Trait>::crend() const
{
    return const_reverse_iterator(m_blocks.crend(), m_blocks.crend(), this, 0);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::event_func& multi_type_vector<ElemBlockFunc, Trait>::event_handler()
{
    return m_hdl_event;
}

template<typename ElemBlockFunc, typename Trait>
const typename multi_type_vector<ElemBlockFunc, Trait>::event_func& multi_type_vector<
    ElemBlockFunc, Trait>::event_handler() const
{
    return m_hdl_event;
}

template<typename ElemBlockFunc, typename Trait>
multi_type_vector<ElemBlockFunc, Trait>::multi_type_vector() : m_cur_size(0)
{}

template<typename ElemBlockFunc, typename Trait>
multi_type_vector<ElemBlockFunc, Trait>::multi_type_vector(const event_func& hdl) : m_hdl_event(hdl), m_cur_size(0)
{}

template<typename ElemBlockFunc, typename Trait>
multi_type_vector<ElemBlockFunc, Trait>::multi_type_vector(event_func&& hdl)
    : m_hdl_event(std::move(hdl)), m_cur_size(0)
{}

template<typename ElemBlockFunc, typename Trait>
multi_type_vector<ElemBlockFunc, Trait>::multi_type_vector(size_type init_size) : m_cur_size(init_size)
{
    if (!init_size)
        return;

    // Initialize with an empty block that spans from 0 to max.
    m_blocks.emplace_back(0, init_size);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
multi_type_vector<ElemBlockFunc, Trait>::multi_type_vector(size_type init_size, const T& value) : m_cur_size(init_size)
{
    if (!init_size)
        return;

    element_block_type* data = mdds_mtv_create_new_block(init_size, value);
    m_hdl_event.element_block_acquired(data);
    m_blocks.emplace_back(0, init_size, data);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
multi_type_vector<ElemBlockFunc, Trait>::multi_type_vector(size_type init_size, const T& it_begin, const T& it_end)
    : m_cur_size(init_size)
{
    if (!m_cur_size)
        return;

    size_type data_len = std::distance(it_begin, it_end);
    if (m_cur_size != data_len)
        throw invalid_arg_error("Specified size does not match the size of the initial data array.");

    element_block_type* data = mdds_mtv_create_new_block(*it_begin, it_begin, it_end);
    m_hdl_event.element_block_acquired(data);
    m_blocks.emplace_back(0, m_cur_size, data);
}

template<typename ElemBlockFunc, typename Trait>
multi_type_vector<ElemBlockFunc, Trait>::multi_type_vector(const multi_type_vector& other)
    : m_hdl_event(other.m_hdl_event), m_cur_size(other.m_cur_size)
{
    // Clone all the blocks.
    m_blocks.reserve(other.m_blocks.size());
    typename blocks_type::const_iterator it = other.m_blocks.begin(), it_end = other.m_blocks.end();
    block tmp;
    for (; it != it_end; ++it)
    {
        it->clone_to(tmp);
        m_blocks.emplace_back(tmp.position, tmp.size, tmp.data);
        if (tmp.data)
            m_hdl_event.element_block_acquired(tmp.data);
    }

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

template<typename ElemBlockFunc, typename Trait>
multi_type_vector<ElemBlockFunc, Trait>::multi_type_vector(multi_type_vector&& other)
    : m_hdl_event(std::move(other.m_hdl_event)), m_blocks(std::move(other.m_blocks)), m_cur_size(other.m_cur_size)
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
        os << "block integrity check failed in move construction" << std::endl;
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif
}

template<typename ElemBlockFunc, typename Trait>
multi_type_vector<ElemBlockFunc, Trait>::~multi_type_vector()
{
    delete_element_blocks(m_blocks.begin(), m_blocks.end());
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::set(
    size_type pos, const T& value)
{
    size_type block_index = get_block_position(pos);
    if (block_index == m_blocks.size())
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

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::set(
    const iterator& pos_hint, size_type pos, const T& value)
{
    size_type block_index = get_block_position(pos_hint->__private_data, pos);
    if (block_index == m_blocks.size())
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

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::delete_element_block(block& blk)
{
    if (!blk.data)
        // This block is empty.
        return;

    m_hdl_event.element_block_released(blk.data);
    element_block_func::delete_block(blk.data);
    blk.data = nullptr;
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::delete_element_blocks(
    typename blocks_type::iterator it, typename blocks_type::iterator it_end)
{
    std::for_each(it, it_end, [&](block& r) { delete_element_block(r); });
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::set_impl(
    size_type pos, size_type block_index, const T& value)
{
    size_type start_row = m_blocks[block_index].position;
    element_category_type cat = mdds_mtv_get_element_type(value);

    typename blocks_type::iterator block_pos = m_blocks.begin();
    std::advance(block_pos, block_index);
    block* blk = &*block_pos;
    assert(blk->size > 0); // block size should never be zero at any time.

    assert(pos >= start_row);
    size_type pos_in_block = pos - start_row;
    assert(pos_in_block < blk->size);

    if (!blk->data)
    {
        // This is an empty block.
        return set_cell_to_empty_block(block_index, pos_in_block, value);
    }

    assert(blk->data);
    element_category_type blk_cat = mdds::mtv::get_block_type(*blk->data);

    if (blk_cat == cat)
    {
        // This block is of the same type as the cell being inserted.
        size_type i = pos - start_row;
        element_block_func::overwrite_values(*blk->data, i, 1);
        mdds_mtv_set_value(*blk->data, i, value);
        return iterator(block_pos, m_blocks.end(), this, block_index);
    }

    assert(blk_cat != cat);

    if (pos == start_row)
    {
        // Insertion point is at the start of the block.
        if (blk->size == 1)
        {
            return set_cell_to_block_of_size_one(block_index, value);
        }

        assert(blk->size > 1);
        block* blk_prev = get_previous_block_of_type(block_index, cat);
        if (blk_prev)
        {
            // Append to the previous block.
            blk->size -= 1;
            blk->position += 1;
            element_block_func::overwrite_values(*blk->data, 0, 1);
            element_block_func::erase(*blk->data, 0);
            blk_prev->size += 1;
            mdds_mtv_append_value(*blk_prev->data, value);
            return get_iterator(block_index - 1);
        }

        set_cell_to_top_of_data_block(block_index, value);
        return get_iterator(block_index);
    }

    if (pos < (start_row + blk->size - 1))
    {
        // Insertion point is somewhere in the middle of the block.
        return set_cell_to_middle_of_block(block_index, pos_in_block, value);
    }

    // Insertion point is at the end of the block.
    assert(pos == (start_row + blk->size - 1));
    assert(pos > start_row);
    assert(blk->size > 1);

    if (block_index == 0)
    {
        if (m_blocks.size() == 1)
        {
            // This is the only block.  Pop the last value from the
            // previous block, and insert a new block for the cell being
            // inserted.
            set_cell_to_bottom_of_data_block(0, value);
            iterator itr = end();
            --itr;
            return itr;
        }

        block* blk_next = get_next_block_of_type(block_index, cat);
        if (!blk_next)
        {
            // Pop the last cell of the current block, and insert a new block
            // with the new cell.
            set_cell_to_bottom_of_data_block(0, value);
            iterator itr = begin();
            ++itr;
            return itr;
        }

        // Pop the last cell off the current block, and prepend the
        // new value to the next block.
        element_block_func::overwrite_values(*blk->data, blk->size - 1, 1);
        element_block_func::erase(*blk->data, blk->size - 1);
        blk->size -= 1;
        mdds_mtv_prepend_value(*blk_next->data, value);
        blk_next->size += 1;
        blk_next->position -= 1;

        return get_iterator(block_index + 1);
    }

    assert(block_index > 0);

    if (block_index == m_blocks.size() - 1)
    {
        // This is the last block.
        set_cell_to_bottom_of_data_block(block_index, value);
        iterator itr = end();
        --itr;
        return itr;
    }

    block* blk_next = get_next_block_of_type(block_index, cat);
    if (!blk_next)
    {
        // Next block is either empty or of different type than that of the cell being inserted.
        set_cell_to_bottom_of_data_block(block_index, value); // This invalidates m_blocks.
        return get_iterator(block_index + 1);
    }

    // Pop the last element from the current block, and prepend the cell
    // into the next block.
    element_block_func::overwrite_values(*blk->data, blk->size - 1, 1);
    element_block_func::erase(*blk->data, blk->size - 1);
    blk->size -= 1;
    mdds_mtv_prepend_value(*blk_next->data, value);
    blk_next->size += 1;
    blk_next->position -= 1;

    return get_iterator(block_index + 1);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::release_impl(
    size_type pos, size_type block_index, T& value)
{
    const block& blk = m_blocks[block_index];
    size_type start_pos = blk.position;

    if (!blk.data)
    {
        // Empty cell block.  There is no element to release.
        mdds_mtv_get_empty_value(value);
        return get_iterator(block_index);
    }

    assert(pos >= start_pos);
    assert(blk.data); // data for non-empty blocks should never be nullptr.
    size_type idx = pos - start_pos;
    mdds_mtv_get_value(*blk.data, idx, value);

    // Set the element slot empty without overwriting it.
    return set_empty_in_single_block(pos, pos, block_index, false);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::set(
    size_type pos, const T& it_begin, const T& it_end)
{
    auto res = mdds::detail::mtv::calc_input_end_position(it_begin, it_end, pos, m_cur_size);

    if (!res.second)
        return end();

    size_type end_pos = res.first;

    size_type block_index1 = get_block_position(pos);
    if (block_index1 == m_blocks.size())
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
        os << "block integrity check failed in set (pos=" << pos << ")" << std::endl;
        os << "previous block state:" << std::endl;
        os << os_prev_block.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif

    return ret;
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::set(
    const iterator& pos_hint, size_type pos, const T& it_begin, const T& it_end)
{
    auto res = mdds::detail::mtv::calc_input_end_position(it_begin, it_end, pos, m_cur_size);
    if (!res.second)
        return end();

    size_type end_pos = res.first;

    size_type block_index1 = get_block_position(pos_hint->__private_data, pos);
    return set_cells_impl(pos, end_pos, block_index1, it_begin, it_end);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::push_back(
    const T& value)
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

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::push_back_impl(
    const T& value)
{
    element_category_type cat = mdds_mtv_get_element_type(value);

    block* blk_last = m_blocks.empty() ? nullptr : &m_blocks.back();
    if (!blk_last || !blk_last->data || cat != get_block_type(*blk_last->data))
    {
        // Either there is no block, or the last block is empty or of
        // different type.  Append a new block.
        size_type block_index = m_blocks.size();
        size_type start_pos = m_cur_size;

        m_blocks.emplace_back(start_pos, 1);
        create_new_block_with_new_cell(m_blocks.back().data, value);
        ++m_cur_size;

        return get_iterator(block_index);
    }

    assert(blk_last);
    assert(blk_last->data);
    assert(cat == get_block_type(*blk_last->data));

    // Append the new value to the last block.
    size_type block_index = m_blocks.size() - 1;

    mdds_mtv_append_value(*blk_last->data, value);
    ++blk_last->size;
    ++m_cur_size;

    return get_iterator(block_index);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::push_back_empty()
{
    size_type block_index = m_blocks.size();

    if (!append_empty(1))
    {
        // Last empty block has been extended.
        --block_index;
    }

    // Get the iterator of the last block.
    typename blocks_type::iterator block_pos = m_blocks.end();
    --block_pos;

    return iterator(block_pos, m_blocks.end(), this, block_index);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::insert(
    size_type pos, const T& it_begin, const T& it_end)
{
    size_type block_index = get_block_position(pos);
    if (block_index == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::insert", __LINE__, pos, block_size(), size());

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_prev_block;
    dump_blocks(os_prev_block);
#endif

    iterator ret = insert_cells_impl(pos, block_index, it_begin, it_end);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    try
    {
        check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        element_category_type cat = mdds_mtv_get_element_type(*it_begin);
        os << "block integrity check failed in insert (pos=" << pos
           << "; value-size=" << std::distance(it_begin, it_end) << "; value-type=" << cat << ")" << std::endl;
        os << "previous block state:" << std::endl;
        os << os_prev_block.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif

    return ret;
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::insert(
    const iterator& pos_hint, size_type pos, const T& it_begin, const T& it_end)
{
    size_type block_index = get_block_position(pos_hint->__private_data, pos);
    if (block_index == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::insert", __LINE__, pos, block_size(), size());

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_prev_block;
    dump_blocks(os_prev_block);
#endif

    iterator ret = insert_cells_impl(pos, block_index, it_begin, it_end);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    try
    {
        check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        element_category_type cat = mdds_mtv_get_element_type(*it_begin);
        os << "block integrity check failed in insert (pos=" << pos
           << "; value-size=" << std::distance(it_begin, it_end) << "; value-type=" << cat << ")" << std::endl;
        os << "previous block state:" << std::endl;
        os << os_prev_block.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif

    return ret;
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::size_type multi_type_vector<ElemBlockFunc, Trait>::get_block_position(
    size_type row, size_type start_block_index) const
{
    if (row >= m_cur_size || start_block_index >= m_blocks.size())
        return m_blocks.size();

    auto it0 = m_blocks.begin();
    std::advance(it0, start_block_index);

    block b(row, 0);
    auto it = std::lower_bound(it0, m_blocks.end(), b, detail::compare_blocks<block>);

    if (it == m_blocks.end() || it->position != row)
    {
        // Binary search has overshot by one block.  Move back one.
        assert(it != it0);
        --it;
    }

    assert(it->position <= row);
    assert(row < it->position + it->size);
    return std::distance(it0, it) + start_block_index;
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::size_type multi_type_vector<ElemBlockFunc, Trait>::get_block_position(
    const typename value_type::private_data& pos_data, size_type row) const
{
    size_type block_index = 0;
    if (pos_data.parent == this && pos_data.block_index < m_blocks.size())
        block_index = pos_data.block_index;

    size_type start_row = m_blocks[block_index].position;
    if (row < start_row)
    {
        // Position hint is past the insertion position.
        // Walk back if that seems efficient.
        if (row > start_row / 2)
        {
            for (size_type i = block_index; i > 0;)
            {
                --i;
                start_row = m_blocks[i].position;
                if (row >= start_row)
                {
                    // Row is in this block.
                    return i;
                }
                // Specified row is not in this block.
            }
            assert(start_row == 0);
        }
        // Otherwise reset.
        block_index = 0;
    }

    return get_block_position(row, block_index);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
void multi_type_vector<ElemBlockFunc, Trait>::create_new_block_with_new_cell(element_block_type*& data, const T& cell)
{
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
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<
    ElemBlockFunc, Trait>::set_cell_to_middle_of_block(size_type block_index, size_type pos_in_block, const T& cell)
{
    block& blk_new = set_new_block_to_middle(block_index, pos_in_block, 1, true);
    create_new_block_with_new_cell(blk_new.data, cell);

    // Return the iterator referencing the inserted block.
    return get_iterator(block_index + 1);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
void multi_type_vector<ElemBlockFunc, Trait>::append_cell_to_block(size_type block_index, const T& cell)
{
    m_blocks[block_index].size += 1;
    mdds_mtv_append_value(*m_blocks[block_index].data, cell);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<
    ElemBlockFunc, Trait>::set_cell_to_empty_block(size_type block_index, size_type pos_in_block, const T& cell)
{
    block* blk = &m_blocks[block_index];
    assert(!blk->data); // In this call, the current block is an empty block.

    if (block_index == 0)
    {
        // Topmost block.
        if (m_blocks.size() == 1)
        {
            // this is the only block.
            assert(blk->size == m_cur_size);
            if (m_cur_size == 1)
            {
                // This column is allowed to have only one row!
                assert(pos_in_block == 0);
                create_new_block_with_new_cell(blk->data, cell);
                return begin();
            }

            // block has multiple rows.
            if (pos_in_block == 0)
            {
                // Insert into the first cell in this block.
                blk->size -= 1;
                assert(blk->size > 0);

                m_blocks.emplace(m_blocks.begin(), 0, 1);
                blk = &m_blocks[0]; // old pointer is invalid.
                create_new_block_with_new_cell(blk->data, cell);

                m_blocks[1].position = 1;
                return begin();
            }

            if (pos_in_block == blk->size - 1)
            {
                // Insert into the last cell in block.
                blk->size -= 1;
                assert(blk->size > 0);

                m_blocks.emplace_back(blk->size, 1);
                blk = &m_blocks.back(); // old pointer is invalid.

                create_new_block_with_new_cell(blk->data, cell);
                iterator ret = end();
                --ret;
                return ret;
            }

            // Insert into the middle of the block.
            return set_cell_to_middle_of_block(block_index, pos_in_block, cell);
        }

        // This topmost empty block is followed by a non-empty block.

        if (pos_in_block == 0)
        {
            assert(block_index < m_blocks.size() - 1);
            if (blk->size == 1)
            {
                // Top empty block with only one cell size.
                element_category_type cat = mdds_mtv_get_element_type(cell);
                block* blk_next = get_next_block_of_type(block_index, cat);
                if (blk_next)
                {
                    // Remove this one-cell empty block from the top, and
                    // prepend the cell to the next block.
                    delete_element_block(m_blocks.front());
                    m_blocks.erase(m_blocks.begin());
                    blk = &m_blocks.front(); // old pointer is invalid.
                    blk->size += 1;
                    blk->position -= 1;
                    mdds_mtv_prepend_value(*blk->data, cell);
                }
                else
                {
                    create_new_block_with_new_cell(blk->data, cell);
                }
            }
            else
            {
                // Shrink this topmost block by one and set the new value above it.
                assert(blk->size > 1);
                blk->size -= 1;
                blk->position = 1;
                m_blocks.emplace(m_blocks.begin(), 0, 1);
                create_new_block_with_new_cell(m_blocks.front().data, cell);
            }

            return begin();
        }

        if (pos_in_block == blk->size - 1)
        {
            // Immediately above a non-empty block.
            element_category_type cat = mdds_mtv_get_element_type(cell);
            block* blk_next = get_next_block_of_type(block_index, cat);
            if (blk_next)
            {
                assert(blk->size > 1);
                // Shrink this empty block by one, and prepend the cell to the next block.
                blk->size -= 1;
                blk_next->size += 1;
                blk_next->position -= 1;
                mdds_mtv_prepend_value(*blk_next->data, cell);
            }
            else
            {
                // Shrink the current empty block by one, and create a new block of size 1 to store the new value.
                blk->size -= 1;
                typename blocks_type::iterator it = m_blocks.begin();
                std::advance(it, block_index + 1);
                size_type new_position = detail::calc_next_block_position(*blk);
                m_blocks.emplace(it, new_position, 1);
                blk = &m_blocks[block_index]; // old pointer is invalid.
                block* blk2 = &m_blocks[block_index + 1];
                create_new_block_with_new_cell(blk2->data, cell);
            }

            return get_iterator(block_index + 1);
        }

        // Inserting into the middle of an empty block.
        return set_cell_to_middle_of_block(block_index, pos_in_block, cell);
    }

    // This empty block is right below a non-empty block.
    assert(block_index > 0 && m_blocks[block_index - 1].data != nullptr);

    if (pos_in_block == 0)
    {
        // New cell is right below the non-empty block.
        element_category_type blk_cat_prev = mdds::mtv::get_block_type(*m_blocks[block_index - 1].data);
        element_category_type cat = mdds_mtv_get_element_type(cell);
        if (blk_cat_prev == cat)
        {
            // Extend the previous block by one to insert this cell.

            if (blk->size == 1)
            {
                // Check if we need to merge with the following block.
                if (block_index == m_blocks.size() - 1)
                {
                    // Last block.  Delete this block and extend the previous
                    // block by one.
                    delete_element_block(m_blocks[block_index]);
                    m_blocks.pop_back();
                    append_cell_to_block(block_index - 1, cell);
                }
                else
                {
                    // Block exists below.
                    block* blk_next = get_next_block_of_type(block_index, blk_cat_prev);
                    if (blk_next)
                    {
                        assert(blk_next->data); // Empty block must not be followed by another empty block.

                        // We need to merge the previous and next blocks, then
                        // delete the current and next blocks.  Be sure to
                        // resize the next block to zero to prevent the
                        // transferred cells to be deleted.
                        block& blk_prev = m_blocks[block_index - 1];

                        // Check if the next block is bigger.
                        if (blk_prev.size < blk_next->size)
                        {
                            // Prepend the new item to the next block, then
                            // prepend the content of the previous block and
                            // release both previous and current blocks.

                            size_type position = blk_prev.position;

                            // Increase the size of block and prepend the new cell
                            blk_next->size += 1;
                            mdds_mtv_prepend_value(*blk_next->data, cell);

                            // Preprend the content of previous block to the next block.
                            element_block_func::prepend_values_from_block(
                                *blk_next->data, *blk_prev.data, 0, blk_prev.size);
                            blk_next->size += blk_prev.size;
                            blk_next->position = position;

                            // Resize the previous block to zero
                            element_block_func::resize_block(*blk_prev.data, 0);
                            m_hdl_event.element_block_released(blk_prev.data);

                            // Release both blocks which are no longer used
                            element_block_func::delete_block(blk->data);
                            element_block_func::delete_block(blk_prev.data);

                            // Get an iterator to previous block
                            typename blocks_type::iterator it = m_blocks.begin() + block_index - 1;

                            // So we can use it to remove the two blocks
                            m_blocks.erase(it, it + 2);
                        }
                        else
                        {
                            // Be sure to resize the next block to zero to prevent the
                            // transferred cells to be deleted.
                            blk_prev.size += 1 + blk_next->size;
                            mdds_mtv_append_value(*blk_prev.data, cell);
                            element_block_func::append_values_from_block(*blk_prev.data, *blk_next->data);
                            element_block_func::resize_block(*blk_next->data, 0);
                            m_hdl_event.element_block_released(blk_next->data);
                            element_block_func::delete_block(blk->data);
                            element_block_func::delete_block(blk_next->data);
                            typename blocks_type::iterator it = m_blocks.begin() + block_index;
                            m_blocks.erase(it, it + 2);
                        }
                    }
                    else
                    {
                        // Ignore the next block. Just extend the previous block.
                        delete_element_block(m_blocks[block_index]);
                        m_blocks.erase(m_blocks.begin() + block_index);
                        append_cell_to_block(block_index - 1, cell);
                    }
                }
            }
            else
            {
                // Extend the previous block to append the cell.
                assert(blk->size > 1);
                blk->size -= 1;
                blk->position += 1;
                append_cell_to_block(block_index - 1, cell);
            }

            return get_iterator(block_index - 1);
        }
        else
        {
            // Cell type is different from the type of the previous block.
            if (blk->size == 1)
            {
                if (block_index == m_blocks.size() - 1)
                {
                    // There is no more block below.  Simply turn this empty block into a non-empty one.
                    create_new_block_with_new_cell(blk->data, cell);
                }
                else
                {
                    // Check the type of the following non-empty block.
                    assert(block_index < m_blocks.size() - 1);
                    block* blk_next = get_next_block_of_type(block_index, cat);
                    if (blk_next)
                    {
                        // Remove this empty block, and prepend the cell to the next block.
                        blk_next->size += 1;
                        blk_next->position -= 1;
                        mdds_mtv_prepend_value(*blk_next->data, cell);
                        delete_element_block(m_blocks[block_index]);
                        m_blocks.erase(m_blocks.begin() + block_index);
                    }
                    else
                    {
                        // Simply turn this empty block into a non-empty one.
                        create_new_block_with_new_cell(blk->data, cell);
                    }
                }
            }
            else
            {
                // Replace the current empty block of size > 1 with a
                // non-empty block of size 1, and insert a new empty block
                // below whose size is one shorter than the current empty
                // block.
                size_type new_block_size = blk->size - 1;
                size_type new_block_position = blk->position + 1;
                blk->size = 1;
                create_new_block_with_new_cell(blk->data, cell);
                m_blocks.emplace(m_blocks.begin() + block_index + 1, new_block_position, new_block_size);
            }

            return get_iterator(block_index);
        }
    }
    else if (pos_in_block == blk->size - 1)
    {
        // New cell is at the end of the current block.
        assert(blk->size > 1);
        if (block_index == m_blocks.size() - 1)
        {
            // The current block is the last block.
            blk->size -= 1;
            size_type new_position = detail::calc_next_block_position(*blk);
            m_blocks.emplace_back(new_position, 1);
            blk = &m_blocks.back(); // old pointer is invalid.
            create_new_block_with_new_cell(blk->data, cell);
            iterator it = end();
            --it;
            return it;
        }
        else
        {
            // A non-empty block exists below.
            element_category_type cat = mdds_mtv_get_element_type(cell);
            block* blk_next = get_next_block_of_type(block_index, cat);
            if (blk_next)
            {
                // Shrink this empty block and extend the next block.
                blk->size -= 1;
                blk_next->size += 1;
                blk_next->position -= 1;
                mdds_mtv_prepend_value(*blk_next->data, cell);
            }
            else
            {
                // Shrink the current empty block by one, and insert a new
                // block of size 1 below it to store the new value.
                blk->size -= 1;
                size_type new_position = detail::calc_next_block_position(*blk);
                m_blocks.emplace(m_blocks.begin() + block_index + 1, new_position, 1);
                blk = &m_blocks[block_index]; // old pointer is invalid.
                block& blk2 = m_blocks[block_index + 1];
                create_new_block_with_new_cell(blk2.data, cell);
            }

            return get_iterator(block_index + 1);
        }
    }

    // New cell is somewhere in the middle of an empty block.
    return set_cell_to_middle_of_block(block_index, pos_in_block, cell);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<
    ElemBlockFunc, Trait>::set_cell_to_block_of_size_one(size_type block_index, const T& cell)
{
    block* blk = &m_blocks[block_index];
    assert(blk->size == 1);
    assert(blk->data);
    element_category_type cat = mdds_mtv_get_element_type(cell);
    assert(mdds::mtv::get_block_type(*blk->data) != cat);

    if (block_index == 0)
    {
        // This is the topmost block of size 1.
        if (block_index == m_blocks.size() - 1)
        {
            // This is the only block.
            create_new_block_with_new_cell(blk->data, cell);
            return begin();
        }

        // There is an existing block below.
        block* blk_next = get_next_block_of_type(block_index, cat);
        if (!blk_next)
        {
            // Next block is empty or of different type.
            create_new_block_with_new_cell(blk->data, cell);
            return begin();
        }

        // Delete the current block, and prepend the cell to the next block.
        blk_next->size += 1;
        blk_next->position -= 1;
        mdds_mtv_prepend_value(*blk_next->data, cell);
        delete_element_block(*blk);
        m_blocks.erase(m_blocks.begin() + block_index);
        return begin();
    }

    assert(block_index > 0);

    if (block_index == m_blocks.size() - 1)
    {
        // This is the last block, and a block exists above.
        block* blk_prev = &m_blocks[block_index - 1];
        if (!blk_prev->data || mdds::mtv::get_block_type(*blk_prev->data) != cat)
        {
            // Previous block is empty. Replace the current block with a new one.
            create_new_block_with_new_cell(blk->data, cell);
        }
        else
        {
            // Append the cell to the previos block, and remove the
            // current block.
            mdds_mtv_append_value(*blk_prev->data, cell);
            blk_prev->size += 1;
            delete_element_block(*blk);
            m_blocks.erase(m_blocks.begin() + block_index);
        }

        iterator itr = end();
        --itr;
        return itr;
    }

    // Remove the current block, and check if the cell can be append
    // to the previous block, or prepended to the following block.
    // Also check if the blocks above and below need to be combined.

    block* blk_prev = &m_blocks[block_index - 1];
    block* blk_next = &m_blocks[block_index + 1];
    if (!blk_prev->data)
    {
        // Previous block is empty.
        if (!blk_next->data)
        {
            // Next block is empty too.
            create_new_block_with_new_cell(blk->data, cell);
            return get_iterator(block_index);
        }

        // Previous block is empty, but the next block is not.
        element_category_type blk_cat_next = mdds::mtv::get_block_type(*blk_next->data);
        if (blk_cat_next == cat)
        {
            // Delete the current block, and prepend the new cell to the next block.
            delete_element_block(*blk);
            m_blocks.erase(m_blocks.begin() + block_index);
            blk = &m_blocks[block_index];
            blk->size += 1;
            blk->position -= 1;
            mdds_mtv_prepend_value(*blk->data, cell);
            return get_iterator(block_index);
        }

        assert(blk_cat_next != cat);
        create_new_block_with_new_cell(blk->data, cell);
        return get_iterator(block_index);
    }

    if (!blk_next->data)
    {
        // Next block is empty, and the previous block is not.
        assert(blk_prev->data);
        element_category_type blk_cat_prev = mdds::mtv::get_block_type(*blk_prev->data);
        if (blk_cat_prev == cat)
        {
            // Append to the previous block.
            blk_prev->size += 1;
            mdds_mtv_append_value(*blk_prev->data, cell);
            delete_element_block(*blk);
            m_blocks.erase(m_blocks.begin() + block_index);
            return get_iterator(block_index - 1);
        }

        // Just overwrite the current block.
        create_new_block_with_new_cell(blk->data, cell);
        return get_iterator(block_index);
    }

    assert(blk_prev && blk_prev->data);
    assert(blk_next && blk_next->data);
    element_category_type blk_cat_prev = mdds::mtv::get_block_type(*blk_prev->data);
    element_category_type blk_cat_next = mdds::mtv::get_block_type(*blk_next->data);

    if (blk_cat_prev == blk_cat_next)
    {
        if (blk_cat_prev == cat)
        {
            // Merge the previous block with the cell being inserted and
            // the next block.  Resize the next block to zero to prevent
            // deletion of mananged cells on block deletion.
            blk_prev->size += 1 + blk_next->size;
            mdds_mtv_append_value(*blk_prev->data, cell);
            element_block_func::append_values_from_block(*blk_prev->data, *blk_next->data);
            element_block_func::resize_block(*blk_next->data, 0);

            // Delete the current and next blocks.
            delete_element_block(*blk);
            delete_element_block(*blk_next);
            typename blocks_type::iterator it = m_blocks.begin() + block_index;
            typename blocks_type::iterator it_last = it + 2;
            m_blocks.erase(it, it_last);
            return get_iterator(block_index - 1);
        }

        // Just overwrite the current block.
        create_new_block_with_new_cell(blk->data, cell);
        return get_iterator(block_index);
    }

    assert(blk_cat_prev != blk_cat_next);

    if (blk_cat_prev == cat)
    {
        // Append to the previous block.
        blk_prev->size += 1;
        mdds_mtv_append_value(*blk_prev->data, cell);
        delete_element_block(*blk);
        m_blocks.erase(m_blocks.begin() + block_index);
        return get_iterator(block_index - 1);
    }

    if (blk_cat_next == cat)
    {
        // Prepend to the next block.
        blk_next->size += 1;
        blk_next->position -= 1;
        mdds_mtv_prepend_value(*blk_next->data, cell);
        delete_element_block(*blk);
        m_blocks.erase(m_blocks.begin() + block_index);
        return get_iterator(block_index);
    }

    // Just overwrite the current block.
    create_new_block_with_new_cell(blk->data, cell);
    return get_iterator(block_index);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
void multi_type_vector<ElemBlockFunc, Trait>::set_cell_to_top_of_data_block(size_type block_index, const T& cell)
{
    block& blk = m_blocks[block_index];
    blk.size -= 1;
    size_type position = blk.position;
    blk.position += 1;

    if (blk.data)
    {
        element_block_func::overwrite_values(*blk.data, 0, 1);
        element_block_func::erase(*blk.data, 0);
    }

    m_blocks.emplace(m_blocks.begin() + block_index, position, 1);
    create_new_block_with_new_cell(m_blocks[block_index].data, cell);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
void multi_type_vector<ElemBlockFunc, Trait>::set_cell_to_bottom_of_data_block(size_type block_index, const T& cell)
{
    assert(block_index < m_blocks.size());
    block& blk = m_blocks[block_index];
    if (blk.data)
    {
        element_block_func::overwrite_values(*blk.data, blk.size - 1, 1);
        element_block_func::erase(*blk.data, blk.size - 1);
    }
    blk.size -= 1;
    size_type next_position = detail::calc_next_block_position(blk);
    m_blocks.emplace(m_blocks.begin() + block_index + 1, next_position, 1);
    create_new_block_with_new_cell(m_blocks[block_index + 1].data, cell);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
void multi_type_vector<ElemBlockFunc, Trait>::get(size_type pos, T& value) const
{
    size_type block_index = get_block_position(pos);
    if (block_index == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::get", __LINE__, pos, block_size(), size());

    const block* blk = &m_blocks[block_index];
    assert(blk);

    if (!blk->data)
    {
        // empty cell block.
        mdds_mtv_get_empty_value(value);
        return;
    }

    size_type start_row = blk->position;
    assert(pos >= start_row);
    assert(blk->data); // data for non-empty blocks should never be nullptr.
    size_type idx = pos - start_row;
    mdds_mtv_get_value(*blk->data, idx, value);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
T multi_type_vector<ElemBlockFunc, Trait>::get(size_type pos) const
{
    T cell;
    get(pos, cell);
    return cell;
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
T multi_type_vector<ElemBlockFunc, Trait>::release(size_type pos)
{
    size_type block_index = get_block_position(pos);
    if (block_index == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::release", __LINE__, pos, block_size(), size());

    T value;
    release_impl(pos, block_index, value);
    return value;
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::release(
    size_type pos, T& value)
{
    size_type block_index = get_block_position(pos);
    if (block_index == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::release", __LINE__, pos, block_size(), size());

    return release_impl(pos, block_index, value);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::release(
    const iterator& pos_hint, size_type pos, T& value)
{
    size_type block_index = get_block_position(pos_hint->__private_data, pos);
    if (block_index == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::release", __LINE__, pos, block_size(), size());

    return release_impl(pos, block_index, value);
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::release()
{
    typename blocks_type::iterator it = m_blocks.begin(), it_end = m_blocks.end();
    for (; it != it_end; ++it)
    {
        block* blk = &(*it);
        if (blk->data)
        {
            element_block_func::resize_block(*blk->data, 0);
            m_hdl_event.element_block_released(blk->data);
            element_block_func::delete_block(blk->data);
        }
    }

    m_blocks.clear();
    m_cur_size = 0;
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::release_range(
    size_type start_pos, size_type end_pos)
{
    size_type block_index1 = get_block_position(start_pos);
    if (block_index1 == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::release_range", __LINE__, start_pos, block_size(), size());

    return set_empty_impl(start_pos, end_pos, block_index1, false);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::release_range(
    const iterator& pos_hint, size_type start_pos, size_type end_pos)
{
    size_type block_index1 = get_block_position(pos_hint->__private_data, start_pos);
    if (block_index1 == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::release_range", __LINE__, start_pos, block_size(), size());

    return set_empty_impl(start_pos, end_pos, block_index1, false);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::position_type multi_type_vector<ElemBlockFunc, Trait>::position(
    size_type pos)
{
    if (pos == m_cur_size)
    {
        // This is a valid end position.  Create a valid position object that
        // represents a valid end position.
        return position_type(end(), 0);
    }

    size_type block_index = get_block_position(pos);
    if (block_index == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::position", __LINE__, pos, block_size(), size());

    size_type start_pos = m_blocks[block_index].position;

    iterator it = get_iterator(block_index);
    return position_type(it, pos - start_pos);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::position_type multi_type_vector<ElemBlockFunc, Trait>::position(
    const iterator& pos_hint, size_type pos)
{
    if (pos == m_cur_size)
    {
        // This is a valid end position.  Create a valid position object that
        // represents a valid end position.
        return position_type(end(), 0);
    }

    size_type block_index = get_block_position(pos_hint->__private_data, pos);
    if (block_index == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::position", __LINE__, pos, block_size(), size());

    iterator it = get_iterator(block_index);
    size_type start_pos = m_blocks[block_index].position;
    return position_type(it, pos - start_pos);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::const_position_type multi_type_vector<ElemBlockFunc, Trait>::position(
    size_type pos) const
{
    if (pos == m_cur_size)
    {
        // This is a valid end position.  Create a valid position object that
        // represents a valid end position.
        return const_position_type(cend(), 0);
    }

    size_type block_index = get_block_position(pos);
    if (block_index == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::position", __LINE__, pos, block_size(), size());

    size_type start_row = m_blocks[block_index].position;

    typename blocks_type::const_iterator block_pos = m_blocks.begin();
    std::advance(block_pos, block_index);
    const_iterator it = const_iterator(block_pos, m_blocks.end(), this, block_index);
    return const_position_type(it, pos - start_row);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::const_position_type multi_type_vector<ElemBlockFunc, Trait>::position(
    const const_iterator& pos_hint, size_type pos) const
{
    if (pos == m_cur_size)
    {
        // This is a valid end position.  Create a valid position object that
        // represents a valid end position.
        return const_position_type(cend(), 0);
    }

    size_type block_index = get_block_position(pos_hint->__private_data, pos);
    if (block_index == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::position", __LINE__, pos, block_size(), size());

    const_iterator it = get_const_iterator(block_index);
    size_type start_pos = m_blocks[block_index].position;
    return const_position_type(it, pos - start_pos);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::transfer(
    size_type start_pos, size_type end_pos, multi_type_vector& dest, size_type dest_pos)
{
    if (&dest == this)
        throw invalid_arg_error("You cannot transfer between the same container.");

    size_type block_index1 = get_block_position(start_pos);
    if (block_index1 == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::transfer", __LINE__, start_pos, block_size(), size());

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_prev_block, os_prev_block_dest;
    dump_blocks(os_prev_block);
    dest.dump_blocks(os_prev_block_dest);
#endif

    iterator ret = transfer_impl(start_pos, end_pos, block_index1, dest, dest_pos);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_block, os_block_dest;
    dump_blocks(os_block);
    dest.dump_blocks(os_block_dest);

    try
    {
        check_block_integrity();
        dest.check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        os << std::endl
           << "block integrity check failed in transfer (start_pos=" << start_pos << "; end_pos=" << end_pos
           << "; dest_pos=" << dest_pos << ")" << std::endl;
        os << std::endl << "previous block state (source):" << std::endl;
        os << os_prev_block.str();
        os << std::endl << "previous block state (destination):" << std::endl;
        os << os_prev_block_dest.str();
        os << std::endl << "altered block state (source):" << std::endl;
        os << os_block.str();
        os << std::endl << "altered block state (destination):" << std::endl;
        os << os_block_dest.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif

    return ret;
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::transfer(
    const iterator& pos_hint, size_type start_pos, size_type end_pos, multi_type_vector& dest, size_type dest_pos)
{
    size_type block_index1 = get_block_position(pos_hint->__private_data, start_pos);
    if (block_index1 == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::transfer", __LINE__, start_pos, block_size(), size());

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_prev_block, os_prev_block_dest;
    dump_blocks(os_prev_block);
    dest.dump_blocks(os_prev_block_dest);
#endif

    iterator ret = transfer_impl(start_pos, end_pos, block_index1, dest, dest_pos);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_block, os_block_dest;
    dump_blocks(os_block);
    dest.dump_blocks(os_block_dest);

    try
    {
        check_block_integrity();
        dest.check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        os << std::endl
           << "block integrity check failed in transfer (start_pos=" << start_pos << "; end_pos=" << end_pos
           << "; dest_pos=" << dest_pos << ")" << std::endl;
        os << std::endl << "previous block state (source):" << std::endl;
        os << os_prev_block.str();
        os << std::endl << "previous block state (destination):" << std::endl;
        os << os_prev_block_dest.str();
        os << std::endl << "altered block state (source):" << std::endl;
        os << os_block.str();
        os << std::endl << "altered block state (destination):" << std::endl;
        os << os_block_dest.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif

    return ret;
}

template<typename ElemBlockFunc, typename Trait>
mtv::element_t multi_type_vector<ElemBlockFunc, Trait>::get_type(size_type pos) const
{
    size_type block_index = get_block_position(pos);
    if (block_index == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::get_type", __LINE__, pos, block_size(), size());

    const block* blk = &m_blocks[block_index];
    if (!blk->data)
        return mtv::element_type_empty;

    return mtv::get_block_type(*blk->data);
}

template<typename ElemBlockFunc, typename Trait>
bool multi_type_vector<ElemBlockFunc, Trait>::is_empty(size_type pos) const
{
    size_type block_index = get_block_position(pos);
    if (block_index == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::is_empty", __LINE__, pos, block_size(), size());

    return m_blocks[block_index].data == nullptr;
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::set_empty(
    size_type start_pos, size_type end_pos)
{
    size_type block_index1 = get_block_position(start_pos);
    if (block_index1 == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::set_empty", __LINE__, start_pos, block_size(), size());

    return set_empty_impl(start_pos, end_pos, block_index1, true);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::set_empty(
    const iterator& pos_hint, size_type start_pos, size_type end_pos)
{
    size_type block_index1 = get_block_position(pos_hint->__private_data, start_pos);
    if (block_index1 == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::set_empty", __LINE__, start_pos, block_size(), size());

    return set_empty_impl(start_pos, end_pos, block_index1, true);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::transfer_impl(
    size_type start_pos, size_type end_pos, size_type block_index1, multi_type_vector& dest, size_type dest_pos)
{
    if (start_pos > end_pos)
    {
        std::ostringstream os;
        os << "multi_type_vector::transfer_impl: start position is larger than the end position. (start=";
        os << start_pos << ", end=" << end_pos << ")";
        throw std::out_of_range(os.str());
    }

    size_type block_index2 = get_block_position(end_pos, block_index1);
    if (block_index2 == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::transfer_impl", __LINE__, end_pos, block_size(), size());

    size_type len = end_pos - start_pos + 1;
    size_type last_dest_pos = dest_pos + len - 1;

    // Make sure the destination container is large enough.
    if (last_dest_pos >= dest.size())
        throw std::out_of_range("Destination vector is too small for the elements being transferred.");

    if (block_index1 == block_index2)
    {
        // All elements are in the same block.
        return transfer_single_block(start_pos, end_pos, block_index1, dest, dest_pos);
    }

    return transfer_multi_blocks(start_pos, end_pos, block_index1, block_index2, dest, dest_pos);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::
    transfer_single_block(
        size_type start_pos, size_type end_pos, size_type block_index1, multi_type_vector& dest, size_type dest_pos)
{
    size_type len = end_pos - start_pos + 1;
    size_type last_dest_pos = dest_pos + len - 1;

    // All elements are in the same block.
    block* blk = &m_blocks[block_index1];
    size_type start_pos_in_block1 = blk->position;

    // Empty the region in the destination container where the elements
    // are to be transferred to. This ensures that the destination region
    // consists of a single block.
    iterator it_dest_blk = dest.set_empty(dest_pos, last_dest_pos);

    if (!blk->data)
        return get_iterator(block_index1);

    element_category_type cat = get_block_type(*blk->data);

    size_type dest_block_index = it_dest_blk->__private_data.block_index;
    block* blk_dest = &dest.m_blocks[dest_block_index];

    size_type dest_pos_in_block = dest_pos - it_dest_blk->position;
    if (dest_pos_in_block == 0)
    {
        // Copy to the top part of the destination block.

        assert(!blk_dest->data); // should be already emptied.

        if (len < blk_dest->size)
        {
            // Shrink the existing block and insert a new block before it.
            assert(len < blk_dest->size);
            size_type position = blk_dest->position;
            blk_dest->position += len;
            blk_dest->size -= len;
            dest.m_blocks.emplace(dest.m_blocks.begin() + dest_block_index, position, len);
            blk_dest = &dest.m_blocks[dest_block_index]; // The old pointer is invalid.
        }
    }
    else if (dest_pos_in_block + len - 1 == it_dest_blk->size - 1)
    {
        // Copy to the bottom part of destination block.

        // Insert a new block below current, and shrink the current block.
        dest.m_blocks[dest_block_index].size -= len;
        size_type position = detail::calc_next_block_position(dest.m_blocks, dest_block_index);
        dest.m_blocks.emplace(dest.m_blocks.begin() + dest_block_index + 1, position, len);
        blk_dest = &dest.m_blocks[dest_block_index + 1];
        ++dest_block_index; // Must point to the new copied block.
    }
    else
    {
        // Copy to the middle of destination block.

        // Insert two new blocks below current.
        size_type blk2_size = blk_dest->size - dest_pos_in_block - len;
        dest.m_blocks.insert(dest.m_blocks.begin() + dest_block_index + 1, 2u, block());
        dest.m_blocks[dest_block_index].size = dest_pos_in_block;
        dest.m_blocks[dest_block_index + 1].size = len;
        dest.m_blocks[dest_block_index + 2].size = blk2_size;

        dest.m_blocks[dest_block_index + 1].position =
            detail::calc_next_block_position(dest.m_blocks, dest_block_index);
        dest.m_blocks[dest_block_index + 2].position =
            detail::calc_next_block_position(dest.m_blocks, dest_block_index + 1);

        blk_dest = &dest.m_blocks[dest_block_index + 1];

        ++dest_block_index; // Must point to the new copied block.
    }

    assert(blk_dest->size == len);
    size_type offset = start_pos - start_pos_in_block1;
    if (offset == 0 && len == blk->size)
    {
        // Just move the whole data array.
        blk_dest->data = blk->data;
        dest.m_hdl_event.element_block_acquired(blk_dest->data);

        m_hdl_event.element_block_released(blk->data);
        blk->data = nullptr;

        dest.merge_with_adjacent_blocks(dest_block_index);
        size_type start_pos_offset = merge_with_adjacent_blocks(block_index1);
        if (start_pos_offset)
        {
            // Merged with the previous block. Adjust the return block position.
            --block_index1;
            start_pos_in_block1 -= start_pos_offset;
        }

        return get_iterator(block_index1);
    }

    blk_dest->data = element_block_func::create_new_block(cat, 0);
    assert(blk_dest->data);
    dest.m_hdl_event.element_block_acquired(blk_dest->data);

    // Shallow-copy the elements to the destination block.
    element_block_func::assign_values_from_block(*blk_dest->data, *blk->data, offset, len);
    dest.merge_with_adjacent_blocks(dest_block_index);

    // Set the source range empty without overwriting the elements.
    return set_empty_in_single_block(start_pos, end_pos, block_index1, false);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::
    transfer_multi_blocks(
        size_type start_pos, size_type end_pos, size_type block_index1, size_type block_index2, multi_type_vector& dest,
        size_type dest_pos)
{
    assert(block_index1 < block_index2);
    size_type start_pos_in_block1 = m_blocks[block_index1].position;
    size_type start_pos_in_block2 = m_blocks[block_index2].position;

    size_type len = end_pos - start_pos + 1;
    size_type last_dest_pos = dest_pos + len - 1;

    // Empty the region in the destination container where the elements
    // are to be transferred to. This ensures that the destination region
    // consists of a single block.
    iterator it_dest_blk = dest.set_empty(dest_pos, last_dest_pos);

    size_type dest_block_index = it_dest_blk->__private_data.block_index;
    size_type dest_pos_in_block = dest_pos - it_dest_blk->position;
    block* blk_dest = &dest.m_blocks[dest_block_index];
    assert(!blk_dest->data); // should be already emptied.

    size_type block_len = block_index2 - block_index1 + 1;

    // Create slots for new blocks in the destination.

    size_type dest_block_index1 = dest_block_index;

    if (dest_pos_in_block == 0)
    {
        // Copy to the top part of destination block.
        if (len < blk_dest->size)
        {
            // Shrink the existing block and insert slots for the new blocks before it.
            blk_dest->size -= len;
            blk_dest->position += len;
            dest.m_blocks.insert(dest.m_blocks.begin() + dest_block_index, block_len, block());
        }
        else
        {
            // Destination block is exactly of the length of the elements being transferred.
            dest.delete_element_block(*blk_dest);
            blk_dest->size = 0;
            if (block_len > 1)
                dest.m_blocks.insert(dest.m_blocks.begin() + dest_block_index, block_len - 1, block());
        }
    }
    else if (dest_pos_in_block + len - 1 == it_dest_blk->size - 1)
    {
        // Copy to the bottom part of destination block. Insert slots for new
        // blocks below current, and shrink the current block.
        dest.m_blocks.insert(dest.m_blocks.begin() + dest_block_index + 1, block_len, block());
        blk_dest = &dest.m_blocks[dest_block_index];
        blk_dest->size -= len;

        ++dest_block_index1;
    }
    else
    {
        // Copy to the middle of the destination block. Insert slots for the
        // new blocks (plus one extra for the bottom empty block) below the
        // current block.
        size_type blk2_size = blk_dest->size - dest_pos_in_block - len;
        dest.m_blocks.insert(dest.m_blocks.begin() + dest_block_index + 1, block_len + 1, block());
        blk_dest = &dest.m_blocks[dest_block_index];
        assert(dest.m_blocks.size() > dest_block_index + block_len + 1);
        blk_dest->size = dest_pos_in_block;

        // Re-calculate the size and position of the lower part of the destination block.
        block& blk_dest_lower = dest.m_blocks[dest_block_index + block_len + 1];
        blk_dest_lower.position = detail::calc_next_block_position(*blk_dest) + len;
        blk_dest_lower.size = blk2_size;

        ++dest_block_index1;
    }

    size_type del_index1 = block_index1, del_index2 = block_index2;

    // Now that the new slots have been created, start transferring the blocks.

    // Transfer the first block.
    size_type offset = start_pos - start_pos_in_block1;
    if (offset)
    {
        // Transfer the lower part of the first block.
        block* blk = &m_blocks[block_index1];

        assert(dest.m_blocks[dest_block_index1].size == 0);
        dest.m_blocks[dest_block_index1].size = blk->size - offset;
        if (dest_block_index1 > 0)
            dest.m_blocks[dest_block_index1].position =
                detail::calc_next_block_position(dest.m_blocks, dest_block_index1 - 1);

        if (blk->data)
        {
            element_category_type cat = mtv::get_block_type(*blk->data);
            blk_dest = &dest.m_blocks[dest_block_index1];
            blk_dest->data = element_block_func::create_new_block(cat, 0);
            assert(blk_dest->data);
            dest.m_hdl_event.element_block_acquired(blk_dest->data);

            // Shallow-copy the elements to the destination block, and shrink
            // the source block to remove the transferred elements.
            element_block_func::assign_values_from_block(*blk_dest->data, *blk->data, offset, blk->size - offset);
            element_block_func::resize_block(*blk->data, offset);
        }

        blk->size = offset;
        ++del_index1; // Retain this block.
    }
    else
    {
        // Just move the whole block over.
        block& blk = m_blocks[block_index1];
        dest.m_blocks[dest_block_index1] = blk; // copied.
        dest.m_blocks[dest_block_index1].position =
            dest_block_index1 > 0 ? detail::calc_next_block_position(dest.m_blocks, dest_block_index1 - 1) : 0;

        if (blk.data)
        {
            dest.m_hdl_event.element_block_acquired(blk.data);
            m_hdl_event.element_block_released(blk.data);
            blk.data = nullptr;
        }

        blk.size = 0;
    }

    if (block_len > 2)
    {
        // Transfer all blocks in between.
        size_type position = detail::calc_next_block_position(dest.m_blocks, dest_block_index1);
        for (size_type i = 0; i < block_len - 2; ++i)
        {
            size_type src_block_pos = block_index1 + 1 + i;
            size_type dest_block_pos = dest_block_index1 + 1 + i;
            assert(dest.m_blocks[dest_block_pos].size == 0);
            block& blk = m_blocks[src_block_pos];
            dest.m_blocks[dest_block_pos] = blk; // copied.
            dest.m_blocks[dest_block_pos].position = position;
            position += blk.size;
            blk.size = 0;

            if (blk.data)
            {
                dest.m_hdl_event.element_block_acquired(blk.data);
                m_hdl_event.element_block_released(blk.data);
                blk.data = nullptr;
            }
        }
    }

    // Transfer the last block.
    if (block_len > 1)
    {
        size_type size_to_trans = end_pos - start_pos_in_block2 + 1;
        size_type dest_block_pos = dest_block_index1 + block_len - 1;
        assert(dest.m_blocks[dest_block_pos].size == 0);

        block& blk = m_blocks[block_index2];
        if (size_to_trans < blk.size)
        {
            // Transfer the upper part of this block.
            assert(dest_block_pos > 0);
            dest.m_blocks[dest_block_pos].position =
                detail::calc_next_block_position(dest.m_blocks, dest_block_pos - 1);
            dest.m_blocks[dest_block_pos].size = size_to_trans;
            blk_dest = &dest.m_blocks[dest_block_pos];
            if (blk.data)
            {
                element_category_type cat = mtv::get_block_type(*blk.data);
                blk_dest->data = element_block_func::create_new_block(cat, 0);
                dest.m_hdl_event.element_block_acquired(blk_dest->data);

                element_block_func::assign_values_from_block(*blk_dest->data, *blk.data, 0, size_to_trans);
                element_block_func::erase(*blk.data, 0, size_to_trans);
            }

            blk.position += size_to_trans;
            blk.size -= size_to_trans;
            --del_index2; // Retain this block.
        }
        else
        {
            // Just move the whole block over.
            dest.m_blocks[dest_block_pos] = blk;
            dest.m_blocks[dest_block_pos].position =
                dest_block_pos > 0 ? detail::calc_next_block_position(dest.m_blocks, dest_block_pos - 1) : 0;

            if (blk.data)
            {
                dest.m_hdl_event.element_block_acquired(blk.data);
                m_hdl_event.element_block_released(blk.data);
                blk.data = nullptr;
            }
            m_blocks[block_index2].size = 0;
        }
    }

    // Now that all the elements have been transferred, check the bordering
    // blocks in the destination and merge them as needed.
    if (block_len > 1)
        dest.merge_with_adjacent_blocks(dest_block_index1 + block_len - 1);
    dest.merge_with_adjacent_blocks(dest_block_index1);

    // Delete all transferred blocks, and replace it with one empty block.
    if (del_index2 < del_index1)
    {
        // No blocks will be deleted.  See if we can just extend one of the
        // neighboring empty blocks.

        block& blk1 = m_blocks[block_index1];
        block& blk2 = m_blocks[block_index2];

        if (!blk1.data)
        {
            assert(blk2.data);

            // Block 1 is empty. Extend this block downward.
            blk1.size += len;
            return get_iterator(block_index1);
        }

        if (!blk2.data)
        {
            assert(blk1.data);

            // Block 2 is empty. Extend this block upward.
            blk2.size += len;
            blk2.position -= len;
            return get_iterator(block_index2);
        }

        // Neither block1 nor block2 are empty. Just insert a new empty block
        // between them. After the insertion, the old block2 position becomes
        // the position of the inserted block.
        size_type position = detail::calc_next_block_position(blk1);
        m_blocks.emplace(m_blocks.begin() + block_index2, position, len);
        // No need to adjust local index vars
        return get_iterator(block_index2);
    }

    if (del_index1 > 0 && !m_blocks[del_index1 - 1].data)
    {
        // The block before the first block to be deleted is empty.  Simply
        // extend that block to cover the deleted block segment.
        block& blk_prev = m_blocks[del_index1 - 1];

        // Extend the previous block.
        blk_prev.size += len;
    }
    else
    {
        // Block before is not empty (or doesn't exist).  Keep the first slot,
        // and erase the rest.
        m_blocks[del_index1].size = len; // Insert an empty
        ++del_index1;
    }

    size_type ret_block_index = del_index1 - 1;

    if (del_index2 >= del_index1)
    {
        typename blocks_type::iterator it_blk = m_blocks.begin();
        typename blocks_type::iterator it_blk_end = m_blocks.begin();
        std::advance(it_blk, del_index1);
        std::advance(it_blk_end, del_index2 + 1);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
        typename blocks_type::iterator it_test = it_blk;
        for (; it_test != it_blk_end; ++it_test)
        {
            // All slots to be erased should have zero size
            assert(it_test->size == 0);
        }
#endif
        m_blocks.erase(it_blk, it_blk_end);
    }

    // The block pointed to by ret_block_index is guaranteed to be empty by
    // this point.
    assert(!m_blocks[ret_block_index].data);
    // Merging with the previous block never happens.
    size_type start_pos_offset = merge_with_adjacent_blocks(ret_block_index);
    (void)start_pos_offset; // avoid unused variable compiler warning.
    assert(!start_pos_offset);

    m_blocks[ret_block_index].position =
        ret_block_index > 0 ? detail::calc_next_block_position(m_blocks, ret_block_index - 1) : 0;

    return get_iterator(ret_block_index);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::set_empty_impl(
    size_type start_pos, size_type end_pos, size_type block_index1, bool overwrite)
{
    if (start_pos > end_pos)
        throw std::out_of_range("Start row is larger than the end row.");

    size_type block_index2 = get_block_position(end_pos, block_index1);
    if (block_index2 == m_blocks.size())
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
        ret_it = set_empty_in_multi_blocks(start_pos, end_pos, block_index1, block_index2, overwrite);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    try
    {
        check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        os << "block integrity check failed in set_empty(start_pos=" << start_pos << "; end_pos=" << end_pos << ")"
           << std::endl;
        os << "previous block state:" << std::endl;
        os << os_prev_block.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif
    return ret_it;
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::swap_impl(
    multi_type_vector& other, size_type start_pos, size_type end_pos, size_type other_pos, size_type block_index1,
    size_type block_index2, size_type dblock_index1, size_type dblock_index2)
{
    if (block_index1 == block_index2)
    {
        // Source range is in a single block.
        if (dblock_index1 == dblock_index2)
        {
            // Destination range is also in a single block.
            swap_single_block(other, start_pos, end_pos, other_pos, block_index1, dblock_index1);
        }
        else
        {
            // Source is single-, and destination is multi-blocks.
            swap_single_to_multi_blocks(
                other, start_pos, end_pos, other_pos, block_index1, dblock_index1, dblock_index2);
        }
    }
    else if (dblock_index1 == dblock_index2)
    {
        // Destination range is over a single block. Switch source and destination.
        size_type len = end_pos - start_pos + 1;
        other.swap_single_to_multi_blocks(
            *this, other_pos, other_pos + len - 1, start_pos, dblock_index1, block_index1, block_index2);
    }
    else
    {
        // Both source and destinations are multi-block.
        swap_multi_to_multi_blocks(
            other, start_pos, end_pos, other_pos, block_index1, block_index2, dblock_index1, dblock_index2);
    }
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::swap_single_block(
    multi_type_vector& other, size_type start_pos, size_type end_pos, size_type other_pos, size_type block_index,
    size_type other_block_index)
{
    block* blk_src = &m_blocks[block_index];
    block* blk_dst = &other.m_blocks[other_block_index];
    size_type start_pos_in_block = blk_src->position;
    size_type start_pos_in_other_block = blk_dst->position;
    element_category_type cat_src = mtv::element_type_empty;
    element_category_type cat_dst = mtv::element_type_empty;

    if (blk_src->data)
        cat_src = mtv::get_block_type(*blk_src->data);
    if (blk_dst->data)
        cat_dst = mtv::get_block_type(*blk_dst->data);

    size_t other_end_pos = other_pos + end_pos - start_pos;
    size_t len = end_pos - start_pos + 1; // length of elements to swap.
    size_type src_offset = start_pos - start_pos_in_block;
    size_type dst_offset = other_pos - start_pos_in_other_block;

    // length of the tail that will not be swapped.
    size_type src_tail_len = blk_src->size - src_offset - len;

    if (cat_src == cat_dst)
    {
        // Source and destination blocks are of the same type.
        if (cat_src == mtv::element_type_empty)
            // Both are empty blocks. Nothing to swap.
            return;

        element_block_func::swap_values(*blk_src->data, *blk_dst->data, src_offset, dst_offset, len);
        return;
    }

    // Source and destination blocks are of different types.

    if (cat_src == mtv::element_type_empty)
    {
        // Source is empty but destination is not. This is equivalent of transfer.
        other.transfer_single_block(other_pos, other_end_pos, other_block_index, *this, start_pos);
        // No update of local index vars needed.
        return;
    }

    if (cat_dst == mtv::element_type_empty)
    {
        // Source is not empty but destination is. Use transfer.
        transfer_single_block(start_pos, end_pos, block_index, other, other_pos);
        // No update of local index vars needed.
        return;
    }

    // Neither the source nor destination blocks are empty, and they are of different types.
    if (src_offset == 0)
    {
        // Source range is at the top of a block.
        if (src_tail_len == 0)
        {
            // the whole block needs to be replaced.
            std::unique_ptr<element_block_type, element_block_deleter> src_data(blk_src->data);
            m_hdl_event.element_block_released(blk_src->data);
            blk_src->data = other.exchange_elements(*src_data, src_offset, other_block_index, dst_offset, len);
            m_hdl_event.element_block_acquired(blk_src->data);

            // Release elements in the source block to prevent double-deletion.
            element_block_func::resize_block(*src_data, 0);
            merge_with_adjacent_blocks(block_index);
            return;
        }

        // Get the new elements from the other container.
        std::unique_ptr<element_block_type, element_block_deleter> dst_data(
            other.exchange_elements(*blk_src->data, src_offset, other_block_index, dst_offset, len));

        // Shrink the current block by erasing the top part.
        element_block_func::erase(*blk_src->data, 0, len);
        blk_src->position += len;
        blk_src->size -= len;

        block* blk_prev = get_previous_block_of_type(block_index, cat_dst);
        if (blk_prev)
        {
            // Append the new elements to the previous block.
            element_block_func::append_values_from_block(*blk_prev->data, *dst_data);
            element_block_func::resize_block(*dst_data, 0); // prevent double-delete.
            blk_prev->size += len;
        }
        else
        {
            // Insert a new block to store the new elements.
            size_type position = blk_src->position - len;
            m_blocks.emplace(m_blocks.begin() + block_index, position, len);
            block& blk = m_blocks[block_index];
            blk.data = dst_data.release();
            m_hdl_event.element_block_acquired(blk.data);
        }
        return;
    }

    // Get the new elements from the other container.
    std::unique_ptr<element_block_type, element_block_deleter> dst_data(
        other.exchange_elements(*blk_src->data, src_offset, other_block_index, dst_offset, len));

    if (src_tail_len == 0)
    {
        // Source range is at the bottom of a block.

        // Shrink the current block.
        element_block_func::resize_block(*blk_src->data, src_offset);
        blk_src->size = src_offset;

        block* blk_next = get_next_block_of_type(block_index, cat_dst);
        if (blk_next)
        {
            // Merge with the next block.
            element_block_func::prepend_values_from_block(*blk_next->data, *dst_data, 0, len);
            element_block_func::resize_block(*dst_data, 0); // prevent double-delete.
            blk_next->size += len;
            blk_next->position -= len;
        }
        else
        {
            size_type position = detail::calc_next_block_position(*blk_src);
            m_blocks.emplace(m_blocks.begin() + block_index + 1, position, len);
            block& blk = m_blocks[block_index + 1];
            blk.data = dst_data.release();
            m_hdl_event.element_block_acquired(blk.data);
        }
        return;
    }

    // Source range is in the middle of a block.
    assert(src_offset && src_tail_len);
    block& blk = set_new_block_to_middle(block_index, src_offset, len, false);
    blk.data = dst_data.release();
    m_hdl_event.element_block_acquired(blk.data);
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::swap_single_to_multi_blocks(
    multi_type_vector& other, size_type start_pos, size_type end_pos, size_type other_pos, size_type block_index,
    size_type dst_block_index1, size_type dst_block_index2)
{
    block* blk_src = &m_blocks[block_index];
    size_type start_pos_in_block = blk_src->position;
    size_type dst_start_pos_in_block1 = other.m_blocks[dst_block_index1].position;
    size_type dst_start_pos_in_block2 = other.m_blocks[dst_block_index2].position;

    element_category_type cat_src = mtv::element_type_empty;
    if (blk_src->data)
        cat_src = mtv::get_block_type(*blk_src->data);

    size_type len = end_pos - start_pos + 1;

    if (cat_src == mtv::element_type_empty)
    {
        // The source block is empty. Use transfer.
        other.transfer_multi_blocks(
            other_pos, other_pos + len - 1, dst_block_index1, dst_block_index2, *this, start_pos);
        return;
    }

    size_type src_offset = start_pos - start_pos_in_block;
    size_type dst_offset1 = other_pos - dst_start_pos_in_block1;
    size_type dst_offset2 = other_pos + len - 1 - dst_start_pos_in_block2;

    // length of the tail that will not be swapped.
    size_type src_tail_len = blk_src->size - src_offset - len;

    // Get the new elements from the other container.
    blocks_type new_blocks;
    other.exchange_elements(
        *blk_src->data, src_offset, dst_block_index1, dst_offset1, dst_block_index2, dst_offset2, len, new_blocks);

    if (new_blocks.empty())
        throw general_error("multi_type_vector::swap_single_to_multi_blocks: failed to exchange elements.");

    if (src_offset == 0)
    {
        // Source range is at the top of a block.

        size_type src_position = blk_src->position;

        if (src_tail_len == 0)
        {
            // the whole block needs to be replaced.  Delete the block, but
            // don't delete the managed elements the block contains since they
            // have been transferred over to the destination block.
            element_block_func::resize_block(*blk_src->data, 0);
            delete_element_block(*blk_src);
            m_blocks.erase(m_blocks.begin() + block_index);
        }
        else
        {
            // Shrink the current block by erasing the top part.
            element_block_func::erase(*blk_src->data, 0, len);
            blk_src->size -= len;
            blk_src->position += len;
        }

        insert_blocks_at(src_position, block_index, new_blocks);
        merge_with_next_block(block_index + new_blocks.size() - 1); // last block inserted.
        if (block_index > 0)
            merge_with_next_block(block_index - 1); // block before the first block inserted.

        return;
    }

    size_type position = 0;

    if (src_tail_len == 0)
    {
        // Source range is at the bottom of a block.

        // Shrink the current block.
        element_block_func::resize_block(*blk_src->data, src_offset);
        blk_src->size = src_offset;
        position = detail::calc_next_block_position(*blk_src);
    }
    else
    {
        // Source range is in the middle of a block.
        assert(src_offset && src_tail_len);

        // This will create two new slots at block_index+1, the first of which
        // we will immediately remove.  The new blocks from the other
        // container will be inserted at the removed slot.
        set_new_block_to_middle(block_index, src_offset, len, false);
        delete_element_block(m_blocks[block_index + 1]);
        m_blocks.erase(m_blocks.begin() + block_index + 1);
        position = detail::calc_next_block_position(m_blocks, block_index);
    }

    insert_blocks_at(position, block_index + 1, new_blocks);
    merge_with_next_block(block_index + new_blocks.size()); // last block inserted.
    merge_with_next_block(block_index); // block before the first block inserted.
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::swap_multi_to_multi_blocks(
    multi_type_vector& other, size_type start_pos, size_type end_pos, size_type other_pos, size_type block_index1,
    size_type block_index2, size_type dblock_index1, size_type dblock_index2)
{
    assert(block_index1 < block_index2);
    assert(dblock_index1 < dblock_index2);

    size_type start_pos_in_block1 = m_blocks[block_index1].position;
    size_type start_pos_in_block2 = m_blocks[block_index2].position;
    size_type start_pos_in_dblock1 = other.m_blocks[dblock_index1].position;
    size_type start_pos_in_dblock2 = other.m_blocks[dblock_index2].position;

    size_type len = end_pos - start_pos + 1;
    size_type src_offset1 = start_pos - start_pos_in_block1;
    size_type src_offset2 = end_pos - start_pos_in_block2;
    size_type dst_offset1 = other_pos - start_pos_in_dblock1;
    size_type dst_offset2 = other_pos + len - 1 - start_pos_in_dblock2;

    blocks_to_transfer src_bucket, dst_bucket;
    prepare_blocks_to_transfer(src_bucket, block_index1, src_offset1, block_index2, src_offset2);
    other.prepare_blocks_to_transfer(dst_bucket, dblock_index1, dst_offset1, dblock_index2, dst_offset2);

    size_type position =
        src_bucket.insert_index > 0 ? detail::calc_next_block_position(m_blocks, src_bucket.insert_index - 1) : 0;
    insert_blocks_at(position, src_bucket.insert_index, dst_bucket.blocks);

    // Merge the boundary blocks in the source.
    merge_with_next_block(src_bucket.insert_index + dst_bucket.blocks.size() - 1);
    if (src_bucket.insert_index > 0)
        merge_with_next_block(src_bucket.insert_index - 1);

    position =
        dst_bucket.insert_index > 0 ? detail::calc_next_block_position(other.m_blocks, dst_bucket.insert_index - 1) : 0;
    other.insert_blocks_at(position, dst_bucket.insert_index, src_bucket.blocks);

    // Merge the boundary blocks in the destination.
    other.merge_with_next_block(dst_bucket.insert_index + src_bucket.blocks.size() - 1);
    if (dst_bucket.insert_index > 0)
        other.merge_with_next_block(dst_bucket.insert_index - 1);
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::insert_blocks_at(
    size_type position, size_type insert_pos, blocks_type& new_blocks)
{
    std::for_each(new_blocks.begin(), new_blocks.end(), [&](block& r) {
        r.position = position;
        position += r.size;

        if (r.data)
            m_hdl_event.element_block_acquired(r.data);
    });

    m_blocks.insert(m_blocks.begin() + insert_pos, new_blocks.begin(), new_blocks.end());
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::prepare_blocks_to_transfer(
    blocks_to_transfer& bucket, size_type block_index1, size_type offset1, size_type block_index2, size_type offset2)
{
    assert(block_index1 < block_index2);
    assert(offset1 < m_blocks[block_index1].size);
    assert(offset2 < m_blocks[block_index2].size);

    block block_first;
    block block_last;
    typename blocks_type::iterator it_begin = m_blocks.begin();
    typename blocks_type::iterator it_end = m_blocks.begin();

    std::advance(it_begin, block_index1 + 1);
    std::advance(it_end, block_index2);
    bucket.insert_index = block_index1 + 1;

    if (offset1 == 0)
    {
        // The whole first block needs to be swapped.
        --it_begin;
        --bucket.insert_index;
    }
    else
    {
        // Copy the lower part of the block for transfer.
        block* blk = &m_blocks[block_index1];
        size_type blk_size = blk->size - offset1;
        block_first.size = blk_size;
        if (blk->data)
        {
            block_first.data = element_block_func::create_new_block(mtv::get_block_type(*blk->data), 0);
            element_block_func::assign_values_from_block(*block_first.data, *blk->data, offset1, blk_size);

            // Shrink the existing block.
            element_block_func::resize_block(*blk->data, offset1);
        }

        blk->size = offset1;
    }

    block* blk = &m_blocks[block_index2];
    if (offset2 == blk->size - 1)
    {
        // The whole last block needs to be swapped.
        ++it_end;
    }
    else
    {
        // Copy the upper part of the block for transfer.
        size_type blk_size = offset2 + 1;
        block_last.size = blk_size;
        if (blk->data)
        {
            block_last.data = element_block_func::create_new_block(mtv::get_block_type(*blk->data), 0);
            element_block_func::assign_values_from_block(*block_last.data, *blk->data, 0, blk_size);

            // Shrink the existing block.
            element_block_func::erase(*blk->data, 0, blk_size);
        }

        blk->position += blk_size;
        blk->size -= blk_size;
    }

    // Copy all blocks into the bucket.
    if (block_first.size)
        bucket.blocks.push_back(std::move(block_first));

    std::for_each(it_begin, it_end, [&](block& r) {
        if (r.data)
            m_hdl_event.element_block_released(r.data);
        bucket.blocks.push_back(r);
    });

    if (block_last.size)
        bucket.blocks.push_back(std::move(block_last));

    // Remove the slots for these blocks (but don't delete the blocks).
    m_blocks.erase(it_begin, it_end);
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::erase(size_type start_pos, size_type end_pos)
{
    if (start_pos > end_pos)
        throw std::out_of_range("Start row is larger than the end row.");

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_prev_block;
    dump_blocks(os_prev_block);
#endif

    erase_impl(start_pos, end_pos);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    try
    {
        check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        os << "block integrity check failed in erase (" << start_pos << "-" << end_pos << ")" << std::endl;
        os << "block integrity check failed in push_back" << std::endl;
        os << "previous block state:" << std::endl;
        os << os_prev_block.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::erase_impl(size_type start_row, size_type end_row)
{
    assert(start_row <= end_row);

    // Keep the logic similar to set_empty().

    size_type block_pos1 = get_block_position(start_row);
    if (block_pos1 == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::erase_impl", __LINE__, start_row, block_size(), size());

    size_type block_pos2 = get_block_position(end_row, block_pos1);
    if (block_pos2 == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::erase_impl", __LINE__, start_row, block_size(), size());

    size_type start_row_in_block1 = m_blocks[block_pos1].position;
    size_type start_row_in_block2 = m_blocks[block_pos2].position;

    if (block_pos1 == block_pos2)
    {
        erase_in_single_block(start_row, end_row, block_pos1);
        return;
    }

    assert(block_pos1 < block_pos2);

    // Initially, we set to erase all blocks between the first and the last.
    typename blocks_type::iterator it_erase_begin = m_blocks.begin() + block_pos1 + 1;
    typename blocks_type::iterator it_erase_end = m_blocks.begin() + block_pos2;

    // First, inspect the first block.
    if (start_row_in_block1 == start_row)
    {
        // Erase the whole block.
        --it_erase_begin;
    }
    else
    {
        // Erase the lower part of the first block.
        block* blk = &m_blocks[block_pos1];
        size_type new_size = start_row - start_row_in_block1;
        if (blk->data)
        {
            // Shrink the data array.
            element_block_func::overwrite_values(*blk->data, new_size, blk->size - new_size);
            element_block_func::resize_block(*blk->data, new_size);
        }
        blk->size = new_size;
    }

    size_type adjust_block_offset = 0;

    // Then inspect the last block.
    block* blk = &m_blocks[block_pos2];
    size_type last_row_in_block = start_row_in_block2 + blk->size - 1;
    if (last_row_in_block == end_row)
    {
        // Delete the whole block.
        ++it_erase_end;
    }
    else
    {
        size_type size_to_erase = end_row - start_row_in_block2 + 1;
        blk->size -= size_to_erase;
        blk->position = start_row;
        if (blk->data)
        {
            // Erase the upper part.
            element_block_func::overwrite_values(*blk->data, 0, size_to_erase);
            element_block_func::erase(*blk->data, 0, size_to_erase);
        }

        adjust_block_offset = 1; // Exclude this block from later block position adjustment.
    }

    // Get the index of the block that sits before the blocks being erased.
    block_pos1 = std::distance(m_blocks.begin(), it_erase_begin);
    if (block_pos1 > 0)
        --block_pos1;

    // Now, erase all blocks in between.
    delete_element_blocks(it_erase_begin, it_erase_end);
    auto it_adjust_block = m_blocks.erase(it_erase_begin, it_erase_end);
    int64_t delta = end_row - start_row + 1;
    m_cur_size -= delta;

    if (m_blocks.empty())
        return;

    size_t adjust_pos = std::distance(m_blocks.begin(), it_adjust_block);
    adjust_pos += adjust_block_offset;
    adjust_block_positions_func{}(m_blocks, adjust_pos, -delta);
    merge_with_next_block(block_pos1);
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::erase_in_single_block(
    size_type start_pos, size_type end_pos, size_type block_pos)
{
    // Range falls within the same block.
    block* blk = &m_blocks[block_pos];
    int64_t size_to_erase = end_pos - start_pos + 1;
    if (blk->data)
    {
        // Erase data in the data block.
        size_type offset = start_pos - blk->position;
        element_block_func::overwrite_values(*blk->data, offset, size_to_erase);
        element_block_func::erase(*blk->data, offset, size_to_erase);
    }

    blk->size -= size_to_erase;
    m_cur_size -= size_to_erase;

    if (blk->size)
    {
        // Block still contains data.  Bail out.
        adjust_block_positions_func{}(m_blocks, block_pos + 1, -size_to_erase);
        return;
    }

    // Delete the current block since it's become empty.
    delete_element_block(*blk);
    m_blocks.erase(m_blocks.begin() + block_pos);
    // No need to update blk_index which is not used again.

    if (block_pos == 0)
    {
        // Deleted block was the first block.
        adjust_block_positions_func{}(m_blocks, block_pos, -size_to_erase);
        return;
    }

    if (block_pos >= m_blocks.size())
        // Deleted block was the last block.
        return;

    // Check the previous and next blocks to see if they should be merged.
    block* blk_prev = &m_blocks[block_pos - 1];
    block* blk_next = &m_blocks[block_pos];
    if (blk_prev->data)
    {
        // Previous block has data.
        if (!blk_next->data)
        {
            // Next block is empty.  Nothing to do.
            adjust_block_positions_func{}(m_blocks, block_pos, -size_to_erase);
            return;
        }

        element_category_type cat1 = mdds::mtv::get_block_type(*blk_prev->data);
        element_category_type cat2 = mdds::mtv::get_block_type(*blk_next->data);
        if (cat1 == cat2)
        {
            // Merge the two blocks.
            element_block_func::append_values_from_block(*blk_prev->data, *blk_next->data);
            blk_prev->size += blk_next->size;
            // Resize to 0 to prevent deletion of cells in case of managed cells.
            element_block_func::resize_block(*blk_next->data, 0);
            delete_element_block(*blk_next);
            m_blocks.erase(m_blocks.begin() + block_pos);
        }

        adjust_block_positions_func{}(m_blocks, block_pos, -size_to_erase);
    }
    else
    {
        // Previous block is empty.
        if (blk_next->data)
        {
            // Next block is not empty.  Nothing to do.
            adjust_block_positions_func{}(m_blocks, block_pos, -size_to_erase);
            return;
        }

        // Both blocks are empty.  Simply increase the size of the
        // previous block.
        blk_prev->size += blk_next->size;
        delete_element_block(*blk_next);
        m_blocks.erase(m_blocks.begin() + block_pos);
        adjust_block_positions_func{}(m_blocks, block_pos, -size_to_erase);
    }
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::insert_empty(
    size_type pos, size_type length)
{
    if (!length)
        // Nothing to insert.
        return end();

    size_type block_index = get_block_position(pos);
    if (block_index == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::insert_empty", __LINE__, pos, block_size(), size());

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_prev_block;
    dump_blocks(os_prev_block);
#endif

    iterator ret = insert_empty_impl(pos, block_index, length);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    try
    {
        check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        os << "block integrity check failed in insert_empty (pos=" << pos << "; length=" << length << ")" << std::endl;
        os << "previous block state:" << std::endl;
        os << os_prev_block.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif

    return ret;
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::insert_empty(
    const iterator& pos_hint, size_type pos, size_type length)
{
    if (!length)
        // Nothing to insert.
        return end();

    size_type block_index = get_block_position(pos_hint->__private_data, pos);
    if (block_index == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::insert_empty", __LINE__, pos, block_size(), size());

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_prev_block;
    dump_blocks(os_prev_block);
#endif

    iterator ret = insert_empty_impl(pos, block_index, length);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    try
    {
        check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        os << "block integrity check failed in insert_empty (pos=" << pos << "; length=" << length << ")" << std::endl;
        os << "previous block state:" << std::endl;
        os << os_prev_block.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif

    return ret;
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::insert_empty_impl(
    size_type pos, size_type block_index, size_type length)
{
    assert(pos < m_cur_size);

    block* blk = &m_blocks[block_index];
    if (!blk->data)
    {
        // Insertion point is already empty.  Just expand its size and be done
        // with it.
        blk->size += length;
        m_cur_size += length;
        adjust_block_positions_func{}(m_blocks, block_index + 1, length);
        return get_iterator(block_index);
    }

    size_type start_pos = blk->position;

    if (start_pos == pos)
    {
        // Insertion point is at the top of an existing non-empty block.
        block* blk_prev = get_previous_block_of_type(block_index, mtv::element_type_empty);
        if (blk_prev)
        {
            assert(!blk_prev->data);
            // Previous block is empty.  Expand the size of the previous
            // block and bail out.
            blk_prev->size += length;
            m_cur_size += length;
            adjust_block_positions_func{}(m_blocks, block_index, length);
            return get_iterator(block_index - 1);
        }

        // Insert a new empty block.
        m_blocks.emplace(m_blocks.begin() + block_index, blk->position, length);
        m_cur_size += length;
        adjust_block_positions_func{}(m_blocks, block_index + 1, length);
        return get_iterator(block_index);
    }

    assert(blk->data);
    assert(pos > start_pos);

    size_type size_blk_prev = pos - start_pos;
    size_type size_blk_next = blk->size - size_blk_prev;

    // Insert two new blocks below the current; one for the empty block being
    // inserted, and the other for the lower part of the current non-empty
    // block.
    m_blocks.insert(m_blocks.begin() + block_index + 1, 2u, block());

    blk = &m_blocks[block_index]; // The old pointer is invalid.
    m_blocks[block_index + 1].size = length;
    m_blocks[block_index + 2].size = size_blk_next;

    block* blk_next = &m_blocks[block_index + 2];
    blk_next->data = element_block_func::create_new_block(mdds::mtv::get_block_type(*blk->data), 0);
    m_hdl_event.element_block_acquired(blk_next->data);

    // Check if the previous block is the bigger one
    if (size_blk_prev > size_blk_next)
    {
        // Upper (previous) block is larger than the lower (next) block. Copy
        // the lower values to the next block.
        element_block_func::assign_values_from_block(*blk_next->data, *blk->data, size_blk_prev, size_blk_next);
        element_block_func::resize_block(*blk->data, size_blk_prev);
        blk->size = size_blk_prev;
    }
    else
    {
        // Lower (next) block is larger than the upper (previous) block. Copy
        // the upper values to the "next" block.
        element_block_func::assign_values_from_block(*blk_next->data, *blk->data, 0, size_blk_prev);
        blk_next->size = size_blk_prev;

        // Remove the copied values and push the rest to the top.
        element_block_func::erase(*blk->data, 0, size_blk_prev);

        // Set the size of the current block to its new size ( what is after the new block )
        blk->size = size_blk_next;

        // And now let's swap the blocks, but save the block position.
        size_type position = blk->position;
        blk->swap(*blk_next);
        blk->position = position;
    }

    m_cur_size += length;
    m_blocks[block_index + 1].position = detail::calc_next_block_position(m_blocks, block_index);
    m_blocks[block_index + 2].position = detail::calc_next_block_position(m_blocks, block_index + 1);
    adjust_block_positions_func{}(m_blocks, block_index + 3, length);

    return get_iterator(block_index + 1);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::set_cells_impl(
    size_type row, size_type end_row, size_type block_index1, const T& it_begin, const T& it_end)
{
    size_type block_index2 = get_block_position(end_row, block_index1);
    if (block_index2 == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::set_cells_impl", __LINE__, end_row, block_size(), size());

    if (block_index1 == block_index2)
    {
        // The whole data array will fit in a single block.
        return set_cells_to_single_block(row, end_row, block_index1, it_begin, it_end);
    }

    return set_cells_to_multi_blocks(row, end_row, block_index1, block_index2, it_begin, it_end);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::insert_cells_impl(
    size_type row, size_type block_index, const T& it_begin, const T& it_end)
{
    size_type start_row = m_blocks[block_index].position;
    size_type length = std::distance(it_begin, it_end);
    if (!length)
        // empty data array.  nothing to do.
        return end();

    element_category_type cat = mdds_mtv_get_element_type(*it_begin);
    block* blk = &m_blocks[block_index];
    if (!blk->data)
    {
        if (row == start_row)
        {
            // Insert into an empty block.  Check the previos block (if
            // exists) to see if the data can be appended to it if inserting
            // at the top of the block.
            block* blk0 = get_previous_block_of_type(block_index, cat);
            if (blk0)
            {
                // Append to the previous block.
                mdds_mtv_append_values(*blk0->data, *it_begin, it_begin, it_end);
                blk0->size += length;
                m_cur_size += length;
                adjust_block_positions_func{}(m_blocks, block_index, length);

                return get_iterator(block_index - 1);
            }

            // Just insert a new block before the current block.
            size_type position = m_blocks[block_index].position;
            m_blocks.emplace(m_blocks.begin() + block_index, position, length);
            blk = &m_blocks[block_index];
            blk->data = element_block_func::create_new_block(cat, 0);
            m_hdl_event.element_block_acquired(blk->data);
            mdds_mtv_assign_values(*blk->data, *it_begin, it_begin, it_end);
            m_cur_size += length;
            adjust_block_positions_func{}(m_blocks, block_index + 1, length);

            return get_iterator(block_index);
        }

        insert_cells_to_middle(row, block_index, it_begin, it_end);
        m_cur_size += length;

        return get_iterator(block_index + 1);
    }

    assert(blk->data);
    element_category_type blk_cat = mdds::mtv::get_block_type(*blk->data);
    if (cat == blk_cat)
    {
        // Simply insert the new data series into existing block.
        assert(it_begin != it_end);
        mdds_mtv_insert_values(*blk->data, row - start_row, *it_begin, it_begin, it_end);
        blk->size += length;
        m_cur_size += length;
        adjust_block_positions_func{}(m_blocks, block_index + 1, length);

        return get_iterator(block_index);
    }

    assert(cat != blk_cat);
    if (row == start_row)
    {
        // Check the previous block to see if we can append the data there.
        block* blk0 = get_previous_block_of_type(block_index, cat);
        if (blk0)
        {
            // Append to the previous block.
            mdds_mtv_append_values(*blk0->data, *it_begin, it_begin, it_end);
            blk0->size += length;
            m_cur_size += length;
            adjust_block_positions_func{}(m_blocks, block_index, length);

            return get_iterator(block_index - 1);
        }

        // Just insert a new block before the current block.
        m_blocks.emplace(m_blocks.begin() + block_index, blk->position, length);
        blk = &m_blocks[block_index];
        blk->data = element_block_func::create_new_block(cat, 0);
        m_hdl_event.element_block_acquired(blk->data);
        mdds_mtv_assign_values(*blk->data, *it_begin, it_begin, it_end);
        blk->size = length;
        m_cur_size += length;
        adjust_block_positions_func{}(m_blocks, block_index + 1, length);

        return get_iterator(block_index);
    }

    insert_cells_to_middle(row, block_index, it_begin, it_end);
    m_cur_size += length;

    return get_iterator(block_index + 1);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
void multi_type_vector<ElemBlockFunc, Trait>::insert_cells_to_middle(
    size_type row, size_type block_index, const T& it_begin, const T& it_end)
{
    size_type start_row = m_blocks[block_index].position;
    size_type length = std::distance(it_begin, it_end);
    element_category_type cat = mdds_mtv_get_element_type(*it_begin);

    // Insert two new blocks after the specified block position.
    size_type n1 = row - start_row;
    size_type n2 = m_blocks[block_index].size - n1;
    m_blocks.insert(m_blocks.begin() + block_index + 1, 2u, block());
    block* blk = &m_blocks[block_index];
    blk->size = n1;

    m_blocks[block_index + 1].size = length;
    m_blocks[block_index + 1].position = detail::calc_next_block_position(*blk);
    m_blocks[block_index + 2].size = n2;
    m_blocks[block_index + 2].position = detail::calc_next_block_position(m_blocks[block_index + 1]);

    // block for data series.
    block* blk2 = &m_blocks[block_index + 1];
    blk2->data = element_block_func::create_new_block(cat, 0);
    m_hdl_event.element_block_acquired(blk2->data);
    mdds_mtv_assign_values(*blk2->data, *it_begin, it_begin, it_end);

    if (blk->data)
    {
        element_category_type blk_cat = mdds::mtv::get_block_type(*blk->data);

        // block to hold data from the lower part of the existing block.
        block* blk3 = &m_blocks[block_index + 2];
        blk3->data = element_block_func::create_new_block(blk_cat, 0);
        m_hdl_event.element_block_acquired(blk3->data);

        // Transfer the lower part of the current block to the new block.
        size_type offset = row - start_row;
        element_block_func::assign_values_from_block(*blk3->data, *blk->data, offset, n2);
        element_block_func::resize_block(*blk->data, blk->size);
    }

    adjust_block_positions_func{}(m_blocks, block_index + 3, length);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::block& multi_type_vector<ElemBlockFunc, Trait>::
    set_new_block_to_middle(size_type block_index, size_type offset, size_type new_block_size, bool overwrite)
{
    assert(block_index < m_blocks.size());

    // First, insert two new blocks at position past the current block.
    size_type lower_block_size = m_blocks[block_index].size - offset - new_block_size;
    m_blocks.insert(m_blocks.begin() + block_index + 1, 2u, block());
    m_blocks[block_index + 1].size = new_block_size; // empty block.
    m_blocks[block_index + 2].size = lower_block_size;

    block& blk = m_blocks[block_index];

    if (blk.data)
    {
        size_type lower_data_start = offset + new_block_size;
        block& blk_lower = m_blocks[block_index + 2];
        assert(blk_lower.size == lower_block_size);
        element_category_type cat = mtv::get_block_type(*blk.data);
        blk_lower.data = element_block_func::create_new_block(cat, 0);
        m_hdl_event.element_block_acquired(blk_lower.data);

        // Try to copy the fewer amount of data to the new non-empty block.
        if (offset > lower_block_size)
        {
            // Keep the upper values in the current block and copy the lower
            // values to the new non-empty block.
            element_block_func::assign_values_from_block(
                *blk_lower.data, *blk.data, lower_data_start, lower_block_size);

            if (overwrite)
            {
                // Overwrite cells that will become empty.
                element_block_func::overwrite_values(*blk.data, offset, new_block_size);
            }

            // Shrink the current data block.
            element_block_func::resize_block(*blk.data, offset);
            blk.size = offset;
            blk_lower.size = lower_block_size;
        }
        else
        {
            // Keep the lower values in the current block and copy the upper
            // values to the new non-empty block (blk_lower), and swap the two
            // later.
            element_block_func::assign_values_from_block(*blk_lower.data, *blk.data, 0, offset);
            blk_lower.size = offset;

            if (overwrite)
            {
                // Overwrite cells that will become empty.
                element_block_func::overwrite_values(*blk.data, offset, new_block_size);
            }

            // Remove the upper and middle values and push the rest to the top.
            element_block_func::erase(*blk.data, 0, lower_data_start);

            // Set the size of the current block to its new size ( what is after the new block )
            blk.size = lower_block_size;
            blk_lower.size = offset;

            // And now let's swap the blocks, while preserving the position of the original block.
            size_type position = blk.position;
            blk.swap(blk_lower);
            blk.position = position;
        }
    }
    else
    {
        // There is no data, we just need to update the size of the block
        blk.size = offset;
    }

    // Re-calculate the block positions.
    m_blocks[block_index + 1].position = detail::calc_next_block_position(m_blocks, block_index);
    m_blocks[block_index + 2].position = detail::calc_next_block_position(m_blocks, block_index + 1);

    return m_blocks[block_index + 1];
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::block* multi_type_vector<
    ElemBlockFunc, Trait>::get_previous_block_of_type(size_type block_index, element_category_type cat)
{
    if (block_index == 0)
        // No previous block.
        return nullptr;

    block& blk = m_blocks[block_index - 1];
    if (blk.data)
        return (cat == mtv::get_block_type(*blk.data)) ? &blk : nullptr;

    return (cat == mtv::element_type_empty) ? &blk : nullptr;
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::block* multi_type_vector<
    ElemBlockFunc, Trait>::get_next_block_of_type(size_type block_index, element_category_type cat)
{
    if (block_index == m_blocks.size() - 1)
        // No next block.
        return nullptr;

    block* blk = &m_blocks[block_index + 1];
    if (blk->data)
        return (cat == mtv::get_block_type(*blk->data)) ? blk : nullptr;

    return (cat == mtv::element_type_empty) ? blk : nullptr;
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::element_block_type* multi_type_vector<ElemBlockFunc, Trait>::
    exchange_elements(
        const element_block_type& src_data, size_type src_offset, size_type dst_index, size_type dst_offset,
        size_type len)
{
    assert(dst_index < m_blocks.size());
    block* blk = &m_blocks[dst_index];
    element_category_type cat_src = mtv::get_block_type(src_data);
    block* blk_next = get_next_block_of_type(dst_index, cat_src);

    if (dst_offset == 0)
    {
        // Set elements to the top of the destination block.
        block* blk_prev = get_previous_block_of_type(dst_index, cat_src);

        if (blk->size == len)
        {
            // The whole block will get replaced.
            std::unique_ptr<element_block_type, element_block_deleter> data(blk->data);
            m_hdl_event.element_block_released(blk->data);
            blk->data = nullptr; // Prevent its deletion when the parent block gets deleted.

            if (blk_prev)
            {
                // Append to the previous block. Remove the current block.
                element_block_func::append_values_from_block(*blk_prev->data, src_data, src_offset, len);
                blk_prev->size += len;
                typename blocks_type::iterator it = m_blocks.begin();
                std::advance(it, dst_index);
                typename blocks_type::iterator it_end = it;
                ++it_end;

                assert(!blk->data);
                // no need to call delete_block since mp_data is null.

                if (blk_next)
                {
                    // Apend elements from the next block too.
                    element_block_func::append_values_from_block(*blk_prev->data, *blk_next->data);
                    blk_prev->size += blk_next->size;
                    ++it_end;
                    delete_element_block(*blk_next);
                }

                m_blocks.erase(it, it_end);
                return data.release();
            }

            // Check the next block to see if we need to merge.
            if (blk_next)
            {
                // We need to merge with the next block.  Remove the current
                // block and use the next block to store the new elements as
                // well as the existing ones.
                element_block_func::prepend_values_from_block(*blk_next->data, src_data, src_offset, len);
                blk_next->position -= len;
                blk_next->size += len;
                m_blocks.erase(m_blocks.begin() + dst_index);
            }
            else
            {
                blk->data = element_block_func::create_new_block(cat_src, 0);
                m_hdl_event.element_block_acquired(blk->data);
                assert(blk->data && blk->data != data.get());
                element_block_func::assign_values_from_block(*blk->data, src_data, src_offset, len);
            }

            // Return this data block as-is.
            return data.release();
        }

        // New block to send back to the caller.
        std::unique_ptr<element_block_type, element_block_deleter> data(nullptr);

        if (blk->data)
        {
            element_category_type cat_dst = mtv::get_block_type(*blk->data);
            data.reset(element_block_func::create_new_block(cat_dst, 0));

            // We need to keep the tail elements of the current block.
            element_block_func::assign_values_from_block(*data, *blk->data, 0, len);
            element_block_func::erase(*blk->data, 0, len);
        }

        size_type position = blk->position;
        blk->position += len;
        blk->size -= len;

        if (blk_prev)
        {
            // Append the new elements to the previous block.
            element_block_func::append_values_from_block(*blk_prev->data, src_data, src_offset, len);
            blk_prev->size += len;
        }
        else
        {
            // Insert a new block to house the new elements.
            m_blocks.emplace(m_blocks.begin() + dst_index, position, len);
            blk = &m_blocks[dst_index];
            blk->data = element_block_func::create_new_block(cat_src, 0);
            m_hdl_event.element_block_acquired(blk->data);
            element_block_func::assign_values_from_block(*blk->data, src_data, src_offset, len);
        }

        return data.release();
    }

    // New block to send back to the caller.
    std::unique_ptr<element_block_type, element_block_deleter> data(nullptr);

    if (blk->data)
    {
        // Copy the elements of the current block to the block being returned.
        element_category_type cat_dst = mtv::get_block_type(*blk->data);
        data.reset(element_block_func::create_new_block(cat_dst, 0));
        element_block_func::assign_values_from_block(*data, *blk->data, dst_offset, len);
    }

    assert(dst_offset > 0);
    size_type dst_end_pos = dst_offset + len;

    if (dst_end_pos == blk->size)
    {
        // The new elements will replace the lower part of the block.
        assert(blk->data && "nullptr block mp_data");
        element_block_func::resize_block(*blk->data, dst_offset);
        blk->size = dst_offset;

        if (blk_next)
        {
            // Merge with the next block.
            element_block_func::prepend_values_from_block(*blk_next->data, src_data, src_offset, len);
            blk_next->position -= len;
            blk_next->size += len;
        }
        else
        {
            // Insert a new block to store the new elements.
            size_type position = blk->position + dst_offset;
            m_blocks.emplace(m_blocks.begin() + dst_index + 1, position, len);
            blk = &m_blocks[dst_index + 1];
            blk->data = element_block_func::create_new_block(cat_src, 0);
            assert(blk->data);
            m_hdl_event.element_block_acquired(blk->data);
            element_block_func::assign_values_from_block(*blk->data, src_data, src_offset, len);
        }
    }
    else
    {
        // The new elements will replace the middle of the block.
        assert(dst_end_pos < blk->size);
        blk = &set_new_block_to_middle(dst_index, dst_offset, len, false);
        assert(blk->size == len);
        blk->data = element_block_func::create_new_block(cat_src, 0);
        assert(blk->data);
        m_hdl_event.element_block_acquired(blk->data);
        element_block_func::assign_values_from_block(*blk->data, src_data, src_offset, len);
    }

    return data.release();
}

template<typename ElemBlockFunc, typename Trait>
bool multi_type_vector<ElemBlockFunc, Trait>::append_empty(size_type len)
{
    // Append empty cells.
    if (m_blocks.empty())
    {
        // No existing block. Create a new one.
        assert(m_cur_size == 0);
        m_blocks.emplace_back(0, len);
        m_cur_size = len;
        return true;
    }

    bool new_block_added = false;
    block* blk_last = &m_blocks.back();

    if (!blk_last->data)
    {
        // Last block is empty.  Just increase its size.
        blk_last->size += len;
    }
    else
    {
        // Append a new empty block.
        m_blocks.emplace_back(m_cur_size, len);
        new_block_added = true;
    }

    m_cur_size += len;

    return new_block_added;
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::exchange_elements(
    const element_block_type& src_data, size_type src_offset, size_type dst_index1, size_type dst_offset1,
    size_type dst_index2, size_type dst_offset2, size_type len, blocks_type& new_blocks)
{
    assert(dst_index1 < dst_index2);
    assert(dst_offset1 < m_blocks[dst_index1].size);
    assert(dst_offset2 < m_blocks[dst_index2].size);

    blocks_to_transfer bucket;
    prepare_blocks_to_transfer(bucket, dst_index1, dst_offset1, dst_index2, dst_offset2);

    m_blocks.emplace(m_blocks.begin() + bucket.insert_index, 0, len);
    block* blk = &m_blocks[bucket.insert_index];
    if (bucket.insert_index > 0)
        blk->position = detail::calc_next_block_position(m_blocks, bucket.insert_index - 1);

    blk->data = element_block_func::create_new_block(mtv::get_block_type(src_data), 0);
    m_hdl_event.element_block_acquired(blk->data);
    element_block_func::assign_values_from_block(*blk->data, src_data, src_offset, len);
    merge_with_adjacent_blocks(bucket.insert_index);

    new_blocks.swap(bucket.blocks);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::
    set_cells_to_single_block(
        size_type start_row, size_type end_row, size_type block_index, const T& it_begin, const T& it_end)
{
    assert(it_begin != it_end);
    assert(!m_blocks.empty());

    element_category_type cat = mdds_mtv_get_element_type(*it_begin);
    block* blk = &m_blocks[block_index];
    size_type start_row_in_block = blk->position;
    size_type data_length = std::distance(it_begin, it_end);

    if (blk->data && mdds::mtv::get_block_type(*blk->data) == cat)
    {
        // simple overwrite.
        size_type offset = start_row - start_row_in_block;
        element_block_func::overwrite_values(*blk->data, offset, data_length);
        if (!offset && data_length == blk->size)
            // Overwrite the whole block.  It's faster to use assign_values.
            mdds_mtv_assign_values(*blk->data, *it_begin, it_begin, it_end);
        else
            mdds_mtv_set_values(*blk->data, offset, *it_begin, it_begin, it_end);

        return get_iterator(block_index);
    }

    size_type end_row_in_block = start_row_in_block + blk->size - 1;
    if (start_row == start_row_in_block)
    {
        if (end_row == end_row_in_block)
        {
            // Check if we could append it to the previous block.
            if (append_to_prev_block(block_index, cat, end_row - start_row + 1, it_begin, it_end))
            {
                delete_element_block(*blk);
                m_blocks.erase(m_blocks.begin() + block_index);

                // Check if we need to merge it with the next block.
                --block_index;
                merge_with_next_block(block_index);
                return get_iterator(block_index);
            }

            // Replace the whole block.
            if (blk->data)
            {
                m_hdl_event.element_block_released(blk->data);
                element_block_func::delete_block(blk->data);
            }

            blk->data = element_block_func::create_new_block(cat, 0);
            m_hdl_event.element_block_acquired(blk->data);
            mdds_mtv_assign_values(*blk->data, *it_begin, it_begin, it_end);
            merge_with_next_block(block_index);
            return get_iterator(block_index);
        }

        // Replace the upper part of the block.

        // Shrink the current block first.
        size_type length = end_row_in_block - end_row;
        blk->size = length;

        if (blk->data)
        {
            // Erase the upper part of the data from the current data array.
            std::unique_ptr<element_block_type, element_block_deleter> new_data(
                element_block_func::create_new_block(mdds::mtv::get_block_type(*blk->data), 0));

            if (!new_data)
                throw std::logic_error("failed to instantiate a new data array.");

            size_type pos = end_row - start_row_in_block + 1;
            element_block_func::assign_values_from_block(*new_data, *blk->data, pos, length);
            element_block_func::overwrite_values(*blk->data, 0, pos);

            // Resize the block to zero before deleting, to prevent the
            // managed cells from being deleted when the block is deleted.
            element_block_func::resize_block(*blk->data, 0);
            element_block_func::delete_block(blk->data);
            blk->data = new_data.release();

            // We don't call element block event listeners here.
        }

        length = end_row - start_row + 1;
        if (append_to_prev_block(block_index, cat, length, it_begin, it_end))
        {
            // The new values have been successfully appended to the previous block.
            blk->position += length;
            return get_iterator(block_index - 1);
        }

        // Insert a new block before the current block, and populate it with
        // the new data.
        size_type position = blk->position;
        blk->position += length;
        m_blocks.emplace(m_blocks.begin() + block_index, position, length);
        blk = &m_blocks[block_index];
        blk->data = element_block_func::create_new_block(cat, 0);
        m_hdl_event.element_block_acquired(blk->data);
        blk->size = length;
        mdds_mtv_assign_values(*blk->data, *it_begin, it_begin, it_end);
        return get_iterator(block_index);
    }

    assert(start_row > start_row_in_block);
    if (end_row == end_row_in_block)
    {
        // Shrink the end of the current block and insert a new block for the new data series after the current block.
        size_type new_size = start_row - start_row_in_block;
        blk->size = new_size;
        if (blk->data)
        {
            element_block_func::overwrite_values(*blk->data, new_size, data_length);
            element_block_func::resize_block(*blk->data, new_size);
        }

        new_size = end_row - start_row + 1; // size of the data array being inserted.

        if (block_index < m_blocks.size() - 1)
        {
            // There is a block (or more) after the current block. Check the next block.
            block* blk_next = get_next_block_of_type(block_index, cat);
            if (blk_next)
            {
                // Prepend it to the next block.
                mdds_mtv_prepend_values(*blk_next->data, *it_begin, it_begin, it_end);
                blk_next->size += new_size;
                blk_next->position -= new_size;
                return get_iterator(block_index + 1);
            }

            // Next block has a different data type. Do the normal insertion.
            size_type position = detail::calc_next_block_position(*blk);
            m_blocks.emplace(m_blocks.begin() + block_index + 1, position, new_size);
            blk = &m_blocks[block_index + 1];
            blk->data = element_block_func::create_new_block(cat, 0);
            m_hdl_event.element_block_acquired(blk->data);
            mdds_mtv_assign_values(*blk->data, *it_begin, it_begin, it_end);
            return get_iterator(block_index + 1);
        }

        // Last block.
        assert(block_index == m_blocks.size() - 1);

        m_blocks.emplace_back(m_cur_size - new_size, new_size);
        blk = &m_blocks.back();
        blk->data = element_block_func::create_new_block(cat, 0);
        m_hdl_event.element_block_acquired(blk->data);
        mdds_mtv_assign_values(*blk->data, *it_begin, it_begin, it_end);
        return get_iterator(block_index + 1);
    }

    // new data array will be in the middle of the current block.
    assert(start_row_in_block < start_row && end_row < end_row_in_block);

    block& blk_new =
        set_new_block_to_middle(block_index, start_row - start_row_in_block, end_row - start_row + 1, true);

    blk_new.data = element_block_func::create_new_block(cat, 0);
    m_hdl_event.element_block_acquired(blk_new.data);
    mdds_mtv_assign_values(*blk_new.data, *it_begin, it_begin, it_end);

    return get_iterator(block_index + 1);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::
    set_cells_to_multi_blocks(
        size_type start_row, size_type end_row, size_type block_index1, size_type block_index2, const T& it_begin,
        const T& it_end)
{
    assert(block_index1 < block_index2);
    assert(it_begin != it_end);
    assert(!m_blocks.empty());

    block* blk1 = &m_blocks[block_index1];
    if (blk1->data)
    {
        return set_cells_to_multi_blocks_block1_non_empty(
            start_row, end_row, block_index1, block_index2, it_begin, it_end);
    }

    // Block 1 is empty.
    assert(!blk1->data);

    return set_cells_to_multi_blocks_block1_non_equal(start_row, end_row, block_index1, block_index2, it_begin, it_end);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::
    set_cells_to_multi_blocks_block1_non_equal(
        size_type start_row, size_type end_row, size_type block_index1, size_type block_index2, const T& it_begin,
        const T& it_end)
{
    element_category_type cat = mdds_mtv_get_element_type(*it_begin);
    block* blk1 = &m_blocks[block_index1];
    block* blk2 = &m_blocks[block_index2];
    size_type start_row_in_block1 = blk1->position;
    size_type start_row_in_block2 = blk2->position;
    size_type length = std::distance(it_begin, it_end);
    size_type offset = start_row - start_row_in_block1;
    size_type end_row_in_block2 = start_row_in_block2 + blk2->size - 1;

    // Initially set to erase blocks between block 1 and block 2 non-inclusive at either end.
    typename blocks_type::iterator it_erase_begin = m_blocks.begin() + block_index1 + 1;
    typename blocks_type::iterator it_erase_end = m_blocks.begin() + block_index2;

    // Create the new data block first.
    block data_blk(start_row, length);

    bool blk0_copied = false;
    if (offset == 0)
    {
        // Remove block 1.
        --it_erase_begin;

        // Check the type of the previous block (block 0) if exists.
        if (block_index1 > 0)
        {
            block* blk0 = &m_blocks[block_index1 - 1];
            if (blk0->data && cat == mdds::mtv::get_block_type(*blk0->data))
            {
                // Transfer the whole data from block 0 to data block.
                data_blk.data = blk0->data;
                blk0->data = nullptr;

                data_blk.size += blk0->size;
                data_blk.position = blk0->position;

                --it_erase_begin;
                blk0_copied = true;
            }
        }
    }
    else
    {
        // Shrink block 1 by the end.
        if (blk1->data)
        {
            size_type n = blk1->size - offset;
            element_block_func::overwrite_values(*blk1->data, offset, n);
            element_block_func::resize_block(*blk1->data, offset);
        }
        blk1->size = offset;
    }

    if (blk0_copied)
        mdds_mtv_append_values(*data_blk.data, *it_begin, it_begin, it_end);
    else
    {
        data_blk.data = element_block_func::create_new_block(cat, 0);
        m_hdl_event.element_block_acquired(data_blk.data);
        mdds_mtv_assign_values(*data_blk.data, *it_begin, it_begin, it_end);
    }

    if (end_row == end_row_in_block2)
    {
        // Remove block 2.
        ++it_erase_end;

        if (block_index2 + 1 < m_blocks.size())
        {
            // There is at least one block after block 2.
            block* blk3 = &m_blocks[block_index2 + 1];
            if (blk3->data && mdds::mtv::get_block_type(*blk3->data) == cat)
            {
                // Merge the whole block 3 with the new data. Remove block 3
                // afterward.  Resize block 3 to zero to prevent invalid free.
                element_block_func::append_values_from_block(*data_blk.data, *blk3->data);
                element_block_func::resize_block(*blk3->data, 0);
                data_blk.size += blk3->size;
                ++it_erase_end;
            }
        }
    }
    else
    {
        bool erase_upper = true;
        if (blk2->data)
        {
            element_category_type blk_cat2 = mdds::mtv::get_block_type(*blk2->data);
            if (blk_cat2 == cat)
            {
                // Merge the lower part of block 2 with the new data, and
                // erase block 2.  Resize block 2 to avoid invalid free on the
                // copied portion of the block.
                size_type copy_pos = end_row - start_row_in_block2 + 1;
                size_type size_to_copy = end_row_in_block2 - end_row;
                element_block_func::append_values_from_block(*data_blk.data, *blk2->data, copy_pos, size_to_copy);
                element_block_func::resize_block(*blk2->data, copy_pos);
                data_blk.size += size_to_copy;

                ++it_erase_end;
                erase_upper = false;
            }
        }

        if (erase_upper)
        {
            // Erase the upper part of block 2.
            size_type size_to_erase = end_row - start_row_in_block2 + 1;
            if (blk2->data)
            {
                element_block_func::overwrite_values(*blk2->data, 0, size_to_erase);
                element_block_func::erase(*blk2->data, 0, size_to_erase);
            }
            blk2->size -= size_to_erase;
            blk2->position += size_to_erase;
        }
    }

    size_type insert_pos = std::distance(m_blocks.begin(), it_erase_begin);

    // Remove the in-between blocks first.
    delete_element_blocks(it_erase_begin, it_erase_end);
    m_blocks.erase(it_erase_begin, it_erase_end);

    // Insert the new data block.
    m_blocks.insert(m_blocks.begin() + insert_pos, std::move(data_blk));

    return get_iterator(insert_pos);
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::
    set_cells_to_multi_blocks_block1_non_empty(
        size_type start_row, size_type end_row, size_type block_index1, size_type block_index2, const T& it_begin,
        const T& it_end)
{
    size_type start_row_in_block1 = m_blocks[block_index1].position;
    size_type start_row_in_block2 = m_blocks[block_index2].position;

    element_category_type cat = mdds_mtv_get_element_type(*it_begin);
    block* blk1 = &m_blocks[block_index1];
    assert(blk1->data);
    element_category_type blk_cat1 = mdds::mtv::get_block_type(*blk1->data);

    if (blk_cat1 == cat)
    {
        block* blk2 = &m_blocks[block_index2];
        size_type length = std::distance(it_begin, it_end);
        size_type offset = start_row - start_row_in_block1;
        size_type end_row_in_block2 = start_row_in_block2 + blk2->size - 1;

        // Initially set to erase blocks between block 1 and block 2 non-inclusive at either end.
        typename blocks_type::iterator it_erase_begin = m_blocks.begin() + block_index1 + 1;
        typename blocks_type::iterator it_erase_end = m_blocks.begin() + block_index2;

        // Extend the first block to store the new data set.

        // Shrink it first to remove the old values, then append new values.
        element_block_func::overwrite_values(*blk1->data, offset, blk1->size - offset);
        element_block_func::resize_block(*blk1->data, offset);
        mdds_mtv_append_values(*blk1->data, *it_begin, it_begin, it_end);
        blk1->size = offset + length;

        if (end_row == end_row_in_block2)
        {
            // Data overlaps the entire block 2. Erase it.
            ++it_erase_end;
        }
        else if (blk2->data)
        {
            element_category_type blk_cat2 = mdds::mtv::get_block_type(*blk2->data);
            if (blk_cat2 == cat)
            {
                // Copy the lower (non-overwritten) part of block 2 to block
                // 1, and remove the whole block 2. Resize block 2 to zero
                // first to prevent the transferred / overwritten cells from
                // being deleted on block deletion.
                size_type data_length = end_row_in_block2 - end_row;
                size_type begin_pos = end_row - start_row_in_block2 + 1;
                element_block_func::append_values_from_block(*blk1->data, *blk2->data, begin_pos, data_length);
                element_block_func::overwrite_values(*blk2->data, 0, begin_pos);
                element_block_func::resize_block(*blk2->data, 0);
                blk1->size += data_length;
                ++it_erase_end;
            }
            else
            {
                // Erase the upper part of block 2.
                size_type size_to_erase = end_row - start_row_in_block2 + 1;
                element_block_func::erase(*blk2->data, 0, size_to_erase);
                blk2->size -= size_to_erase;
                blk2->position += size_to_erase;
            }
        }
        else
        {
            // Last block is empty.
            size_type size_to_erase = end_row - start_row_in_block2 + 1;
            blk2->size -= size_to_erase;
            blk2->position += size_to_erase;
        }

        delete_element_blocks(it_erase_begin, it_erase_end);
        m_blocks.erase(it_erase_begin, it_erase_end);

        return get_iterator(block_index1);
    }

    // The first block type is different.
    assert(blk_cat1 != cat);

    return set_cells_to_multi_blocks_block1_non_equal(start_row, end_row, block_index1, block_index2, it_begin, it_end);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::size_type multi_type_vector<
    ElemBlockFunc, Trait>::merge_with_adjacent_blocks(size_type block_index)
{
    assert(!m_blocks.empty());
    assert(block_index < m_blocks.size());
    block* blk_prev = block_index > 0 ? &m_blocks[block_index - 1] : nullptr;

    if (!blk_prev)
    {
        // No previous block.
        merge_with_next_block(block_index);
        return 0;
    }

    size_type size_prev = blk_prev->size; // size of previous block.
    block* blk = &m_blocks[block_index];
    block* blk_next = block_index < (m_blocks.size() - 1) ? &m_blocks[block_index + 1] : nullptr;

    // Check the previous block.
    if (blk_prev->data)
    {
        // Previous block has data.
        element_category_type cat_prev = mtv::get_block_type(*blk_prev->data);
        if (!blk->data || cat_prev != mtv::get_block_type(*blk->data))
        {
            // Current block is empty or is of different type from the previous one.
            merge_with_next_block(block_index);
            return 0;
        }

        // Previous and current blocks are of the same type.
        if (blk_next && blk_next->data && cat_prev == get_block_type(*blk_next->data))
        {
            // Merge all three blocks.
            blk_prev->size += blk->size + blk_next->size;
            element_block_func::append_values_from_block(*blk_prev->data, *blk->data);
            element_block_func::append_values_from_block(*blk_prev->data, *blk_next->data);
            // Avoid overwriting the transferred elements.
            element_block_func::resize_block(*blk->data, 0);
            element_block_func::resize_block(*blk_next->data, 0);

            delete_element_block(*blk);
            delete_element_block(*blk_next);

            typename blocks_type::iterator it = m_blocks.begin();
            std::advance(it, block_index);
            typename blocks_type::iterator it_end = it;
            std::advance(it_end, 2);
            m_blocks.erase(it, it_end);
            return size_prev;
        }

        // Merge only the previous and current blocks.
        bool merged = merge_with_next_block(block_index - 1);
        if (!merged)
            assert(!"Blocks were not merged!");

        return size_prev;
    }

    assert(!blk_prev->data); // Previous block is empty.

    if (blk->data)
    {
        // Current block is not empty. Check with the next block.
        merge_with_next_block(block_index);
        return 0;
    }

    // Previous and current blocks are both empty.
    assert(!blk->data);

    if (blk_next && !blk_next->data)
    {
        // Next block is empty too. Merge all three.
        blk_prev->size += blk->size + blk_next->size;
        // No need to call delete_element_block() since we know both blocks are empty.

        typename blocks_type::iterator it = m_blocks.begin();
        std::advance(it, block_index);
        typename blocks_type::iterator it_end = it;
        std::advance(it_end, 2);
        m_blocks.erase(it, it_end);
        return size_prev;
    }

    // Next block is not empty, or does not exist. Merge the current block with the previous one.
    bool merged = merge_with_next_block(block_index - 1);
    if (!merged)
        assert(!"Blocks were not merged!");

    return size_prev;
}

template<typename ElemBlockFunc, typename Trait>
bool multi_type_vector<ElemBlockFunc, Trait>::merge_with_next_block(size_type block_index)
{
    assert(!m_blocks.empty());
    assert(block_index < m_blocks.size());

    if (block_index >= m_blocks.size() - 1)
        // No more block below this one.
        return false;

    // Block exists below.
    block* blk = &m_blocks[block_index];
    block* blk_next = &m_blocks[block_index + 1];
    if (!blk->data)
    {
        // Empty block. Merge only if the next block is also empty.
        if (blk_next->data)
            // Next block is not empty.
            return false;

        // Merge the two blocks.
        blk->size += blk_next->size;
        m_blocks.erase(m_blocks.begin() + block_index + 1);
        return true;
    }

    if (!blk_next->data)
        return false;

    if (mdds::mtv::get_block_type(*blk->data) != mdds::mtv::get_block_type(*blk_next->data))
        // Block types differ.  Don't merge.
        return false;

    // Merge it with the next block.
    element_block_func::append_values_from_block(*blk->data, *blk_next->data);
    element_block_func::resize_block(*blk_next->data, 0);
    blk->size += blk_next->size;
    delete_element_block(*blk_next);
    m_blocks.erase(m_blocks.begin() + block_index + 1);
    return true;
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
bool multi_type_vector<ElemBlockFunc, Trait>::append_to_prev_block(
    size_type block_index, element_category_type cat, size_type length, const T& it_begin, const T& it_end)
{
    block* blk_prev = get_previous_block_of_type(block_index, cat);
    if (!blk_prev)
        return false;

    // Append to the previous block.
    mdds_mtv_append_values(*blk_prev->data, *it_begin, it_begin, it_end);
    blk_prev->size += length;
    return true;
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::clear()
{
    delete_element_blocks(m_blocks.begin(), m_blocks.end());
    m_blocks.clear();
    m_cur_size = 0;
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::size_type multi_type_vector<ElemBlockFunc, Trait>::size() const
{
    return m_cur_size;
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::size_type multi_type_vector<ElemBlockFunc, Trait>::block_size() const
{
    return m_blocks.size();
}

template<typename ElemBlockFunc, typename Trait>
bool multi_type_vector<ElemBlockFunc, Trait>::empty() const
{
    return m_blocks.empty();
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::resize(size_type new_size)
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

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::resize_impl(size_type new_size)
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
    if (block_index == m_blocks.size())
        mdds::detail::mtv::throw_block_position_not_found(
            "multi_type_vector::resize", __LINE__, new_end_row, block_size(), size());

    block* blk = &m_blocks[block_index];
    size_type start_row_in_block = blk->position;
    size_type end_row_in_block = start_row_in_block + blk->size - 1;

    if (new_end_row < end_row_in_block)
    {
        // Shrink the size of the current block.
        size_type new_block_size = new_end_row - start_row_in_block + 1;
        if (blk->data)
        {
            element_block_func::overwrite_values(*blk->data, new_end_row + 1, end_row_in_block - new_end_row);
            element_block_func::resize_block(*blk->data, new_block_size);
        }
        blk->size = new_block_size;
    }

    // Remove all blocks that are below this one.
    typename blocks_type::iterator it = m_blocks.begin() + block_index + 1;
    delete_element_blocks(it, m_blocks.end());
    m_blocks.erase(it, m_blocks.end());
    m_cur_size = new_size;
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::swap(multi_type_vector& other)
{
    std::swap(m_hdl_event, other.m_hdl_event);
    std::swap(m_cur_size, other.m_cur_size);
    m_blocks.swap(other.m_blocks);
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::swap(
    size_type start_pos, size_type end_pos, multi_type_vector& other, size_type other_pos)
{
    if (start_pos > end_pos)
        throw std::out_of_range("multi_type_vector::swap: start position is larger than the end position!");

    size_type other_end_pos = other_pos + end_pos - start_pos;

    if (end_pos >= m_cur_size || other_end_pos >= other.m_cur_size)
        throw std::out_of_range("multi_type_vector::swap: end position is out of bound!");

    size_type block_index1 = get_block_position(start_pos);
    if (block_index1 == m_blocks.size())
        throw std::out_of_range("multi_type_vector::swap: start block position in source not found!");

    size_type block_index2 = get_block_position(end_pos, block_index1);
    if (block_index2 == m_blocks.size())
        throw std::out_of_range("multi_type_vector::swap: end block position in source not found!");

    size_type dest_block_index1 = other.get_block_position(other_pos);
    if (dest_block_index1 == other.m_blocks.size())
        throw std::out_of_range("multi_type_vector::swap: start block position in destination not found!");

    size_type dest_block_index2 = other.get_block_position(other_end_pos, dest_block_index1);
    if (dest_block_index2 == other.m_blocks.size())
        throw std::out_of_range("multi_type_vector::swap: end block position in destination not found!");

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_prev_block, os_prev_block_other;
    dump_blocks(os_prev_block);
    other.dump_blocks(os_prev_block_other);
#endif

    swap_impl(other, start_pos, end_pos, other_pos, block_index1, block_index2, dest_block_index1, dest_block_index2);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_block, os_block_other;
    dump_blocks(os_block);
    other.dump_blocks(os_block_other);

    try
    {
        check_block_integrity();
        other.check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        os << std::endl
           << "block integrity check failed in swap (start_pos=" << start_pos << "; end_pos=" << end_pos
           << "; other_pos=" << other_pos << ")" << std::endl;
        os << std::endl << "previous block state (source):" << std::endl;
        os << os_prev_block.str();
        os << std::endl << "previous block state (destination):" << std::endl;
        os << os_prev_block_other.str();
        os << std::endl << "altered block state (source):" << std::endl;
        os << os_block.str();
        os << std::endl << "altered block state (destination):" << std::endl;
        os << os_block_other.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::shrink_to_fit()
{
    typename blocks_type::iterator it = m_blocks.begin(), it_end = m_blocks.end();
    for (; it != it_end; ++it)
    {
        block* blk = &(*it);
        assert(blk);
        if (blk->data)
            element_block_func::shrink_to_fit(*blk->data);
    }
}

template<typename ElemBlockFunc, typename Trait>
bool multi_type_vector<ElemBlockFunc, Trait>::operator==(const multi_type_vector& other) const
{
    if (this == &other)
        // Comparing to self is always equal.
        return true;

    if (m_blocks.size() != other.m_blocks.size())
        // Block sizes differ.
        return false;

    if (m_cur_size != other.m_cur_size)
        // Row sizes differ.
        return false;

    typename blocks_type::const_iterator it = m_blocks.begin(), it_end = m_blocks.end();
    typename blocks_type::const_iterator it2 = other.m_blocks.begin();
    for (; it != it_end; ++it, ++it2)
    {
        const block* blk1 = &(*it);
        const block* blk2 = &(*it2);

        if (blk1->size != blk2->size)
            // Block sizes differ.
            return false;

        if (blk1->data)
        {
            if (!blk2->data)
                // left is non-empty while right is empty.
                return false;
        }
        else
        {
            if (blk2->data)
                // left is empty while right is non-empty.
                return false;
        }

        if (!blk1->data)
        {
            assert(!blk2->data);
            continue;
        }

        assert(blk1->data && blk2->data);
        if (!element_block_func::equal_block(*blk1->data, *blk2->data))
            return false;
    }

    return true;
}

template<typename ElemBlockFunc, typename Trait>
bool multi_type_vector<ElemBlockFunc, Trait>::operator!=(const multi_type_vector& other) const
{
    return !operator==(other);
}

template<typename ElemBlockFunc, typename Trait>
multi_type_vector<ElemBlockFunc, Trait>& multi_type_vector<ElemBlockFunc, Trait>::operator=(
    const multi_type_vector& other)
{
    multi_type_vector assigned(other);
    swap(assigned);
    return *this;
}

template<typename ElemBlockFunc, typename Trait>
multi_type_vector<ElemBlockFunc, Trait>& multi_type_vector<ElemBlockFunc, Trait>::operator=(multi_type_vector&& other)
{
    multi_type_vector assigned(std::move(other));
    swap(assigned);
    return *this;
}

template<typename ElemBlockFunc, typename Trait>
template<typename T>
mtv::element_t multi_type_vector<ElemBlockFunc, Trait>::get_element_type(const T& elem)
{
    return mdds_mtv_get_element_type(elem);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<
    ElemBlockFunc, Trait>::set_whole_block_empty(size_type block_index, bool overwrite)
{
    block* blk = &m_blocks[block_index];
    if (!overwrite)
        // Resize block to 0 before deleting, to prevent its elements from getting deleted.
        element_block_func::resize_block(*blk->data, 0);

    delete_element_block(*blk);

    block* blk_prev = get_previous_block_of_type(block_index, mtv::element_type_empty);
    block* blk_next = get_next_block_of_type(block_index, mtv::element_type_empty);

    // Merge with adjacent block(s) if necessary.
    if (blk_prev)
    {
        assert(!blk_prev->data);

        if (blk_next)
        {
            // Both preceding and next blocks are empty.
            assert(!blk_next->data);

            blk_prev->size += blk->size + blk_next->size;

            // no need to call delete_block() on the current and next blocks
            // since they are empty.

            typename blocks_type::iterator it = m_blocks.begin();
            std::advance(it, block_index);
            typename blocks_type::iterator it_end = it;
            std::advance(it_end, 2);
            m_blocks.erase(it, it_end);

            return get_iterator(block_index - 1);
        }

        // Only the preceding block is empty. Merge the current block with the previous.
        blk_prev->size += blk->size;
        typename blocks_type::iterator it = m_blocks.begin();
        std::advance(it, block_index);
        m_blocks.erase(it);

        return get_iterator(block_index - 1);
    }
    else if (blk_next)
    {
        assert(!blk_next->data);

        // Only the next block is empty. Merge the next block with the current.
        blk->size += blk_next->size;
        typename blocks_type::iterator it = m_blocks.begin();
        std::advance(it, block_index + 1);
        m_blocks.erase(it);

        return get_iterator(block_index);
    }

    return get_iterator(block_index);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::
    set_empty_in_single_block(size_type start_row, size_type end_row, size_type block_index, bool overwrite)
{
    // Range is within a single block.
    block* blk = &m_blocks[block_index];
    if (!blk->data)
        // This block is already empty.  Do nothing.
        return get_iterator(block_index);

    size_type start_row_in_block = blk->position;
    assert(start_row_in_block + blk->size >= 1);
    size_type end_row_in_block = start_row_in_block + blk->size - 1;
    size_type empty_block_size = end_row - start_row + 1;

    if (start_row == start_row_in_block)
    {
        // start row coincides with the start of a block.

        if (end_row == end_row_in_block)
            return set_whole_block_empty(block_index, overwrite);

        // Set the upper part of the block empty.
        if (overwrite)
            element_block_func::overwrite_values(*blk->data, 0, empty_block_size);
        element_block_func::erase(*blk->data, 0, empty_block_size);
        blk->size -= empty_block_size;

        // Check if the preceding block (if exists) is also empty.
        block* blk_prev = get_previous_block_of_type(block_index, mtv::element_type_empty);
        if (blk_prev)
        {
            // Extend the previous empty block.
            blk_prev->size += empty_block_size;
            blk->position += empty_block_size;
            return get_iterator(block_index - 1);
        }

        // Insert a new empty block before the current one.
        size_type block_position = blk->position;
        blk->position += empty_block_size;
        m_blocks.emplace(m_blocks.begin() + block_index, block_position, empty_block_size);
        return get_iterator(block_index);
    }

    if (end_row == end_row_in_block)
    {
        // end row equals the end of a block.
        assert(start_row > start_row_in_block);

        // Set the lower part of the block empty.
        size_type start_pos = start_row - start_row_in_block;
        if (overwrite)
            element_block_func::overwrite_values(*blk->data, start_pos, empty_block_size);
        element_block_func::erase(*blk->data, start_pos, empty_block_size);
        blk->size -= empty_block_size;

        // Check if the following block (if exists) is also empty.
        block* blk_next = get_next_block_of_type(block_index, mtv::element_type_empty);
        if (blk_next)
        {
            // Extend the next empty block to cover the new empty segment.
            blk_next->size += empty_block_size;
            blk_next->position = start_row;
        }
        else
        {
            // Insert a new empty block after the current one.
            m_blocks.emplace(m_blocks.begin() + block_index + 1, start_row, empty_block_size);
        }

        return get_iterator(block_index + 1);
    }

    // Empty the middle part of a block.
    assert(end_row_in_block - end_row > 0);
    set_new_block_to_middle(block_index, start_row - start_row_in_block, empty_block_size, overwrite);

    return get_iterator(block_index + 1);
}

template<typename ElemBlockFunc, typename Trait>
typename multi_type_vector<ElemBlockFunc, Trait>::iterator multi_type_vector<ElemBlockFunc, Trait>::
    set_empty_in_multi_blocks(
        size_type start_row, size_type end_row, size_type block_index1, size_type block_index2, bool overwrite)
{
    assert(block_index1 < block_index2);
    size_type start_row_in_block1 = m_blocks[block_index1].position;
    size_type start_row_in_block2 = m_blocks[block_index2].position;

    {
        // Empty the lower part of the first block.
        block* blk = &m_blocks[block_index1];
        if (blk->data)
        {
            if (start_row_in_block1 == start_row)
            {
                // Empty the whole block.

                // Check if the previos block (if exists) is also empty.
                block* blk_prev = nullptr;
                if (block_index1 > 0)
                {
                    blk_prev = &m_blocks[block_index1 - 1];
                    if (blk_prev->data)
                        // Not empty.  Ignore it.
                        blk_prev = nullptr;
                }

                if (blk_prev)
                {
                    // Previous block is empty.  Move the start row to the
                    // first row of the previous block, and make the previous
                    // block 'block 1'.
                    start_row -= blk_prev->size;
                    --block_index1;
                }
                else
                {
                    // Make block 1 empty.
                    if (!overwrite)
                        element_block_func::resize_block(*blk->data, 0);

                    delete_element_block(*blk);
                }
            }
            else
            {
                // Empty the lower part.
                size_type new_size = start_row - start_row_in_block1;
                if (overwrite)
                    element_block_func::overwrite_values(*blk->data, new_size, blk->size - new_size);

                element_block_func::resize_block(*blk->data, new_size);
                blk->size = new_size;
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
        block* blk = &m_blocks[block_index2];
        size_type last_row_in_block = start_row_in_block2 + blk->size - 1;
        if (blk->data)
        {
            if (last_row_in_block == end_row)
            {
                // Delete the whole block.
                ++end_block_to_erase;

                // Check if the following block (if exists) is also empty.
                block* blk_next = nullptr;
                if (block_index2 + 1 < m_blocks.size())
                {
                    blk_next = &m_blocks[block_index2 + 1];
                    if (blk_next->data)
                        // Not empty.  Ignore it.
                        blk_next = nullptr;
                }

                if (blk_next)
                {
                    // The following block is also empty.
                    end_row += blk_next->size;
                    ++end_block_to_erase;
                }
            }
            else
            {
                // Empty the upper part.
                size_type size_to_erase = end_row - start_row_in_block2 + 1;
                if (overwrite)
                    element_block_func::overwrite_values(*blk->data, 0, size_to_erase);

                element_block_func::erase(*blk->data, 0, size_to_erase);
                blk->size -= size_to_erase;
                blk->position = start_row_in_block2 + size_to_erase;
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
            block& blk = m_blocks[i];
            if (!overwrite && blk.data)
                element_block_func::resize_block(*blk.data, 0);

            delete_element_block(blk);
        }

        typename blocks_type::iterator it = m_blocks.begin() + block_index1 + 1;
        typename blocks_type::iterator it_end = m_blocks.begin() + end_block_to_erase;
        m_blocks.erase(it, it_end);
    }

    block* blk = &m_blocks[block_index1];
    size_type empty_block_size = end_row - start_row + 1;
    if (blk->data)
    {
        // Insert a new empty block after the first block.
        m_blocks.emplace(m_blocks.begin() + block_index1 + 1, start_row, empty_block_size);
        return get_iterator(block_index1 + 1);
    }

    // Current block is already empty. Just extend its size.
    blk->size = empty_block_size;
    blk->position = start_row;
    return get_iterator(block_index1);
}

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::dump_blocks(std::ostream& os) const
{
    os << "--- blocks" << endl;
    for (size_type i = 0, n = m_blocks.size(); i < n; ++i)
    {
        const block* blk = &m_blocks[i];
        element_category_type cat = mtv::element_type_empty;
        if (blk->data)
            cat = mtv::get_block_type(*blk->data);
        os << "  block " << i << ": position=" << blk->position << " size=" << blk->size << " type=" << cat << endl;
    }
}

template<typename ElemBlockFunc, typename Trait>
void multi_type_vector<ElemBlockFunc, Trait>::check_block_integrity() const
{
    if (m_blocks.empty())
        // Nothing to check.
        return;

    if (m_blocks.size() == 1 && m_blocks[0].size == 0)
        throw mdds::integrity_error("block should never be zero sized!");

    const block* blk_prev = m_blocks.data();
    if (blk_prev->size == 0)
        throw mdds::integrity_error("block should never be zero sized!");

    if (blk_prev->position != 0)
    {
        std::ostringstream os;
        os << "position of the first block should be zero!" << std::endl;

        dump_blocks(os);
        mdds::integrity_error(os.str());
    }

    element_category_type cat_prev = mtv::element_type_empty;
    if (blk_prev->data)
        cat_prev = mtv::get_block_type(*blk_prev->data);

    size_type cur_position = blk_prev->size;
    size_type total_size = blk_prev->size;

    for (size_type i = 1, n = m_blocks.size(); i < n; ++i)
    {
        const block* blk = &m_blocks[i];
        if (blk->size == 0)
            throw mdds::integrity_error("block should never be zero sized!");

        if (blk->position != cur_position)
        {
            std::ostringstream os;
            os << "position of the current block is wrong! (expected=" << cur_position << "; actual=" << blk->position
               << ")" << std::endl;

            dump_blocks(os);
            throw mdds::integrity_error(os.str());
        }

        element_category_type cat = mtv::element_type_empty;
        if (blk->data)
        {
            cat = mtv::get_block_type(*blk->data);

            if (element_block_func::size(*blk->data) != blk->size)
                throw mdds::integrity_error("block size cache and the actual element block size differ!");
        }

        if (cat_prev == cat)
        {
            std::ostringstream os;
            os << "Two adjacent blocks should never be of the same type." << std::endl;
            dump_blocks(os);
            mdds::integrity_error(os.str());
        }

        blk_prev = blk;
        cat_prev = cat;

        total_size += blk->size;
        cur_position += blk->size;
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

}}} // namespace mdds::mtv::aos

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
