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

#ifndef __MDDS_MULTI_TYPE_VECTOR_HPP__
#define __MDDS_MULTI_TYPE_VECTOR_HPP__

#include "default_deleter.hpp"
#include "compat/unique_ptr.hpp"
#include "global.hpp"
#include "multi_type_vector_types.hpp"
#include "multi_type_vector_itr.hpp"

#include <vector>
#include <algorithm>
#include <cassert>

namespace mdds {

/**
 * Multi-type vector consists of a series of blocks, and each block stores a
 * series of non-empty elements of identical type.  In this container,
 * elements are represented simply as values that they store; there are no
 * separate element objects that the user of this container needs to deal
 * with.  The user accesses directly with the raw values.
 */
template<typename _ElemBlockFunc>
class multi_type_vector
{
public:
    typedef size_t size_type;

    typedef typename mdds::mtv::base_element_block element_block_type;
    typedef typename mdds::mtv::element_t element_category_type;

private:
    typedef _ElemBlockFunc element_block_func;

    struct block
    {
        size_type m_size;
        element_block_type* mp_data;

        block();
        block(size_type _size);
        block(const block& other);
        ~block();
    };

    typedef std::vector<block*> blocks_type;

public:

    typedef __mtv::iterator_base<multi_type_vector, blocks_type, typename blocks_type::iterator> iterator;
    typedef __mtv::iterator_base<multi_type_vector, blocks_type, typename blocks_type::reverse_iterator> reverse_iterator;
    typedef __mtv::const_iterator_base<multi_type_vector, blocks_type, typename blocks_type::const_iterator, iterator> const_iterator;
    typedef __mtv::const_iterator_base<multi_type_vector, blocks_type, typename blocks_type::const_reverse_iterator, reverse_iterator> const_reverse_iterator;

    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

    const_reverse_iterator rbegin() const;
    const_reverse_iterator rend() const;

    /**
     * Default constructor.  It initializes the container with empty size.
     */
    multi_type_vector();

    /**
     * Constructor that takes initial size of the container.  When the size
     * specified is greater than 0, it initializes the container with empty
     * elements.
     *
     * @param init_row_size initial container size
     */
    multi_type_vector(size_type init_size);

    template<typename _T>
    multi_type_vector(size_type init_size, const _T& value);

    /**
     * Copy constructor.
     *
     * @param other other column instance to copy values from.
     */
    multi_type_vector(const multi_type_vector& other);

    /**
     * Destructor.  It deletes all allocated data blocks.
     */
    ~multi_type_vector();

    /**
     * Set a value of an arbitrary type to a specified position.  The type of
     * the value is inferred from the value passed to this method.  The new
     * value will overwrite an existing value at the specified position
     * position if any.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception
     * if the specified position is outside the current container range.</p>
     *
     * <p>Calling this method will not change the size of the container.</p>
     *
     * @param pos position to insert the value to.
     * @param value value to insert.
     */
    template<typename _T>
    void set(size_type pos, const _T& value);

    /**
     * Set multiple values of identical type to a range of elements starting
     * at specified position.  Any existing values will be overwritten by the
     * new values.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the range of new values would fall outside the current container
     * range.</p>
     *
     * <p>Calling this method will not change the size of the container.</p>
     *
     * @param pos position of the first value of the series of new values
     *            being inserted.
     * @param it_begin iterator that points to the begin position of the
     *                 values being set.
     * @param it_end iterator that points to the end position of the values
     *               being set.
     */
    template<typename _T>
    void set(size_type pos, const _T& it_begin, const _T& it_end);

    /**
     * Insert multiple values of identical type to a specified position.
     * Existing values that occur at or below the specified position will get
     * shifted after the insertion.  No existing values will be overwritten by
     * the inserted values.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception
     * if the specified position is outside the current container range.</p>
     *
     * <p>Calling this method will increase the size of the container by
     * the length of the new values inserted.</p>
     *
     * @param pos position at which the new values are to be inserted.
     * @param it_begin iterator that points to the begin position of the
     *                 values being inserted.
     * @param it_end iterator that points to the end position of the values
     *               being inserted.
     */
    template<typename _T>
    void insert(size_type pos, const _T& it_begin, const _T& it_end);

    /**
     * Get the value of an element at specified position.  The caller must
     * pass a variable of the correct type to store the value.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the specified position is outside the current container range.</p>
     *
     * @param pos position of the element value to retrieve.
     * @param value (out) variable to store the retrieved value.
     */
    template<typename _T>
    void get(size_type pos, _T& value) const;

    /**
     * Get the value of an element at specified position.  The caller must
     * specify the type of the element as the template parameter e.g.
     * get<double>(1).
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the specified position is outside the current container range.</p>
     *
     * @param position position of the element value to retrieve.
     * @return element value.
     */
    template<typename _T>
    _T get(size_type pos) const;

    mtv::element_t get_type(size_type pos) const;

    /**
     * Check if element at specified position is empty of not.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the specified position is outside the current container range.</p>
     *
     * @param pos position of the element to check.
     *
     * @return true if the element is empty, false otherwise.
     */
    bool is_empty(size_type pos) const;

