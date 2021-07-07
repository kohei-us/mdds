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

#ifndef INCLUDED_MDDS_MULTI_TYPE_VECTOR_SOA_MAIN_HPP
#define INCLUDED_MDDS_MULTI_TYPE_VECTOR_SOA_MAIN_HPP

#include "../../global.hpp"
#include "../types.hpp"
#include "./iterator.hpp"

namespace mdds { namespace multi_type_vector { namespace soa {

namespace detail {

/**
 * Empty event function handler structure, used when no custom function
 * handler is specified.
 *
 * @see mdds::multi_type_vector::soa::multi_type_vector
 */
struct event_func
{
    void element_block_acquired(const mdds::mtv::base_element_block* /*block*/) {}

    void element_block_released(const mdds::mtv::base_element_block* /*block*/) {}
};

}

template<typename _ElemBlockFunc, typename _EventFunc = detail::event_func>
class multi_type_vector
{
public:
    using size_type = std::size_t;

    using element_block_type = mdds::mtv::base_element_block;
    using element_category_type = mdds::mtv::element_t;
    using element_block_func = _ElemBlockFunc;

    /**
     * Optional event handler function structure, whose functions get called
     * at specific events.  The following events are currently supported:
     *
     * <ul>
     * <li><code>element_block_acquired</code> - this gets called whenever
     * the container acquires a new element block either as a result of a new
     * element block creation or a tranfer of an existing
     * element block from another container.</li>
     * <li><code>element_block_released</code> - this gets called whenever
     * the container releases an existing element block either because
     * the block gets deleted or gets transferred to another container.</li>
     * </ul>
     *
     * @see mdds::detail::mtv_event_func for the precise function signatures
     *      of the event handler functions.
     */
    using event_func = _EventFunc;

private:
    struct block_slot_type
    {
        size_type position;
        size_type size;
        element_block_type* element_block;

        block_slot_type(size_type _position, size_type _size) :
            position(_position), size(_size), element_block(nullptr) {}
    };

    struct blocks_type
    {
        std::vector<size_type> positions;
        std::vector<size_type> sizes;
        std::vector<element_block_type*> element_blocks;

        blocks_type();
        blocks_type(const blocks_type& other);

        void pop_back()
        {
            positions.pop_back();
            sizes.pop_back();
            element_blocks.pop_back();
        }

        void push_back(size_type pos, size_type size, element_block_type* data)
        {
            positions.push_back(pos);
            sizes.push_back(size);
            element_blocks.push_back(data);
        }

        void erase(size_type index);
        void erase(size_type index, size_type size);
        void insert(size_type index, size_type size);
        void insert(size_type index, size_type pos, size_type size, element_block_type* data);

        /**
         * Calculate the position of specified block based on the position and
         * size of the previous block.
         *
         * @param index index of the block to calculate the position for.
         */
        void calc_block_position(size_type index);

        void swap(size_type index1, size_type index2);

        void swap(blocks_type& other);

        void reserve(size_type n);

        bool equals(const blocks_type& other) const;

        void clear();
    };

    struct iterator_trait
    {
        using parent = multi_type_vector;
        using positions_type = std::vector<size_type>;
        using sizes_type = std::vector<size_type>;
        using element_blocks_type = std::vector<element_block_type*>;
    };

    using itr_forward_update = mdds::detail::mtv::private_data_forward_update<size_type>;

    struct element_block_deleter
    {
        void operator() (const element_block_type* p)
        {
            element_block_func::delete_block(p);
        }
    };

public:

    using iterator = detail::iterator_base<iterator_trait, itr_forward_update>;
    using const_iterator = detail::const_iterator_base<iterator_trait, itr_forward_update, iterator>;

    /**
     * Default constructor.  It initializes the container with empty size.
     */
    multi_type_vector();

    /**
     * Constructor that takes initial size of the container.  When the size
     * specified is greater than 0, it initializes the container with empty
     * elements.
     *
     * @param init_size initial container size.
     */
    multi_type_vector(size_type init_size);

    /**
     * Constructor that takes initial size of the container and an element
     * value to initialize the elements to. When the size specified is greater
     * than 0, it initializes the container with elements that are copies of
     * the value specified.
     *
     * @param init_size initial container size.
     * @param value initial element value.
     */
    template<typename _T>
    multi_type_vector(size_type init_size, const _T& value);

    /**
     * Constructor that takes initial size of the container and begin and end
     * iterator positions that specify a series of elements to initialize the
     * container to.  The container will contain copies of the elements
     * specified after this call returns.
     *
     * @param init_size initial container size.
     * @param it_begin iterator that points to the begin position of the
     *                 values the container is being initialized to.
     * @param it_end iterator that points to the end position of the values
     *               the container is being initialized to.  The end position
     *               is <i>not</i> inclusive.
                                                                              */
    template<typename _T>
    multi_type_vector(size_type init_size, const _T& it_begin, const _T& it_end);

