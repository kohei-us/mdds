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

namespace mdds { namespace __gridmap {

/**
 * Each column consists of a series of blocks, and each block stores a
 * series of non-empty cells of identical type.  In this container, cells
 * are represented simply as values that they store; there are no separate
 * cell objects that the user of this container needs to deal with.  The
 * user accesses directly with the raw values.
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

    struct block
    {
        size_type m_size;
        cell_block_type* mp_data;

        block();
        block(size_type _size);
        block(const block& other);
        ~block();
    };

public:

    /**
     * Default constructor.  It initializes the container with empty size.
     */
    column();

    /**
     * Constructor that takes initial size of the container.  When the size
     * specified is greater than 0, it initializes the container with empty
     * cells.
     *
     * @param init_row_size initial container size
     */
    column(size_type init_row_size);

    /**
     * Copy constructor.
     *
     * @param other other column instance to copy values from.
     */
    column(const column& other);

    /**
     * Destructor.  It deletes all allocated data blocks.
     */
    ~column();

    /**
     * Set a value of an arbitrary type to a specified row.  The type of the
     * value is inferred from the value passed to this method.  The new value
     * will overwrite an existing value at the specified row position if any.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the specified row is outside the current container range.</p>
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
     * range.</p>
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
     * the specified row position is outside the current container range.</p>
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
     * the specified row position is outside the current container range.</p>
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
     * the specified row position is outside the current container range.</p>
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
     * the specified row position is outside the current container range.</p>
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

    /**
     * Erase cells located between specified start and end row positions.  The
     * end positions are both inclusive.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * either the starting or the ending row position is outside the current
     * container range.</p>
     *
     * <p>Calling this method will decrease the size of the container by
     * the length of the erased range.</p>
     *
     * @param start_row starting row position
     * @param end_row ending row position, inclusive.
     */
    void erase(row_key_type start_row, row_key_type end_row);

    /**
     * Insert a range of empty cells at specified row position.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * either the specified row position is outside the current container
     * range.</p>
     *
     * <p>Calling this method will increase the size of the container by
     * the length of the inserted empty cells.</p>
     *
     * @param row row position at which to insert a range of empty cells.
     * @param length number of empty cells to insert.
     */
    void insert_empty(row_key_type row, size_type length);

    /**
     * Clear the content of the container.  The size of the container will
     * become zero after calling this method.
     */
    void clear();

    /**
     * Return the current container size.
     *
     * @return current container size.
     */
    size_type size() const;

    /**
     * Return the current number of data blocks.  Each data block stores a
     * series of contiguous cells of identical type.  A series of empty cells
     * is also represented by a separate data block.
     *
     * <p>For instance, if the container stores values of double-precision
     * type at rows 0 to 2, values of std::string type at 3 to 7, and empty
     * values at 8 to 10, it consists of 3 data blocks: one that stores double
     * values, one that stores std::string values, and one that represents the
     * empty value range. In this specific scenario, <code>block_size()</code>
     * returns 3, and <code>size()</code> returns 11.</p>
     *
     * @return current number of data blocks.
     */
    size_type block_size() const;

    /**
     * Return whether or not the container is empty.
     *
     * @return true if the container is empty, false otherwise.
     */
    bool empty() const;

    /**
     * Extend or shrink the container.  When extending the container, it
     * appends a series of empty cells to the end.  When shrinking, the cells
     * at the end of the container get stripped off.
     *
     * @param new_size size of the container after the resize.
     */
    void resize(size_type new_size);

    /**
     * Swap the content with another container.
     *
     * @param other another container to swap content with.
     */
    void swap(column& other);

    bool operator== (const column& other) const;
    bool operator!= (const column& other) const;

    column& operator= (const column& other);

    /**
     * Return the numerical identifier that represents passed cell.
     *
     * @param cell cell value.
     *
     * @return gridmap::cell_t numerical identifier representing the cell.
     */
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
