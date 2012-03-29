/*************************************************************************
 *
 * Copyright (c) 2011-2012 Kohei Yoshida
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

#ifndef __MDDS_GRID_MAP_COLUMN_HPP__
#define __MDDS_GRID_MAP_COLUMN_HPP__

#include "default_deleter.hpp"
#include "compat/unique_ptr.hpp"
#include "global.hpp"
#include "grid_map_types.hpp"

#include <vector>
#include <algorithm>
#include <cassert>

#include <boost/noncopyable.hpp>

namespace mdds { namespace __gridmap {

/**
 * Each column consists of a series of blocks, and each block stores a
 * series of non-empty cells of identical type.
 */
template<typename _Trait>
class column
{
public:
    typedef size_t size_type;

    typedef typename mdds::gridmap::base_cell_block cell_block_type;
    typedef typename mdds::gridmap::cell_t cell_category_type;
    typedef typename _Trait::row_key_type row_key_type;

private:
    typedef _Trait trait;

    typedef typename _Trait::cell_block_func cell_block_func;

    struct block : boost::noncopyable
    {
        size_type m_size;
        cell_block_type* mp_data;

        block();
        block(size_type _size);
        block(const block& other);
        ~block();
    };

public:
    column();
    column(size_type init_row_size);
    column(const column& other);
    ~column();

    /**
     * Set a value of an arbitrary type to a specified row.  The type of the
     * value is inferred from the value passed to this method.  The new value
     * will overwrite an existing value at the specified row position if any.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the specified row is outside the current container size.</p>
     *
     * <p>Calling this method will not change the size of the container.</p>
     *
     * @param row row to insert the value to.
     * @param cell value to insert.
     */
    template<typename _T>
    void set_cell(row_key_type row, const _T& cell);

    /**
     * Set multiple cell values of identical type to a range of cells starting
     * at specified row.  Any existing cell values will be overwritten by the
     * new values.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the range of new values would fall outside the current container
     * size.</p>
     *
     * <p>Calling this method will not change the size of the container.</p>
     *
     * @param row position of the first value of the series of new values
     *            being inserted.
     * @param it_begin iterator that points to the begin position of the
     *                 values being set.
     * @param it_end iterator that points to the end position of the values
     *               being set.
     */
    template<typename _T>
    void set_cells(row_key_type row, const _T& it_begin, const _T& it_end);

    /**
     * Insert multiple cell values of identical type to a specified row
     * position.  Existing values that occur at or below the specified row
     * position will get shifted after the insertion.  No existing values will
     * be overwritten by the inserted values.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the specified row position is outside the current container size.</p>
     *
     * <p>Calling this method will increase the size of the container by
     * the length of the new values inserted.</p>
     *
     * @param row row position at which the new values are to be inserted.
     * @param it_begin iterator that points to the begin position of the
     *                 values being inserted.
     * @param it_end iterator that points to the end position of the values
     *               being inserted.
     */
    template<typename _T>
    void insert_cells(row_key_type row, const _T& it_begin, const _T& it_end);

    /**
     * Get the value of a cell at specified row.  The caller must pass a
     * variable of the correct type to store the value.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the specified row position is outside the current container size.</p>
     *
     * @param row row position of the cell value to retrieve.
     * @param cell (out) variable to store the retrieved value.
     */
    template<typename _T>
    void get_cell(row_key_type row, _T& cell) const;

    /**
     * Get the value of a cell at specified row.  The caller must specify the
     * type of the cell as the template parameter e.g. get_cell<double>(1).
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the specified row position is outside the current container size.</p>
     *
     * @param row row position of the cell value to retrieve.
     * @return cell value.
     */
    template<typename _T>
    _T get_cell(row_key_type row) const;

    /**
     * Check if cell at specified row is empty of not.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the specified row position is outside the current container size.</p>
     *
     * @param row row position of the cell to check.
     *
     * @return true if the cell is empty, false otherwise.
     */
    bool is_empty(row_key_type row) const;

