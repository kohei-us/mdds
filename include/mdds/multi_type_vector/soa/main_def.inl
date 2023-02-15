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

#include "../env.hpp"
#include "../util.hpp"
#if MDDS_MTV_USE_STANDARD_ELEMENT_BLOCKS
#include "../standard_element_blocks.hpp"
#endif

namespace mdds { namespace mtv { namespace soa {

namespace detail {

template<typename SizeT, typename VecT>
void erase(VecT& arr, SizeT index, SizeT size)
{
    auto it = arr.begin() + index;
    arr.erase(it, it + size);
}

} // namespace detail

template<typename Traits>
multi_type_vector<Traits>::blocks_type::blocks_type()
{}

template<typename Traits>
multi_type_vector<Traits>::blocks_type::blocks_type(const blocks_type& other)
    : positions(other.positions), sizes(other.sizes), element_blocks(other.element_blocks)
{
    for (element_block_type*& data : element_blocks)
    {
        if (data)
            data = block_funcs::clone_block(*data);
    }
}

template<typename Traits>
multi_type_vector<Traits>::blocks_type::blocks_type(blocks_type&& other)
    : positions(std::move(other.positions)), sizes(std::move(other.sizes)),
      element_blocks(std::move(other.element_blocks))
{}

template<typename Traits>
void multi_type_vector<Traits>::blocks_type::erase(size_type index)
{
    positions.erase(positions.begin() + index);
    sizes.erase(sizes.begin() + index);
    element_blocks.erase(element_blocks.begin() + index);
}

template<typename Traits>
void multi_type_vector<Traits>::blocks_type::erase(size_type index, size_type size)
{
    detail::erase(positions, index, size);
    detail::erase(sizes, index, size);
    detail::erase(element_blocks, index, size);
}

template<typename Traits>
void multi_type_vector<Traits>::blocks_type::insert(size_type index, size_type size)
{
    positions.insert(positions.begin() + index, size, 0);
    sizes.insert(sizes.begin() + index, size, 0);
    element_blocks.insert(element_blocks.begin() + index, size, nullptr);
}

template<typename Traits>
void multi_type_vector<Traits>::blocks_type::insert(
    size_type index, size_type pos, size_type size, element_block_type* data)
{
    positions.insert(positions.begin() + index, pos);
    sizes.insert(sizes.begin() + index, size);
    element_blocks.insert(element_blocks.begin() + index, data);
}

template<typename Traits>
void multi_type_vector<Traits>::blocks_type::insert(size_type index, const blocks_type& new_blocks)
{
    positions.insert(positions.begin() + index, new_blocks.positions.begin(), new_blocks.positions.end());
    sizes.insert(sizes.begin() + index, new_blocks.sizes.begin(), new_blocks.sizes.end());
    element_blocks.insert(
        element_blocks.begin() + index, new_blocks.element_blocks.begin(), new_blocks.element_blocks.end());
}

template<typename Traits>
void multi_type_vector<Traits>::blocks_type::calc_block_position(size_type index)
{
    if (index == 0)
    {
        positions[index] = 0;
        return;
    }

    assert(index < positions.size());
    positions[index] = positions[index - 1] + sizes[index - 1];
}

template<typename Traits>
typename multi_type_vector<Traits>::size_type multi_type_vector<Traits>::blocks_type::calc_next_block_position(
    size_type index)
{
    return positions[index] + sizes[index];
}

template<typename Traits>
void multi_type_vector<Traits>::blocks_type::swap(size_type index1, size_type index2)
{
    std::swap(positions[index1], positions[index2]);
    std::swap(sizes[index1], sizes[index2]);
    std::swap(element_blocks[index1], element_blocks[index2]);
}

template<typename Traits>
void multi_type_vector<Traits>::blocks_type::swap(blocks_type& other)
{
    positions.swap(other.positions);
    sizes.swap(other.sizes);
    element_blocks.swap(other.element_blocks);
}

template<typename Traits>
void multi_type_vector<Traits>::blocks_type::reserve(size_type n)
{
    positions.reserve(n);
    sizes.reserve(n);
    element_blocks.reserve(n);
}

template<typename Traits>
bool multi_type_vector<Traits>::blocks_type::equals(const blocks_type& other) const
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
        if (!block_funcs::equal_block(*data1, *data2))
            return false;
    }

