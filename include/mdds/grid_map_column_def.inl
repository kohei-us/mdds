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

#include <stdexcept>

#if UNIT_TEST
#include <iostream>
using std::cout;
using std::endl;
#endif

namespace mdds { namespace __gridmap {


template<typename _Trait>
column<_Trait>::block::block() : m_size(0), mp_data(NULL) {}

template<typename _Trait>
column<_Trait>::block::block(row_key_type _size) : m_size(_size), mp_data(NULL) {}

template<typename _Trait>
column<_Trait>::block::~block()
{
    cell_block_modifier::delete_block(mp_data);
}

template<typename _Trait>
column<_Trait>::column(row_key_type max_row_size) : m_max_row_size(max_row_size)
{
    // Initialize with an empty block that spans from 0 to max.
    m_blocks.push_back(new block(max_row_size));
}

template<typename _Trait>
column<_Trait>::~column()
{
    std::for_each(m_blocks.begin(), m_blocks.end(), default_deleter<block>());
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::set_cell(row_key_type row, const _T& cell)
{
    if (row < 0 || row >= m_max_row_size)
        throw std::out_of_range("Specified row index is out-of-bound.");

    cell_category_type cat = get_type(cell);

    // Find the right block ID from the row ID.
    row_key_type start_row = 0; // row ID of the first cell in a block.
    size_t block_index = 0;
    for (size_t i = 0, n = m_blocks.size(); i < n; ++i)
    {
        const block& blk = *m_blocks[i];
        if (row < start_row + blk.m_size)
        {
            // Row is in this block.
            block_index = i;
            break;
        }

        // Specified row is not in this block.
        start_row += blk.m_size;
    }

    block* blk = m_blocks[block_index];
    assert(blk->m_size > 0); // block size should never be zero at any time.

    row_key_type pos_in_block = row - start_row;
    assert(pos_in_block < blk->m_size);

    if (!blk->mp_data)
    {
        // This is an empty block.
        set_cell_to_empty_block(block_index, pos_in_block, cell);
        return;
    }

    assert(blk->mp_data);
    cell_category_type block_cat = get_block_type(*blk->mp_data);

    if (block_cat == cat)
    {
        // This block is of the same type as the cell being inserted.
        row_key_type i = row - start_row;
        cell_block_modifier::set_value(blk->mp_data, i, cell);
    }
    else if (row == start_row)
    {
        // Insertion point is at the start of the block.
    }
    else if (row == (start_row + blk->m_size - 1))
    {
        // Insertion point is at the end of the block.
    }
    else
    {
        // Insertion point is somewhere in the middle of the block.
    }
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::create_new_block_with_new_cell(cell_block_type*& data, const _T& cell)
{
    cell_category_type cat = get_type(cell);

    // New cell block is always size 1.
    data = cell_block_modifier::create_new_block(cat);
    if (!data)
        throw general_error("Failed to create new block.");

    cell_block_modifier::set_value(data, 0, cell);
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::insert_cell_to_middle_of_empty_block(
    size_t block_index, row_key_type pos_in_block, const _T& cell)
{
    block* blk = m_blocks[block_index];

    assert(pos_in_block > 0 && pos_in_block < blk->m_size - 1);
    assert(blk->m_size >= 3);
    row_key_type orig_size = blk->m_size;
    blk->m_size = pos_in_block;

    typename blocks_type::iterator it = m_blocks.begin();
    std::advance(it, block_index+1);
    m_blocks.insert(it, new block(1));
    it = m_blocks.begin();
    std::advance(it, block_index+1);
    blk = *it;
    ++it;
    m_blocks.insert(it, new block(orig_size-pos_in_block-1));

    create_new_block_with_new_cell(blk->mp_data, cell);
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::append_cell_to_block(size_t block_index, const _T& cell)
{
    block* blk = m_blocks[block_index];
    blk->m_size += 1;
    cell_block_modifier::append_value(blk->mp_data, cell);
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::set_cell_to_empty_block(
    size_t block_index, row_key_type pos_in_block, const _T& cell)
{
    block* blk = m_blocks[block_index];

    if (block_index == 0)
    {
        // Topmost block.
        if (m_blocks.size() == 1)
        {
            // this is the only block.
            assert(blk->m_size == m_max_row_size);
            if (m_max_row_size == 1)
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
                    insert_cell_to_middle_of_empty_block(block_index, pos_in_block, cell);
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
                    cell_category_type cat = get_type(cell);
                    cell_category_type cat_next = get_block_type(*blk_next->mp_data);

                    if (cat == cat_next)
                    {
                        // Remove this one-cell empty block from the top, and
                        // prepend the cell to the next block.
                        delete m_blocks.front();
                        m_blocks.erase(m_blocks.begin());
                        blk = m_blocks.front();
                        blk->m_size += 1;
                        cell_block_modifier::prepend_value(blk->mp_data, cell);
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
                cell_category_type cat = get_type(cell);
                cell_category_type cat_next = get_block_type(*blk_next->mp_data);
                assert(blk->m_size > 1);

                if (cat == cat_next)
                {
                    // Shrink this empty block by one, and prepend the cell to the next block.
                    blk->m_size -= 1;
                    blk_next->m_size += 1;
                    cell_block_modifier::prepend_value(blk_next->mp_data, cell);
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
                insert_cell_to_middle_of_empty_block(block_index, pos_in_block, cell);
            }
        }

        return;
    }

    // This empty block is right below a non-empty block.
    assert(block_index > 0 && m_blocks[block_index-1]->mp_data != NULL);

    if (pos_in_block == 0)
    {
        // New cell is right below the non-empty block.
        cell_category_type blk_cat_prev = get_block_type(*m_blocks[block_index-1]->mp_data);
        cell_category_type cat = get_type(cell);
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
                    block* blk_next = m_blocks[block_index+1];
                    cell_block_type* data_next = blk_next->mp_data;
                    assert(data_next); // Empty block must not be followed by another empty block.
                    cell_category_type blk_cat_next = get_block_type(*data_next);
                    if (blk_cat_prev == blk_cat_next)
                    {
                        // We need to merge the previous and next blocks.
                        blk = m_blocks[block_index-1];
                        blk->m_size += 1 + blk_next->m_size;
                        cell_block_modifier::append_value(blk->mp_data, cell);
                        cell_block_modifier::append_value(blk->mp_data, data_next);
                        delete m_blocks.back();
                        m_blocks.pop_back();
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
            // cell type is different from the type of the previous block.
            assert(!"not implemented yet.");
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
            cell_category_type cat = get_type(cell);
            block* blk_next = m_blocks[block_index+1];
            assert(blk_next->mp_data);
            cell_category_type blk_cat_next = get_block_type(*blk_next->mp_data);
            if (cat == blk_cat_next)
            {
                // Shrink this empty block and extend the next block.
                blk->m_size -= 1;
                blk_next->m_size += 1;
                cell_block_modifier::prepend_value(blk_next->mp_data, cell);
            }
            else
            {
                // Just insert this new cell.
                assert(!"not implemented yet");
            }
        }
    }
    else
    {
        // New cell is somewhere in the middle of an empty block.
        insert_cell_to_middle_of_empty_block(block_index, pos_in_block, cell);
    }
}

template<typename _Trait>
template<typename _T>
void column<_Trait>::get_cell(row_key_type row, _T& cell) const
{
    if (row >= m_max_row_size)
        throw std::out_of_range("Specified row index is out-of-bound.");

    row_key_type start_row = 0;
    const block* blk = NULL;
    for (size_t i = 0, n = m_blocks.size(); i < n; ++i)
    {
        blk = m_blocks[i];
        assert(blk->m_size > 0);
        if (row < start_row + blk->m_size)
            break;

        // Specified row is not in this block.
        start_row += blk->m_size;
    }

    assert(blk);

    if (!blk->mp_data)
    {
        // empty cell block.
        cell_block_modifier::get_empty_value(cell);
        return;
    }

    assert(row >= start_row);
    assert(blk->mp_data); // data for non-empty blocks should never be NULL.
    row_key_type idx = row - start_row;
    cell_block_modifier::get_value(blk->mp_data, idx, cell);
}

}}