    /**
     * Copy constructor.
     *
     * @param other other column instance to copy values from.
     */
    multi_type_vector(const multi_type_vector& other);

    /**
     * Destructor.  It deletes all allocated element blocks.
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
     * @return iterator position pointing to the block where the value is
     *         inserted.
     */
    template<typename _T>
    iterator set(size_type pos, const _T& value);

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
     * @return iterator position pointing to the block where the value is
     *         inserted.  When no value insertion occurs because the value set
     *         is empty, the end iterator position is returned.
     */
    template<typename _T>
    iterator set(size_type pos, const _T& it_begin, const _T& it_end);

    /**
     * Append a new value to the end of the container.
     *
     * @param value new value to be appended to the end of the container.
     *
     * @return iterator position pointing to the block where the value is
     *         appended, which in this case is always the last block of the
     *         container.
     */
    template<typename _T>
    iterator push_back(const _T& value);

    /**
     * Append a new empty element to the end of the container.
     *
     * @return iterator position pointing to the block where the new empty
     *         element is appended, which in this case is always the last
     *         block of the container.
     */
    iterator push_back_empty();

    /**
     * Get the type of an element at specified position.
     *
     * @param pos position of the element.
     *
     * @return element type.
     */
    mtv::element_t get_type(size_type pos) const;

    /**
     * Check if element at specified position is empty of not.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the specified position is outside the current container
     * range.</p>
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
     * @return iterator position pointing to the block where the elements are
     *         emptied.
     */
    iterator set_empty(size_type start_pos, size_type end_pos);

    /**
     * Erase elements located between specified start and end positions. The
     * end positions are both inclusive.  Those elements originally located
     * after the specified end position will get shifted up after the erasure.
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
     * Insert a range of empty elements at specified position.  Those elements
     * originally located after the insertion position will get shifted down
     * after the insertion.
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
     * @return iterator position pointing to the block where the empty range
     *         is inserted. When no insertion occurs because the length is
     *         zero, the end iterator position is returned.
     */
    iterator insert_empty(size_type pos, size_type length);

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
     * Return the current number of blocks in the primary array.  Each
     * non-empty block stores a secondary block that stores elements in a
     * contiguous memory region (element block) and the number of elements it
     * stores.  An empty block only stores its logical size and does not store
     * an actual element block.
     *
     * <p>For instance, if the container stores values of double-precision
     * type at rows 0 to 2, values of std::string type at 3 to 7, and empty
     * values at 8 to 10, it would consist of three blocks: one that stores
     * double values, one that stores std::string values, and one that
     * represents the empty value range in this exact order.  In this specific
     * scenario, <code>block_size()</code> returns 3, and <code>size()</code>
     * returns 11.</p>
     *
     * @return current number of blocks in the primary array.
     */
    size_type block_size() const;

    /**
     * Return whether or not the container is empty.
     *
     * @return true if the container is empty, false otherwise.
     */
    bool empty() const;

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
     * @param pos position of the element value to retrieve.
     * @return element value.
     */
    template<typename _T>
    _T get(size_type pos) const;

    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

    const_iterator cbegin() const;
    const_iterator cend() const;

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

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    void dump_blocks(std::ostream& os) const;

    void check_block_integrity() const;
#endif

private:
    void adjust_block_positions(int64_t start_block_index, int64_t delta);

    void delete_element_block(size_type block_index);

    void delete_element_blocks(size_type start, size_type end);

    template<typename _T>
    bool set_cells_precheck(
        size_type row, const _T& it_begin, const _T& it_end, size_type& end_pos);

    template<typename _T>
    iterator set_impl(size_type pos, size_type block_index, const _T& value);

    void resize_impl(size_type new_size);

    /**
     * @param start_pos logical start position.
     * @param end_pos logical end position.
     * @param start_pos_in_block1 logical position of the first element of the
     *                            first block.
     * @param block_index1 index of the first block
     * @param overwrite when true, and when the stored values are pointers to
     *                  heap objects, objects pointed to by the overwritten
     *                  pointers should be freed from memory.
     */
    iterator set_empty_impl(
        size_type start_pos, size_type end_pos, size_type block_index1, bool overwrite);

    iterator set_empty_in_single_block(
        size_type start_row, size_type end_row, size_type block_index, bool overwrite);

    /**
     * @param start_pos logical start position.
     * @param end_pos logical end position.
     * @param block_index1 index of the first block.
     * @param block_index2 index of the last block.
     * @param overwrite when true, and when the stored values are pointers to
     *                  heap objects, objects pointed to by the overwritten
     *                  pointers should be freed from memory.
     */
    iterator set_empty_in_multi_blocks(
        size_type start_row, size_type end_row, size_type block_index1, size_type block_index2,
        bool overwrite);

    void erase_impl(size_type start_pos, size_type end_pos);
    void erase_in_single_block(size_type start_pos, size_type end_pos, size_type block_index);

