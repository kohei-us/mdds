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
    cell_category_type blk_cat = get_block_type(*blk->mp_data);

    if (blk_cat == cat)
    {
        // This block is of the same type as the cell being inserted.
        row_key_type i = row - start_row;
        cell_block_modifier::set_value(blk->mp_data, i, cell);
        return;
    }

    assert(blk_cat != cat);

    if (row == start_row)
    {
        // Insertion point is at the start of the block.
        if (blk->m_size == 1)
        {
            if (block_index == 0)
            {
                // This is the topmost block of size 1.
                if (block_index == m_blocks.size()-1)
                {
                    // This is the only block.
                    cell_block_modifier::delete_block(blk->mp_data);
                    create_new_block_with_new_cell(blk->mp_data, cell);
                    return;
                }

                // There is an existing block below.
                block* blk_next = m_blocks[block_index+1];
                if (!blk_next->mp_data)
                {
                    // Next block is empty.
                    cell_block_modifier::delete_block(blk->mp_data);
                    create_new_block_with_new_cell(blk->mp_data, cell);
                    return;
                }

                // Next block is not empty.
                cell_category_type blk_cat_next = get_block_type(*blk_next->mp_data);
                if (blk_cat_next != cat)
                {
                    // Cell being inserted is of different type than that of the next block.
                    cell_block_modifier::delete_block(blk->mp_data);
                    create_new_block_with_new_cell(blk->mp_data, cell);
                    return;
                }

                // Delete the current block, and prepend the cell to the next block.
                assert(!"not implemented yet.");
                return;
            }

            if (block_index == m_blocks.size()-1)
            {
                // This is the last block.
                assert(!"not implemented yet.");
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
                assert(!"not implemented yet.");
                return;
            }

            if (!blk_next->mp_data)
            {
                // Next block is empty.
                assert(!"not implemented yet.");
                return;
            }

            assert(blk_prev && blk_prev->mp_data);
            assert(blk_next && blk_next->mp_data);
            cell_category_type blk_cat_prev = get_block_type(*blk_prev->mp_data);
            cell_category_type blk_cat_next = get_block_type(*blk_next->mp_data);

            if (blk_cat_prev == blk_cat_next)
            {
                // Merge the previous block with the cell being inserted and
                // the next block.
                blk_prev->m_size += 1 + blk_next->m_size;
                cell_block_modifier::append_value(blk_prev->mp_data, cell);
                cell_block_modifier::append_value(blk_prev->mp_data, blk_next->mp_data);

                // Delete the current and next blocks.
                delete blk;
                delete blk_next;
                typename blocks_type::iterator it = m_blocks.begin() + block_index;
                typename blocks_type::iterator it_last = it + 2;
                m_blocks.erase(it, it_last);
            }
            else
            {
                assert(!"not implemented yet.");
            }
            return;
        }

        assert(blk->m_size > 1);
        if (block_index == 0)
        {
            // No preceding block.
            blk->m_size -= 1;
            cell_block_modifier::erase(blk->mp_data, 0);
            m_blocks.insert(m_blocks.begin(), new block(1));
            blk = m_blocks[0];
            create_new_block_with_new_cell(blk->mp_data, cell);
            return;
        }

        // Append to the previous block if the types match.
        block* blk_prev = m_blocks[block_index-1];
        cell_category_type blk_cat_prev = get_block_type(*blk_prev->mp_data);
        if (blk_cat_prev == cat)
        {
            // Append to the previous block.
            blk->m_size -= 1;
            cell_block_modifier::erase(blk->mp_data, 0);
            blk_prev->m_size += 1;
            cell_block_modifier::append_value(blk_prev->mp_data, cell);
            return;
        }

        assert(!"not implemented yet");
    }
    else if (row == (start_row + blk->m_size - 1))
    {
        // Insertion point is at the end of the block.
        assert(!"not implemented yet");
    }
    else
    {
        // Insertion point is somewhere in the middle of the block.
        assert(!"not implemented yet");
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
                    // Block exists below.
                    block* blk_next = m_blocks[block_index+1];
                    cell_block_type* data_next = blk_next->mp_data;
                    assert(data_next); // Empty block must not be followed by another empty block.
                    cell_category_type blk_cat_next = get_block_type(*data_next);
                    if (blk_cat_prev == blk_cat_next)
                    {
                        // We need to merge the previous and next blocks, then
                        // delete the current and next blocks.
                        block* blk_prev = m_blocks[block_index-1];
                        blk_prev->m_size += 1 + blk_next->m_size;
                        cell_block_modifier::append_value(blk_prev->mp_data, cell);
                        cell_block_modifier::append_value(blk_prev->mp_data, data_next);

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
                    cell_category_type blk_cat_next = get_block_type(*blk_next->mp_data);
                    if (cat == blk_cat_next)
                    {
                        // Remove this empty block, and prepend the cell to the next block.
                        blk_next->m_size += 1;
                        cell_block_modifier::prepend_value(blk_next->mp_data, cell);
                        delete m_blocks[block_index];
                        m_blocks.erase(m_blocks.begin()+block_index);
                    }
                    else
                        create_new_block_with_new_cell(blk->mp_data, cell);
                }
            }
            else
            {
                create_new_block_with_new_cell(blk->mp_data, cell);
                m_blocks.push_back(new block(blk->m_size-1));
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