    return true;
}

template<typename Traits>
void multi_type_vector<Traits>::blocks_type::clear()
{
    positions.clear();
    sizes.clear();
    element_blocks.clear();
}

template<typename Traits>
void multi_type_vector<Traits>::blocks_type::check_integrity() const
{
    if (positions.size() != sizes.size())
        throw mdds::integrity_error("position and size arrays are of different sizes!");

    if (positions.size() != element_blocks.size())
        throw mdds::integrity_error("position and element-block arrays are of different sizes!");
}

template<typename Traits>
typename multi_type_vector<Traits>::position_type multi_type_vector<Traits>::next_position(const position_type& pos)
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

template<typename Traits>
typename multi_type_vector<Traits>::position_type multi_type_vector<Traits>::advance_position(
    const position_type& pos, int steps)
{
    return mdds::mtv::detail::advance_position<position_type>(pos, steps);
}

template<typename Traits>
typename multi_type_vector<Traits>::const_position_type multi_type_vector<Traits>::next_position(
    const const_position_type& pos)
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

template<typename Traits>
typename multi_type_vector<Traits>::const_position_type multi_type_vector<Traits>::advance_position(
    const const_position_type& pos, int steps)
{
    return mdds::mtv::detail::advance_position<const_position_type>(pos, steps);
}

template<typename Traits>
typename multi_type_vector<Traits>::size_type multi_type_vector<Traits>::logical_position(
    const const_position_type& pos)
{
    return pos.first->position + pos.second;
}

template<typename Traits>
template<typename _Blk>
typename _Blk::value_type multi_type_vector<Traits>::get(const const_position_type& pos)
{
    return mdds::mtv::detail::get_block_element_at<_Blk>(*pos.first->data, pos.second);
}

template<typename Traits>
typename multi_type_vector<Traits>::event_func& multi_type_vector<Traits>::event_handler()
{
    MDDS_MTV_TRACE(accessor);

    return m_hdl_event;
}

template<typename Traits>
const typename multi_type_vector<Traits>::event_func& multi_type_vector<Traits>::event_handler() const
{
    MDDS_MTV_TRACE(accessor);

    return m_hdl_event;
}

template<typename Traits>
multi_type_vector<Traits>::multi_type_vector() : m_cur_size(0)
{
    MDDS_MTV_TRACE(constructor);
}

template<typename Traits>
multi_type_vector<Traits>::multi_type_vector(const event_func& hdl) : m_hdl_event(hdl), m_cur_size(0)
{
    MDDS_MTV_TRACE_ARGS(constructor, "event_func=?");
}

template<typename Traits>
multi_type_vector<Traits>::multi_type_vector(event_func&& hdl) : m_hdl_event(std::move(hdl)), m_cur_size(0)
{
    MDDS_MTV_TRACE_ARGS(constructor, "event_func=? (move)");
}

template<typename Traits>
multi_type_vector<Traits>::multi_type_vector(size_type init_size) : m_cur_size(init_size)
{
    MDDS_MTV_TRACE_ARGS(constructor, "init_size=" << init_size);

    if (!init_size)
        return;

    // Initialize with an empty block that spans from 0 to max.
    m_block_store.positions.emplace_back(0);
    m_block_store.sizes.emplace_back(init_size);
    m_block_store.element_blocks.emplace_back(nullptr);
}

template<typename Traits>
template<typename T>
multi_type_vector<Traits>::multi_type_vector(size_type init_size, const T& value) : m_cur_size(init_size)
{
    MDDS_MTV_TRACE_ARGS(
        constructor, "init_size=" << init_size << "; value=? (type=" << mdds_mtv_get_element_type(value) << ")");

    if (!init_size)
        return;

    element_block_type* data = mdds_mtv_create_new_block(init_size, value);
    m_hdl_event.element_block_acquired(data);
    m_block_store.positions.emplace_back(0);
    m_block_store.sizes.emplace_back(init_size);
    m_block_store.element_blocks.emplace_back(data);
}

template<typename Traits>
template<typename T>
multi_type_vector<Traits>::multi_type_vector(size_type init_size, const T& it_begin, const T& it_end)
    : m_cur_size(init_size)
{
    MDDS_MTV_TRACE_ARGS(
        constructor,
        "init_size=" << init_size << "; it_begin=?; it_end=? (length=" << std::distance(it_begin, it_end) << ")");

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

template<typename Traits>
multi_type_vector<Traits>::multi_type_vector(const multi_type_vector& other)
    : m_hdl_event(other.m_hdl_event), m_block_store(other.m_block_store), m_cur_size(other.m_cur_size)
{
    MDDS_MTV_TRACE_ARGS(constructor, "other=? (copy)");

    for (const element_block_type* data : m_block_store.element_blocks)
    {
        if (data)
            m_hdl_event.element_block_acquired(data);
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

template<typename Traits>
multi_type_vector<Traits>::multi_type_vector(multi_type_vector&& other)
    : m_hdl_event(std::move(other.m_hdl_event)), m_block_store(std::move(other.m_block_store)),
      m_cur_size(other.m_cur_size)
{
    MDDS_MTV_TRACE_ARGS(constructor, "other=? (move)");

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

template<typename Traits>
multi_type_vector<Traits>::~multi_type_vector()
{
    MDDS_MTV_TRACE(destructor);

    delete_element_blocks(0, m_block_store.positions.size());
}

template<typename Traits>
void multi_type_vector<Traits>::delete_element_block(size_type block_index)
{
    element_block_type* data = m_block_store.element_blocks[block_index];
    if (!data)
        // This block is empty.
        return;

    m_hdl_event.element_block_released(data);
    block_funcs::delete_block(data);
    m_block_store.element_blocks[block_index] = nullptr;
}

template<typename Traits>
void multi_type_vector<Traits>::delete_element_blocks(size_type start, size_type end)
{
    for (size_type i = start; i < end; ++i)
        delete_element_block(i);
}

template<typename Traits>
template<typename T>
void multi_type_vector<Traits>::get_impl(size_type pos, T& value) const
{
    size_type block_index = get_block_position(pos);
    if (block_index == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
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

template<typename Traits>
typename multi_type_vector<Traits>::position_type multi_type_vector<Traits>::position(size_type pos)
{
    MDDS_MTV_TRACE_ARGS(accessor, "pos=" << pos);

    if (pos == m_cur_size)
    {
        // This is a valid end position.  Create a valid position object that
        // represents a valid end position.
        return position_type(end(), 0);
    }

    size_type block_index = get_block_position(pos);
    if (block_index == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
            "multi_type_vector::position", __LINE__, pos, block_size(), size());

    size_type start_pos = m_block_store.positions[block_index];

    iterator it = get_iterator(block_index);
    return position_type(it, pos - start_pos);
}

template<typename Traits>
typename multi_type_vector<Traits>::position_type multi_type_vector<Traits>::position(
    const iterator& pos_hint, size_type pos)
{
    MDDS_MTV_TRACE_ARGS(accessor_with_pos_hint, "pos_hint=" << pos_hint << "; pos=" << pos);

    if (pos == m_cur_size)
    {
        // This is a valid end position.  Create a valid position object that
        // represents a valid end position.
        return position_type(end(), 0);
    }

    size_type block_index = get_block_position(pos_hint->__private_data, pos);
    if (block_index == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
            "multi_type_vector::position", __LINE__, pos, block_size(), size());

    iterator it = get_iterator(block_index);
    size_type start_pos = m_block_store.positions[block_index];
    return position_type(it, pos - start_pos);
}

template<typename Traits>
typename multi_type_vector<Traits>::const_position_type multi_type_vector<Traits>::position(size_type pos) const
{
    MDDS_MTV_TRACE_ARGS(accessor, "pos=" << pos);

    if (pos == m_cur_size)
    {
        // This is a valid end position.  Create a valid position object that
        // represents a valid end position.
        return const_position_type(cend(), 0);
    }

    size_type block_index = get_block_position(pos);
    if (block_index == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
            "multi_type_vector::position", __LINE__, pos, block_size(), size());

    size_type start_pos = m_block_store.positions[block_index];

    const_iterator it = get_const_iterator(block_index);
    return const_position_type(it, pos - start_pos);
}

template<typename Traits>
typename multi_type_vector<Traits>::const_position_type multi_type_vector<Traits>::position(
    const const_iterator& pos_hint, size_type pos) const
{
    MDDS_MTV_TRACE_ARGS(accessor_with_pos_hint, "pos_hint=" << pos_hint << "; pos=" << pos);

    if (pos == m_cur_size)
    {
        // This is a valid end position.  Create a valid position object that
        // represents a valid end position.
        return const_position_type(cend(), 0);
    }

    size_type block_index = get_block_position(pos_hint->__private_data, pos);
    if (block_index == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
            "multi_type_vector::position", __LINE__, pos, block_size(), size());

    const_iterator it = get_const_iterator(block_index);
    size_type start_pos = m_block_store.positions[block_index];
    return const_position_type(it, pos - start_pos);
}

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::transfer(
    size_type start_pos, size_type end_pos, multi_type_vector& dest, size_type dest_pos)
{
    MDDS_MTV_TRACE_ARGS(
        mutator, "start_pos=" << start_pos << "; end_pos=" << end_pos << "; dest=?; dest_pos=" << dest_pos);

    if (&dest == this)
        throw invalid_arg_error("You cannot transfer between the same container.");

    size_type block_index1 = get_block_position(start_pos);
    if (block_index1 == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
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

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::transfer(
    const iterator& pos_hint, size_type start_pos, size_type end_pos, multi_type_vector& dest, size_type dest_pos)
{
    MDDS_MTV_TRACE_ARGS(
        mutator_with_pos_hint, "pos_hint=" << pos_hint << "; start_pos=" << start_pos << "; end_pos=" << end_pos
                                           << "; dest=?; dest_pos=" << dest_pos);

    if (&dest == this)
        throw invalid_arg_error("You cannot transfer between the same container.");

    size_type block_index1 = get_block_position(pos_hint->__private_data, start_pos);
    if (block_index1 == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
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

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set(size_type pos, const T& value)
{
    MDDS_MTV_TRACE_ARGS(mutator, "pos=" << pos << "; value=? (type=" << mdds_mtv_get_element_type(value) << ")");

    size_type block_index = get_block_position(pos);
    if (block_index == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
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

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set(
    const iterator& pos_hint, size_type pos, const T& value)
{
    MDDS_MTV_TRACE_ARGS(
        mutator_with_pos_hint,
        "pos_hint=" << pos_hint << "; pos=" << pos << "; value=? (type=" << mdds_mtv_get_element_type(value) << ")");

    size_type block_index = get_block_position(pos_hint->__private_data, pos);
    if (block_index == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
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

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set(
    size_type pos, const T& it_begin, const T& it_end)
{
    MDDS_MTV_TRACE_ARGS(
        mutator, "pos=" << pos << "; it_begin=?; it_end=? (length=" << std::distance(it_begin, it_end) << ")");

    auto res = mdds::mtv::detail::calc_input_end_position(it_begin, it_end, pos, m_cur_size);

    if (!res.second)
        return end();

    size_type end_pos = res.first;
    size_type block_index1 = get_block_position(pos);

    if (block_index1 == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
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

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set(
    const iterator& pos_hint, size_type pos, const T& it_begin, const T& it_end)
{
    MDDS_MTV_TRACE_ARGS(
        mutator_with_pos_hint, "pos_hint=" << pos_hint << "; pos=" << pos << "; it_begin=?; it_end=? (length="
                                           << std::distance(it_begin, it_end) << ")");

    auto res = mdds::mtv::detail::calc_input_end_position(it_begin, it_end, pos, m_cur_size);
    if (!res.second)
        return end();

    size_type end_pos = res.first;
    size_type block_index1 = get_block_position(pos_hint->__private_data, pos);

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

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::push_back(const T& value)
{
    MDDS_MTV_TRACE_ARGS(mutator, "value=? (type=" << mdds_mtv_get_element_type(value) << ")");

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

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::push_back_empty()
{
    MDDS_MTV_TRACE(mutator);

    size_type block_index = m_block_store.positions.size();

    if (!append_empty(1))
    {
        // Last empty block has been extended.
        --block_index;
    }

    // Get the iterator of the last block.
    return get_iterator(block_index);
}

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::insert(
    size_type pos, const T& it_begin, const T& it_end)
{
    MDDS_MTV_TRACE_ARGS(
        mutator, "pos=" << pos << "it_begin=?; it_end=? (length=" << std::distance(it_begin, it_end) << ")");

    size_type block_index = get_block_position(pos);
    if (block_index == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
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

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::insert(
    const iterator& pos_hint, size_type pos, const T& it_begin, const T& it_end)
{
    MDDS_MTV_TRACE_ARGS(
        mutator_with_pos_hint, "pos_hint=" << pos_hint << "; pos=" << pos << "; it_begin=?; it_end=? (length="
                                           << std::distance(it_begin, it_end) << ")");

    size_type block_index = get_block_position(pos_hint->__private_data, pos);
    if (block_index == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
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

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::push_back_impl(const T& value)
{
    element_category_type cat = mdds_mtv_get_element_type(value);
    element_block_type* last_data =
        m_block_store.element_blocks.empty() ? nullptr : m_block_store.element_blocks.back();

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

template<typename Traits>
mtv::element_t multi_type_vector<Traits>::get_type(size_type pos) const
{
    MDDS_MTV_TRACE_ARGS(accessor, "pos=" << pos);

    size_type block_index = get_block_position(pos);
    if (block_index == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
            "multi_type_vector::get_type", __LINE__, pos, block_size(), size());

    element_block_type* blk_data = m_block_store.element_blocks[block_index];
    if (!blk_data)
        return mtv::element_type_empty;

    return mtv::get_block_type(*blk_data);
}

template<typename Traits>
bool multi_type_vector<Traits>::is_empty(size_type pos) const
{
    MDDS_MTV_TRACE_ARGS(accessor, "pos=" << pos);

    size_type block_index = get_block_position(pos);
    if (block_index == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
            "multi_type_vector::is_empty", __LINE__, pos, block_size(), size());

    return m_block_store.element_blocks[block_index] == nullptr;
}

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set_empty(
    size_type start_pos, size_type end_pos)
{
    MDDS_MTV_TRACE_ARGS(mutator, "start_pos=" << start_pos << "; end_pos=" << end_pos);

    size_type block_index1 = get_block_position(start_pos);
    if (block_index1 == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
            "multi_type_vector::set_empty", __LINE__, start_pos, block_size(), size());

    return set_empty_impl(start_pos, end_pos, block_index1, true);
}

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set_empty(
    const iterator& pos_hint, size_type start_pos, size_type end_pos)
{
    MDDS_MTV_TRACE_ARGS(
        mutator_with_pos_hint, "pos_hint=" << pos_hint << "; start_pos=" << start_pos << "; end_pos=" << end_pos);

    size_type block_index1 = get_block_position(pos_hint->__private_data, start_pos);
    if (block_index1 == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
            "multi_type_vector::set_empty", __LINE__, start_pos, block_size(), size());

    return set_empty_impl(start_pos, end_pos, block_index1, true);
}

template<typename Traits>
void multi_type_vector<Traits>::erase(size_type start_pos, size_type end_pos)
{
    MDDS_MTV_TRACE_ARGS(mutator, "start_pos=" << start_pos << "; end_pos=" << end_pos);

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

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::insert_empty(size_type pos, size_type length)
{
    MDDS_MTV_TRACE_ARGS(mutator, "pos=" << pos << "; length=" << length);

    if (!length)
        // Nothing to insert.
        return end();

    size_type block_index = get_block_position(pos);
    if (block_index == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
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

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::insert_empty(
    const iterator& pos_hint, size_type pos, size_type length)
{
    MDDS_MTV_TRACE_ARGS(mutator_with_pos_hint, "pos_hint=" << pos_hint << "; pos=" << pos << "; length=" << length);

    if (!length)
        // Nothing to insert.
        return end();

    size_type block_index = get_block_position(pos_hint->__private_data, pos);
    if (block_index == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
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

template<typename Traits>
void multi_type_vector<Traits>::clear()
{
    MDDS_MTV_TRACE(mutator);

    delete_element_blocks(0, m_block_store.element_blocks.size());
    m_block_store.clear();
    m_cur_size = 0;
}

template<typename Traits>
template<typename T>
bool multi_type_vector<Traits>::set_cells_precheck(
    size_type pos, const T& it_begin, const T& it_end, size_type& end_pos)
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

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set_impl(
    size_type pos, size_type block_index, const T& value)
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
        block_funcs::overwrite_values(*blk_data, i, 1);
        mdds_mtv_set_value(*blk_data, i, value);
        return get_iterator(block_index);
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
            block_funcs::overwrite_values(*m_block_store.element_blocks[block_index], 0, 1);
            block_funcs::erase(*m_block_store.element_blocks[block_index], 0);
            m_block_store.sizes[block_index - 1] += 1;
            mdds_mtv_append_value(*m_block_store.element_blocks[block_index - 1], value);
            return get_iterator(block_index - 1);
        }

        // t|---|x--|???|b
        set_cell_to_top_of_data_block(block_index, value);
        return get_iterator(block_index);
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
            iterator itr = end();
            return --itr;
        }

        bool blk_next = is_next_block_of_type(block_index, cat);
        if (!blk_next)
        {
            // t|--x|---|b - Next block is of different type.
            set_cell_to_bottom_of_data_block(0, value);
            iterator itr = begin();
            return ++itr;
        }

        // t|--x|xxx|b - Next block is of the same type as the new value.

        block_funcs::overwrite_values(*blk_data, blk_size - 1, 1);
        block_funcs::erase(*blk_data, blk_size - 1);
        m_block_store.sizes[block_index] -= 1;
        mdds_mtv_prepend_value(*m_block_store.element_blocks[block_index + 1], value);
        m_block_store.sizes[block_index + 1] += 1;
        m_block_store.positions[block_index + 1] -= 1;

        return get_iterator(block_index + 1);
    }

    assert(block_index > 0);

    if (block_index == m_block_store.positions.size() - 1)
    {
        // t|???|--x|b - This is the last block.
        set_cell_to_bottom_of_data_block(block_index, value);
        iterator itr = end();
        return --itr;
    }

    bool blk_next = is_next_block_of_type(block_index, cat);
    if (!blk_next)
    {
        // t|???|--x|---|b - Next block is of different type than the new
        // value's.
        set_cell_to_bottom_of_data_block(block_index, value);
        return get_iterator(block_index + 1);
    }

    // t|???|--x|xxx|b - The next block is of the same type as the new value.
    block_funcs::overwrite_values(*blk_data, blk_size - 1, 1);
    block_funcs::erase(*blk_data, blk_size - 1);
    m_block_store.sizes[block_index] -= 1;
    mdds_mtv_prepend_value(*m_block_store.element_blocks[block_index + 1], value);
    m_block_store.sizes[block_index + 1] += 1;
    m_block_store.positions[block_index + 1] -= 1;

    return get_iterator(block_index + 1);
}

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::release_impl(
    size_type pos, size_type block_index, T& value)
{
    const element_block_type* blk_data = m_block_store.element_blocks[block_index];
    size_type start_pos = m_block_store.positions[block_index];

    if (!blk_data)
    {
        // Empty cell block.  There is no element to release.
        mdds_mtv_get_empty_value(value);
        return get_iterator(block_index);
    }

    assert(pos >= start_pos);
    assert(blk_data); // data for non-empty blocks should never be nullptr.
    size_type offset = pos - start_pos;
    mdds_mtv_get_value(*blk_data, offset, value);

    // Set the element slot empty without overwriting it.
    return set_empty_in_single_block(pos, pos, block_index, false);
}

template<typename Traits>
void multi_type_vector<Traits>::swap_impl(
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

template<typename Traits>
void multi_type_vector<Traits>::swap_single_block(
    multi_type_vector& other, size_type start_pos, size_type end_pos, size_type other_pos, size_type block_index,
    size_type other_block_index)
{
    element_block_type* src_data = m_block_store.element_blocks[block_index];
    element_block_type* dst_data = other.m_block_store.element_blocks[other_block_index];
    size_type start_pos_in_block = m_block_store.positions[block_index];
    size_type start_pos_in_other_block = other.m_block_store.positions[other_block_index];
    element_category_type cat_src = mtv::element_type_empty;
    element_category_type cat_dst = mtv::element_type_empty;

    if (src_data)
        cat_src = mtv::get_block_type(*src_data);
    if (dst_data)
        cat_dst = mtv::get_block_type(*dst_data);

    size_t other_end_pos = other_pos + end_pos - start_pos;
    size_t len = end_pos - start_pos + 1; // length of elements to swap.
    size_type src_offset = start_pos - start_pos_in_block;
    size_type dst_offset = other_pos - start_pos_in_other_block;

    // length of the tail that will not be swapped.
    size_type src_tail_len = m_block_store.sizes[block_index] - src_offset - len;

    if (cat_src == cat_dst)
    {
        // Source and destination blocks are of the same type.
        if (cat_src == mtv::element_type_empty)
            // Both are empty blocks. Nothing to swap.
            return;

        block_funcs::swap_values(*src_data, *dst_data, src_offset, dst_offset, len);
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
            // the entire source block needs to be replaced.
            std::unique_ptr<element_block_type, element_block_deleter> src_data_original(src_data);
            m_hdl_event.element_block_released(src_data);
            m_block_store.element_blocks[block_index] =
                other.exchange_elements(*src_data_original, src_offset, other_block_index, dst_offset, len);
            src_data = m_block_store.element_blocks[block_index];
            m_hdl_event.element_block_acquired(src_data);

            // Release elements in the source block to prevent double-deletion.
            block_funcs::resize_block(*src_data_original, 0);
            merge_with_adjacent_blocks(block_index);
            return;
        }

        // Get the new elements from the other container.
        std::unique_ptr<element_block_type, element_block_deleter> new_dst_data(
            other.exchange_elements(*src_data, src_offset, other_block_index, dst_offset, len));

        // Shrink the current block by erasing the top part.
        block_funcs::erase(*src_data, 0, len);
        m_block_store.positions[block_index] += len;
        m_block_store.sizes[block_index] -= len;

        bool blk_prev = is_previous_block_of_type(block_index, cat_dst);
        if (blk_prev)
        {
            // Append the new elements to the previous block.
            element_block_type* prev_data = m_block_store.element_blocks[block_index - 1];
            block_funcs::append_block(*prev_data, *new_dst_data);
            block_funcs::resize_block(*new_dst_data, 0); // prevent double-delete.
            m_block_store.sizes[block_index - 1] += len;
        }
        else
        {
            // Insert a new block to store the new elements.
            size_type position = m_block_store.positions[block_index] - len;
            m_block_store.insert(block_index, position, len, nullptr);
            m_block_store.element_blocks[block_index] = new_dst_data.release();
            m_hdl_event.element_block_acquired(m_block_store.element_blocks[block_index]);
        }
        return;
    }

    // Get the new elements from the destination instance.
    std::unique_ptr<element_block_type, element_block_deleter> data_from_dst(
        other.exchange_elements(*src_data, src_offset, other_block_index, dst_offset, len));

    if (src_tail_len == 0)
    {
        // Source range is at the bottom of a block.

        // Shrink the current block.
        block_funcs::resize_block(*src_data, src_offset);
        m_block_store.sizes[block_index] = src_offset;

        if (is_next_block_of_type(block_index, cat_dst))
        {
            // Merge with the next block.
            element_block_type* next_data = m_block_store.element_blocks[block_index + 1];
            block_funcs::prepend_values_from_block(*next_data, *data_from_dst, 0, len);
            block_funcs::resize_block(*data_from_dst, 0); // prevent double-delete.
            m_block_store.sizes[block_index + 1] += len;
            m_block_store.positions[block_index + 1] -= len;
        }
        else
        {
            m_block_store.insert(block_index + 1, 0, len, nullptr);
            m_block_store.calc_block_position(block_index + 1);
            m_block_store.element_blocks[block_index + 1] = data_from_dst.release();
            m_hdl_event.element_block_acquired(m_block_store.element_blocks[block_index + 1]);
        }
        return;
    }

    // Source range is in the middle of a block.
    assert(src_offset && src_tail_len);
    block_index = set_new_block_to_middle(block_index, src_offset, len, false);
    m_block_store.element_blocks[block_index] = data_from_dst.release();
    m_hdl_event.element_block_acquired(m_block_store.element_blocks[block_index]);
}

template<typename Traits>
void multi_type_vector<Traits>::swap_single_to_multi_blocks(
    multi_type_vector& other, size_type start_pos, size_type end_pos, size_type other_pos, size_type block_index,
    size_type dst_block_index1, size_type dst_block_index2)
{
    element_block_type* src_data = m_block_store.element_blocks[block_index];
    size_type start_pos_in_block = m_block_store.positions[block_index];
    size_type dst_start_pos_in_block1 = other.m_block_store.positions[dst_block_index1];
    size_type dst_start_pos_in_block2 = other.m_block_store.positions[dst_block_index2];

    element_category_type cat_src = src_data ? mtv::get_block_type(*src_data) : mtv::element_type_empty;

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
    size_type src_tail_len = m_block_store.sizes[block_index] - src_offset - len;

    // Get the new elements from the other instance.
    blocks_type new_blocks;
    other.exchange_elements(
        *src_data, src_offset, dst_block_index1, dst_offset1, dst_block_index2, dst_offset2, len, new_blocks);

    new_blocks.check_integrity();

    if (new_blocks.positions.empty())
        throw general_error("multi_type_vector::swap_single_to_multi_blocks: failed to exchange elements.");

    if (src_offset == 0)
    {
        // Source range is at the top of a block.

        size_type src_position = m_block_store.positions[block_index];

        if (src_tail_len == 0)
        {
            // the whole block needs to be replaced.  Delete the block, but
            // don't delete the managed elements the block contains since they
            // have been transferred over to the destination block.
            block_funcs::resize_block(*src_data, 0);
            delete_element_block(block_index);
            m_block_store.erase(block_index);
        }
        else
        {
            // Shrink the current block by erasing the top part.
            block_funcs::erase(*src_data, 0, len);
            m_block_store.sizes[block_index] -= len;
            m_block_store.positions[block_index] += len;
        }

        insert_blocks_at(src_position, block_index, new_blocks);
        merge_with_next_block(block_index + new_blocks.positions.size() - 1); // last block inserted.
        if (block_index > 0)
            merge_with_next_block(block_index - 1); // block before the first block inserted.

        return;
    }

    size_type position = 0;

    if (src_tail_len == 0)
    {
        // Source range is at the bottom of a block.

        // Shrink the current block.
        block_funcs::resize_block(*src_data, src_offset);
        m_block_store.sizes[block_index] = src_offset;
        position = m_block_store.positions[block_index] + m_block_store.sizes[block_index];
    }
    else
    {
        // Source range is in the middle of a block.
        assert(src_offset && src_tail_len);

        // This will create two new slots at block_index+1, the first of which
        // we will immediately remove.  The new blocks from the other
        // container will be inserted at the removed slot.
        set_new_block_to_middle(block_index, src_offset, len, false);
        delete_element_block(block_index + 1);
        m_block_store.erase(block_index + 1);
        position = m_block_store.positions[block_index] + m_block_store.sizes[block_index];
    }

    insert_blocks_at(position, block_index + 1, new_blocks);
    merge_with_next_block(block_index + new_blocks.positions.size()); // last block inserted.
    merge_with_next_block(block_index); // block before the first block inserted.
}

template<typename Traits>
void multi_type_vector<Traits>::swap_multi_to_multi_blocks(
    multi_type_vector& other, size_type start_pos, size_type end_pos, size_type other_pos, size_type block_index1,
    size_type block_index2, size_type dblock_index1, size_type dblock_index2)
{
    assert(block_index1 < block_index2);
    assert(dblock_index1 < dblock_index2);

    size_type start_pos_in_block1 = m_block_store.positions[block_index1];
    size_type start_pos_in_block2 = m_block_store.positions[block_index2];
    size_type start_pos_in_dblock1 = other.m_block_store.positions[dblock_index1];
    size_type start_pos_in_dblock2 = other.m_block_store.positions[dblock_index2];

    size_type len = end_pos - start_pos + 1;
    size_type src_offset1 = start_pos - start_pos_in_block1;
    size_type src_offset2 = end_pos - start_pos_in_block2;
    size_type dst_offset1 = other_pos - start_pos_in_dblock1;
    size_type dst_offset2 = other_pos + len - 1 - start_pos_in_dblock2;

    blocks_to_transfer src_bucket, dst_bucket;
    prepare_blocks_to_transfer(src_bucket, block_index1, src_offset1, block_index2, src_offset2);
    other.prepare_blocks_to_transfer(dst_bucket, dblock_index1, dst_offset1, dblock_index2, dst_offset2);

    size_type position = 0;
    if (src_bucket.insert_index > 0)
    {
        size_type i = src_bucket.insert_index - 1;
        position = m_block_store.positions[i] + m_block_store.sizes[i];
    }
    insert_blocks_at(position, src_bucket.insert_index, dst_bucket.blocks);

    // Merge the boundary blocks in the source.
    merge_with_next_block(src_bucket.insert_index + dst_bucket.blocks.positions.size() - 1);
    if (src_bucket.insert_index > 0)
        merge_with_next_block(src_bucket.insert_index - 1);

    position = 0;
    if (dst_bucket.insert_index > 0)
    {
        size_type i = dst_bucket.insert_index - 1;
        position = other.m_block_store.positions[i] + other.m_block_store.sizes[i];
    }
    other.insert_blocks_at(position, dst_bucket.insert_index, src_bucket.blocks);

    // Merge the boundary blocks in the destination.
    other.merge_with_next_block(dst_bucket.insert_index + src_bucket.blocks.positions.size() - 1);
    if (dst_bucket.insert_index > 0)
        other.merge_with_next_block(dst_bucket.insert_index - 1);
}

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::insert_cells_impl(
    size_type row, size_type block_index, const T& it_begin, const T& it_end)
{
    size_type start_row = m_block_store.positions[block_index];
    size_type length = std::distance(it_begin, it_end);
    if (!length)
        // empty data array.  nothing to do.
        return end();

    element_category_type cat = mdds_mtv_get_element_type(*it_begin);
    element_block_type* blk_data = m_block_store.element_blocks[block_index];
    if (!blk_data)
    {
        if (row == start_row)
        {
            // Insert into an empty block.  Check the previos block (if exists)
            // to see if the data can be appended to it if inserting at the top
            // of the block.
            bool blk0 = is_previous_block_of_type(block_index, cat);

            if (blk0)
            {
                // Append to the previous block.
                element_block_type* blk0_data = m_block_store.element_blocks[block_index - 1];
                mdds_mtv_append_values(*blk0_data, *it_begin, it_begin, it_end);
                m_block_store.sizes[block_index - 1] += length;
                m_cur_size += length;
                adjust_block_positions_func{}(m_block_store, block_index, length);

                return get_iterator(block_index - 1);
            }

            // Just insert a new block before the current block.
            size_type position = m_block_store.positions[block_index];
            m_block_store.insert(block_index, position, length, nullptr);
            m_block_store.element_blocks[block_index] = block_funcs::create_new_block(cat, 0);
            blk_data = m_block_store.element_blocks[block_index];
            m_hdl_event.element_block_acquired(blk_data);
            mdds_mtv_assign_values(*blk_data, *it_begin, it_begin, it_end);
            m_cur_size += length;
            adjust_block_positions_func{}(m_block_store, block_index + 1, length);

            return get_iterator(block_index);
        }

        insert_cells_to_middle(row, block_index, it_begin, it_end);
        m_cur_size += length;

        return get_iterator(block_index + 1);
    }

    assert(blk_data);
    element_category_type blk_cat = mdds::mtv::get_block_type(*blk_data);
    if (cat == blk_cat)
    {
        // Simply insert the new data series into existing block.
        assert(it_begin != it_end);
        mdds_mtv_insert_values(*blk_data, row - start_row, *it_begin, it_begin, it_end);
        m_block_store.sizes[block_index] += length;
        m_cur_size += length;
        adjust_block_positions_func{}(m_block_store, block_index + 1, length);

        return get_iterator(block_index);
    }

    assert(cat != blk_cat);
    if (row == start_row)
    {
        // Check the previous block to see if we can append the data there.
        bool blk0 = is_previous_block_of_type(block_index, cat);
        if (blk0)
        {
            // Append to the previous block.
            element_block_type* blk0_data = m_block_store.element_blocks[block_index - 1];
            mdds_mtv_append_values(*blk0_data, *it_begin, it_begin, it_end);
            m_block_store.sizes[block_index - 1] += length;
            m_cur_size += length;
            adjust_block_positions_func{}(m_block_store, block_index, length);

            return get_iterator(block_index - 1);
        }

        // Just insert a new block before the current block.
        m_block_store.insert(block_index, m_block_store.positions[block_index], length, nullptr);
        m_block_store.element_blocks[block_index] = block_funcs::create_new_block(cat, 0);
        m_hdl_event.element_block_acquired(blk_data);
        blk_data = m_block_store.element_blocks[block_index];
        mdds_mtv_assign_values(*blk_data, *it_begin, it_begin, it_end);
        m_block_store.sizes[block_index] = length;
        m_cur_size += length;
        adjust_block_positions_func{}(m_block_store, block_index + 1, length);

        return get_iterator(block_index);
    }

    insert_cells_to_middle(row, block_index, it_begin, it_end);
    m_cur_size += length;

    return get_iterator(block_index + 1);
}

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set_empty_impl(
    size_type start_pos, size_type end_pos, size_type block_index1, bool overwrite)
{
    if (start_pos > end_pos)
        throw std::out_of_range("Start row is larger than the end row.");

    size_type block_index2 = get_block_position(end_pos, block_index1);
    if (block_index2 == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
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

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set_empty_in_single_block(
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
            block_funcs::overwrite_values(*blk_data, 0, empty_block_size);

        block_funcs::erase(*blk_data, 0, empty_block_size);
        m_block_store.sizes[block_index] -= empty_block_size;

        // Check if the preceding block (if exists) is also empty.
        bool blk_prev = is_previous_block_of_type(block_index, mtv::element_type_empty);
        if (blk_prev)
        {
            // Extend the previous empty block.
            m_block_store.sizes[block_index - 1] += empty_block_size;
            m_block_store.positions[block_index] += empty_block_size;
            return get_iterator(block_index - 1);
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
            block_funcs::overwrite_values(*blk_data, start_pos, empty_block_size);

        block_funcs::erase(*blk_data, start_pos, empty_block_size);
        m_block_store.sizes[block_index] -= empty_block_size;

        // Check if the following block (if exists) is also empty.
        bool blk_next = is_next_block_of_type(block_index, mtv::element_type_empty);
        if (blk_next)
        {
            // Extend the next empty block to cover the new empty segment.
            m_block_store.sizes[block_index + 1] += empty_block_size;
            m_block_store.positions[block_index + 1] = start_row;
        }
        else
        {
            // Insert a new empty block after the current one.
            m_block_store.insert(block_index + 1, start_row, empty_block_size, nullptr);
        }

        return get_iterator(block_index + 1);
    }

    // Empty the middle part of a block.
    assert(end_row_in_block - end_row > 0);
    set_new_block_to_middle(block_index, start_row - start_row_in_block, empty_block_size, overwrite);

    return get_iterator(block_index + 1);
}

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set_empty_in_multi_blocks(
    size_type start_row, size_type end_row, size_type block_index1, size_type block_index2, bool overwrite)
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
                    start_row -= m_block_store.sizes[block_index1 - 1];
                    --block_index1;
                }
                else
                {
                    // Make block 1 empty.
                    if (!overwrite)
                        block_funcs::resize_block(*blk_data, 0);

                    delete_element_block(block_index1);
                }
            }
            else
            {
                // Empty the lower part.
                size_type new_size = start_row - start_row_in_block1;
                if (overwrite)
                    block_funcs::overwrite_values(*blk_data, new_size, m_block_store.sizes[block_index1] - new_size);

                block_funcs::resize_block(*blk_data, new_size);
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
                    end_row += m_block_store.sizes[block_index2 + 1];
                    ++end_block_to_erase;
                }
            }
            else
            {
                // Empty the upper part.
                size_type size_to_erase = end_row - start_row_in_block2 + 1;
                if (overwrite)
                    block_funcs::overwrite_values(*blk_data, 0, size_to_erase);

                block_funcs::erase(*blk_data, 0, size_to_erase);
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
            element_block_type* data = m_block_store.element_blocks[i];
            if (!overwrite && data)
                block_funcs::resize_block(*data, 0);

            delete_element_block(i);
        }

        size_type n_erase_blocks = end_block_to_erase - block_index1 - 1;
        m_block_store.erase(block_index1 + 1, n_erase_blocks);
    }

    element_block_type* blk_data = m_block_store.element_blocks[block_index1];
    size_type empty_block_size = end_row - start_row + 1;
    if (blk_data)
    {
        // Insert a new empty block after the first block.
        m_block_store.insert(block_index1 + 1, start_row, empty_block_size, nullptr);
        return get_iterator(block_index1 + 1);
    }

    // Current block is already empty. Just extend its size.
    m_block_store.sizes[block_index1] = empty_block_size;
    m_block_store.positions[block_index1] = start_row;
    return get_iterator(block_index1);
}

template<typename Traits>
void multi_type_vector<Traits>::erase_impl(size_type start_row, size_type end_row)
{
    assert(start_row <= end_row);

    // Keep the logic similar to set_empty().

    size_type block_pos1 = get_block_position(start_row);
    if (block_pos1 == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
            "multi_type_vector::erase_impl", __LINE__, start_row, block_size(), size());

    size_type block_pos2 = get_block_position(end_row, block_pos1);
    if (block_pos2 == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
            "multi_type_vector::erase_impl", __LINE__, start_row, block_size(), size());

    size_type start_row_in_block1 = m_block_store.positions[block_pos1];
    size_type start_row_in_block2 = m_block_store.positions[block_pos2];

    if (block_pos1 == block_pos2)
    {
        erase_in_single_block(start_row, end_row, block_pos1);
        return;
    }

    assert(block_pos1 < block_pos2);

    // Initially, we set to erase all blocks between the first and the last.
    size_type index_erase_begin = block_pos1 + 1;
    size_type index_erase_end = block_pos2;

    // First, inspect the first block.
    if (start_row_in_block1 == start_row)
    {
        // Erase the whole block.
        --index_erase_begin;
    }
    else
    {
        // Erase the lower part of the first element block.
        element_block_type* blk_data = m_block_store.element_blocks[block_pos1];
        size_type new_size = start_row - start_row_in_block1;
        if (blk_data)
        {
            // Shrink the element block.
            block_funcs::overwrite_values(*blk_data, new_size, m_block_store.sizes[block_pos1] - new_size);
            block_funcs::resize_block(*blk_data, new_size);
        }
        m_block_store.sizes[block_pos1] = new_size;
    }

    size_type adjust_block_offset = 0;

    // Then inspect the last block.
    size_type last_row_in_block = start_row_in_block2 + m_block_store.sizes[block_pos2] - 1;
    if (last_row_in_block == end_row)
    {
        // Delete the whole block.
        ++index_erase_end;
    }
    else
    {
        size_type size_to_erase = end_row - start_row_in_block2 + 1;
        m_block_store.sizes[block_pos2] -= size_to_erase;
        m_block_store.positions[block_pos2] = start_row;
        element_block_type* blk_data = m_block_store.element_blocks[block_pos2];
        if (blk_data)
        {
            // Erase the upper part.
            block_funcs::overwrite_values(*blk_data, 0, size_to_erase);
            block_funcs::erase(*blk_data, 0, size_to_erase);
        }

        adjust_block_offset = 1; // Exclude this block from later block position adjustment.
    }

    // Get the index of the block that sits before the blocks being erased.
    block_pos1 = index_erase_begin;
    if (block_pos1 > 0)
        --block_pos1;

    // Now, erase all blocks in between.
    delete_element_blocks(index_erase_begin, index_erase_end);
    m_block_store.erase(index_erase_begin, index_erase_end - index_erase_begin);
    int64_t delta = end_row - start_row + 1;
    m_cur_size -= delta;

    if (m_block_store.positions.empty())
        return;

    // Adjust the positions of the blocks following the erased.
    size_type adjust_pos = index_erase_begin;
    adjust_pos += adjust_block_offset;
    adjust_block_positions_func{}(m_block_store, adjust_pos, -delta);
    merge_with_next_block(block_pos1);
}

template<typename Traits>
void multi_type_vector<Traits>::erase_in_single_block(size_type start_pos, size_type end_pos, size_type block_index)
{
    // Range falls within the same block.
    element_block_type* blk_data = m_block_store.element_blocks[block_index];
    int64_t size_to_erase = end_pos - start_pos + 1;

    if (blk_data)
    {
        // Erase data in the data block.
        size_type offset = start_pos - m_block_store.positions[block_index];
        block_funcs::overwrite_values(*blk_data, offset, size_to_erase);
        block_funcs::erase(*blk_data, offset, size_to_erase);
    }

    m_block_store.sizes[block_index] -= size_to_erase;
    m_cur_size -= size_to_erase;

    if (m_block_store.sizes[block_index])
    {
        // Block still contains data.  Bail out.
        adjust_block_positions_func{}(m_block_store, block_index + 1, -size_to_erase);
        return;
    }

    // Delete the current block since it has become empty.
    delete_element_block(block_index);
    m_block_store.erase(block_index);

    if (block_index == 0)
    {
        // Deleted block was the first block.
        adjust_block_positions_func{}(m_block_store, block_index, -size_to_erase);
        return;
    }

    if (block_index >= m_block_store.positions.size())
        // Deleted block was the last block.
        return;

    // Check the previous and next blocks to see if they should be merged.
    element_block_type* prev_data = m_block_store.element_blocks[block_index - 1];
    element_block_type* next_data = m_block_store.element_blocks[block_index];

    if (prev_data)
    {
        // Previous block has data.
        if (!next_data)
        {
            // Next block is empty.  Nothing to do.
            adjust_block_positions_func{}(m_block_store, block_index, -size_to_erase);
            return;
        }

        element_category_type cat1 = mdds::mtv::get_block_type(*prev_data);
        element_category_type cat2 = mdds::mtv::get_block_type(*next_data);

        if (cat1 == cat2)
        {
            // Merge the two blocks.
            block_funcs::append_block(*prev_data, *next_data);
            m_block_store.sizes[block_index - 1] += m_block_store.sizes[block_index];
            // Resize to 0 to prevent deletion of cells in case of managed cells.
            block_funcs::resize_block(*next_data, 0);
            delete_element_block(block_index);
            m_block_store.erase(block_index);
        }

        adjust_block_positions_func{}(m_block_store, block_index, -size_to_erase);
    }
    else
    {
        // Previous block is empty.
        if (next_data)
        {
            // Next block is not empty.  Nothing to do.
            adjust_block_positions_func{}(m_block_store, block_index, -size_to_erase);
            return;
        }

        // Both blocks are empty.  Simply increase the size of the previous
        // block.
        m_block_store.sizes[block_index - 1] += m_block_store.sizes[block_index];
        delete_element_block(block_index);
        m_block_store.erase(block_index);
        adjust_block_positions_func{}(m_block_store, block_index, -size_to_erase);
    }
}

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::insert_empty_impl(
    size_type pos, size_type block_index, size_type length)
{
    assert(pos < m_cur_size);

    element_block_type* blk_data = m_block_store.element_blocks[block_index];

    if (!blk_data)
    {
        // Insertion point is already empty.  Just expand its size and be done
        // with it.
        m_block_store.sizes[block_index] += length;
        m_cur_size += length;
        adjust_block_positions_func{}(m_block_store, block_index + 1, length);
        return get_iterator(block_index);
    }

    size_type start_pos = m_block_store.positions[block_index];

    if (start_pos == pos)
    {
        // Insertion point is at the top of an existing non-empty block.
        bool blk_prev = is_previous_block_of_type(block_index, mtv::element_type_empty);
        if (blk_prev)
        {
            // Previous block is empty.  Expand the size of the previous block.
            assert(!m_block_store.element_blocks[block_index - 1]);
            m_block_store.sizes[block_index - 1] += length;
            m_cur_size += length;
            adjust_block_positions_func{}(m_block_store, block_index, length);
            return get_iterator(block_index - 1);
        }

        // Insert a new empty block.
        m_block_store.insert(block_index, start_pos, length, nullptr);
        m_cur_size += length;
        adjust_block_positions_func{}(m_block_store, block_index + 1, length);
        return get_iterator(block_index);
    }

    assert(blk_data);
    assert(pos > start_pos);

    size_type size_blk_prev = pos - start_pos;
    size_type size_blk_next = m_block_store.sizes[block_index] - size_blk_prev;

    // Insert two new blocks below the current; one for the empty block being
    // inserted, and the other for the lower part of the current non-empty
    // block.
    m_block_store.insert(block_index + 1, 2u);

    m_block_store.sizes[block_index + 1] = length;
    m_block_store.sizes[block_index + 2] = size_blk_next;

    m_block_store.element_blocks[block_index + 2] =
        block_funcs::create_new_block(mdds::mtv::get_block_type(*blk_data), 0);
    element_block_type* next_data = m_block_store.element_blocks[block_index + 2];
    m_hdl_event.element_block_acquired(next_data);

    // Check if the previous block is the bigger one
    if (size_blk_prev > size_blk_next)
    {
        // Upper (previous) block is larger than the lower (next) block. Copy
        // the lower values to the next block.
        block_funcs::assign_values_from_block(*next_data, *blk_data, size_blk_prev, size_blk_next);
        block_funcs::resize_block(*blk_data, size_blk_prev);
        m_block_store.sizes[block_index] = size_blk_prev;
    }
    else
    {
        // Lower (next) block is larger than the upper (previous) block. Copy
        // the upper values to the "next" block.
        block_funcs::assign_values_from_block(*next_data, *blk_data, 0, size_blk_prev);
        m_block_store.sizes[block_index + 2] = size_blk_prev;

        // Remove the copied values and push the rest to the top.
        block_funcs::erase(*blk_data, 0, size_blk_prev);

        // Set the size of the current block to its new size ( what is after the new block )
        m_block_store.sizes[block_index] = size_blk_next;

        // And now let's swap the blocks, but save the block position.
        size_type position = m_block_store.positions[block_index];
        m_block_store.swap(block_index, block_index + 2);
        m_block_store.positions[block_index] = position;
    }

    m_cur_size += length;
    m_block_store.calc_block_position(block_index + 1);
    m_block_store.calc_block_position(block_index + 2);
    adjust_block_positions_func{}(m_block_store, block_index + 3, length);

    return get_iterator(block_index + 1);
}

template<typename Traits>
void multi_type_vector<Traits>::insert_blocks_at(size_type position, size_type insert_pos, blocks_type& new_blocks)
{
    for (size_type i = 0; i < new_blocks.positions.size(); ++i)
    {
        new_blocks.positions[i] = position;
        position += new_blocks.sizes[i];

        const element_block_type* data = new_blocks.element_blocks[i];

        if (data)
            m_hdl_event.element_block_acquired(data);
    }

    m_block_store.insert(insert_pos, new_blocks);
}

template<typename Traits>
void multi_type_vector<Traits>::prepare_blocks_to_transfer(
    blocks_to_transfer& bucket, size_type block_index1, size_type offset1, size_type block_index2, size_type offset2)
{
    assert(block_index1 < block_index2);
    assert(offset1 < m_block_store.sizes[block_index1]);
    assert(offset2 < m_block_store.sizes[block_index2]);

    block_slot_type block_first;
    block_slot_type block_last;
    size_type index_begin = block_index1 + 1;
    size_type index_end = block_index2;

    bucket.insert_index = block_index1 + 1;

    if (offset1 == 0)
    {
        // The whole first block needs to be swapped.
        --index_begin;
        --bucket.insert_index;
    }
    else
    {
        // Copy the lower part of the block for transfer.
        size_type blk_size = m_block_store.sizes[block_index1] - offset1;
        block_first.size = blk_size;

        element_block_type* blk_data1 = m_block_store.element_blocks[block_index1];
        if (blk_data1)
        {
            block_first.element_block = block_funcs::create_new_block(mtv::get_block_type(*blk_data1), 0);
            block_funcs::assign_values_from_block(*block_first.element_block, *blk_data1, offset1, blk_size);

            // Shrink the existing block.
            block_funcs::resize_block(*blk_data1, offset1);
        }

        m_block_store.sizes[block_index1] = offset1;
    }

    if (offset2 == m_block_store.sizes[block_index2] - 1)
    {
        // The entire last block needs to be swapped.
        ++index_end;
    }
    else
    {
        // Copy the upper part of the last block for transfer.
        size_type blk_size = offset2 + 1;
        block_last.size = blk_size;
        element_block_type* blk_data2 = m_block_store.element_blocks[block_index2];

        if (blk_data2)
        {
            block_last.element_block = block_funcs::create_new_block(mtv::get_block_type(*blk_data2), 0);
            block_funcs::assign_values_from_block(*block_last.element_block, *blk_data2, 0, blk_size);

            // Shrink the existing block.
            block_funcs::erase(*blk_data2, 0, blk_size);
        }

        m_block_store.positions[block_index2] += blk_size;
        m_block_store.sizes[block_index2] -= blk_size;
    }

    // Copy all blocks into the bucket.
    if (block_first.size)
        bucket.blocks.push_back(block_first);

    for (size_type i = index_begin; i < index_end; ++i)
    {
        element_block_type* data = m_block_store.element_blocks[i];
        if (data)
            m_hdl_event.element_block_released(data);

        bucket.blocks.push_back(m_block_store.positions[i], m_block_store.sizes[i], m_block_store.element_blocks[i]);
    }

    if (block_last.size)
        bucket.blocks.push_back(block_last);

    // Remove the slots for these blocks (but don't delete the blocks).
    m_block_store.erase(index_begin, index_end - index_begin);
}

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set_whole_block_empty(
    size_type block_index, bool overwrite)
{
    element_block_type* blk_data = m_block_store.element_blocks[block_index];
    if (!overwrite)
        // Resize block to 0 before deleting, to prevent its elements from getting deleted.
        block_funcs::resize_block(*blk_data, 0);

    delete_element_block(block_index);

    bool blk_prev = is_previous_block_of_type(block_index, mtv::element_type_empty);
    bool blk_next = is_next_block_of_type(block_index, mtv::element_type_empty);

    // Merge with adjacent block(s) if necessary.
    if (blk_prev)
    {
        assert(!m_block_store.element_blocks[block_index - 1]);

        if (blk_next)
        {
            // Both preceding and next blocks are empty.
            assert(!m_block_store.element_blocks[block_index + 1]);

            m_block_store.sizes[block_index - 1] +=
                m_block_store.sizes[block_index] + m_block_store.sizes[block_index + 1];

            // No need delete the current and next element blocks since they are both empty.
            m_block_store.erase(block_index, 2);

            return get_iterator(block_index - 1);
        }

        // Only the preceding block is empty. Merge the current block with the previous.
        m_block_store.sizes[block_index - 1] += m_block_store.sizes[block_index];
        m_block_store.erase(block_index);

        return get_iterator(block_index - 1);
    }
    else if (blk_next)
    {
        assert(!m_block_store.element_blocks[block_index + 1]);

        // Only the next block is empty. Merge the next block with the current.
        m_block_store.sizes[block_index] += m_block_store.sizes[block_index + 1];
        m_block_store.erase(block_index + 1);

        return get_iterator(block_index);
    }

    return get_iterator(block_index);
}

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set_cells_impl(
    size_type row, size_type end_row, size_type block_index1, const T& it_begin, const T& it_end)
{
    size_type block_index2 = get_block_position(end_row, block_index1);
    if (block_index2 == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
            "multi_type_vector::set_cells_impl", __LINE__, end_row, block_size(), size());

    if (block_index1 == block_index2)
    {
        // The whole data array will fit in a single block.
        return set_cells_to_single_block(row, end_row, block_index1, it_begin, it_end);
    }

    return set_cells_to_multi_blocks(row, end_row, block_index1, block_index2, it_begin, it_end);
}

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set_cells_to_single_block(
    size_type start_row, size_type end_row, size_type block_index, const T& it_begin, const T& it_end)
{
    assert(it_begin != it_end);
    assert(!m_block_store.positions.empty());

    element_category_type cat = mdds_mtv_get_element_type(*it_begin);
    size_type start_row_in_block = m_block_store.positions[block_index];
    size_type data_length = std::distance(it_begin, it_end);
    element_block_type* blk_data = m_block_store.element_blocks[block_index];

    if (blk_data && mdds::mtv::get_block_type(*blk_data) == cat)
    {
        // t|---|xxx|---|b - Simple overwrite.
        size_type offset = start_row - start_row_in_block;
        block_funcs::overwrite_values(*blk_data, offset, data_length);
        if (!offset && data_length == m_block_store.sizes[block_index])
            // Overwrite the whole block.  It's faster to use assign_values.
            mdds_mtv_assign_values(*blk_data, *it_begin, it_begin, it_end);
        else
            mdds_mtv_set_values(*blk_data, offset, *it_begin, it_begin, it_end);

        return get_iterator(block_index);
    }

    size_type end_row_in_block = start_row_in_block + m_block_store.sizes[block_index] - 1;
    if (start_row == start_row_in_block)
    {
        if (end_row == end_row_in_block)
        {
            // t|???|xxx|???|b - Replace the entire current block, but first try
            // to see if the values can be appended to the previous block.
            if (append_to_prev_block(block_index, cat, end_row - start_row + 1, it_begin, it_end))
            {
                delete_element_block(block_index);
                m_block_store.erase(block_index);

                // Check if we need to merge it with the next block.
                --block_index;
                merge_with_next_block(block_index);

                return get_iterator(block_index);
            }

            // Replace the whole block.
            if (blk_data)
            {
                m_hdl_event.element_block_released(blk_data);
                block_funcs::delete_block(blk_data);
            }

            m_block_store.element_blocks[block_index] = block_funcs::create_new_block(cat, 0);
            blk_data = m_block_store.element_blocks[block_index];
            m_hdl_event.element_block_acquired(blk_data);
            mdds_mtv_assign_values(*blk_data, *it_begin, it_begin, it_end);
            merge_with_next_block(block_index);
            return get_iterator(block_index);
        }

        // t|???|xxx--|???|b - Replace the upper part of the block. Shrink the
        // current block first.
        size_type length = end_row_in_block - end_row;
        m_block_store.sizes[block_index] = length;

        if (blk_data)
        {
            // Erase the upper part of the data from the current element block.
            std::unique_ptr<element_block_type, element_block_deleter> new_data(
                block_funcs::create_new_block(mdds::mtv::get_block_type(*blk_data), 0));

            if (!new_data)
                throw std::logic_error("failed to create a new element block.");

            size_type pos = end_row - start_row_in_block + 1;
            block_funcs::assign_values_from_block(*new_data, *blk_data, pos, length);
            block_funcs::overwrite_values(*blk_data, 0, pos);

            block_funcs::resize_block(*blk_data, 0); // to prevent deletion of elements
            block_funcs::delete_block(blk_data);
            m_block_store.element_blocks[block_index] = new_data.release();

            // We intentionally don't trigger element block events here.
        }

        length = end_row - start_row + 1;
        if (append_to_prev_block(block_index, cat, length, it_begin, it_end))
        {
            // The new values have been successfully appended to the previous block.
            m_block_store.positions[block_index] += length;
            return get_iterator(block_index - 1);
        }

        // Insert a new block before the current block, and populate it with
        // the new data.
        size_type position = m_block_store.positions[block_index];
        m_block_store.positions[block_index] += length;
        m_block_store.insert(block_index, position, length, nullptr);
        m_block_store.element_blocks[block_index] = block_funcs::create_new_block(cat, 0);
        blk_data = m_block_store.element_blocks[block_index];
        m_hdl_event.element_block_acquired(blk_data);
        m_block_store.sizes[block_index] = length;
        mdds_mtv_assign_values(*blk_data, *it_begin, it_begin, it_end);

        return get_iterator(block_index);
    }

    assert(start_row > start_row_in_block);
    if (end_row == end_row_in_block)
    {
        // t|???|--xxx|???|b - Shrink the end of the current block and insert a
        // new block for the new data series after the current block.
        size_type new_size = start_row - start_row_in_block;
        m_block_store.sizes[block_index] = new_size;

        if (blk_data)
        {
            block_funcs::overwrite_values(*blk_data, new_size, data_length);
            block_funcs::resize_block(*blk_data, new_size);
        }

        new_size = end_row - start_row + 1; // size of the data array being inserted.

        if (block_index < m_block_store.positions.size() - 1)
        {
            // t|???|--xxx|???|b - There is a block after the current block..
            // There is a block (or more) after the current block. Check the next block.
            bool blk_next = is_next_block_of_type(block_index, cat);
            if (blk_next)
            {
                // t|???|--xxx|xxx|b - Prepend it to the next block.
                mdds_mtv_prepend_values(*m_block_store.element_blocks[block_index + 1], *it_begin, it_begin, it_end);
                m_block_store.sizes[block_index + 1] += new_size;
                m_block_store.positions[block_index + 1] -= new_size;
                return get_iterator(block_index + 1);
            }

            // t|???|--xxx|---|b - Next block has a different data type. Do the
            // normal insertion.
            m_block_store.insert(block_index + 1, 0, new_size, nullptr);
            m_block_store.calc_block_position(block_index + 1);
            m_block_store.element_blocks[block_index + 1] = block_funcs::create_new_block(cat, 0);
            blk_data = m_block_store.element_blocks[block_index + 1];
            m_hdl_event.element_block_acquired(blk_data);
            mdds_mtv_assign_values(*blk_data, *it_begin, it_begin, it_end);

            return get_iterator(block_index + 1);
        }

        // t|???|--xxx|b - Last block.
        assert(block_index == m_block_store.positions.size() - 1);

        m_block_store.push_back(m_cur_size - new_size, new_size, nullptr);
        m_block_store.element_blocks.back() = block_funcs::create_new_block(cat, 0);
        blk_data = m_block_store.element_blocks.back();
        m_hdl_event.element_block_acquired(blk_data);
        mdds_mtv_assign_values(*blk_data, *it_begin, it_begin, it_end);

        return get_iterator(block_index + 1);
    }

    // t|???|-xx-|???|b - New values will be in the middle of the current block.
    assert(start_row_in_block < start_row && end_row < end_row_in_block);

    block_index = set_new_block_to_middle(block_index, start_row - start_row_in_block, end_row - start_row + 1, true);

    m_block_store.element_blocks[block_index] = block_funcs::create_new_block(cat, 0);
    blk_data = m_block_store.element_blocks[block_index];
    m_hdl_event.element_block_acquired(blk_data);
    mdds_mtv_assign_values(*blk_data, *it_begin, it_begin, it_end);

    return get_iterator(block_index);
}

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set_cells_to_multi_blocks(
    size_type start_row, size_type end_row, size_type block_index1, size_type block_index2, const T& it_begin,
    const T& it_end)
{
    assert(block_index1 < block_index2);
    assert(it_begin != it_end);
    assert(!m_block_store.positions.empty());

    element_block_type* blk1_data = m_block_store.element_blocks[block_index1];
    if (blk1_data)
    {
        return set_cells_to_multi_blocks_block1_non_empty(
            start_row, end_row, block_index1, block_index2, it_begin, it_end);
    }

    // Block 1 is empty.
    assert(!blk1_data);

    return set_cells_to_multi_blocks_block1_non_equal(start_row, end_row, block_index1, block_index2, it_begin, it_end);
}

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set_cells_to_multi_blocks_block1_non_equal(
    size_type start_row, size_type end_row, size_type block_index1, size_type block_index2, const T& it_begin,
    const T& it_end)
{
    element_category_type cat = mdds_mtv_get_element_type(*it_begin);
    element_block_type* blk1_data = m_block_store.element_blocks[block_index1];
    element_block_type* blk2_data = m_block_store.element_blocks[block_index2];

    size_type start_row_in_block1 = m_block_store.positions[block_index1];
    size_type start_row_in_block2 = m_block_store.positions[block_index2];
    size_type length = std::distance(it_begin, it_end);
    size_type offset = start_row - start_row_in_block1;
    size_type end_row_in_block2 = start_row_in_block2 + m_block_store.sizes[block_index2] - 1;

    // Initially set to erase blocks between block 1 and block 2 non-inclusive at either end.
    size_type index_erase_begin = block_index1 + 1;
    size_type index_erase_end = block_index2;

    // Create the new data block first.
    block_slot_type data_blk(start_row, length);

    bool blk0_copied = false;
    if (offset == 0)
    {
        // Remove block 1.
        --index_erase_begin;

        // Check the type of the previous block (block 0) if exists.
        if (block_index1 > 0)
        {
            element_block_type* blk0_data = m_block_store.element_blocks[block_index1 - 1];
            if (blk0_data && cat == mdds::mtv::get_block_type(*blk0_data))
            {
                // Transfer the whole data from block 0 to data block.
                data_blk.element_block = blk0_data;
                m_block_store.element_blocks[block_index1 - 1] = nullptr;

                data_blk.size += m_block_store.sizes[block_index1 - 1];
                data_blk.position = m_block_store.positions[block_index1 - 1];

                --index_erase_begin;
                blk0_copied = true;
            }
        }
    }
    else
    {
        // Shrink block 1 by the end.
        if (blk1_data)
        {
            size_type n = m_block_store.sizes[block_index1] - offset;
            block_funcs::overwrite_values(*blk1_data, offset, n);
            block_funcs::resize_block(*blk1_data, offset);
        }
        m_block_store.sizes[block_index1] = offset;
    }

    if (blk0_copied)
    {
        mdds_mtv_append_values(*data_blk.element_block, *it_begin, it_begin, it_end);
    }
    else
    {
        data_blk.element_block = block_funcs::create_new_block(cat, 0);
        m_hdl_event.element_block_acquired(data_blk.element_block);
        mdds_mtv_assign_values(*data_blk.element_block, *it_begin, it_begin, it_end);
    }

    if (end_row == end_row_in_block2)
    {
        // Remove block 2.
        ++index_erase_end;

        if (block_index2 + 1 < m_block_store.positions.size())
        {
            // There is at least one block after block 2.
            element_block_type* blk3_data = m_block_store.element_blocks[block_index2 + 1];
            if (blk3_data && mdds::mtv::get_block_type(*blk3_data) == cat)
            {
                // Merge the whole block 3 with the new data. Remove block 3
                // afterward.  Resize block 3 to zero to prevent invalid free.
                block_funcs::append_block(*data_blk.element_block, *blk3_data);
                block_funcs::resize_block(*blk3_data, 0);
                data_blk.size += m_block_store.sizes[block_index2 + 1];
                ++index_erase_end;
            }
        }
    }
    else
    {
        bool erase_upper = true;

        if (blk2_data)
        {
            element_category_type blk_cat2 = mdds::mtv::get_block_type(*blk2_data);
            if (blk_cat2 == cat)
            {
                // Merge the lower part of block 2 with the new data, and erase
                // block 2.  Resize block 2 to avoid invalid free on the copied
                // portion of the block.
                size_type copy_pos = end_row - start_row_in_block2 + 1;
                size_type size_to_copy = end_row_in_block2 - end_row;
                block_funcs::append_values_from_block(*data_blk.element_block, *blk2_data, copy_pos, size_to_copy);
                block_funcs::resize_block(*blk2_data, copy_pos);
                data_blk.size += size_to_copy;

                ++index_erase_end;
                erase_upper = false;
            }
        }

        if (erase_upper)
        {
            // Erase the upper part of block 2.
            size_type size_to_erase = end_row - start_row_in_block2 + 1;

            if (blk2_data)
            {
                block_funcs::overwrite_values(*blk2_data, 0, size_to_erase);
                block_funcs::erase(*blk2_data, 0, size_to_erase);
            }

            m_block_store.sizes[block_index2] -= size_to_erase;
            m_block_store.positions[block_index2] += size_to_erase;
        }
    }

    size_type insert_pos = index_erase_begin;

    // Remove the in-between blocks first.
    delete_element_blocks(index_erase_begin, index_erase_end);
    m_block_store.erase(index_erase_begin, index_erase_end - index_erase_begin);

    // Insert the new data block.

    m_block_store.insert(insert_pos, data_blk.position, data_blk.size, data_blk.element_block);
    return get_iterator(insert_pos);
}

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set_cells_to_multi_blocks_block1_non_empty(
    size_type start_row, size_type end_row, size_type block_index1, size_type block_index2, const T& it_begin,
    const T& it_end)
{
    size_type start_row_in_block1 = m_block_store.positions[block_index1];
    size_type start_row_in_block2 = m_block_store.positions[block_index2];

    element_category_type cat = mdds_mtv_get_element_type(*it_begin);
    element_block_type* blk1_data = m_block_store.element_blocks[block_index1];
    assert(blk1_data);
    element_category_type blk_cat1 = mdds::mtv::get_block_type(*blk1_data);

    if (blk_cat1 == cat)
    {
        size_type length = std::distance(it_begin, it_end);
        size_type offset = start_row - start_row_in_block1;
        size_type end_row_in_block2 = start_row_in_block2 + m_block_store.sizes[block_index2] - 1;

        // Initially set to erase blocks between block 1 and block 2 non-inclusive at either end.
        size_type index_erase_begin = block_index1 + 1;
        size_type index_erase_end = block_index2;

        // Extend the first block to store the new data set.

        // Shrink it first to remove the old values, then append new values.
        block_funcs::overwrite_values(*blk1_data, offset, m_block_store.sizes[block_index1] - offset);
        block_funcs::resize_block(*blk1_data, offset);
        mdds_mtv_append_values(*blk1_data, *it_begin, it_begin, it_end);
        m_block_store.sizes[block_index1] = offset + length;

        element_block_type* blk2_data = m_block_store.element_blocks[block_index2];

        if (end_row == end_row_in_block2)
        {
            // Data overlaps the entire block 2. Erase it.
            ++index_erase_end;
        }
        else if (blk2_data)
        {
            element_category_type blk_cat2 = mdds::mtv::get_block_type(*blk2_data);

            if (blk_cat2 == cat)
            {
                // Copy the lower (non-overwritten) part of block 2 to block 1,
                // and remove the whole block 2. Resize block 2 to zero first to
                // prevent the transferred / overwritten cells from being
                // deleted on block deletion.
                size_type data_length = end_row_in_block2 - end_row;
                size_type begin_pos = end_row - start_row_in_block2 + 1;
                block_funcs::append_values_from_block(*blk1_data, *blk2_data, begin_pos, data_length);
                block_funcs::overwrite_values(*blk2_data, 0, begin_pos);
                block_funcs::resize_block(*blk2_data, 0);
                m_block_store.sizes[block_index1] += data_length;
                ++index_erase_end;
            }
            else
            {
                // Erase the upper part of block 2.
                size_type size_to_erase = end_row - start_row_in_block2 + 1;
                block_funcs::erase(*blk2_data, 0, size_to_erase);
                m_block_store.sizes[block_index2] -= size_to_erase;
                m_block_store.positions[block_index2] += size_to_erase;
            }
        }
        else
        {
            // Last block is empty.
            size_type size_to_erase = end_row - start_row_in_block2 + 1;
            m_block_store.sizes[block_index2] -= size_to_erase;
            m_block_store.positions[block_index2] += size_to_erase;
        }

        delete_element_blocks(index_erase_begin, index_erase_end);
        m_block_store.erase(index_erase_begin, index_erase_end - index_erase_begin);

        return get_iterator(block_index1);
    }

    // The first block type is different.
    assert(blk_cat1 != cat);

    return set_cells_to_multi_blocks_block1_non_equal(start_row, end_row, block_index1, block_index2, it_begin, it_end);
}

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set_cell_to_empty_block(
    size_type block_index, size_type pos_in_block, const T& cell)
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
                return begin();
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
                return begin();
            }

            if (size_type& blk_size = m_block_store.sizes[block_index]; pos_in_block == blk_size - 1)
            {
                // Insert into the last cell in block.
                blk_size -= 1;
                assert(blk_size > 0);

                m_block_store.positions.push_back(blk_size);
                m_block_store.sizes.push_back(1);
                m_block_store.element_blocks.push_back(nullptr);

                create_new_block_with_new_cell(block_index + 1, cell);
                iterator ret = end();
                return --ret;
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

            return begin();
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
                m_block_store.sizes[block_index + 1] += 1;
                m_block_store.positions[block_index + 1] -= 1;
                mdds_mtv_prepend_value(*m_block_store.element_blocks[block_index + 1], cell);
            }
            else
            {
                // Shrink the current empty block by one, and create a new block of size 1 to store the new value.
                m_block_store.sizes[block_index] -= 1;
                m_block_store.insert(block_index + 1, 1);
                m_block_store.calc_block_position(block_index + 1);
                m_block_store.sizes[block_index + 1] = 1;

                create_new_block_with_new_cell(block_index + 1, cell);
            }

            return get_iterator(block_index + 1);
        }

        // Inserting into the middle of an empty block.
        return set_cell_to_middle_of_block(block_index, pos_in_block, cell);
    }

    // This empty block is right below a non-empty block.
    assert(block_index > 0 && m_block_store.element_blocks[block_index - 1] != nullptr);

    if (pos_in_block == 0)
    {
        // Set the value to the top of the block, right below a non-empty block.
        element_category_type blk_cat_prev = mdds::mtv::get_block_type(*m_block_store.element_blocks[block_index - 1]);
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
                    append_cell_to_block(block_index - 1, cell);
                }
                else
                {
                    // Block exists below.
                    bool blk_next = is_next_block_of_type(block_index, blk_cat_prev);
                    if (blk_next)
                    {
                        // Empty block must not be followed by another empty block.
                        assert(m_block_store.element_blocks[block_index + 1]);

                        // We need to merge the previous and next blocks, then
                        // delete the current and next blocks.  Be sure to
                        // resize the next block to zero to prevent the
                        // transferred cells to be deleted.

                        // Check if the next block is bigger.
                        if (m_block_store.sizes[block_index - 1] < m_block_store.sizes[block_index + 1])
                        {
                            // Prepend the new item to the next block, then
                            // prepend the content of the previous block and
                            // release both previous and current blocks.

                            size_type position = m_block_store.positions[block_index - 1];

                            element_block_type* data = m_block_store.element_blocks[block_index];
                            element_block_type* prev_data = m_block_store.element_blocks[block_index - 1];
                            element_block_type* next_data = m_block_store.element_blocks[block_index + 1];

                            // Increase the size of block and prepend the new cell
                            m_block_store.sizes[block_index + 1] += 1;
                            mdds_mtv_prepend_value(*next_data, cell);

                            // Preprend the content of previous block to the next block.
                            size_type prev_size = m_block_store.sizes[block_index - 1];
                            block_funcs::prepend_values_from_block(*next_data, *prev_data, 0, prev_size);
                            m_block_store.sizes[block_index + 1] += prev_size;
                            m_block_store.positions[block_index + 1] = position;

                            // Resize the previous block to zero
                            block_funcs::resize_block(*prev_data, 0);
                            m_hdl_event.element_block_released(prev_data);

                            // Release both blocks which are no longer used
                            block_funcs::delete_block(data);
                            block_funcs::delete_block(prev_data);

                            // Remove the previous and current blocks.
                            m_block_store.erase(block_index - 1, 2);
                        }
                        else
                        {
                            // Be sure to resize the next block to zero to prevent the
                            // transferred cells to be deleted.
                            m_block_store.sizes[block_index - 1] += 1 + m_block_store.sizes[block_index + 1];
                            element_block_type* data = m_block_store.element_blocks[block_index];
                            element_block_type* data_prev = m_block_store.element_blocks[block_index - 1];
                            element_block_type* data_next = m_block_store.element_blocks[block_index + 1];
                            mdds_mtv_append_value(*data_prev, cell);
                            block_funcs::append_block(*data_prev, *data_next);
                            block_funcs::resize_block(*data_next, 0);
                            m_hdl_event.element_block_released(data_next);
                            block_funcs::delete_block(data);
                            block_funcs::delete_block(data_next);
                            m_block_store.erase(block_index, 2);
                        }
                    }
                    else
                    {
                        // Ignore the next block. Just extend the previous block.
                        delete_element_block(block_index);
                        m_block_store.erase(block_index);
                        append_cell_to_block(block_index - 1, cell);
                    }
                }
            }
            else
            {
                // Extend the previous block to append the cell.
                assert(m_block_store.sizes[block_index] > 1);
                m_block_store.sizes[block_index] -= 1;
                m_block_store.positions[block_index] += 1;
                append_cell_to_block(block_index - 1, cell);
            }

            return get_iterator(block_index - 1);
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
                        m_block_store.sizes[block_index + 1] += 1;
                        m_block_store.positions[block_index + 1] -= 1;
                        mdds_mtv_prepend_value(*m_block_store.element_blocks[block_index + 1], cell);
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
                m_block_store.insert(block_index + 1, new_block_position, new_block_size, nullptr);
            }

            return get_iterator(block_index);
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
            m_block_store.calc_block_position(block_index + 1);
            create_new_block_with_new_cell(block_index + 1, cell);

            iterator it = end();
            return --it;
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
                m_block_store.sizes[block_index + 1] += 1;
                m_block_store.positions[block_index + 1] -= 1;
                mdds_mtv_prepend_value(*m_block_store.element_blocks[block_index + 1], cell);
            }
            else
            {
                // t|???|  x|---|b - Shrink this block by one and insert a new block for the new cell.
                m_block_store.sizes[block_index] -= 1;
                m_block_store.insert(block_index + 1, 0, 1, nullptr);
                m_block_store.calc_block_position(block_index + 1);
                create_new_block_with_new_cell(block_index + 1, cell);
            }

            return get_iterator(block_index + 1);
        }
    }

    // New cell is somewhere in the middle of an empty block.
    return set_cell_to_middle_of_block(block_index, pos_in_block, cell);
}

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set_cell_to_non_empty_block_of_size_one(
    size_type block_index, const T& cell)
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
            return begin();
        }

        // There is a block below.
        bool blk_next = is_next_block_of_type(block_index, cat);
        if (!blk_next)
        {
            // t|x|---|b - Next block is of different type.
            create_new_block_with_new_cell(block_index, cell);
            return begin();
        }

        // t|x|xxx|b - Delete this block and prepend the cell to the next block.
        m_block_store.sizes[block_index + 1] += 1;
        m_block_store.positions[block_index + 1] -= 1;
        mdds_mtv_prepend_value(*m_block_store.element_blocks[block_index + 1], cell);
        delete_element_block(block_index);
        m_block_store.erase(block_index);

        return begin();
    }

    assert(block_index > 0);

    if (block_index == m_block_store.positions.size() - 1)
    {
        // t|???|x|b - This is the last block and another block exists above.
        element_block_type* prev_data = m_block_store.element_blocks[block_index - 1];
        if (!prev_data || mdds::mtv::get_block_type(*prev_data) != cat)
        {
            // t|---|x|b - The previous block is of different type.
            create_new_block_with_new_cell(block_index, cell);
        }
        else
        {
            // t|xxx|x|b - Append the cell to the previous block and remove the current one.
            mdds_mtv_append_value(*m_block_store.element_blocks[block_index - 1], cell);
            m_block_store.sizes[block_index - 1] += 1;
            delete_element_block(block_index);
            m_block_store.erase(block_index);
        }

        iterator itr = end();
        return --itr;
    }

    // Remove the current block, and check if the cell can be append to the
    // previous block, or prepended to the following block. Also check if the
    // blocks above and below need to be combined.

    if (!m_block_store.element_blocks[block_index - 1])
    {
        // t|   |x|???|b - Previous block is empty.
        if (!m_block_store.element_blocks[block_index + 1])
        {
            // t|   |x|   |b - Next block is empty too.
            create_new_block_with_new_cell(block_index, cell);
            return get_iterator(block_index);
        }

        // Previous block is empty, but the next block is not.
        element_category_type blk_cat_next = mdds::mtv::get_block_type(*m_block_store.element_blocks[block_index + 1]);

        if (blk_cat_next == cat)
        {
            // t|   |x|xxx|b - Next block is of the same type as the new value.
            delete_element_block(block_index);
            m_block_store.erase(block_index);
            m_block_store.sizes[block_index] += 1;
            m_block_store.positions[block_index] -= 1;
            mdds_mtv_prepend_value(*m_block_store.element_blocks[block_index], cell);
            return get_iterator(block_index);
        }

        // t|   |x|---|b
        assert(blk_cat_next != cat);
        create_new_block_with_new_cell(block_index, cell);
        return get_iterator(block_index);
    }

    if (!m_block_store.element_blocks[block_index + 1])
    {
        // t|---|x|   |b - Next block is empty and the previous block is not.
        element_block_type* prev_data = m_block_store.element_blocks[block_index - 1];
        assert(prev_data);
        element_category_type prev_cat = mdds::mtv::get_block_type(*prev_data);

        if (prev_cat == cat)
        {
            // t|xxx|x|   |b - Append to the previous block.
            m_block_store.sizes[block_index - 1] += 1;
            mdds_mtv_append_value(*prev_data, cell);
            delete_element_block(block_index);
            m_block_store.erase(block_index);
            return get_iterator(block_index - 1);
        }

        // Just overwrite the current block.
        create_new_block_with_new_cell(block_index, cell);
        return get_iterator(block_index);
    }

    // t|???|x|???|b - Neither previous nor next blocks are empty.
    element_block_type* prev_data = m_block_store.element_blocks[block_index - 1];
    element_block_type* next_data = m_block_store.element_blocks[block_index + 1];
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
            m_block_store.sizes[block_index - 1] += 1 + m_block_store.sizes[block_index + 1];
            mdds_mtv_append_value(*prev_data, cell);
            block_funcs::append_block(*prev_data, *next_data);
            block_funcs::resize_block(*next_data, 0); // to prevent deletion of managed cells on block deletion

            // Delete the current and next blocks.
            delete_element_block(block_index);
            delete_element_block(block_index + 1);
            m_block_store.erase(block_index, 2);

            return get_iterator(block_index - 1);
        }

        // t|---|x|---|b - Just overwrite the current block.
        create_new_block_with_new_cell(block_index, cell);
        return get_iterator(block_index);
    }

    assert(prev_cat != next_cat);

    if (prev_cat == cat)
    {
        // t|xxx|x|---|b - Append to the previous block.
        m_block_store.sizes[block_index - 1] += 1;
        mdds_mtv_append_value(*m_block_store.element_blocks[block_index - 1], cell);
        delete_element_block(block_index);
        m_block_store.erase(block_index);
        return get_iterator(block_index - 1);
    }

    if (next_cat == cat)
    {
        // t|---|x|xxx|b - Prepend to the next block.
        m_block_store.sizes[block_index + 1] += 1;
        m_block_store.positions[block_index + 1] -= 1;
        mdds_mtv_prepend_value(*m_block_store.element_blocks[block_index + 1], cell);
        delete_element_block(block_index);
        m_block_store.erase(block_index);
        return get_iterator(block_index);
    }

    // t|---|x|+++|b - Just overwrite the current block.
    create_new_block_with_new_cell(block_index, cell);
    return get_iterator(block_index);
}

template<typename Traits>
typename multi_type_vector<Traits>::size_type multi_type_vector<Traits>::size() const
{
    MDDS_MTV_TRACE(accessor);

    return m_cur_size;
}

template<typename Traits>
typename multi_type_vector<Traits>::size_type multi_type_vector<Traits>::block_size() const
{
    MDDS_MTV_TRACE(accessor);

    return m_block_store.positions.size();
}

template<typename Traits>
bool multi_type_vector<Traits>::empty() const
{
    MDDS_MTV_TRACE(accessor);

    return m_block_store.positions.empty();
}

template<typename Traits>
template<typename T>
void multi_type_vector<Traits>::get(size_type pos, T& value) const
{
    MDDS_MTV_TRACE_ARGS(accessor, "pos=" << pos << "; value=? (type=" << mdds_mtv_get_element_type(value) << ")");
    get_impl(pos, value);
}

template<typename Traits>
template<typename T>
T multi_type_vector<Traits>::get(size_type pos) const
{
    MDDS_MTV_TRACE_ARGS(accessor, "pos=" << pos);

    T cell;
    get_impl(pos, cell);
    return cell;
}

template<typename Traits>
template<typename T>
T multi_type_vector<Traits>::release(size_type pos)
{
    MDDS_MTV_TRACE_ARGS(mutator, "pos=" << pos);

    size_type block_index = get_block_position(pos);
    if (block_index == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
            "multi_type_vector::release", __LINE__, pos, block_size(), size());

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_prev_block;
    dump_blocks(os_prev_block);
#endif

    T value;
    release_impl(pos, block_index, value);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    try
    {
        check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        os << "block integrity check failed in release(pos=" << pos << ")" << std::endl;
        os << "previous block state:" << std::endl;
        os << os_prev_block.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif

    return value;
}

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::release(size_type pos, T& value)
{
    MDDS_MTV_TRACE_ARGS(mutator, "pos=" << pos << "; value=? (type=" << mdds_mtv_get_element_type(value) << ")");

    size_type block_index = get_block_position(pos);
    if (block_index == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
            "multi_type_vector::release", __LINE__, pos, block_size(), size());

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_prev_block;
    dump_blocks(os_prev_block);
#endif

    auto ret = release_impl(pos, block_index, value);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    try
    {
        check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        os << "block integrity check failed in release(pos=" << pos << ")" << std::endl;
        os << "previous block state:" << std::endl;
        os << os_prev_block.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif

    return ret;
}

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::release(
    const iterator& pos_hint, size_type pos, T& value)
{
    MDDS_MTV_TRACE_ARGS(
        mutator_with_pos_hint,
        "pos_hint=" << pos_hint << "; pos=" << pos << "; value=? (type=" << mdds_mtv_get_element_type(value) << ")");

    size_type block_index = get_block_position(pos_hint->__private_data, pos);
    if (block_index == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
            "multi_type_vector::release", __LINE__, pos, block_size(), size());

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_prev_block;
    dump_blocks(os_prev_block);
#endif

    auto ret = release_impl(pos, block_index, value);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    try
    {
        check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        os << "block integrity check failed in release(pos=" << pos << ")" << std::endl;
        os << "previous block state:" << std::endl;
        os << os_prev_block.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif

    return ret;
}

template<typename Traits>
void multi_type_vector<Traits>::release()
{
    MDDS_MTV_TRACE(mutator);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    std::ostringstream os_prev_block;
    dump_blocks(os_prev_block);
#endif

    for (auto* data : m_block_store.element_blocks)
    {
        if (!data)
            continue;

        block_funcs::resize_block(*data, 0);
        m_hdl_event.element_block_released(data);
        block_funcs::delete_block(data);
    }

    m_block_store.clear();
    m_cur_size = 0;

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    try
    {
        check_block_integrity();
    }
    catch (const mdds::integrity_error& e)
    {
        std::ostringstream os;
        os << e.what() << std::endl;
        os << "block integrity check failed in release()" << std::endl;
        os << "previous block state:" << std::endl;
        os << os_prev_block.str();
        std::cerr << os.str() << std::endl;
        abort();
    }
#endif
}

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::release_range(
    size_type start_pos, size_type end_pos)
{
    MDDS_MTV_TRACE_ARGS(mutator, "start_pos=" << start_pos << "; end_pos=" << end_pos);

    size_type block_index1 = get_block_position(start_pos);
    if (block_index1 == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
            "multi_type_vector::release_range", __LINE__, start_pos, block_size(), size());

    return set_empty_impl(start_pos, end_pos, block_index1, false);
}

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::release_range(
    const iterator& pos_hint, size_type start_pos, size_type end_pos)
{
    MDDS_MTV_TRACE_ARGS(
        mutator_with_pos_hint, "pos_hint=" << pos_hint << "; start_pos=" << start_pos << "; end_pos=" << end_pos);

    size_type block_index1 = get_block_position(pos_hint->__private_data, start_pos);
    if (block_index1 == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
            "multi_type_vector::release_range", __LINE__, start_pos, block_size(), size());

    return set_empty_impl(start_pos, end_pos, block_index1, false);
}

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::begin()
{
    MDDS_MTV_TRACE(accessor);

    return iterator(
        {m_block_store.positions.begin(), m_block_store.sizes.begin(), m_block_store.element_blocks.begin()},
        {m_block_store.positions.end(), m_block_store.sizes.end(), m_block_store.element_blocks.end()}, this, 0);
}

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::end()
{
    MDDS_MTV_TRACE(accessor);

    return iterator(
        {m_block_store.positions.end(), m_block_store.sizes.end(), m_block_store.element_blocks.end()},
        {m_block_store.positions.end(), m_block_store.sizes.end(), m_block_store.element_blocks.end()}, this,
        m_block_store.positions.size());
}

template<typename Traits>
typename multi_type_vector<Traits>::const_iterator multi_type_vector<Traits>::begin() const
{
    MDDS_MTV_TRACE(accessor);

    return cbegin();
}

template<typename Traits>
typename multi_type_vector<Traits>::const_iterator multi_type_vector<Traits>::end() const
{
    MDDS_MTV_TRACE(accessor);

    return cend();
}

template<typename Traits>
typename multi_type_vector<Traits>::const_iterator multi_type_vector<Traits>::cbegin() const
{
    MDDS_MTV_TRACE(accessor);

    return const_iterator(
        {m_block_store.positions.cbegin(), m_block_store.sizes.cbegin(), m_block_store.element_blocks.cbegin()},
        {m_block_store.positions.cend(), m_block_store.sizes.cend(), m_block_store.element_blocks.cend()}, this, 0);
}

template<typename Traits>
typename multi_type_vector<Traits>::const_iterator multi_type_vector<Traits>::cend() const
{
    MDDS_MTV_TRACE(accessor);

    return const_iterator(
        {m_block_store.positions.cend(), m_block_store.sizes.cend(), m_block_store.element_blocks.cend()},
        {m_block_store.positions.cend(), m_block_store.sizes.cend(), m_block_store.element_blocks.cend()}, this,
        m_block_store.positions.size());
}

template<typename Traits>
typename multi_type_vector<Traits>::reverse_iterator multi_type_vector<Traits>::rbegin()
{
    MDDS_MTV_TRACE(accessor);

    return reverse_iterator(
        {m_block_store.positions.rbegin(), m_block_store.sizes.rbegin(), m_block_store.element_blocks.rbegin()},
        {m_block_store.positions.rend(), m_block_store.sizes.rend(), m_block_store.element_blocks.rend()}, this, 0);
}

template<typename Traits>
typename multi_type_vector<Traits>::reverse_iterator multi_type_vector<Traits>::rend()
{
    MDDS_MTV_TRACE(accessor);

    return reverse_iterator(
        {m_block_store.positions.rend(), m_block_store.sizes.rend(), m_block_store.element_blocks.rend()},
        {m_block_store.positions.rend(), m_block_store.sizes.rend(), m_block_store.element_blocks.rend()}, this, 0);
}

template<typename Traits>
typename multi_type_vector<Traits>::const_reverse_iterator multi_type_vector<Traits>::rbegin() const
{
    MDDS_MTV_TRACE(accessor);

    return crbegin();
}

template<typename Traits>
typename multi_type_vector<Traits>::const_reverse_iterator multi_type_vector<Traits>::rend() const
{
    MDDS_MTV_TRACE(accessor);

    return crend();
}

template<typename Traits>
typename multi_type_vector<Traits>::const_reverse_iterator multi_type_vector<Traits>::crbegin() const
{
    MDDS_MTV_TRACE(accessor);

    return const_reverse_iterator(
        {m_block_store.positions.rbegin(), m_block_store.sizes.rbegin(), m_block_store.element_blocks.rbegin()},
        {m_block_store.positions.rend(), m_block_store.sizes.rend(), m_block_store.element_blocks.rend()}, this, 0);
}

template<typename Traits>
typename multi_type_vector<Traits>::const_reverse_iterator multi_type_vector<Traits>::crend() const
{
    MDDS_MTV_TRACE(accessor);

    return const_reverse_iterator(
        {m_block_store.positions.rend(), m_block_store.sizes.rend(), m_block_store.element_blocks.rend()},
        {m_block_store.positions.rend(), m_block_store.sizes.rend(), m_block_store.element_blocks.rend()}, this, 0);
}

template<typename Traits>
typename multi_type_vector<Traits>::size_type multi_type_vector<Traits>::get_block_position(
    size_type row, size_type start_block_index) const
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

template<typename Traits>
typename multi_type_vector<Traits>::size_type multi_type_vector<Traits>::get_block_position(
    const typename value_type::private_data& pos_data, size_type row) const
{
    size_type block_index = 0;
    if (pos_data.parent == this && pos_data.block_index < m_block_store.positions.size())
        block_index = pos_data.block_index;

    size_type start_row = m_block_store.positions[block_index];

    if (row < start_row)
    {
        // Position hint is past the insertion position.
        // Walk back if that seems efficient.
        if (row > start_row / 2)
        {
            for (size_type i = block_index; i > 0;)
            {
                --i;
                start_row = m_block_store.positions[i];
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

template<typename Traits>
template<typename T>
void multi_type_vector<Traits>::create_new_block_with_new_cell(size_type block_index, const T& cell)
{
    element_block_type* data = m_block_store.element_blocks[block_index];
    if (data)
    {
        m_hdl_event.element_block_released(data);
        block_funcs::delete_block(data);
    }

    // New cell block with size 1.
    data = mdds_mtv_create_new_block(1, cell);
    if (!data)
        throw general_error("Failed to create new block.");

    m_hdl_event.element_block_acquired(data);

    m_block_store.element_blocks[block_index] = data;
}

template<typename Traits>
template<typename T>
void multi_type_vector<Traits>::append_cell_to_block(size_type block_index, const T& cell)
{
    m_block_store.sizes[block_index] += 1;
    mdds_mtv_append_value(*m_block_store.element_blocks[block_index], cell);
}

template<typename Traits>
template<typename T>
bool multi_type_vector<Traits>::append_to_prev_block(
    size_type block_index, element_category_type cat, size_type length, const T& it_begin, const T& it_end)
{
    bool blk_prev = is_previous_block_of_type(block_index, cat);
    if (!blk_prev)
        return false;

    // Append to the previous block.
    mdds_mtv_append_values(*m_block_store.element_blocks[block_index - 1], *it_begin, it_begin, it_end);
    m_block_store.sizes[block_index - 1] += length;
    return true;
}

template<typename Traits>
template<typename T>
void multi_type_vector<Traits>::insert_cells_to_middle(
    size_type row, size_type block_index, const T& it_begin, const T& it_end)
{
    size_type start_row = m_block_store.positions[block_index];
    size_type length = std::distance(it_begin, it_end);
    element_category_type cat = mdds_mtv_get_element_type(*it_begin);
    element_block_type* blk_data = m_block_store.element_blocks[block_index];

    // Insert two new blocks after the specified block position.
    size_type n1 = row - start_row;
    size_type n2 = m_block_store.sizes[block_index] - n1;
    m_block_store.insert(block_index + 1, 2u);

    m_block_store.sizes[block_index] = n1;
    m_block_store.sizes[block_index + 1] = length;
    m_block_store.sizes[block_index + 2] = n2;
    m_block_store.calc_block_position(block_index + 1);
    m_block_store.calc_block_position(block_index + 2);

    // block for data series.
    m_block_store.element_blocks[block_index + 1] = block_funcs::create_new_block(cat, 0);
    element_block_type* blk2_data = m_block_store.element_blocks[block_index + 1];
    m_hdl_event.element_block_acquired(blk2_data);
    mdds_mtv_assign_values(*blk2_data, *it_begin, it_begin, it_end);

    if (blk_data)
    {
        element_category_type blk_cat = mdds::mtv::get_block_type(*blk_data);

        // block to hold data from the lower part of the existing block.
        m_block_store.element_blocks[block_index + 2] = block_funcs::create_new_block(blk_cat, 0);
        element_block_type* blk3_data = m_block_store.element_blocks[block_index + 2];
        m_hdl_event.element_block_acquired(blk3_data);

        // Transfer the lower part of the current block to the new block.
        size_type offset = row - start_row;
        block_funcs::assign_values_from_block(*blk3_data, *blk_data, offset, n2);
        block_funcs::resize_block(*blk_data, m_block_store.sizes[block_index]);
    }

    adjust_block_positions_func{}(m_block_store, block_index + 3, length);
}

template<typename Traits>
template<typename T>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::set_cell_to_middle_of_block(
    size_type block_index, size_type pos_in_block, const T& cell)
{
    block_index = set_new_block_to_middle(block_index, pos_in_block, 1, true);
    create_new_block_with_new_cell(block_index, cell);

    // Return the iterator referencing the inserted block.
    return get_iterator(block_index);
}

template<typename Traits>
template<typename T>
void multi_type_vector<Traits>::set_cell_to_top_of_data_block(size_type block_index, const T& cell)
{
    // t|---|x--|???|b

    m_block_store.sizes[block_index] -= 1;
    size_type position = m_block_store.positions[block_index];
    m_block_store.positions[block_index] += 1;

    element_block_type* data = m_block_store.element_blocks[block_index];
    if (data)
    {
        block_funcs::overwrite_values(*data, 0, 1);
        block_funcs::erase(*data, 0);
    }

    m_block_store.insert(block_index, position, 1, nullptr);
    create_new_block_with_new_cell(block_index, cell);
}

template<typename Traits>
template<typename T>
void multi_type_vector<Traits>::set_cell_to_bottom_of_data_block(size_type block_index, const T& cell)
{
    // Erase the last value of the block.
    assert(block_index < m_block_store.positions.size());
    element_block_type* blk_data = m_block_store.element_blocks[block_index];
    size_type& blk_size = m_block_store.sizes[block_index];
    if (blk_data)
    {
        block_funcs::overwrite_values(*blk_data, blk_size - 1, 1);
        block_funcs::erase(*blk_data, blk_size - 1);
    }
    blk_size -= 1;

    // Insert a new block of size one with the new value.
    m_block_store.insert(block_index + 1, 0, 1, nullptr);
    m_block_store.calc_block_position(block_index + 1);
    create_new_block_with_new_cell(block_index + 1, cell);
}

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::transfer_impl(
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
    if (block_index2 == m_block_store.positions.size())
        mdds::mtv::detail::throw_block_position_not_found(
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

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::transfer_single_block(
    size_type start_pos, size_type end_pos, size_type block_index1, multi_type_vector& dest, size_type dest_pos)
{
    size_type len = end_pos - start_pos + 1;
    size_type last_dest_pos = dest_pos + len - 1;

    // All elements are in the same block.
    element_block_type* src_data = m_block_store.element_blocks[block_index1];
    size_type start_pos_in_block1 = m_block_store.positions[block_index1];

    // Empty the region in the destination instance where the source elements
    // are to be transferred to. This also ensures that the destination region
    // consists of a single block.
    iterator it_dest_blk = dest.set_empty(dest_pos, last_dest_pos);

    if (!src_data)
        return get_iterator(block_index1);

    element_category_type cat = get_block_type(*src_data);

    size_type dest_block_index = it_dest_blk->__private_data.block_index;
    element_block_type* dst_data = dest.m_block_store.element_blocks[dest_block_index];

    size_type dest_pos_in_block = dest_pos - it_dest_blk->position;
    if (dest_pos_in_block == 0)
    {
        // Copy to the top part of the destination block.

        assert(!dst_data); // should be already emptied.
        size_type dst_size = dest.m_block_store.sizes[dest_block_index];
        if (len < dst_size)
        {
            // Shrink the existing block and insert a new block before it.
            size_type position = dest.m_block_store.positions[dest_block_index];
            dest.m_block_store.positions[dest_block_index] += len;
            dest.m_block_store.sizes[dest_block_index] -= len;
            dest.m_block_store.insert(dest_block_index, position, len, nullptr);
        }
    }
    else if (dest_pos_in_block + len - 1 == it_dest_blk->size - 1)
    {
        // Copy to the bottom part of destination block.

        // Insert a new block below current, and shrink the current block.
        dest.m_block_store.sizes[dest_block_index] -= len;
        dest.m_block_store.insert(dest_block_index + 1, 0, len, nullptr);
        dest.m_block_store.calc_block_position(dest_block_index + 1);
        ++dest_block_index; // Must point to the new copied block.
    }
    else
    {
        // Copy to the middle of the destination block.

        // Insert two new blocks below current.
        size_type blk2_size = dest.m_block_store.sizes[dest_block_index] - dest_pos_in_block - len;
        dest.m_block_store.insert(dest_block_index + 1, 2);
        dest.m_block_store.sizes[dest_block_index] = dest_pos_in_block;
        dest.m_block_store.sizes[dest_block_index + 1] = len;
        dest.m_block_store.sizes[dest_block_index + 2] = blk2_size;

        dest.m_block_store.calc_block_position(dest_block_index + 1);
        dest.m_block_store.calc_block_position(dest_block_index + 2);

        ++dest_block_index; // Must point to the new copied block.
    }

    assert(dest.m_block_store.sizes[dest_block_index] == len);
    size_type offset = start_pos - start_pos_in_block1;
    if (offset == 0 && len == m_block_store.sizes[block_index1])
    {
        // Just move the whole element block.
        dest.m_block_store.element_blocks[dest_block_index] = src_data;
        dest.m_hdl_event.element_block_acquired(src_data);

        m_hdl_event.element_block_released(src_data);
        m_block_store.element_blocks[block_index1] = nullptr;

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

    dest.m_block_store.element_blocks[dest_block_index] = block_funcs::create_new_block(cat, 0);
    dst_data = dest.m_block_store.element_blocks[dest_block_index];
    assert(dst_data);
    dest.m_hdl_event.element_block_acquired(dst_data);

    // Shallow-copy the elements to the destination block.
    block_funcs::assign_values_from_block(*dst_data, *src_data, offset, len);
    dest.merge_with_adjacent_blocks(dest_block_index);

    // Set the source range empty without overwriting the elements.
    return set_empty_in_single_block(start_pos, end_pos, block_index1, false);
}

template<typename Traits>
typename multi_type_vector<Traits>::size_type multi_type_vector<Traits>::merge_with_adjacent_blocks(
    size_type block_index)
{
    assert(!m_block_store.positions.empty());
    assert(block_index < m_block_store.positions.size());

    if (block_index == 0)
    {
        // No previous block.
        merge_with_next_block(block_index);
        return 0;
    }

    size_type size_prev = m_block_store.sizes[block_index - 1];
    element_block_type* prev_data = m_block_store.element_blocks[block_index - 1];
    element_block_type* blk_data = m_block_store.element_blocks[block_index];
    bool has_next = block_index < (m_block_store.element_blocks.size() - 1);
    element_block_type* next_data = has_next ? m_block_store.element_blocks[block_index + 1] : nullptr;

    // Check the previous block.
    if (prev_data)
    {
        // Previous block has data.
        element_category_type cat_prev = mtv::get_block_type(*prev_data);
        if (!blk_data || cat_prev != mtv::get_block_type(*blk_data))
        {
            // Current block is empty or is of different type from the previous one.
            merge_with_next_block(block_index);
            return 0;
        }

        // Previous and current blocks are of the same type.
        if (next_data && cat_prev == get_block_type(*next_data))
        {
            // Merge all three blocks.
            m_block_store.sizes[block_index - 1] +=
                m_block_store.sizes[block_index] + m_block_store.sizes[block_index + 1];
            block_funcs::append_block(*prev_data, *blk_data);
            block_funcs::append_block(*prev_data, *next_data);

            // Avoid overwriting the transferred elements.
            block_funcs::resize_block(*blk_data, 0);
            block_funcs::resize_block(*next_data, 0);

            delete_element_block(block_index);
            delete_element_block(block_index + 1);

            m_block_store.erase(block_index, 2);
            return size_prev;
        }

        // Merge only the previous and current blocks.
        bool merged = merge_with_next_block(block_index - 1);
        if (!merged)
            assert(!"Blocks were not merged!");

        return size_prev;
    }

    assert(!prev_data); // Previous block is empty.

    if (blk_data)
    {
        // Current block is not empty. Check with the next block.
        merge_with_next_block(block_index);
        return 0;
    }

    // Previous and current blocks are both empty.
    assert(!blk_data);

    if (has_next && !next_data)
    {
        // Next block is empty too. Merge all three.
        m_block_store.sizes[block_index - 1] += m_block_store.sizes[block_index] + m_block_store.sizes[block_index + 1];

        m_block_store.erase(block_index, 2);

        return size_prev;
    }

    // Next block is not empty, or does not exist. Merge the current block with the previous one.
    bool merged = merge_with_next_block(block_index - 1);
    if (!merged)
        assert(!"Blocks were not merged!");

    return size_prev;
}

template<typename Traits>
bool multi_type_vector<Traits>::merge_with_next_block(size_type block_index)
{
    assert(!m_block_store.positions.empty());
    assert(block_index < m_block_store.positions.size());

    if (block_index >= m_block_store.positions.size() - 1)
        // No more block below this one.
        return false;

    // Block exists below.
    element_block_type* blk_data = m_block_store.element_blocks[block_index];
    element_block_type* next_data = m_block_store.element_blocks[block_index + 1];

    if (!blk_data)
    {
        // Empty block. Merge only if the next block is also empty.
        if (next_data)
            // Next block is not empty.
            return false;

        // Merge the two blocks.
        m_block_store.sizes[block_index] += m_block_store.sizes[block_index + 1];
        m_block_store.erase(block_index + 1);
        return true;
    }

    if (!next_data)
        return false;

    if (mdds::mtv::get_block_type(*blk_data) != mdds::mtv::get_block_type(*next_data))
        // Block types differ.  Don't merge.
        return false;

    // Merge it with the next block.
    block_funcs::append_block(*blk_data, *next_data);
    block_funcs::resize_block(*next_data, 0);
    m_block_store.sizes[block_index] += m_block_store.sizes[block_index + 1];
    delete_element_block(block_index + 1);
    m_block_store.erase(block_index + 1);
    return true;
}

template<typename Traits>
typename multi_type_vector<Traits>::size_type multi_type_vector<Traits>::set_new_block_to_middle(
    size_type block_index, size_type offset, size_type new_block_size, bool overwrite)
{
    assert(block_index < m_block_store.positions.size());

    // First, insert two new blocks after the current block.
    size_type lower_block_size = m_block_store.sizes[block_index] - offset - new_block_size;
    m_block_store.insert(block_index + 1, 2);
    m_block_store.sizes[block_index + 1] = new_block_size; // empty block.
    m_block_store.sizes[block_index + 2] = lower_block_size;

    element_block_type* blk_data = m_block_store.element_blocks[block_index];
    if (blk_data)
    {
        size_type lower_data_start = offset + new_block_size;
        assert(m_block_store.sizes[block_index + 2] == lower_block_size);
        element_category_type cat = mtv::get_block_type(*blk_data);
        m_block_store.element_blocks[block_index + 2] = block_funcs::create_new_block(cat, 0);
        m_hdl_event.element_block_acquired(m_block_store.element_blocks[block_index + 2]);

        // Try to copy the fewer amount of data to the new non-empty block.
        if (offset > lower_block_size)
        {
            // Keep the upper values in the current block and copy the lower
            // values to the new non-empty block.
            block_funcs::assign_values_from_block(
                *m_block_store.element_blocks[block_index + 2], *blk_data, lower_data_start, lower_block_size);

            if (overwrite)
            {
                // Overwrite cells that will become empty.
                block_funcs::overwrite_values(*blk_data, offset, new_block_size);
            }

            // Shrink the current data block.
            block_funcs::resize_block(*blk_data, offset);
            m_block_store.sizes[block_index] = offset;
            m_block_store.sizes[block_index + 2] = lower_block_size;
        }
        else
        {
            // Keep the lower values in the current block and copy the upper
            // values to the new non-empty block (blk_lower), and swap the two
            // later.
            element_block_type* blk_lower_data = m_block_store.element_blocks[block_index + 2];
            block_funcs::assign_values_from_block(*blk_lower_data, *blk_data, 0, offset);
            m_block_store.sizes[block_index + 2] = offset;

            if (overwrite)
            {
                // Overwrite cells that will become empty.
                block_funcs::overwrite_values(*blk_data, offset, new_block_size);
            }

            // Remove the upper and middle values and push the rest to the top.
            block_funcs::erase(*blk_data, 0, lower_data_start);

            // Set the size of the current block to its new size ( what is after the new block )
            m_block_store.sizes[block_index] = lower_block_size;
            m_block_store.sizes[block_index + 2] = offset;

            // And now let's swap the blocks, while preserving the position of the original block.
            size_type position = m_block_store.positions[block_index];
            m_block_store.swap(block_index, block_index + 2);
            m_block_store.positions[block_index] = position;
        }
    }
    else
    {
        // There is no data, we just need to update the size of the block
        m_block_store.sizes[block_index] = offset;
    }

    // Re-calculate the block positions.
    m_block_store.calc_block_position(block_index + 1);
    m_block_store.calc_block_position(block_index + 2);

    return block_index + 1;
}

template<typename Traits>
bool multi_type_vector<Traits>::is_previous_block_of_type(size_type block_index, element_category_type cat) const
{
    if (block_index == 0)
        // No previous block.
        return false;

    const element_block_type* data = m_block_store.element_blocks[block_index - 1];
    if (data)
        return cat == mdds::mtv::get_block_type(*data);

    return cat == mtv::element_type_empty;
}

template<typename Traits>
bool multi_type_vector<Traits>::is_next_block_of_type(size_type block_index, element_category_type cat) const
{
    if (block_index == m_block_store.positions.size() - 1)
        // No next block.
        return false;

    const element_block_type* data = m_block_store.element_blocks[block_index + 1];
    if (data)
        return cat == mdds::mtv::get_block_type(*data);

    return cat == mtv::element_type_empty;
}

template<typename Traits>
typename multi_type_vector<Traits>::element_block_type* multi_type_vector<Traits>::exchange_elements(
    const element_block_type& src_data, size_type src_offset, size_type dst_index, size_type dst_offset, size_type len)
{
    assert(dst_index < m_block_store.positions.size());
    element_block_type* dst_blk_data = m_block_store.element_blocks[dst_index];
    assert(dst_blk_data);
    size_type dst_blk_size = m_block_store.sizes[dst_index];
    element_category_type cat_src = mtv::get_block_type(src_data);
    bool blk_next = is_next_block_of_type(dst_index, cat_src);

    if (dst_offset == 0)
    {
        // Set elements to the top of the destination block.
        bool blk_prev = is_previous_block_of_type(dst_index, cat_src);

        if (dst_blk_size == len)
        {
            // The whole block will get replaced.
            std::unique_ptr<element_block_type, element_block_deleter> data(dst_blk_data);
            m_hdl_event.element_block_released(dst_blk_data);
            m_block_store.element_blocks[dst_index] =
                nullptr; // Prevent its deletion when the parent block gets deleted.
            dst_blk_data = nullptr;

            if (blk_prev)
            {
                // Append to the previous block. Remove the current block.
                element_block_type* dst_prev_data = m_block_store.element_blocks[dst_index - 1];
                block_funcs::append_values_from_block(*dst_prev_data, src_data, src_offset, len);
                m_block_store.sizes[dst_index - 1] += len;

                size_type dst_erase_size = 1;

                // no need to call delete_block since dst_blk_data is null.

                if (blk_next)
                {
                    // Apend elements from the next block too.
                    element_block_type* dst_next_data = m_block_store.element_blocks[dst_index + 1];
                    block_funcs::append_block(*dst_prev_data, *dst_next_data);
                    m_block_store.sizes[dst_index - 1] += m_block_store.sizes[dst_index + 1];
                    ++dst_erase_size;
                    delete_element_block(dst_index + 1);
                }

                m_block_store.erase(dst_index, dst_erase_size);
                return data.release();
            }

            // Check the next block to see if we need to merge.
            if (blk_next)
            {
                // We need to merge with the next block.  Remove the current
                // block and use the next block to store the new elements as
                // well as the existing ones.
                element_block_type* dst_next_data = m_block_store.element_blocks[dst_index + 1];
                block_funcs::prepend_values_from_block(*dst_next_data, src_data, src_offset, len);
                m_block_store.positions[dst_index + 1] -= len;
                m_block_store.sizes[dst_index + 1] += len;
                m_block_store.erase(dst_index);
            }
            else
            {
                dst_blk_data = block_funcs::create_new_block(cat_src, 0);
                m_block_store.element_blocks[dst_index] = dst_blk_data;
                m_hdl_event.element_block_acquired(dst_blk_data);
                assert(dst_blk_data && dst_blk_data != data.get());
                block_funcs::assign_values_from_block(*dst_blk_data, src_data, src_offset, len);
            }

            // Return this data block as-is.
            return data.release();
        }

        // New block to send back to the caller.
        std::unique_ptr<element_block_type, element_block_deleter> data(nullptr);

        if (dst_blk_data)
        {
            element_category_type cat_dst = mtv::get_block_type(*dst_blk_data);
            data.reset(block_funcs::create_new_block(cat_dst, 0));

            // We need to keep the tail elements of the current block.
            block_funcs::assign_values_from_block(*data, *dst_blk_data, 0, len);
            block_funcs::erase(*dst_blk_data, 0, len);
        }

        size_type position = m_block_store.positions[dst_index];
        m_block_store.positions[dst_index] += len;
        m_block_store.sizes[dst_index] -= len;

        if (blk_prev)
        {
            // Append the new elements to the previous block.
            element_block_type* dst_prev_data = m_block_store.element_blocks[dst_index - 1];
            block_funcs::append_values_from_block(*dst_prev_data, src_data, src_offset, len);
            m_block_store.sizes[dst_index - 1] += len;
        }
        else
        {
            // Insert a new block to house the new elements.
            m_block_store.insert(dst_index, position, len, nullptr);
            dst_blk_data = block_funcs::create_new_block(cat_src, 0);
            m_block_store.element_blocks[dst_index] = dst_blk_data;
            m_hdl_event.element_block_acquired(dst_blk_data);
            block_funcs::assign_values_from_block(*dst_blk_data, src_data, src_offset, len);
        }

        return data.release();
    }

    // New block to send back to the caller.
    std::unique_ptr<element_block_type, element_block_deleter> data(nullptr);

    if (dst_blk_data)
    {
        // Copy the elements of the current block to the block being returned.
        element_category_type cat_dst = mtv::get_block_type(*dst_blk_data);
        data.reset(block_funcs::create_new_block(cat_dst, 0));
        block_funcs::assign_values_from_block(*data, *dst_blk_data, dst_offset, len);
    }

    assert(dst_offset > 0);
    size_type dst_end_pos = dst_offset + len;

    if (dst_end_pos == dst_blk_size)
    {
        // The new elements will replace the lower part of the block.
        assert(dst_blk_data);
        block_funcs::resize_block(*dst_blk_data, dst_offset);
        m_block_store.sizes[dst_index] = dst_offset;

        if (blk_next)
        {
            // Merge with the next block.
            element_block_type* dst_next_data = m_block_store.element_blocks[dst_index + 1];
            block_funcs::prepend_values_from_block(*dst_next_data, src_data, src_offset, len);
            m_block_store.positions[dst_index + 1] -= len;
            m_block_store.sizes[dst_index + 1] += len;
        }
        else
        {
            // Insert a new block to store the new elements.
            size_type position = m_block_store.positions[dst_index] + dst_offset;
            m_block_store.insert(dst_index + 1, position, len, nullptr);
            m_block_store.element_blocks[dst_index + 1] = block_funcs::create_new_block(cat_src, 0);
            dst_blk_data = m_block_store.element_blocks[dst_index + 1];
            assert(dst_blk_data);
            m_hdl_event.element_block_acquired(dst_blk_data);
            block_funcs::assign_values_from_block(*dst_blk_data, src_data, src_offset, len);
        }
    }
    else
    {
        // The new elements will replace the middle of the block.
        assert(dst_end_pos < m_block_store.sizes[dst_index]);
        dst_index = set_new_block_to_middle(dst_index, dst_offset, len, false);
        assert(m_block_store.sizes[dst_index] == len);
        m_block_store.element_blocks[dst_index] = block_funcs::create_new_block(cat_src, 0);
        dst_blk_data = m_block_store.element_blocks[dst_index];
        assert(dst_blk_data);
        m_hdl_event.element_block_acquired(dst_blk_data);
        block_funcs::assign_values_from_block(*dst_blk_data, src_data, src_offset, len);
    }

    return data.release();
}

template<typename Traits>
void multi_type_vector<Traits>::exchange_elements(
    const element_block_type& src_blk, size_type src_offset, size_type dst_index1, size_type dst_offset1,
    size_type dst_index2, size_type dst_offset2, size_type len, blocks_type& new_blocks)
{
    assert(dst_index1 < dst_index2);
    assert(dst_offset1 < m_block_store.sizes[dst_index1]);
    assert(dst_offset2 < m_block_store.sizes[dst_index2]);

    blocks_to_transfer bucket;
    prepare_blocks_to_transfer(bucket, dst_index1, dst_offset1, dst_index2, dst_offset2);

    m_block_store.insert(bucket.insert_index, 0, len, nullptr);
    if (bucket.insert_index > 0)
        m_block_store.calc_block_position(bucket.insert_index);

    m_block_store.element_blocks[bucket.insert_index] = block_funcs::create_new_block(mtv::get_block_type(src_blk), 0);

    element_block_type* blk_data = m_block_store.element_blocks[bucket.insert_index];

    m_hdl_event.element_block_acquired(blk_data);
    block_funcs::assign_values_from_block(*blk_data, src_blk, src_offset, len);
    merge_with_adjacent_blocks(bucket.insert_index);

    new_blocks.swap(bucket.blocks);
}

template<typename Traits>
bool multi_type_vector<Traits>::append_empty(size_type len)
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

template<typename Traits>
void multi_type_vector<Traits>::resize(size_type new_size)
{
    MDDS_MTV_TRACE_ARGS(mutator, "new_size=" << new_size);

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

template<typename Traits>
void multi_type_vector<Traits>::resize_impl(size_type new_size)
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
        mdds::mtv::detail::throw_block_position_not_found(
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
            block_funcs::overwrite_values(*data, new_end_row + 1, end_row_in_block - new_end_row);
            block_funcs::resize_block(*data, new_block_size);
        }
        m_block_store.sizes[block_index] = new_block_size;
    }

    // Remove all blocks below the current one.
    delete_element_blocks(block_index + 1, m_block_store.element_blocks.size());
    size_type len = m_block_store.element_blocks.size() - block_index - 1;
    m_block_store.erase(block_index + 1, len);
    m_cur_size = new_size;
}

template<typename Traits>
typename multi_type_vector<Traits>::iterator multi_type_vector<Traits>::transfer_multi_blocks(
    size_type start_pos, size_type end_pos, size_type block_index1, size_type block_index2, multi_type_vector& dest,
    size_type dest_pos)
{
    assert(block_index1 < block_index2);
    size_type start_pos_in_block1 = m_block_store.positions[block_index1];
    size_type start_pos_in_block2 = m_block_store.positions[block_index2];

    size_type len = end_pos - start_pos + 1;
    size_type last_dest_pos = dest_pos + len - 1;

    // Empty the region in the destination container where the elements
    // are to be transferred to. This ensures that the destination region
    // consists of a single block.
    iterator it_dest_blk = dest.set_empty(dest_pos, last_dest_pos);

    size_type dest_block_index = it_dest_blk->__private_data.block_index;
    size_type dest_pos_in_block = dest_pos - it_dest_blk->position;
    assert(!dest.m_block_store.element_blocks[dest_block_index]); // should be already emptied.

    size_type block_len = block_index2 - block_index1 + 1;

    // Create slots for new blocks in the destination.

    size_type dest_block_index1 = dest_block_index;

    if (dest_pos_in_block == 0)
    {
        // Copy to the top part of destination block.
        if (len < dest.m_block_store.sizes[dest_block_index])
        {
            // Shrink the existing block and insert slots for the new blocks before it.
            dest.m_block_store.sizes[dest_block_index] -= len;
            dest.m_block_store.positions[dest_block_index] += len;
            dest.m_block_store.insert(dest_block_index, block_len);
        }
        else
        {
            // Destination block is exactly of the length of the elements being transferred.
            dest.delete_element_block(dest_block_index);
            dest.m_block_store.sizes[dest_block_index] = 0;
            if (block_len > 1)
                dest.m_block_store.insert(dest_block_index, block_len - 1);
        }
    }
    else if (dest_pos_in_block + len - 1 == it_dest_blk->size - 1)
    {
        // Copy to the bottom part of destination block. Insert slots for new
        // blocks below current, and shrink the current block.
        dest.m_block_store.insert(dest_block_index + 1, block_len);
        dest.m_block_store.sizes[dest_block_index] -= len;

        ++dest_block_index1;
    }
    else
    {
        // Copy to the middle of the destination block. Insert slots for the
        // new blocks (plus one extra for the bottom empty block) below the
        // current block.
        size_type blk2_size = dest.m_block_store.sizes[dest_block_index] - dest_pos_in_block - len;
        dest.m_block_store.insert(dest_block_index + 1, block_len + 1);
        assert(dest.m_block_store.positions.size() > dest_block_index + block_len + 1);
        dest.m_block_store.sizes[dest_block_index] = dest_pos_in_block;

        // Re-calculate the size and position of the lower part of the destination block.
        dest.m_block_store.positions[dest_block_index + block_len + 1] =
            dest.m_block_store.calc_next_block_position(dest_block_index) + len;
        dest.m_block_store.sizes[dest_block_index + block_len + 1] = blk2_size;

        ++dest_block_index1;
    }

    size_type del_index1 = block_index1, del_index2 = block_index2;

    // Now that the new slots have been created, start transferring the blocks.

    // Transfer the first block.
    size_type offset = start_pos - start_pos_in_block1;
    if (offset)
    {
        // Transfer the lower part of the first block.

        assert(dest.m_block_store.sizes[dest_block_index1] == 0);
        dest.m_block_store.sizes[dest_block_index1] = m_block_store.sizes[block_index1] - offset;
        if (dest_block_index1 > 0)
            dest.m_block_store.calc_block_position(dest_block_index1);

        if (m_block_store.element_blocks[block_index1])
        {
            element_block_type* blk_data1 = m_block_store.element_blocks[block_index1];
            element_category_type cat = mtv::get_block_type(*blk_data1);
            dest.m_block_store.element_blocks[dest_block_index1] = block_funcs::create_new_block(cat, 0);
            element_block_type* dst_data1 = dest.m_block_store.element_blocks[dest_block_index1];
            assert(dst_data1);
            dest.m_hdl_event.element_block_acquired(dst_data1);

            // Shallow-copy the elements to the destination block, and shrink
            // the source block to remove the transferred elements.
            block_funcs::assign_values_from_block(
                *dst_data1, *blk_data1, offset, m_block_store.sizes[block_index1] - offset);
            block_funcs::resize_block(*blk_data1, offset);
        }

        m_block_store.sizes[block_index1] = offset;
        ++del_index1; // Retain this block.
    }
    else
    {
        // Just move the whole block over.
        element_block_type* data = m_block_store.element_blocks[block_index1];
        dest.m_block_store.element_blocks[dest_block_index1] = data;
        dest.m_block_store.sizes[dest_block_index1] = m_block_store.sizes[block_index1];
        dest.m_block_store.calc_block_position(dest_block_index1);

        if (data)
        {
            dest.m_hdl_event.element_block_acquired(data);
            m_hdl_event.element_block_released(data);
            m_block_store.element_blocks[block_index1] = nullptr;
        }

        m_block_store.sizes[block_index1] = 0;
    }

    if (block_len > 2)
    {
        size_type position = dest.m_block_store.calc_next_block_position(dest_block_index1);

        for (size_type i = 0; i < block_len - 2; ++i)
        {
            size_type src_block_pos = block_index1 + 1 + i;
            size_type dest_block_pos = dest_block_index1 + 1 + i;
            assert(dest.m_block_store.sizes[dest_block_pos] == 0);

            element_block_type* data = m_block_store.element_blocks[src_block_pos];
            dest.m_block_store.element_blocks[dest_block_pos] = data;
            dest.m_block_store.sizes[dest_block_pos] = m_block_store.sizes[src_block_pos];
            dest.m_block_store.positions[dest_block_pos] = position;
            position += m_block_store.sizes[src_block_pos];
            m_block_store.sizes[src_block_pos] = 0;

            if (data)
            {
                dest.m_hdl_event.element_block_acquired(data);
                m_hdl_event.element_block_released(data);
                m_block_store.element_blocks[src_block_pos] = nullptr;
            }
        }
    }

    // Transfer the last block.
    if (block_len > 1)
    {
        size_type size_to_trans = end_pos - start_pos_in_block2 + 1;
        size_type dest_block_pos = dest_block_index1 + block_len - 1;
        assert(dest.m_block_store.sizes[dest_block_pos] == 0);

        element_block_type* blk_data2 = m_block_store.element_blocks[block_index2];

        if (size_to_trans < m_block_store.sizes[block_index2])
        {
            // Transfer the upper part of this block.
            assert(dest_block_pos > 0);
            dest.m_block_store.calc_block_position(dest_block_pos);
            dest.m_block_store.sizes[dest_block_pos] = size_to_trans;

            if (blk_data2)
            {
                element_category_type cat = mtv::get_block_type(*blk_data2);
                dest.m_block_store.element_blocks[dest_block_pos] = block_funcs::create_new_block(cat, 0);
                element_block_type* blk_dst_data = dest.m_block_store.element_blocks[dest_block_pos];
                dest.m_hdl_event.element_block_acquired(blk_dst_data);

                block_funcs::assign_values_from_block(*blk_dst_data, *blk_data2, 0, size_to_trans);
                block_funcs::erase(*blk_data2, 0, size_to_trans);
            }

            m_block_store.positions[block_index2] += size_to_trans;
            m_block_store.sizes[block_index2] -= size_to_trans;
            --del_index2; // Retain this block.
        }
        else
        {
            // Just move the whole block over.
            dest.m_block_store.sizes[dest_block_pos] = m_block_store.sizes[block_index2];
            dest.m_block_store.element_blocks[dest_block_pos] = m_block_store.element_blocks[block_index2];
            dest.m_block_store.calc_block_position(dest_block_pos);

            if (blk_data2)
            {
                dest.m_hdl_event.element_block_acquired(blk_data2);
                m_hdl_event.element_block_released(blk_data2);
                m_block_store.element_blocks[block_index2] = nullptr;
            }

            m_block_store.sizes[block_index2] = 0;
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

        element_block_type* blk1_data = m_block_store.element_blocks[block_index1];
        element_block_type* blk2_data = m_block_store.element_blocks[block_index2];

        if (!blk1_data)
        {
            assert(blk2_data);

            // Block 1 is empty. Extend this block downward.
            m_block_store.sizes[block_index1] += len;
            return get_iterator(block_index1);
        }

        if (!blk2_data)
        {
            assert(blk1_data);

            // Block 2 is empty. Extend this block upward.
            m_block_store.sizes[block_index2] += len;
            m_block_store.positions[block_index2] -= len;
            return get_iterator(block_index2);
        }

        // Neither block1 nor block2 are empty. Just insert a new empty block
        // between them. After the insertion, the old block2 position becomes
        // the position of the inserted block.
        m_block_store.insert(block_index2, 0, len, nullptr);
        m_block_store.calc_block_position(block_index2);

        // No need to adjust local index vars
        return get_iterator(block_index2);
    }

    if (del_index1 > 0 && !m_block_store.element_blocks[del_index1 - 1])
    {
        // The block before the first block to be deleted is empty. Simply
        // extend that block to cover the deleted block segment.
        m_block_store.sizes[del_index1 - 1] += len;
    }
    else
    {
        // Block before is not empty (or doesn't exist).  Keep the first slot,
        // and erase the rest.
        m_block_store.sizes[del_index1] = len; // Insert an empty
        ++del_index1;
    }

    size_type ret_block_index = del_index1 - 1;

    if (del_index2 >= del_index1)
    {
        size_type n_del_blocks = del_index2 - del_index1 + 1;

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
        for (size_type i = del_index1; i <= del_index2; ++i)
        {
            // All slots to be erased should have zero size
            assert(m_block_store.sizes[i] == 0);
        }
#endif
        m_block_store.erase(del_index1, n_del_blocks);
    }

    // The block pointed to by ret_block_index is guaranteed to be empty by
    // this point.
    assert(!m_block_store.element_blocks[ret_block_index]);

    // Merging with the previous block never happens.
    size_type start_pos_offset = merge_with_adjacent_blocks(ret_block_index);
    (void)start_pos_offset; // avoid unused variable compiler warning.
    assert(!start_pos_offset);

    m_block_store.calc_block_position(ret_block_index);
    return get_iterator(ret_block_index);
}

template<typename Traits>
void multi_type_vector<Traits>::swap(multi_type_vector& other)
{
    MDDS_MTV_TRACE_ARGS(mutator, "other=?");

    std::swap(m_hdl_event, other.m_hdl_event);
    std::swap(m_cur_size, other.m_cur_size);
    m_block_store.swap(other.m_block_store);
}

template<typename Traits>
void multi_type_vector<Traits>::swap(
    size_type start_pos, size_type end_pos, multi_type_vector& other, size_type other_pos)
{
    MDDS_MTV_TRACE_ARGS(
        mutator, "start_pos=" << start_pos << "; end_pos=" << end_pos << "; other=?; other_pos=" << other_pos);

    if (start_pos > end_pos)
        throw std::out_of_range("multi_type_vector::swap: start position is larger than the end position!");

    size_type other_end_pos = other_pos + end_pos - start_pos;

    if (end_pos >= m_cur_size || other_end_pos >= other.m_cur_size)
        throw std::out_of_range("multi_type_vector::swap: end position is out of bound!");

    size_type block_index1 = get_block_position(start_pos);
    if (block_index1 == m_block_store.positions.size())
        throw std::out_of_range("multi_type_vector::swap: start block position in source not found!");

    size_type block_index2 = get_block_position(end_pos, block_index1);
    if (block_index2 == m_block_store.positions.size())
        throw std::out_of_range("multi_type_vector::swap: end block position in source not found!");

    size_type dest_block_index1 = other.get_block_position(other_pos);
    if (dest_block_index1 == other.m_block_store.positions.size())
        throw std::out_of_range("multi_type_vector::swap: start block position in destination not found!");

    size_type dest_block_index2 = other.get_block_position(other_end_pos, dest_block_index1);
    if (dest_block_index2 == other.m_block_store.positions.size())
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

template<typename Traits>
void multi_type_vector<Traits>::shrink_to_fit()
{
    MDDS_MTV_TRACE(mutator);

    for (auto* data : m_block_store.element_blocks)
    {
        if (data)
            block_funcs::shrink_to_fit(*data);
    }
}

template<typename Traits>
bool multi_type_vector<Traits>::operator==(const multi_type_vector& other) const
{
    MDDS_MTV_TRACE_ARGS(accessor, "other=?");

    if (this == &other)
        // Comparing to self is always equal.
        return true;

    if (m_cur_size != other.m_cur_size)
        // Row sizes differ.
        return false;

    return m_block_store.equals(other.m_block_store);
}

template<typename Traits>
bool multi_type_vector<Traits>::operator!=(const multi_type_vector& other) const
{
    MDDS_MTV_TRACE_ARGS(accessor, "other=?");

    return !operator==(other);
}

template<typename Traits>
multi_type_vector<Traits>& multi_type_vector<Traits>::operator=(const multi_type_vector& other)
{
    MDDS_MTV_TRACE_ARGS(mutator, "other=? (copy)");

    multi_type_vector assigned(other);
    swap(assigned);
    return *this;
}

template<typename Traits>
multi_type_vector<Traits>& multi_type_vector<Traits>::operator=(multi_type_vector&& other)
{
    MDDS_MTV_TRACE_ARGS(mutator, "other=? (move)");

    multi_type_vector assigned(std::move(other));
    swap(assigned);
    return *this;
}

template<typename Traits>
template<typename T>
mtv::element_t multi_type_vector<Traits>::get_element_type(const T& elem)
{
    return mdds_mtv_get_element_type(elem);
}

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG

template<typename Traits>
void multi_type_vector<Traits>::dump_blocks(std::ostream& os) const
{
    os << "--- blocks" << std::endl;
    std::ios_base::fmtflags origflags = os.flags();
    for (size_type i = 0, n = m_block_store.positions.size(); i < n; ++i)
    {
        size_type pos = m_block_store.positions[i];
        size_type size = m_block_store.sizes[i];
        const element_block_type* data = m_block_store.element_blocks[i];
        element_category_type cat = mtv::element_type_empty;
        if (data)
            cat = mtv::get_block_type(*data);
        os << "  block " << i << ": position=" << pos << " size=" << size << " type=" << std::dec << cat
           << " data=" << std::hex << data << std::endl;
    }
    os.setf(origflags);
}

template<typename Traits>
void multi_type_vector<Traits>::check_block_integrity() const
{
    m_block_store.check_integrity();

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
    element_category_type cat_prev = data_prev ? mtv::get_block_type(*data_prev) : mtv::element_type_empty;

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
            os << "position of the current block is wrong! (expected=" << cur_position
               << "; actual=" << m_block_store.positions[i] << ")" << std::endl;

            dump_blocks(os);
            throw mdds::integrity_error(os.str());
        }

        element_category_type cat = mtv::element_type_empty;

        const element_block_type* data = m_block_store.element_blocks[i];

        if (data)
        {
            cat = mtv::get_block_type(*data);

            if (block_funcs::size(*data) != this_size)
                throw mdds::integrity_error("block size cache and the actual element block size differ!");
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

}}} // namespace mdds::mtv::soa

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