    /**
     * @param row logical position at which to insert an empty segment.
     * @param block_index index of the block.
     * @param length length of the emtpy segment to insert.
     */
    iterator insert_empty_impl(size_type pos, size_type block_index, size_type length);

    iterator set_whole_block_empty(size_type block_index, bool overwrite);

    template<typename _T>
    iterator push_back_impl(const _T& value);

    template<typename _T>
    iterator set_cells_impl(
        size_type row, size_type end_row, size_type block_index1, const _T& it_begin, const _T& it_end);

    template<typename _T>
    iterator set_cells_to_single_block(
        size_type start_row, size_type end_row, size_type block_index,
        const _T& it_begin, const _T& it_end);

    template<typename _T>
    iterator set_cells_to_multi_blocks(
        size_type start_row, size_type end_row, size_type block_index1, size_type block_index2,
        const _T& it_begin, const _T& it_end);

    template<typename _T>
    iterator set_cells_to_multi_blocks_block1_non_equal(
        size_type start_row, size_type end_row, size_type block_index1, size_type block_index2,
        const _T& it_begin, const _T& it_end);

    template<typename _T>
    iterator set_cells_to_multi_blocks_block1_non_empty(
        size_type start_row, size_type end_row, size_type block_index1, size_type block_index2,
        const _T& it_begin, const _T& it_end);

    template<typename _T>
    iterator set_cell_to_empty_block(size_type block_index, size_type pos_in_block, const _T& cell);

    template<typename _T>
    iterator set_cell_to_non_empty_block_of_size_one(size_type block_index, const _T& cell);

    /**
     * Find the correct block position for a given logical row ID.
     *
     * @param start_block_index index of the first block to start the search
     *                          from.
     *
     * @return index of the block that contains the specified logical row ID.
     */
    size_type get_block_position(size_type row, size_type start_block_index=0) const;

    template<typename _T>
    void create_new_block_with_new_cell(size_type block_index, const _T& cell);

    template<typename _T>
    void append_cell_to_block(size_type block_index, const _T& cell);

    /**
     * Try to append a sequence of values to the previous block if the previous
     * block exists and is of the same type as the new values.
     *
     * @return true if the values have been appended successfully, otherwise
     *         false.
     */
    template<typename _T>
    bool append_to_prev_block(
        size_type block_index, element_category_type cat, size_type length,
        const _T& it_begin, const _T& it_end);

    template<typename _T>
    iterator set_cell_to_middle_of_block(
        size_type block_index, size_type pos_in_block, const _T& cell);

    /**
     * Set a new value to the top of specified non-empty block. The block is
     * expected to be of size greater than one, and the previous block is not of
     * the same type as the value being inserted.
     */
    template<typename _T>
    void set_cell_to_top_of_data_block(size_type block_index, const _T& cell);

    template<typename _T>
    void set_cell_to_bottom_of_data_block(size_type block_index, const _T& cell);

    /**
     * Merge only with the next block if the two are of the same type.
     *
     * @param block_index index of the block that may need merging.
     *
     * @return true if merge occurs, false otherwise.
     */
    bool merge_with_next_block(size_type block_index);

    /**
     * Set a new block in the middle of an existing block. This call inserts
     * two new blocks below the specificed block position. The first one will
     * be empty, and the second one will contain the lower elements of the
     * existing block.
     *
     * @param block_index index of block into which to set a new block.
     * @param offset position in the existing block to set the new block to.
     * @param new_block_size size of the new block
     * @param overwrite whether or not to overwrite the elements replaced by
     *                  the new block.
     * @return index of the inserted middle block.
     */
    size_type set_new_block_to_middle(
        size_type block_index, size_type offset, size_type new_block_size, bool overwrite);

    /**
     * Check if the previous block is of specified type, if exists.
     *
     * @param block_index index of the current block.
     * @param cat desired block type.
     *
     * @return true if the previous block exists and it's of specified type,
     *         otherwise false.
     */
    bool is_previous_block_of_type(size_type block_index, element_category_type cat) const;

    /**
     * Check if the next block is of specified type, if exists.
     *
     * @param block_index index of the current block.
     * @param cat desired block type.
     *
     * @return true if the next block exists and it's of specified type,
     *         otherwise false.
     */
    bool is_next_block_of_type(size_type block_index, element_category_type cat) const;

    bool append_empty(size_type len);

    inline iterator get_iterator(size_type block_index)
    {
        auto iter_pos = m_block_store.positions.begin();
        std::advance(iter_pos, block_index);
        auto iter_size = m_block_store.sizes.begin();
        std::advance(iter_size, block_index);
        auto iter_elem = m_block_store.element_blocks.begin();
        std::advance(iter_elem, block_index);

        return iterator(
            { iter_pos, iter_size, iter_elem },
            { m_block_store.positions.end(), m_block_store.sizes.end(), m_block_store.element_blocks.end() },
            block_index
        );
    }

private:
    event_func m_hdl_event;
    blocks_type m_block_store;
    size_type m_cur_size;
};

}}}

#include "main_def.inl"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

