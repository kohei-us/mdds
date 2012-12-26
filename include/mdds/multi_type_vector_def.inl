/*************************************************************************
 *
 * Copyright (c) 2012 Kohei Yoshida
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

#include "multi_type_vector_macro.hpp"

#include <stdexcept>

#if UNIT_TEST
#include <iostream>
using std::cout;
using std::endl;
#endif

namespace mdds {

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(double, mtv::element_type_numeric, 0.0, mtv::numeric_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(std::string, mtv::element_type_string, std::string(), mtv::string_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(short, mtv::element_type_short, 0, mtv::short_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(unsigned short, mtv::element_type_ushort, 0, mtv::ushort_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(int, mtv::element_type_int, 0, mtv::int_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(unsigned int, mtv::element_type_uint, 0, mtv::uint_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(long, mtv::element_type_long, 0, mtv::long_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(unsigned long, mtv::element_type_ulong, 0, mtv::ulong_element_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(bool, mtv::element_type_boolean, false, mtv::boolean_element_block)

template<typename _CellBlockFunc>
multi_type_vector<_CellBlockFunc>::block::block() : m_size(0), mp_data(NULL) {}

template<typename _CellBlockFunc>
multi_type_vector<_CellBlockFunc>::block::block(size_type _size) : m_size(_size), mp_data(NULL) {}

template<typename _CellBlockFunc>
multi_type_vector<_CellBlockFunc>::block::block(const block& other) :
    m_size(other.m_size), mp_data(NULL)
{
    if (other.mp_data)
        mp_data = element_block_func::clone_block(*other.mp_data);
}

template<typename _CellBlockFunc>
multi_type_vector<_CellBlockFunc>::block::~block()
{
    element_block_func::delete_block(mp_data);
}

template<typename _CellBlockFunc>
typename multi_type_vector<_CellBlockFunc>::iterator
multi_type_vector<_CellBlockFunc>::begin()
{
    return iterator(m_blocks.begin(), m_blocks.end());
}

template<typename _CellBlockFunc>
typename multi_type_vector<_CellBlockFunc>::iterator
multi_type_vector<_CellBlockFunc>::end()
{
    return iterator(m_blocks.end(), m_blocks.end());
}

template<typename _CellBlockFunc>
typename multi_type_vector<_CellBlockFunc>::const_iterator
multi_type_vector<_CellBlockFunc>::begin() const
{
    return const_iterator(m_blocks.begin(), m_blocks.end());
}

template<typename _CellBlockFunc>
typename multi_type_vector<_CellBlockFunc>::const_iterator
multi_type_vector<_CellBlockFunc>::end() const
{
    return const_iterator(m_blocks.end(), m_blocks.end());
}

template<typename _CellBlockFunc>
typename multi_type_vector<_CellBlockFunc>::const_reverse_iterator
multi_type_vector<_CellBlockFunc>::rbegin() const
{
    return const_reverse_iterator(m_blocks.rbegin(), m_blocks.rend());
}

template<typename _CellBlockFunc>
typename multi_type_vector<_CellBlockFunc>::const_reverse_iterator
multi_type_vector<_CellBlockFunc>::rend() const
{
    return const_reverse_iterator(m_blocks.rend(), m_blocks.rend());
}

template<typename _CellBlockFunc>
multi_type_vector<_CellBlockFunc>::multi_type_vector() : m_cur_size(0) {}

template<typename _CellBlockFunc>
multi_type_vector<_CellBlockFunc>::multi_type_vector(size_type init_size) : m_cur_size(init_size)
{
    if (!init_size)
        return;

    // Initialize with an empty block that spans from 0 to max.
    m_blocks.push_back(new block(init_size));
}

template<typename _CellBlockFunc>
template<typename _T>
multi_type_vector<_CellBlockFunc>::multi_type_vector(size_type init_size, const _T& value) :
    m_cur_size(init_size)
{
    if (!init_size)
        return;

    mdds::unique_ptr<block> blk(new block(init_size));
    blk->mp_data = mdds_mtv_create_new_block(init_size, value);
    m_blocks.push_back(blk.release());
}

template<typename _CellBlockFunc>
multi_type_vector<_CellBlockFunc>::multi_type_vector(const multi_type_vector& other) :
    m_cur_size(other.m_cur_size)
{
    // Clone all the blocks.
    m_blocks.reserve(other.m_blocks.size());
    typename blocks_type::const_iterator it = other.m_blocks.begin(), it_end = other.m_blocks.end();
    for (; it != it_end; ++it)
        m_blocks.push_back(new block(**it));
}

template<typename _CellBlockFunc>
multi_type_vector<_CellBlockFunc>::~multi_type_vector()
{
    std::for_each(m_blocks.begin(), m_blocks.end(), default_deleter<block>());
}

template<typename _CellBlockFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc>::set(size_type pos, const _T& value)
{
    element_category_type cat = mdds_mtv_get_element_type(value);

    // Find the right block ID from the row ID.
    size_type start_row = 0; // row ID of the first cell in a block.
    size_type block_index = 0;
    get_block_position(pos, start_row, block_index);

    block* blk = m_blocks[block_index];
    assert(blk->m_size > 0); // block size should never be zero at any time.

    assert(pos >= start_row);
    size_type pos_in_block = pos - start_row;
    assert(pos_in_block < blk->m_size);

    if (!blk->mp_data)
    {
        // This is an empty block.
        set_cell_to_empty_block(block_index, pos_in_block, value);
        return;
    }

    assert(blk->mp_data);
    element_category_type blk_cat = mdds::mtv::get_block_type(*blk->mp_data);

    if (blk_cat == cat)
    {
        // This block is of the same type as the cell being inserted.
        size_type i = pos - start_row;
        element_block_func::overwrite_values(*blk->mp_data, i, 1);
        mdds_mtv_set_value(*blk->mp_data, i, value);
        return;
    }

    assert(blk_cat != cat);

    if (pos == start_row)
    {
        // Insertion point is at the start of the block.
        if (blk->m_size == 1)
        {
            set_cell_to_block_of_size_one(block_index, value);
            return;
        }

        assert(blk->m_size > 1);
        if (block_index == 0)
        {
            // No preceding block.
            set_cell_to_top_of_data_block(0, value);
            return;
        }

        // Append to the previous block if the types match.
        block* blk_prev = m_blocks[block_index-1];
        if (!blk_prev->mp_data)
        {
            // Previous block is empty.
            set_cell_to_top_of_data_block(block_index, value);
            return;
        }

        element_category_type blk_cat_prev = mdds::mtv::get_block_type(*blk_prev->mp_data);
        if (blk_cat_prev == cat)
        {
            // Append to the previous block.
            blk->m_size -= 1;
            element_block_func::erase(*blk->mp_data, 0);
            blk_prev->m_size += 1;
            mdds_mtv_append_value(*blk_prev->mp_data, value);
            return;
        }

        set_cell_to_top_of_data_block(block_index, value);
        return;
    }

    if (pos < (start_row + blk->m_size - 1))
    {
        // Insertion point is somewhere in the middle of the block.
        set_cell_to_middle_of_block(block_index, pos_in_block, value);
        return;
    }

    // Insertion point is at the end of the block.
    assert(pos == (start_row + blk->m_size - 1));
    assert(pos > start_row);
    assert(blk->m_size > 1);

    if (block_index == 0)
    {
        if (m_blocks.size() == 1)
        {
            // This is the only block.  Pop the last value from the
            // previous block, and insert a new block for the cell being
            // inserted.
            set_cell_to_bottom_of_data_block(0, value);
            return;
        }

        assert(block_index < m_blocks.size()-1);
        block* blk_next = m_blocks[block_index+1];
        if (!blk_next->mp_data)
        {
            // Next block is empty.  Pop the last cell of the current
            // block, and insert a new block with the new cell.
            set_cell_to_bottom_of_data_block(0, value);
            return;
        }

        // Next block is not empty.
        element_category_type blk_cat_next = mdds::mtv::get_block_type(*blk_next->mp_data);
        if (blk_cat_next != cat)
        {
            set_cell_to_bottom_of_data_block(0, value);
            return;
        }

        // Pop the last cell off the current block, and prepend the
        // new cell to the next block.
        element_block_func::erase(*blk->mp_data, blk->m_size-1);
        blk->m_size -= 1;
        mdds_mtv_prepend_value(*blk_next->mp_data, value);
        return;
    }

    assert(block_index > 0);

    if (block_index == m_blocks.size()-1)
    {
        // This is the last block.
        set_cell_to_bottom_of_data_block(block_index, value);
        return;
    }

    block* blk_next = m_blocks[block_index+1];
    if (!blk_next->mp_data)
    {
        // Next block is empty.
        set_cell_to_bottom_of_data_block(block_index, value);
        return;
    }

    element_category_type cat_blk_next = mdds::mtv::get_block_type(*blk_next->mp_data);
    if (cat_blk_next != cat)
    {
        // Next block is of different type than that of the cell being inserted.
        set_cell_to_bottom_of_data_block(block_index, value);
        return;
    }

    // Pop the last element from the current block, and prepend the cell
    // into the next block.
    element_block_func::erase(*blk->mp_data, blk->m_size-1);
    blk->m_size -= 1;
    mdds_mtv_prepend_value(*blk_next->mp_data, value);
    blk_next->m_size += 1;
}

template<typename _CellBlockFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc>::set(size_type pos, const _T& it_begin, const _T& it_end)
{
    set_cells_impl(pos, it_begin, it_end);
}

template<typename _CellBlockFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc>::insert(size_type pos, const _T& it_begin, const _T& it_end)
{
    insert_cells_impl(pos, it_begin, it_end);
}

template<typename _CellBlockFunc>
void multi_type_vector<_CellBlockFunc>::get_block_position(
    size_type row, size_type& start_row, size_type& block_index, size_type start_block, size_type start_block_row) const
{
    start_row = start_block_row;
    for (size_type i = start_block, n = m_blocks.size(); i < n; ++i)
    {
        const block& blk = *m_blocks[i];
        if (row < start_row + blk.m_size)
        {
            // Row is in this block.
            block_index = i;
            return;
        }

        // Specified row is not in this block.
        start_row += blk.m_size;
    }

    assert(!"Block position not found.");
}

template<typename _CellBlockFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc>::create_new_block_with_new_cell(element_block_type*& data, const _T& cell)
{
    if (data)
        element_block_func::delete_block(data);

    // New cell block with size 1.
    data = mdds_mtv_create_new_block(1, cell);
    if (!data)
        throw general_error("Failed to create new block.");
}

template<typename _CellBlockFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc>::set_cell_to_middle_of_block(
    size_type block_index, size_type pos_in_block, const _T& cell)
{
    block* blk = m_blocks[block_index];

    assert(pos_in_block > 0 && pos_in_block < blk->m_size - 1);
    assert(blk->m_size >= 3);
    size_type orig_size = blk->m_size;

    m_blocks.insert(m_blocks.begin()+block_index+1, new block(1));
    block* blk_new = m_blocks[block_index+1];
    m_blocks.insert(m_blocks.begin()+block_index+2, new block(orig_size-pos_in_block-1));
    block* blk_tail = m_blocks[block_index+2];

    if (blk->mp_data)
    {
        element_category_type blk_cat = mdds::mtv::get_block_type(*blk->mp_data);

        // Transfer the tail values from the original to the new block.
        blk_tail->mp_data = element_block_func::create_new_block(blk_cat, 0);
        element_block_func::assign_values_from_block(
            *blk_tail->mp_data, *blk->mp_data, pos_in_block+1, orig_size-pos_in_block-1);

        // Overwrite the cell and shrink the original block.
        element_block_func::overwrite_values(*blk->mp_data, pos_in_block, 1);
        element_block_func::resize_block(*blk->mp_data, pos_in_block);
    }

    blk->m_size = pos_in_block;

    create_new_block_with_new_cell(blk_new->mp_data, cell);
}

template<typename _CellBlockFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc>::append_cell_to_block(size_type block_index, const _T& cell)
{
    block* blk = m_blocks[block_index];
    blk->m_size += 1;
    mdds_mtv_append_value(*blk->mp_data, cell);
}

template<typename _CellBlockFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc>::set_cell_to_empty_block(
    size_type block_index, size_type pos_in_block, const _T& cell)
{
    block* blk = m_blocks[block_index];

    if (block_index == 0)
    {
        // Topmost block.
        if (m_blocks.size() == 1)
        {
            // this is the only block.
            assert(blk->m_size == m_cur_size);
            if (m_cur_size == 1)
            {
                // This column is allowed to have only one row!
                assert(pos_in_block == 0);
                create_new_block_with_new_cell(blk->mp_data, cell);
            }
            else
            {
                // block has multiple rows.
                if (pos_in_block == 0)
                {
                    // Insert into the first cell in block.
                    blk->m_size -= 1;
                    assert(blk->m_size > 0);

                    m_blocks.insert(m_blocks.begin(), new block(1));
                    blk = m_blocks[block_index];
                    create_new_block_with_new_cell(blk->mp_data, cell);
                }
                else if (pos_in_block == blk->m_size - 1)
                {
                    // Insert into the last cell in block.
                    blk->m_size -= 1;
                    assert(blk->m_size > 0);

                    m_blocks.push_back(new block(1));
                    blk = m_blocks.back();

                    create_new_block_with_new_cell(blk->mp_data, cell);
                }
                else
                {
                    // Insert into the middle of the block.
                    set_cell_to_middle_of_block(block_index, pos_in_block, cell);
                }
            }
        }
        else
        {
            // this empty block is followed by a non-empty block.
            assert(block_index < m_blocks.size()-1);
            if (pos_in_block == 0)
            {
                if (blk->m_size == 1)
                {
                    // Top empty block with only one cell size.
                    block* blk_next = m_blocks[block_index+1];
                    assert(blk_next->mp_data);
                    element_category_type cat = mdds_mtv_get_element_type(cell);
                    element_category_type cat_next = mdds::mtv::get_block_type(*blk_next->mp_data);

                    if (cat == cat_next)
                    {
                        // Remove this one-cell empty block from the top, and
                        // prepend the cell to the next block.
                        delete m_blocks.front();
                        m_blocks.erase(m_blocks.begin());
                        blk = m_blocks.front();
                        blk->m_size += 1;
                        mdds_mtv_prepend_value(*blk->mp_data, cell);
                    }
                    else
                        create_new_block_with_new_cell(blk->mp_data, cell);
                }
                else
                {
                    assert(blk->m_size > 1);
                    blk->m_size -= 1;
                    m_blocks.insert(m_blocks.begin(), new block(1));
                    blk = m_blocks.front();
                    create_new_block_with_new_cell(blk->mp_data, cell);
                }
            }
            else if (pos_in_block == blk->m_size - 1)
            {
                // Immediately above a non-empty block.
                block* blk_next = m_blocks[block_index+1];
                assert(blk_next->mp_data);
                element_category_type cat = mdds_mtv_get_element_type(cell);
                element_category_type cat_next = mdds::mtv::get_block_type(*blk_next->mp_data);
                assert(blk->m_size > 1);

                if (cat == cat_next)
                {
                    // Shrink this empty block by one, and prepend the cell to the next block.
                    blk->m_size -= 1;
                    blk_next->m_size += 1;
                    mdds_mtv_prepend_value(*blk_next->mp_data, cell);
                }
                else
                {
                    blk->m_size -= 1;
                    typename blocks_type::iterator it = m_blocks.begin();
                    std::advance(it, block_index+1);
                    m_blocks.insert(it, new block(1));
                    blk = m_blocks[block_index+1];
                    create_new_block_with_new_cell(blk->mp_data, cell);
                }
            }
            else
            {
                // Inserting into the middle of an empty block.
                set_cell_to_middle_of_block(block_index, pos_in_block, cell);
            }
        }

        return;
    }

    // This empty block is right below a non-empty block.
    assert(block_index > 0 && m_blocks[block_index-1]->mp_data != NULL);

    if (pos_in_block == 0)
    {
        // New cell is right below the non-empty block.
        element_category_type blk_cat_prev = mdds::mtv::get_block_type(*m_blocks[block_index-1]->mp_data);
        element_category_type cat = mdds_mtv_get_element_type(cell);
        if (blk_cat_prev == cat)
        {
            // Extend the previous block by one to insert this cell.
            if (blk->m_size == 1)
            {
                // Check if we need to merge with the following block.
                if (block_index == m_blocks.size()-1)
                {
                    // Last block.  Delete this block and extend the previous
                    // block by one.
                    delete m_blocks[block_index];
                    m_blocks.pop_back();
                    append_cell_to_block(block_index-1, cell);
                }
                else
                {
                    // Block exists below.
                    block* blk_next = m_blocks[block_index+1];
                    element_block_type* data_next = blk_next->mp_data;
                    assert(data_next); // Empty block must not be followed by another empty block.
                    element_category_type blk_cat_next = mdds::mtv::get_block_type(*data_next);
                    if (blk_cat_prev == blk_cat_next)
                    {
                        // We need to merge the previous and next blocks, then
                        // delete the current and next blocks.  Be sure to
                        // resize the next block to zero to prevent the
                        // transferred cells to be deleted.
                        block* blk_prev = m_blocks[block_index-1];
                        blk_prev->m_size += 1 + blk_next->m_size;
                        mdds_mtv_append_value(*blk_prev->mp_data, cell);
                        element_block_func::append_values_from_block(*blk_prev->mp_data, *data_next);
                        element_block_func::resize_block(*data_next, 0);

                        delete blk;
                        delete blk_next;
                        typename blocks_type::iterator it = m_blocks.begin() + block_index;
                        m_blocks.erase(it, it+2);
                    }
                    else
                    {
                        // Ignore the next block. Just extend the previous block.
                        delete m_blocks[block_index];
                        m_blocks.erase(m_blocks.begin() + block_index);
                        append_cell_to_block(block_index-1, cell);
                    }
                }
            }
            else
            {
                // Extend the previous block to append the cell.
                assert(blk->m_size > 1);
                blk->m_size -= 1;
                append_cell_to_block(block_index-1, cell);
            }
        }
        else
        {
            // Cell type is different from the type of the previous block.
            if (blk->m_size == 1)
            {
                if (block_index == m_blocks.size()-1)
                {
                    // There is no more block below.
                    create_new_block_with_new_cell(blk->mp_data, cell);
                }
                else
                {
                    // Check the type of the following non-empty block.
                    assert(block_index < m_blocks.size()-1);
                    block* blk_next = m_blocks[block_index+1];
                    assert(blk_next->mp_data);
                    element_category_type blk_cat_next = mdds::mtv::get_block_type(*blk_next->mp_data);
                    if (cat == blk_cat_next)
                    {
                        // Remove this empty block, and prepend the cell to the next block.
                        blk_next->m_size += 1;
                        mdds_mtv_prepend_value(*blk_next->mp_data, cell);
                        delete m_blocks[block_index];
                        m_blocks.erase(m_blocks.begin()+block_index);
                    }
                    else
                        create_new_block_with_new_cell(blk->mp_data, cell);
                }
            }
            else
            {
                // Replace the current empty block of size > 1 with a
                // non-empty block of size 1, and insert a new empty block
                // below whose size is one shorter than the current empty
                // block.
                create_new_block_with_new_cell(blk->mp_data, cell);
                m_blocks.insert(m_blocks.begin()+block_index+1, new block(blk->m_size-1));
                blk->m_size = 1;
            }
        }
    }
    else if (pos_in_block == blk->m_size - 1)
    {
        // New cell is at the last cell position.
        assert(blk->m_size > 1);
        if (block_index == m_blocks.size()-1)
        {
            // This is the last block.
            blk->m_size -= 1;
            m_blocks.push_back(new block(1));
            blk = m_blocks.back();
            create_new_block_with_new_cell(blk->mp_data, cell);
        }
        else
        {
            // A non-empty block exists below.
            element_category_type cat = mdds_mtv_get_element_type(cell);
            block* blk_next = m_blocks[block_index+1];
            assert(blk_next->mp_data);
            element_category_type blk_cat_next = mdds::mtv::get_block_type(*blk_next->mp_data);
            if (cat == blk_cat_next)
            {
                // Shrink this empty block and extend the next block.
                blk->m_size -= 1;
                blk_next->m_size += 1;
                mdds_mtv_prepend_value(*blk_next->mp_data, cell);
            }
            else
            {
                // Just insert this new cell.
                blk->m_size -= 1;
                m_blocks.insert(m_blocks.begin()+1, new block(1));
                blk = m_blocks[block_index+1];
                create_new_block_with_new_cell(blk->mp_data, cell);
            }
        }
    }
    else
    {
        // New cell is somewhere in the middle of an empty block.
        set_cell_to_middle_of_block(block_index, pos_in_block, cell);
    }
}

template<typename _CellBlockFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc>::set_cell_to_block_of_size_one(size_type block_index, const _T& cell)
{
    block* blk = m_blocks[block_index];
    assert(blk->m_size == 1);
    assert(blk->mp_data);
    element_category_type cat = mdds_mtv_get_element_type(cell);
    assert(mdds::mtv::get_block_type(*blk->mp_data) != cat);

    if (block_index == 0)
    {
        // This is the topmost block of size 1.
        if (block_index == m_blocks.size()-1)
        {
            // This is the only block.
            create_new_block_with_new_cell(blk->mp_data, cell);
            return;
        }

        // There is an existing block below.
        block* blk_next = m_blocks[block_index+1];
        if (!blk_next->mp_data)
        {
            // Next block is empty.
            create_new_block_with_new_cell(blk->mp_data, cell);
            return;
        }

        // Next block is not empty.
        element_category_type blk_cat_next = mdds::mtv::get_block_type(*blk_next->mp_data);
        if (blk_cat_next != cat)
        {
            // Cell being inserted is of different type than that of the next block.
            create_new_block_with_new_cell(blk->mp_data, cell);
            return;
        }

        // Delete the current block, and prepend the cell to the next block.
        blk_next->m_size += 1;
        mdds_mtv_prepend_value(*blk_next->mp_data, cell);
        delete blk;
        m_blocks.erase(m_blocks.begin()+block_index);
        return;
    }

    assert(block_index > 0);

    if (block_index == m_blocks.size()-1)
    {
        // This is the last block, and a block exists above.
        block* blk_prev = m_blocks[block_index-1];
        if (!blk_prev->mp_data)
        {
            // Previous block is empty. Replace the current block with a new one.
            create_new_block_with_new_cell(blk->mp_data, cell);
            return;
        }

        element_category_type blk_cat_prev = mdds::mtv::get_block_type(*blk_prev->mp_data);
        if (blk_cat_prev == cat)
        {
            // Append the cell to the previos block, and remove the
            // current block.
            mdds_mtv_append_value(*blk_prev->mp_data, cell);
            blk_prev->m_size += 1;
            delete blk;
            m_blocks.erase(m_blocks.begin()+block_index);
            return;
        }

        // Simply replace the current block with a new block of new type.
        create_new_block_with_new_cell(blk->mp_data, cell);
        return;
    }

    // Remove the current block, and check if the cell can be append
    // to the previous block, or prepended to the following block.
    // Also check if the blocks above and below need to be combined.

    block* blk_prev = m_blocks[block_index-1];
    block* blk_next = m_blocks[block_index+1];
    if (!blk_prev->mp_data)
    {
        // Previous block is empty.
        if (!blk_next->mp_data)
        {
            // Next block is empty too.
            create_new_block_with_new_cell(blk->mp_data, cell);
            return;
        }

        // Previous block is empty, but the next block is not.
        element_category_type blk_cat_next = mdds::mtv::get_block_type(*blk_next->mp_data);
        if (blk_cat_next == cat)
        {
            // Delete the current block, and prepend the new cell to the next block.
            delete blk;
            m_blocks.erase(m_blocks.begin()+block_index);
            blk = m_blocks[block_index];
            blk->m_size += 1;
            mdds_mtv_prepend_value(*blk->mp_data, cell);
            return;
        }

        assert(blk_cat_next != cat);
        create_new_block_with_new_cell(blk->mp_data, cell);
        return;
    }

    if (!blk_next->mp_data)
    {
        // Next block is empty.
        assert(blk_prev->mp_data);
        element_category_type blk_cat_prev = mdds::mtv::get_block_type(*blk_prev->mp_data);
        if (blk_cat_prev == cat)
        {
            // Append to the previous block.
            blk_prev->m_size += 1;
            mdds_mtv_append_value(*blk_prev->mp_data, cell);
            delete blk;
            m_blocks.erase(m_blocks.begin()+block_index);
            return;
        }

        // Just overwrite the current block.
        create_new_block_with_new_cell(blk->mp_data, cell);
        return;
    }

    assert(blk_prev && blk_prev->mp_data);
    assert(blk_next && blk_next->mp_data);
    element_category_type blk_cat_prev = mdds::mtv::get_block_type(*blk_prev->mp_data);
    element_category_type blk_cat_next = mdds::mtv::get_block_type(*blk_next->mp_data);

    if (blk_cat_prev == blk_cat_next)
    {
        if (blk_cat_prev == cat)
        {
            // Merge the previous block with the cell being inserted and
            // the next block.  Resize the next block to zero to prevent
            // deletion of mananged cells on block deletion.
            blk_prev->m_size += 1 + blk_next->m_size;
            mdds_mtv_append_value(*blk_prev->mp_data, cell);
            element_block_func::append_values_from_block(*blk_prev->mp_data, *blk_next->mp_data);
            element_block_func::resize_block(*blk_next->mp_data, 0);

            // Delete the current and next blocks.
            delete blk;
            delete blk_next;
            typename blocks_type::iterator it = m_blocks.begin() + block_index;
            typename blocks_type::iterator it_last = it + 2;
            m_blocks.erase(it, it_last);
            return;
        }

        // Just overwrite the current block.
        create_new_block_with_new_cell(blk->mp_data, cell);
        return;
    }

    assert(blk_cat_prev != blk_cat_next);

    if (blk_cat_prev == cat)
    {
        // Append to the previous block.
        blk_prev->m_size += 1;
        mdds_mtv_append_value(*blk_prev->mp_data, cell);
        delete blk;
        m_blocks.erase(m_blocks.begin()+block_index);
        return;
    }

    if (blk_cat_next == cat)
    {
        // Prepend to the next block.
        blk_next->m_size += 1;
        mdds_mtv_prepend_value(*blk_next->mp_data, cell);
        delete blk;
        m_blocks.erase(m_blocks.begin()+block_index);
        return;
    }

    // Just overwrite the current block.
    create_new_block_with_new_cell(blk->mp_data, cell);
}

template<typename _CellBlockFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc>::set_cell_to_top_of_data_block(size_type block_index, const _T& cell)
{
    block* blk = m_blocks[block_index];
    blk->m_size -= 1;
    if (blk->mp_data)
        element_block_func::erase(*blk->mp_data, 0);
    m_blocks.insert(m_blocks.begin()+block_index, new block(1));
    blk = m_blocks[block_index];
    create_new_block_with_new_cell(blk->mp_data, cell);
}

template<typename _CellBlockFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc>::set_cell_to_bottom_of_data_block(size_type block_index, const _T& cell)
{
    assert(block_index < m_blocks.size());
    block* blk = m_blocks[block_index];
    if (blk->mp_data)
        element_block_func::erase(*blk->mp_data, blk->m_size-1);
    blk->m_size -= 1;
    m_blocks.insert(m_blocks.begin()+block_index+1, new block(1));
    blk = m_blocks[block_index+1];
    create_new_block_with_new_cell(blk->mp_data, cell);
}

template<typename _CellBlockFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc>::get(size_type pos, _T& value) const
{
    size_type start_row = 0;
    size_type block_index = static_cast<size_type>(-1);
    get_block_position(pos, start_row, block_index);
    const block* blk = m_blocks[block_index];
    assert(blk);

    if (!blk->mp_data)
    {
        // empty cell block.
        mdds_mtv_get_empty_value(value);
        return;
    }

    assert(pos >= start_row);
    assert(blk->mp_data); // data for non-empty blocks should never be NULL.
    size_type idx = pos - start_row;
    mdds_mtv_get_value(*blk->mp_data, idx, value);
}

template<typename _CellBlockFunc>
template<typename _T>
_T multi_type_vector<_CellBlockFunc>::get(size_type pos) const
{
    _T cell;
    get(pos, cell);
    return cell;
}

template<typename _CellBlockFunc>
mtv::element_t multi_type_vector<_CellBlockFunc>::get_type(size_type pos) const
{
    size_type start_row = 0;
    size_type block_index = static_cast<size_type>(-1);
    get_block_position(pos, start_row, block_index);
    const block* blk = m_blocks[block_index];
    if (!blk->mp_data)
        return mtv::element_type_empty;

    return mtv::get_block_type(*blk->mp_data);
}

template<typename _CellBlockFunc>
bool multi_type_vector<_CellBlockFunc>::is_empty(size_type pos) const
{
    size_type start_row = 0;
    size_type block_index = 0;
    get_block_position(pos, start_row, block_index);

    return m_blocks[block_index]->mp_data == NULL;
}

template<typename _CellBlockFunc>
void multi_type_vector<_CellBlockFunc>::set_empty(size_type start_pos, size_type end_pos)
{
    if (start_pos > end_pos)
        throw std::out_of_range("Start row is larger than the end row.");

    size_type start_row_in_block1 = 0, start_row_in_block2 = 0;
    size_type block_pos1 = 0, block_pos2 = 0;
    get_block_position(start_pos, start_row_in_block1, block_pos1);
    get_block_position(end_pos, start_row_in_block2, block_pos2, block_pos1, start_row_in_block1);

    if (block_pos1 == block_pos2)
    {
        set_empty_in_single_block(start_pos, end_pos, block_pos1, start_row_in_block1);
        return;
    }

    set_empty_in_multi_blocks(
        start_pos, end_pos, block_pos1, start_row_in_block1, block_pos2, start_row_in_block2);
}

template<typename _CellBlockFunc>
void multi_type_vector<_CellBlockFunc>::erase(size_type start_pos, size_type end_pos)
{
    if (start_pos > end_pos)
        throw std::out_of_range("Start row is larger than the end row.");

    erase_impl(start_pos, end_pos);
}

template<typename _CellBlockFunc>
void multi_type_vector<_CellBlockFunc>::erase_impl(size_type start_row, size_type end_row)
{
    assert(start_row <= end_row);

    // Keep the logic similar to set_empty().

    size_type start_row_in_block1 = 0, start_row_in_block2 = 0;
    size_type block_pos1 = 0, block_pos2 = 0;
    get_block_position(start_row, start_row_in_block1, block_pos1);
    get_block_position(end_row, start_row_in_block2, block_pos2, block_pos1, start_row_in_block1);

    if (block_pos1 == block_pos2)
    {
        // Range falls within the same block.
        block* blk = m_blocks[block_pos1];
        size_type size_to_erase = end_row - start_row + 1;
        if (blk->mp_data)
        {
            // Erase data in the data block.
            size_type offset = start_row - start_row_in_block1;
            element_block_func::overwrite_values(*blk->mp_data, offset, size_to_erase);
            element_block_func::erase(*blk->mp_data, offset, size_to_erase);
        }

        blk->m_size -= size_to_erase;
        m_cur_size -= size_to_erase;

        if (blk->m_size == 0)
        {
            delete blk;
            m_blocks.erase(m_blocks.begin()+block_pos1);

            if (block_pos1 > 0 && block_pos1 < m_blocks.size())
            {
                // Check the previous and next blocks to see if they should be merged.
                block* blk_prev = m_blocks[block_pos1-1];
                block* blk_next = m_blocks[block_pos1];
                if (blk_prev->mp_data)
                {
                    // Previous block has data.
                    if (!blk_next->mp_data)
                        // Next block is empty.  Nothing to do.
                        return;

                    element_category_type cat1 = mdds::mtv::get_block_type(*blk_prev->mp_data);
                    element_category_type cat2 = mdds::mtv::get_block_type(*blk_next->mp_data);
                    if (cat1 == cat2)
                    {
                        // Merge the two blocks.
                        element_block_func::append_values_from_block(*blk_prev->mp_data, *blk_next->mp_data);
                        blk_prev->m_size += blk_next->m_size;
                        // Resize to 0 to prevent deletion of cells in case of managed cells.
                        element_block_func::resize_block(*blk_next->mp_data, 0);
                        delete blk_next;
                        m_blocks.erase(m_blocks.begin()+block_pos1);
                    }
                }
                else
                {
                    // Previous block is empty.
                    if (blk_next->mp_data)
                        // Next block is not empty.  Nothing to do.
                        return;

                    // Both blocks are empty.  Simply increase the size of the
                    // previous block.
                    blk_prev->m_size += blk_next->m_size;
                    delete blk_next;
                    m_blocks.erase(m_blocks.begin()+block_pos1);
                }
            }
        }
        return;
    }

    assert(block_pos1 < block_pos2);

    // Initially, we set to erase all blocks between the first and the last.
    typename blocks_type::iterator it_erase_begin = m_blocks.begin() + block_pos1 + 1;
    typename blocks_type::iterator it_erase_end   = m_blocks.begin() + block_pos2;

    // First, inspect the first block.
    if (start_row_in_block1 == start_row)
    {
        // Erase the whole block.
        --it_erase_begin;
    }
    else
    {
        // Erase the lower part of the first block.
        block* blk = m_blocks[block_pos1];
        size_type new_size = start_row - start_row_in_block1;
        if (blk->mp_data)
        {
            // Shrink the data array.
            element_block_func::overwrite_values(*blk->mp_data, new_size, blk->m_size-new_size);
            element_block_func::resize_block(*blk->mp_data, new_size);
        }
        blk->m_size = new_size;
    }

    // Then inspect the last block.
    block* blk = m_blocks[block_pos2];
    size_type last_row_in_block = start_row_in_block2 + blk->m_size - 1;
    if (last_row_in_block == end_row)
    {
        // Delete the whole block.
        ++it_erase_end;
    }
    else
    {
        size_type size_to_erase = end_row - start_row_in_block2 + 1;
        blk->m_size -= size_to_erase;
        if (blk->mp_data)
        {
            // Erase the upper part.
            element_block_func::overwrite_values(*blk->mp_data, 0, size_to_erase);
            element_block_func::erase(*blk->mp_data, 0, size_to_erase);
        }
    }

    // Now, erase all blocks in between.
    std::for_each(it_erase_begin, it_erase_end, default_deleter<block>());
    m_blocks.erase(it_erase_begin, it_erase_end);
    m_cur_size -= end_row - start_row + 1;
}

template<typename _CellBlockFunc>
void multi_type_vector<_CellBlockFunc>::insert_empty(size_type pos, size_type length)
{
    if (!length)
        // Nothing to insert.
        return;

    insert_empty_impl(pos, length);
}

template<typename _CellBlockFunc>
void multi_type_vector<_CellBlockFunc>::insert_empty_impl(size_type row, size_type length)
{
    assert(row < m_cur_size);

    size_type start_row = 0, block_index = 0;
    get_block_position(row, start_row, block_index);

    block* blk = m_blocks[block_index];
    if (!blk->mp_data)
    {
        // Insertion point is already empty.  Just expand its size and be done
        // with it.
        blk->m_size += length;
        m_cur_size += length;
        return;
    }

    if (start_row == row)
    {
        // Insertion point is at the top of an existing non-empty block.
        if (block_index > 0)
        {
            block* blk_prev = m_blocks[block_index-1];
            if (!blk_prev->mp_data)
            {
                // Previous block is empty.  Expand the size of the previous
                // block and bail out.
                blk_prev->m_size += length;
                m_cur_size += length;
                return;
            }
        }

        // Insert a new empty block.
        m_blocks.insert(m_blocks.begin()+block_index, new block(length));
        m_cur_size += length;
        return;
    }

    assert(blk->mp_data);
    assert(row > start_row);

    size_type size_blk_prev = row - start_row;
    size_type size_blk_next = blk->m_size - size_blk_prev;

    // Insert two new block below the current; one for the empty block being
    // inserted, and one for the lower part of the current non-empty block.
    m_blocks.insert(m_blocks.begin()+block_index+1, 2u, NULL);

    m_blocks[block_index+1] = new block(length);
    m_blocks[block_index+2] = new block(size_blk_next);

    block* blk_next = m_blocks[block_index+2];
    blk_next->mp_data = element_block_func::create_new_block(mdds::mtv::get_block_type(*blk->mp_data), 0);
    element_block_func::assign_values_from_block(*blk_next->mp_data, *blk->mp_data, size_blk_prev, size_blk_next);

    element_block_func::resize_block(*blk->mp_data, size_blk_prev);
    blk->m_size = size_blk_prev;

    m_cur_size += length;
}

template<typename _CellBlockFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc>::set_cells_impl(size_type row, const _T& it_begin, const _T& it_end)
{
    size_type length = std::distance(it_begin, it_end);
    if (!length)
        // empty data array.  nothing to do.
        return;

    size_type end_row = row + length - 1;
    if (end_row >= m_cur_size)
        throw std::out_of_range("Data array is too long.");

    size_t block_index1 = 0, block_index2 = 0, start_row1 = 0, start_row2 = 0;
    get_block_position(row, start_row1, block_index1);
    get_block_position(end_row, start_row2, block_index2, block_index1, start_row1);

    if (block_index1 == block_index2)
    {
        // The whole data array will fit in a single block.
        set_cells_to_single_block(row, end_row, block_index1, start_row1, it_begin, it_end);
        return;
    }

    set_cells_to_multi_blocks(
        row, end_row, block_index1, start_row1, block_index2, start_row2, it_begin, it_end);
}

template<typename _CellBlockFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc>::insert_cells_impl(size_type row, const _T& it_begin, const _T& it_end)
{
    size_type length = std::distance(it_begin, it_end);
    if (!length)
        // empty data array.  nothing to do.
        return;

    size_type block_index = 0, start_row = 0;
    get_block_position(row, start_row, block_index);

    element_category_type cat = mdds_mtv_get_element_type(*it_begin);
    block* blk = m_blocks[block_index];
    if (!blk->mp_data)
    {
        if (row == start_row)
        {
            // Insert into an empty block.  Check the previos block (if
            // exists) to see if the data can be appended to it if inserting
            // at the top of the block.
            if (block_index > 0)
            {
                block* blk0 = m_blocks[block_index-1];
                assert(blk0->mp_data);
                element_category_type blk_cat0 = mdds::mtv::get_block_type(*blk0->mp_data);
                if (blk_cat0 == cat)
                {
                    // Append to the previous block.
                    mdds_mtv_append_values(*blk0->mp_data, *it_begin, it_begin, it_end);
                    blk0->m_size += length;
                    m_cur_size += length;
                    return;
                }
            }

            // Just insert a new block before the current block.
            m_blocks.insert(m_blocks.begin()+block_index, new block(length));
            blk = m_blocks[block_index];
            blk->mp_data = element_block_func::create_new_block(cat, 0);
            mdds_mtv_assign_values(*blk->mp_data, *it_begin, it_begin, it_end);
            blk->m_size = length;
            m_cur_size += length;
            return;
        }

        insert_cells_to_middle(row, block_index, start_row, it_begin, it_end);
        m_cur_size += length;
        return;
    }

    assert(blk->mp_data);
    element_category_type blk_cat = mdds::mtv::get_block_type(*blk->mp_data);
    if (cat == blk_cat)
    {
        // Simply insert the new data series into existing block.
        assert(it_begin != it_end);
        mdds_mtv_insert_values(*blk->mp_data, row-start_row, *it_begin, it_begin, it_end);
        blk->m_size += length;
        m_cur_size += length;
        return;
    }

    assert(cat != blk_cat);
    if (row == start_row)
    {
        if (block_index > 0)
        {
            // Check the previous block to see if we can append the data there.
            block* blk0 = m_blocks[block_index-1];
            if (blk0->mp_data)
            {
                element_category_type blk_cat0 = mdds::mtv::get_block_type(*blk0->mp_data);
                if (cat == blk_cat0)
                {
                    // Append to the previous block.
                    mdds_mtv_append_values(*blk0->mp_data, *it_begin, it_begin, it_end);
                    blk0->m_size += length;
                    m_cur_size += length;
                    return;
                }
            }
        }

        // Just insert a new block before the current block.
        m_blocks.insert(m_blocks.begin()+block_index, new block(length));
        blk = m_blocks[block_index];
        blk->mp_data = element_block_func::create_new_block(cat, 0);
        mdds_mtv_assign_values(*blk->mp_data, *it_begin, it_begin, it_end);
        blk->m_size = length;
        m_cur_size += length;
        return;
    }

    insert_cells_to_middle(row, block_index, start_row, it_begin, it_end);
    m_cur_size += length;
}

template<typename _CellBlockFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc>::insert_cells_to_middle(
    size_type row, size_type block_index, size_type start_row,
    const _T& it_begin, const _T& it_end)
{
    size_type length = std::distance(it_begin, it_end);
    block* blk = m_blocks[block_index];
    element_category_type cat = mdds_mtv_get_element_type(*it_begin);

    // Insert two new blocks.
    size_type n1 = row - start_row;
    size_type n2 = blk->m_size - n1;
    m_blocks.insert(m_blocks.begin()+block_index+1, 2u, NULL);
    blk->m_size = n1;

    m_blocks[block_index+1] = new block(length);
    m_blocks[block_index+2] = new block(n2);

    // block for data series.
    block* blk2 = m_blocks[block_index+1];
    blk2->mp_data = element_block_func::create_new_block(cat, 0);
    mdds_mtv_assign_values(*blk2->mp_data, *it_begin, it_begin, it_end);

    if (blk->mp_data)
    {
        element_category_type blk_cat = mdds::mtv::get_block_type(*blk->mp_data);

        // block to hold data from the lower part of the existing block.
        block* blk3 = m_blocks[block_index+2];
        blk3->mp_data = element_block_func::create_new_block(blk_cat, 0);

        // Transfer the lower part of the current block to the new block.
        size_type offset = row - start_row;
        element_block_func::assign_values_from_block(*blk3->mp_data, *blk->mp_data, offset, n2);
        element_block_func::resize_block(*blk->mp_data, blk->m_size);
    }
}

template<typename _CellBlockFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc>::set_cells_to_single_block(
    size_type start_row, size_type end_row, size_type block_index,
    size_type start_row_in_block, const _T& it_begin, const _T& it_end)
{
    assert(it_begin != it_end);
    assert(!m_blocks.empty());

    element_category_type cat = mdds_mtv_get_element_type(*it_begin);
    block* blk = m_blocks[block_index];
    size_type data_length = std::distance(it_begin, it_end);

    if (blk->mp_data)
    {
        element_category_type blk_cat = mdds::mtv::get_block_type(*blk->mp_data);
        if (cat == blk_cat)
        {
            // simple overwrite.
            size_type offset = start_row - start_row_in_block;
            element_block_func::overwrite_values(*blk->mp_data, offset, data_length);
            mdds_mtv_set_values(*blk->mp_data, offset, *it_begin, it_begin, it_end);
            return;
        }
    }

    size_type end_row_in_block = start_row_in_block + blk->m_size - 1;
    if (start_row == start_row_in_block)
    {
        if (end_row == end_row_in_block)
        {
            // Check if we could append it to the previous block.
            if (append_to_prev_block(block_index, cat, end_row-start_row+1, it_begin, it_end))
            {
                delete blk;
                m_blocks.erase(m_blocks.begin()+block_index);
                return;
            }

            // Replace the whole block.
            if (blk->mp_data)
                element_block_func::delete_block(blk->mp_data);

            blk->mp_data = element_block_func::create_new_block(cat, 0);
            mdds_mtv_assign_values(*blk->mp_data, *it_begin, it_begin, it_end);
            return;
        }

        // Replace the upper part of the block.

        // Shrink the current block first.
        size_type length = end_row_in_block - end_row;
        blk->m_size = length;
        if (blk->mp_data)
        {
            // Erase the upper part of the data from the current data array.
            mdds::unique_ptr<element_block_type> new_data(
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
        }

        length = end_row - start_row + 1;
        if (append_to_prev_block(block_index, cat, length, it_begin, it_end))
            return;

        // Insert a new block before the current block, and populate it with
        // the new data.
        m_blocks.insert(m_blocks.begin()+block_index, new block(length));
        blk = m_blocks[block_index];
        blk->mp_data = element_block_func::create_new_block(cat, 0);
        blk->m_size = length;
        mdds_mtv_assign_values(*blk->mp_data, *it_begin, it_begin, it_end);
        return;
    }

    assert(start_row > start_row_in_block);
    if (end_row == end_row_in_block)
    {
        // Shrink the current block and insert a new block for the new data series.
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
            // Check the next block.
            block* blk_next = m_blocks[block_index+1];
            if (blk_next->mp_data)
            {
                element_category_type blk_cat_next = mdds::mtv::get_block_type(*blk_next->mp_data);
                if (blk_cat_next == cat)
                {
                    // Prepend it to the next block.
                    mdds_mtv_prepend_values(*blk_next->mp_data, *it_begin, it_begin, it_end);
                    blk_next->m_size += end_row - start_row + 1;
                    return;
                }
            }

            // Next block has a different data type. Do the normal insertion.
            m_blocks.insert(m_blocks.begin()+block_index+1, new block(new_size));
            blk = m_blocks[block_index+1];
            blk->mp_data = element_block_func::create_new_block(cat, 0);
            mdds_mtv_assign_values(*blk->mp_data, *it_begin, it_begin, it_end);
            return;
        }

        // Last block.
        assert(block_index == m_blocks.size() - 1);

        m_blocks.push_back(new block(new_size));
        blk = m_blocks.back();
        blk->mp_data = element_block_func::create_new_block(cat, 0);
        mdds_mtv_assign_values(*blk->mp_data, *it_begin, it_begin, it_end);
        return;
    }

    // new data array will be in the middle of the current block.
    assert(start_row_in_block < start_row && end_row < end_row_in_block);

    // Insert two new blocks below the current one.
    m_blocks.insert(m_blocks.begin()+block_index+1, 2u, NULL);

    // first new block is for the data array being inserted.
    size_type new_size = end_row - start_row + 1;
    m_blocks[block_index+1] = new block(new_size);
    block* blk_new = m_blocks[block_index+1];
    blk_new->mp_data = element_block_func::create_new_block(cat, 0);
    mdds_mtv_assign_values(*blk_new->mp_data, *it_begin, it_begin, it_end);

    // second new block is to transfer the lower part of the current block.
    new_size = end_row_in_block - end_row;
    m_blocks[block_index+2] = new block(new_size);
    size_type new_cur_size = start_row - start_row_in_block;
    if (blk->mp_data)
    {
        // current block is not empty. Transfer the lower part of the data.
        element_category_type blk_cat = mdds::mtv::get_block_type(*blk->mp_data);

        blk_new = m_blocks[block_index+2];
        blk_new->mp_data = element_block_func::create_new_block(blk_cat, 0);
        size_type offset = end_row - start_row_in_block + 1;
        element_block_func::assign_values_from_block(
            *blk_new->mp_data, *blk->mp_data, offset, new_size);

        // Resize the current block.
        element_block_func::overwrite_values(*blk->mp_data, new_cur_size, data_length);
        element_block_func::resize_block(*blk->mp_data, new_cur_size);
    }
    blk->m_size = new_cur_size;
}

template<typename _CellBlockFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc>::set_cells_to_multi_blocks(
    size_type start_row, size_type end_row,
    size_type block_index1, size_type start_row_in_block1,
    size_type block_index2, size_type start_row_in_block2,
    const _T& it_begin, const _T& it_end)
{
    assert(block_index1 < block_index2);
    assert(it_begin != it_end);
    assert(!m_blocks.empty());

    block* blk1 = m_blocks[block_index1];
    if (blk1->mp_data)
    {
        set_cells_to_multi_blocks_block1_non_empty(
            start_row, end_row, block_index1, start_row_in_block1,
            block_index2, start_row_in_block2, it_begin, it_end);

        return;
    }

    // Block 1 is empty.
    assert(!blk1->mp_data);

    set_cells_to_multi_blocks_block1_non_equal(
        start_row, end_row, block_index1, start_row_in_block1,
        block_index2, start_row_in_block2, it_begin, it_end);
}

template<typename _CellBlockFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc>::set_cells_to_multi_blocks_block1_non_equal(
    size_type start_row, size_type end_row,
    size_type block_index1, size_type start_row_in_block1,
    size_type block_index2, size_type start_row_in_block2,
    const _T& it_begin, const _T& it_end)
{
    element_category_type cat = mdds_mtv_get_element_type(*it_begin);
    block* blk1 = m_blocks[block_index1];
    block* blk2 = m_blocks[block_index2];
    size_type length = std::distance(it_begin, it_end);
    size_type offset = start_row - start_row_in_block1;
    size_type end_row_in_block2 = start_row_in_block2 + blk2->m_size - 1;

    // Initially set to erase blocks between block 1 and block 2.
    typename blocks_type::iterator it_erase_begin = m_blocks.begin() + block_index1 + 1;
    typename blocks_type::iterator it_erase_end = m_blocks.begin() + block_index2;

    // Create the new data block first.
    mdds::unique_ptr<block> data_blk(new block(length));

    bool blk0_copied = false;
    if (offset == 0)
    {
        // Remove block 1.
        --it_erase_begin;

        // Check the type of the previous block (block 0) if exists.
        if (block_index1 > 0)
        {
            block* blk0 = m_blocks[block_index1-1];
            if (blk0->mp_data)
            {
                if (cat == mdds::mtv::get_block_type(*blk0->mp_data))
                {
                    // Transfer the whole data from block 0 to data block.
                    data_blk->mp_data = blk0->mp_data;
                    blk0->mp_data = NULL;

                    data_blk->m_size += blk0->m_size;
                    --it_erase_begin;
                    blk0_copied = true;
                }
            }
        }
    }
    else
    {
        // Shrink block 1.
        if (blk1->mp_data)
        {
            size_type n = blk1->m_size - offset;
            element_block_func::overwrite_values(*blk1->mp_data, offset, n);
            element_block_func::resize_block(*blk1->mp_data, offset);
        }
        blk1->m_size = offset;
    }

    if (blk0_copied)
        mdds_mtv_append_values(*data_blk->mp_data, *it_begin, it_begin, it_end);
    else
    {
        data_blk->mp_data = element_block_func::create_new_block(cat, 0);
        mdds_mtv_assign_values(*data_blk->mp_data, *it_begin, it_begin, it_end);
    }

    if (end_row == end_row_in_block2)
    {
        // Remove block 2.
        ++it_erase_end;

        if (block_index2+1 < m_blocks.size())
        {
            block* blk3 = m_blocks[block_index2+1];
            if (blk3->mp_data && mdds::mtv::get_block_type(*blk3->mp_data) == cat)
            {
                // Merge the whole block 3 with the new data. Remove block 3
                // afterward.  Resize block 3 to zero to prevent invalid free.
                element_block_func::append_values_from_block(*data_blk->mp_data, *blk3->mp_data);
                element_block_func::resize_block(*blk3->mp_data, 0);
                data_blk->m_size += blk3->m_size;
                ++it_erase_end;
            }
        }
    }
    else
    {
        bool erase_upper = true;
        if (blk2->mp_data)
        {
            element_category_type blk_cat2 = mdds::mtv::get_block_type(*blk2->mp_data);
            if (blk_cat2 == cat)
            {
                // Merge the lower part of block 2 with the new data, and
                // erase block 2.  Resize block 2 to avoid invalid free on the
                // copied portion of the block.
                size_type copy_pos = end_row - start_row_in_block2 + 1;
                size_type size_to_copy = end_row_in_block2 - end_row;
                element_block_func::append_values_from_block(
                    *data_blk->mp_data, *blk2->mp_data, copy_pos, size_to_copy);
                element_block_func::resize_block(*blk2->mp_data, copy_pos);
                data_blk->m_size += size_to_copy;

                ++it_erase_end;
                erase_upper = false;
            }
        }

        if (erase_upper)
        {
            // Erase the upper part of block 2.
            size_type size_to_erase = end_row - start_row_in_block2 + 1;
            if (blk2->mp_data)
            {
                element_block_func::overwrite_values(*blk2->mp_data, 0, size_to_erase);
                element_block_func::erase(*blk2->mp_data, 0, size_to_erase);
            }
            blk2->m_size -= size_to_erase;
        }
    }

    size_type insert_pos = std::distance(m_blocks.begin(), it_erase_begin);

    // Remove the in-between blocks first.
    std::for_each(it_erase_begin, it_erase_end, default_deleter<block>());
    m_blocks.erase(it_erase_begin, it_erase_end);

    // Insert the new data block.
    m_blocks.insert(m_blocks.begin()+insert_pos, data_blk.release());
}

template<typename _CellBlockFunc>
template<typename _T>
void multi_type_vector<_CellBlockFunc>::set_cells_to_multi_blocks_block1_non_empty(
    size_type start_row, size_type end_row,
    size_type block_index1, size_type start_row_in_block1,
    size_type block_index2, size_type start_row_in_block2,
    const _T& it_begin, const _T& it_end)
{
    element_category_type cat = mdds_mtv_get_element_type(*it_begin);
    block* blk1 = m_blocks[block_index1];
    assert(blk1->mp_data);
    element_category_type blk_cat1 = mdds::mtv::get_block_type(*blk1->mp_data);

    if (blk_cat1 == cat)
    {
        block* blk2 = m_blocks[block_index2];
        size_type length = std::distance(it_begin, it_end);
        size_type offset = start_row - start_row_in_block1;
        size_type end_row_in_block2 = start_row_in_block2 + blk2->m_size - 1;

        // Initially set to erase blocks between block 1 and block 2.
        typename blocks_type::iterator it_erase_begin = m_blocks.begin() + block_index1 + 1;
        typename blocks_type::iterator it_erase_end = m_blocks.begin() + block_index2;

        // Extend the first block to store the new data set.

        // Shrink it first to remove the old values, then append new values.
        element_block_func::overwrite_values(*blk1->mp_data, offset, blk1->m_size-offset);
        element_block_func::resize_block(*blk1->mp_data, offset);
        mdds_mtv_append_values(*blk1->mp_data, *it_begin, it_begin, it_end);
        blk1->m_size = offset + length;

        if (end_row == end_row_in_block2)
        {
            // Data overlaps the entire block 2. Erase it.
            ++it_erase_end;
        }
        else if (blk2->mp_data)
        {
            element_category_type blk_cat2 = mdds::mtv::get_block_type(*blk2->mp_data);
            if (blk_cat2 == cat)
            {
                // Copy the lower part of block 2 to the new block, and
                // remove it.  Resize block 2 to zero to prevent the
                // transferred / overwritten cells from being deleted on block
                // deletion.
                size_type data_length = end_row_in_block2 - end_row;
                size_type begin_pos = end_row - start_row_in_block2 + 1;
                element_block_func::append_values_from_block(*blk1->mp_data, *blk2->mp_data, begin_pos, data_length);
                element_block_func::overwrite_values(*blk2->mp_data, 0, begin_pos);
                element_block_func::resize_block(*blk2->mp_data, 0);
                blk1->m_size += data_length;
                ++it_erase_end;
            }
            else
            {
                // Erase the upper part of block 2.
                size_type size_to_erase = end_row - start_row_in_block2 + 1;
                element_block_func::erase(*blk2->mp_data, 0, size_to_erase);
                blk2->m_size -= size_to_erase;
            }
        }
        else
        {
            // Last block is empty.
            size_type size_to_erase = end_row - start_row_in_block2 + 1;
            blk2->m_size -= size_to_erase;
        }

        std::for_each(it_erase_begin, it_erase_end, default_deleter<block>());
        m_blocks.erase(it_erase_begin, it_erase_end);
        return;
    }

    // The first block type is different.
    assert(blk_cat1 != cat);

    set_cells_to_multi_blocks_block1_non_equal(
        start_row, end_row, block_index1, start_row_in_block1,
        block_index2, start_row_in_block2, it_begin, it_end);
}

template<typename _CellBlockFunc>
template<typename _T>
bool multi_type_vector<_CellBlockFunc>::append_to_prev_block(
    size_type block_index, element_category_type cat, size_type length,
    const _T& it_begin, const _T& it_end)
{
    if (block_index == 0)
        return false;

    block* blk_prev = m_blocks[block_index-1];
    if (!blk_prev->mp_data)
        return false;

    element_category_type blk_cat_prev = mdds::mtv::get_block_type(*blk_prev->mp_data);
    if (blk_cat_prev != cat)
        return false;

    // Append to the previous block.
    mdds_mtv_append_values(*blk_prev->mp_data, *it_begin, it_begin, it_end);
    blk_prev->m_size += length;
    return true;
}

template<typename _CellBlockFunc>
void multi_type_vector<_CellBlockFunc>::clear()
{
    std::for_each(m_blocks.begin(), m_blocks.end(), default_deleter<block>());
    m_blocks.clear();
    m_cur_size = 0;
}

template<typename _CellBlockFunc>
typename multi_type_vector<_CellBlockFunc>::size_type
multi_type_vector<_CellBlockFunc>::size() const
{
    return m_cur_size;
}

template<typename _CellBlockFunc>
typename multi_type_vector<_CellBlockFunc>::size_type
multi_type_vector<_CellBlockFunc>::block_size() const
{
    return m_blocks.size();
}

template<typename _CellBlockFunc>
bool multi_type_vector<_CellBlockFunc>::empty() const
{
    return m_blocks.empty();
}

template<typename _CellBlockFunc>
void multi_type_vector<_CellBlockFunc>::resize(size_type new_size)
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
        if (m_blocks.empty())
        {
            // No existing block. Create a new one.
            assert(m_cur_size == 0);
            m_blocks.push_back(new block(new_size));
            m_cur_size = new_size;
            return;
        }

        block* blk_last = m_blocks.back();
        size_type delta = new_size - m_cur_size;

        if (!blk_last->mp_data)
        {
            // Last block is empty.  Just increase its size.
            blk_last->m_size += delta;
        }
        else
        {
            // Append a new empty block.
            m_blocks.push_back(new block(delta));
        }
        m_cur_size = new_size;
        return;
    }

    assert(new_size < m_cur_size && new_size > 0);

    // Find out in which block the new end row will be.
    size_type new_end_row = new_size - 1;
    size_type start_row_in_block = 0, block_index = 0;
    get_block_position(new_end_row, start_row_in_block, block_index);

    block* blk = m_blocks[block_index];
    size_type end_row_in_block = start_row_in_block + blk->m_size - 1;

    if (new_end_row < end_row_in_block)
    {
        // Shrink the size of the current block.
        size_type new_block_size = new_end_row - start_row_in_block + 1;
        if (blk->mp_data)
        {
            element_block_func::overwrite_values(*blk->mp_data, new_end_row+1, end_row_in_block-new_end_row);
            element_block_func::resize_block(*blk->mp_data, new_block_size);
        }
        blk->m_size = new_block_size;
    }

    // Remove all blocks that are below this one.
    typename blocks_type::iterator it = m_blocks.begin() + block_index + 1;
    std::for_each(it, m_blocks.end(), default_deleter<block>());
    m_blocks.erase(it, m_blocks.end());
    m_cur_size = new_size;
}

template<typename _CellBlockFunc>
void multi_type_vector<_CellBlockFunc>::swap(multi_type_vector& other)
{
    std::swap(m_cur_size, other.m_cur_size);
    m_blocks.swap(other.m_blocks);
}

template<typename _CellBlockFunc>
bool multi_type_vector<_CellBlockFunc>::operator== (const multi_type_vector& other) const
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
        const block* blk1 = *it;
        const block* blk2 = *it2;

        if (!blk1->mp_data)
            return blk2->mp_data == NULL;

        if (!blk2->mp_data)
            // left is non-empty while right is empty.
            return false;

        assert(blk1->mp_data && blk2->mp_data);
        if (!element_block_func::equal_block(*blk1->mp_data, *blk2->mp_data))
            return false;
    }

    return true;
}

template<typename _CellBlockFunc>
bool multi_type_vector<_CellBlockFunc>::operator!= (const multi_type_vector& other) const
{
    return !operator== (other);
}

template<typename _CellBlockFunc>
multi_type_vector<_CellBlockFunc>& multi_type_vector<_CellBlockFunc>::operator= (const multi_type_vector& other)
{
    multi_type_vector assigned(other);
    swap(assigned);
    return *this;
}

template<typename _CellBlockFunc>
template<typename _T>
mtv::element_t multi_type_vector<_CellBlockFunc>::get_element_type(const _T& elem)
{
    return mdds_mtv_get_element_type(elem);
}

template<typename _CellBlockFunc>
void multi_type_vector<_CellBlockFunc>::set_empty_in_single_block(
    size_type start_row, size_type end_row, size_type block_index, size_type start_row_in_block)
{
    // Range is within a single block.
    block* blk = m_blocks[block_index];
    if (!blk->mp_data)
        // This block is already empty.  Do nothing.
        return;

    assert(start_row_in_block + blk->m_size >= 1);
    size_type end_row_in_block = start_row_in_block + blk->m_size - 1;
    size_type empty_block_size = end_row - start_row + 1;

    if (start_row == start_row_in_block)
    {
        // start row coincides with the start of a block.

        if (end_row == end_row_in_block)
        {
            // Set the whole block empty.
            element_block_func::delete_block(blk->mp_data);
            blk->mp_data = NULL;
            return;
        }

        // Set the upper part of the block empty.
        element_block_func::overwrite_values(*blk->mp_data, 0, empty_block_size);
        element_block_func::erase(*blk->mp_data, 0, empty_block_size);
        blk->m_size -= empty_block_size;

        // Insert a new empty block before the current one.
        m_blocks.insert(m_blocks.begin()+block_index, new block(empty_block_size));
        return;
    }

    if (end_row == end_row_in_block)
    {
        // end row coincides with the end of a block.
        assert(start_row > start_row_in_block);

        // Set the lower part of the block empty.
        size_type start_pos = start_row - start_row_in_block;
        element_block_func::overwrite_values(*blk->mp_data, start_pos, empty_block_size);
        element_block_func::erase(*blk->mp_data, start_pos, empty_block_size);
        blk->m_size -= empty_block_size;

        // Insert a new empty block after the current one.
        m_blocks.insert(m_blocks.begin()+block_index+1, new block(empty_block_size));
        return;
    }

    // Empty the middle part of a block.
    assert(end_row_in_block - end_row > 0);

    // First, insert two new blocks at position past the current block.
    size_type lower_block_size = end_row_in_block - end_row;
    m_blocks.insert(m_blocks.begin()+block_index+1, 2u, NULL);
    m_blocks[block_index+1] = new block(empty_block_size); // empty block.
    m_blocks[block_index+2] = new block(lower_block_size);

    // Copy the lower values from the current block to the new non-empty block.
    block* blk_lower = m_blocks[block_index+2];
    assert(blk_lower->m_size == lower_block_size);
    element_category_type blk_cat = mdds::mtv::get_block_type(*blk->mp_data);
    blk_lower->mp_data = element_block_func::create_new_block(blk_cat, 0);
    element_block_func::assign_values_from_block(
        *blk_lower->mp_data, *blk->mp_data,
        end_row_in_block-start_row_in_block-lower_block_size+1,
        lower_block_size);

    // Overwrite cells that will become empty.
    size_type new_cur_size = start_row - start_row_in_block;
    element_block_func::overwrite_values(
        *blk->mp_data, new_cur_size, empty_block_size);

    // Shrink the current data block.
    element_block_func::erase(
        *blk->mp_data, new_cur_size, end_row_in_block-start_row+1);
    blk->m_size = new_cur_size;
}

template<typename _CellBlockFunc>
void multi_type_vector<_CellBlockFunc>::set_empty_in_multi_blocks(
    size_type start_row, size_type end_row,
    size_type block_index1, size_type start_row_in_block1,
    size_type block_index2, size_type start_row_in_block2)
{
    assert(block_index1 < block_index2);

    {
        // Empty the lower part of the first block.
        block* blk = m_blocks[block_index1];
        if (blk->mp_data)
        {
            if (start_row_in_block1 == start_row)
            {
                // Empty the whole block.
                element_block_func::delete_block(blk->mp_data);
                blk->mp_data = NULL;
            }
            else
            {
                // Empty the lower part.
                size_type new_size = start_row - start_row_in_block1;
                element_block_func::overwrite_values(*blk->mp_data, new_size, blk->m_size-new_size);
                element_block_func::resize_block(*blk->mp_data, new_size);
                blk->m_size = new_size;
            }
        }
        else
        {
            // First block is already empty.  Adjust the start row of the new
            // empty range.
            start_row = start_row_in_block1;
        }
    }

    {
        // Empty the upper part of the last block.
        block* blk = m_blocks[block_index2];
        size_type last_row_in_block = start_row_in_block2 + blk->m_size - 1;
        if (blk->mp_data)
        {
            if (last_row_in_block == end_row)
            {
                // Delete the whole block.
                delete blk;
                m_blocks.erase(m_blocks.begin()+block_index2);
            }
            else
            {
                // Empty the upper part.
                size_type size_to_erase = end_row - start_row_in_block2 + 1;
                element_block_func::overwrite_values(*blk->mp_data, 0, size_to_erase);
                element_block_func::erase(*blk->mp_data, 0, size_to_erase);
                blk->m_size -= size_to_erase;
            }
        }
        else
        {
            // Last block is empty.  Delete this block and adjust the end row
            // of the new empty range.
            delete blk;
            m_blocks.erase(m_blocks.begin()+block_index2);
            end_row = last_row_in_block;
        }
    }

    if (block_index2 - block_index1 > 1)
    {
        // Remove all blocks in-between, from block_index1+1 to block_index2-1.

        for (size_type i = block_index1 + 1; i < block_index2; ++i)
            delete m_blocks[i];

        typename blocks_type::iterator it = m_blocks.begin() + block_index1 + 1;
        typename blocks_type::iterator it_end = m_blocks.begin() + block_index2;
        m_blocks.erase(it, it_end);
    }

    // Insert a single empty block.
    block* blk = m_blocks[block_index1];
    size_type empty_block_size = end_row - start_row + 1;
    if (blk->mp_data)
    {
        // Insert a new empty block after the first block.
        m_blocks.insert(m_blocks.begin()+block_index1+1, new block(empty_block_size));
    }
    else
    {
        // Current block is already empty. Just extend its size.
        blk->m_size = empty_block_size;
    }
}

}