    /**
     * Set specified range of cells to be empty.  Any existing values will be
     * overwritten.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * either the starting or the ending row position is outside the current
     * container size.</p>
     *
     * @param start_row starting row position
     * @param end_row ending row position, inclusive.
     */
    void set_empty(row_key_type start_row, row_key_type end_row);

    void erase(row_key_type start_row, row_key_type end_row);

    void insert_empty(row_key_type row, size_type length);

    void clear();

    size_type size() const;

    size_type block_size() const;

    bool empty() const;

    void resize(size_type new_size);

    void swap(column& other);

    bool operator== (const column& other) const;
    bool operator!= (const column& other) const;

    column& operator= (const column& other);

    template<typename _T>
    static gridmap::cell_t get_cell_type(const _T& cell);

private:
    /**
     * Check the row value to make sure it's within specified range, and
     * convert it to size_type for internal use.
     */
    size_type check_row_range(row_key_type row) const;

    void get_block_position(
        size_type row, size_type& start_row, size_type& block_index, size_type start_block=0, size_type start_block_row=0) const;

    template<typename _T>
    void create_new_block_with_new_cell(cell_block_type*& data, const _T& cell);

    template<typename _T>
    void set_cell_to_middle_of_block(
        size_type block_index, size_type pos_in_block, const _T& cell);

    template<typename _T>
    void append_cell_to_block(size_type block_index, const _T& cell);

    template<typename _T>
    void set_cell_impl(size_type row, const _T& cell);

    template<typename _T>
    void set_cell_to_empty_block(
        size_type block_index, size_type pos_in_block, const _T& cell);

    template<typename _T>
    void set_cell_to_block_of_size_one(
        size_type block_index, const _T& cell);

    template<typename _T>
    void set_cell_to_top_of_data_block(
        size_type block_index, const _T& cell);

    template<typename _T>
    void set_cell_to_bottom_of_data_block(
        size_type block_index, const _T& cell);

    void set_empty_in_single_block(
        size_type start_row, size_type end_row, size_type block_index, size_type start_row_in_block);

    void set_empty_in_multi_blocks(
        size_type start_row, size_type end_row,
        size_type block_index1, size_type start_row_in_block1,
        size_type block_index2, size_type start_row_in_block2);

    void erase_impl(size_type start_row, size_type end_row);

    void insert_empty_impl(size_type row, size_type length);

    template<typename _T>
    void set_cells_impl(size_type row, const _T& it_begin, const _T& it_end);

    template<typename _T>
    void insert_cells_impl(size_type row, const _T& it_begin, const _T& it_end);

    template<typename _T>
    void set_cells_to_single_block(
        size_type start_row, size_type end_row, size_type block_index,
        size_type start_row_in_block, const _T& it_begin, const _T& it_end);

    template<typename _T>
    void set_cells_to_multi_blocks(
        size_type start_row, size_type end_row,
        size_type block_index1, size_type start_row_in_block1,
        size_type block_index2, size_type start_row_in_block2,
        const _T& it_begin, const _T& it_end);

    template<typename _T>
    void set_cells_to_multi_blocks_block1_non_equal(
        size_type start_row, size_type end_row,
        size_type block_index1, size_type start_row_in_block1,
        size_type block_index2, size_type start_row_in_block2,
        const _T& it_begin, const _T& it_end);

    template<typename _T>
    void set_cells_to_multi_blocks_block1_non_empty(
        size_type start_row, size_type end_row,
        size_type block_index1, size_type start_row_in_block1,
        size_type block_index2, size_type start_row_in_block2,
        const _T& it_begin, const _T& it_end);

    template<typename _T>
    bool append_to_prev_block(
        size_type block_index, cell_category_type cat, size_type length,
        const _T& it_begin, const _T& it_end);

    template<typename _T>
    void insert_cells_to_middle(
        size_type row, size_type block_index, size_type start_row,
        const _T& it_begin, const _T& it_end);

private:
    typedef std::vector<block*> blocks_type;
    blocks_type m_blocks;
    size_type m_cur_size;
};

}}

#include "grid_map_column_def.inl"

#endif