    /**
     * Set specified range of elements to be empty.  Any existing values will
     * be overwritten.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * either the starting or the ending position is outside the current
     * container size.</p>
     *
     * @param start_pos starting position
     * @param end_pos ending position, inclusive.
     */
    void set_empty(size_type start_pos, size_type end_pos);

    /**
     * Erase elements located between specified start and end positions. The
     * end positions are both inclusive.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * either the starting or the ending position is outside the current
     * container range.</p>
     *
     * <p>Calling this method will decrease the size of the container by
     * the length of the erased range.</p>
     *
     * @param start_pos starting position
     * @param end_pos ending position, inclusive.
     */
    void erase(size_type start_pos, size_type end_pos);

    /**
     * Insert a range of empty elements at specified position.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * either the specified position is outside the current container
     * range.</p>
     *
     * <p>Calling this method will increase the size of the container by
     * the length of the inserted empty elements.</p>
     *
     * @param pos position at which to insert a range of empty elements.
     * @param length number of empty elements to insert.
     */
    void insert_empty(size_type pos, size_type length);

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
     * series of contiguous elements of identical type.  A series of empty
     * elements is also represented by a separate data block.
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
     * appends a series of empty elements to the end.  When shrinking, the
     * elements at the end of the container get stripped off.
     *
     * @param new_size size of the container after the resize.
     */
    void resize(size_type new_size);

    /**
     * Swap the content with another container.
     *
     * @param other another container to swap content with.
     */
    void swap(multi_type_vector& other);

    bool operator== (const multi_type_vector& other) const;
    bool operator!= (const multi_type_vector& other) const;

    multi_type_vector& operator= (const multi_type_vector& other);

    /**
     * Return the numerical identifier that represents passed element.
     *
     * @param elem element value.
     *
     * @return numerical identifier representing the element.
     */
    template<typename _T>
    static mtv::element_t get_element_type(const _T& elem);

private:

    /**
     * Find the correct block position for given logical row ID.
     *
     * @param row logical ID of the row that belongs to the block being looked
     *            up for.
     *
     * @param start_pos logical ID of the first row of the block being looked
     *                  up for. The caller needs to assign its initial value
     *                  before calling this method in case the search needs to
     *                  start with a block that's not the first block.  Assign
     *                  0 if the search starts from the first block.
     *
     * @param block_index index of the block being looked up for. The caller
     *                    needs to assign its initial index which will be the
     *                    index of the block from which the search starts.
     *                    Assign 0 if the search starts from the first block.
     *
     * @return true if block position is found, false otherwise.
     */
    bool get_block_position(size_type row, size_type& start_pos, size_type& block_index) const;

    template<typename _T>
    static void create_new_block_with_new_cell(element_block_type*& data, const _T& cell);

    template<typename _T>
    void set_cell_to_middle_of_block(
        size_type block_index, size_type pos_in_block, const _T& cell);

    template<typename _T>
    void append_cell_to_block(size_type block_index, const _T& cell);

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
        size_type start_pos, size_type end_pos, size_type block_index, size_type start_pos_in_block);

    void set_empty_in_multi_blocks(
        size_type start_pos, size_type end_pos,
        size_type block_index1, size_type start_pos_in_block1,
        size_type block_index2, size_type start_pos_in_block2);

    void erase_impl(size_type start_pos, size_type end_pos);

    void insert_empty_impl(size_type row, size_type length);

    template<typename _T>
    void set_cells_impl(size_type row, const _T& it_begin, const _T& it_end);

    template<typename _T>
    void insert_cells_impl(size_type row, const _T& it_begin, const _T& it_end);

    template<typename _T>
    void set_cells_to_single_block(
        size_type start_pos, size_type end_pos, size_type block_index,
        size_type start_pos_in_block, const _T& it_begin, const _T& it_end);

    template<typename _T>
    void set_cells_to_multi_blocks(
        size_type start_pos, size_type end_pos,
        size_type block_index1, size_type start_pos_in_block1,
        size_type block_index2, size_type start_pos_in_block2,
        const _T& it_begin, const _T& it_end);

    template<typename _T>
    void set_cells_to_multi_blocks_block1_non_equal(
        size_type start_pos, size_type end_pos,
        size_type block_index1, size_type start_pos_in_block1,
        size_type block_index2, size_type start_pos_in_block2,
        const _T& it_begin, const _T& it_end);

    template<typename _T>
    void set_cells_to_multi_blocks_block1_non_empty(
        size_type start_pos, size_type end_pos,
        size_type block_index1, size_type start_pos_in_block1,
        size_type block_index2, size_type start_pos_in_block2,
        const _T& it_begin, const _T& it_end);

    template<typename _T>
    bool append_to_prev_block(
        size_type block_index, element_category_type cat, size_type length,
        const _T& it_begin, const _T& it_end);

    template<typename _T>
    void insert_cells_to_middle(
        size_type row, size_type block_index, size_type start_pos,
        const _T& it_begin, const _T& it_end);

private:
    blocks_type m_blocks;
    size_type m_cur_size;
};

}

#include "multi_type_vector_def.inl"

#endif
