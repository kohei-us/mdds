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
#include "../util.hpp"
#include "./block_util.hpp"
#include "./iterator.hpp"

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
#include <iostream>
#endif

namespace mdds { namespace mtv { namespace soa {

/**
 * Multi-type vector consists of a series of one or more blocks, and each
 * block may either be empty, or stores a series of non-empty elements of
 * identical type.  These blocks collectively represent a single logical
 * one-dimensional array that may store elements of different types.  It is
 * guaranteed that the block types of neighboring blocks are always
 * different.
 *
 * Structurally, the primary array stores block instances whose types
 * are of <code>value_type</code>, which in turn consists of the following
 * data members:
 *
 * <ul>
 * <li><code>type</code> which indicates the block type,</li>
 * <li><code>position</code> which stores the logical position of the
 * first element of the block,</li>
 * <li><code>size</code> which stores the logical size of the block,
 * and</li>
 * <li><code>data</code> which stores the pointer to a secondary array
 * (a.k.a. element block) which stores the actual element values, or
 * <code>nullptr</code> in case the block represents an empty segment.</li>
 * </ul>
 *
 * This variant implements a structure-of-arrays (SoA) storage.
 *
 * @see mdds::mtv::soa::multi_type_vector::value_type
 */
template<typename Traits = mdds::mtv::default_traits>
class multi_type_vector
{
public:
    using size_type = std::size_t;

    using element_block_type = mdds::mtv::base_element_block;
    using element_category_type = mdds::mtv::element_t;
    using block_funcs = typename Traits::block_funcs;

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
     * @see mdds::mtv::empty_event_func for the precise function signatures of
     *      the event handler functions.
     */
    using event_func = typename Traits::event_func;

private:
    struct block_slot_type
    {
        size_type position = 0;
        size_type size = 0;
        element_block_type* element_block = nullptr;

        block_slot_type()
        {}
        block_slot_type(size_type _position, size_type _size) : position(_position), size(_size)
        {}
    };

    struct blocks_type
    {
        std::vector<size_type> positions;
        std::vector<size_type> sizes;
        std::vector<element_block_type*> element_blocks;

        blocks_type();
        blocks_type(const blocks_type& other);
        blocks_type(blocks_type&& other);

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

        void push_back(const block_slot_type& slot)
        {
            positions.push_back(slot.position);
            sizes.push_back(slot.size);
            element_blocks.push_back(slot.element_block);
        }

        void erase(size_type index);
        void erase(size_type index, size_type size);
        void insert(size_type index, size_type size);
        void insert(size_type index, size_type pos, size_type size, element_block_type* data);
        void insert(size_type index, const blocks_type& new_blocks);

        /**
         * Calculate the position of specified block based on the position and
         * size of the previous block.
         *
         * @param index index of the block to calculate the position for.
         */
        void calc_block_position(size_type index);

        size_type calc_next_block_position(size_type index);

        void swap(size_type index1, size_type index2);

        void swap(blocks_type& other);

        void reserve(size_type n);

        bool equals(const blocks_type& other) const;

        void clear();

        void check_integrity() const;
    };

    struct blocks_to_transfer
    {
        blocks_type blocks;
        size_type insert_index = 0;
    };

    struct iterator_trait
    {
        using parent = multi_type_vector;
        using positions_type = std::vector<size_type>;
        using sizes_type = std::vector<size_type>;
        using element_blocks_type = std::vector<element_block_type*>;

        using positions_iterator_type = typename positions_type::iterator;
        using sizes_iterator_type = typename sizes_type::iterator;
        using element_blocks_iterator_type = typename element_blocks_type::iterator;

        using private_data_update = mdds::detail::mtv::private_data_forward_update<multi_type_vector, size_type>;
    };

    struct const_iterator_trait
    {
        using parent = multi_type_vector;
        using positions_type = std::vector<size_type>;
        using sizes_type = std::vector<size_type>;
        using element_blocks_type = std::vector<element_block_type*>;

        using positions_iterator_type = typename positions_type::const_iterator;
        using sizes_iterator_type = typename sizes_type::const_iterator;
        using element_blocks_iterator_type = typename element_blocks_type::const_iterator;

        using private_data_update = mdds::detail::mtv::private_data_forward_update<multi_type_vector, size_type>;
    };

    struct reverse_iterator_trait
    {
        using parent = multi_type_vector;
        using positions_type = std::vector<size_type>;
        using sizes_type = std::vector<size_type>;
        using element_blocks_type = std::vector<element_block_type*>;

        using positions_iterator_type = typename positions_type::reverse_iterator;
        using sizes_iterator_type = typename sizes_type::reverse_iterator;
        using element_blocks_iterator_type = typename element_blocks_type::reverse_iterator;

        using private_data_update = mdds::detail::mtv::private_data_no_update<multi_type_vector, size_type>;
    };

    struct const_reverse_iterator_trait
    {
        using parent = multi_type_vector;
        using positions_type = std::vector<size_type>;
        using sizes_type = std::vector<size_type>;
        using element_blocks_type = std::vector<element_block_type*>;

        using positions_iterator_type = typename positions_type::const_reverse_iterator;
        using sizes_iterator_type = typename sizes_type::const_reverse_iterator;
        using element_blocks_iterator_type = typename element_blocks_type::const_reverse_iterator;

        using private_data_update = mdds::detail::mtv::private_data_no_update<multi_type_vector, size_type>;
    };

    struct element_block_deleter
    {
        void operator()(const element_block_type* p)
        {
            block_funcs::delete_block(p);
        }
    };

public:
    using iterator = detail::iterator_base<iterator_trait>;
    using const_iterator = detail::const_iterator_base<const_iterator_trait, iterator>;

    using reverse_iterator = detail::iterator_base<reverse_iterator_trait>;
    using const_reverse_iterator = detail::const_iterator_base<const_reverse_iterator_trait, reverse_iterator>;

    using position_type = std::pair<iterator, size_type>;
    using const_position_type = std::pair<const_iterator, size_type>;

    /**
     * value_type is the type of a block stored in the primary array.  It
     * consists of the following data members:
     *
     * <ul>
     * <li><code>type</code> which indicates the block type,</li>
     * <li><code>position</code> which stores the logical position of the
     * first element of the block,</li>
     * <li><code>size</code> which stores the logical size of the block,
     * and</li>
     * <li><code>data</code> which stores the pointer to a secondary array
     * (a.k.a. element block) which stores the actual element values, or
     * <code>nullptr</code> in case the block represents an empty segment.</li>
     * </ul>
     */
    using value_type = mdds::detail::mtv::iterator_value_node<multi_type_vector, size_type>;

    /**
     * Move the position object to the next logical position.  Caller must
     * ensure the the position object is valid.
     *
     * @param pos position object.
     *
     * @return position object that points to the next logical position.
     */
    static position_type next_position(const position_type& pos);

    /**
     * Increment or decrement the position object by specified steps. Caller
     * must ensure the the position object is valid.
     *
     * @param pos position object.
     * @param steps steps to advance the position object.
     *
     * @return position object that points to the new logical position.
     */
    static position_type advance_position(const position_type& pos, int steps);

    /**
     * Move the position object to the next logical position.  Caller must
     * ensure the the position object is valid.
     *
     * @param pos position object.
     *
     * @return position object that points to the next logical position.
     */
    static const_position_type next_position(const const_position_type& pos);

    /**
     * Increment or decrement the position object by specified steps. Caller
     * must ensure the the position object is valid.
     *
     * @param pos position object.
     * @param steps steps to advance the position object.
     *
     * @return position object that points to the new logical position.
     */
    static const_position_type advance_position(const const_position_type& pos, int steps);

    /**
     * Extract the logical position from a position object.
     *
     * @param pos position object.
     *
     * @return logical position of the element that the position object
     *         references.
     */
    static size_type logical_position(const const_position_type& pos);

    /**
     * Get element value from a position object. The caller must specify the
     * type of block in which the element is expected to be stored.
     *
     * @param pos position object.
     *
     * @return element value.
     */
    template<typename _Blk>
    static typename _Blk::value_type get(const const_position_type& pos);

    event_func& event_handler();
    const event_func& event_handler() const;

    /**
     * Default constructor.  It initializes the container with empty size.
     */
    multi_type_vector();

    /**
     * Constructor that takes an lvalue reference to an event handler object.
     * The event handler instance will be copy-constructed.
     *
     * @param hdl lvalue reference to an event handler object.
     */
    multi_type_vector(const event_func& hdl);

    /**
     * Constructor that takes an rvalue reference to an event handler object.
     * The event handler instance will be move-constructed.
     *
     * @param hdl rvalue reference to an event handler object.
     */
    multi_type_vector(event_func&& hdl);

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
    template<typename T>
    multi_type_vector(size_type init_size, const T& value);

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
    template<typename T>
    multi_type_vector(size_type init_size, const T& it_begin, const T& it_end);

    /**
     * Copy constructor.
     *
     * @param other the other instance to copy values from.
     */
    multi_type_vector(const multi_type_vector& other);

    /**
     * Move constructor.
     *
     * @param other the other instance to move values from.
     */
    multi_type_vector(multi_type_vector&& other);

    /**
     * Destructor.  It deletes all allocated element blocks.
     */
    ~multi_type_vector();

    /**
     * Given the logical position of an element, get the iterator of the block
     * where the element is located, and its offset from the first element of
     * that block.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the specified position is outside the current container range, except
     * when the specified position is the position immediately after the last
     * valid position, it will return a valid position object representing
     * the end position.</p>
     *
     * @param pos logical position of the element.
     * @return position object that stores an iterator referencing the element
     *         block where the element resides, and its offset within that
     *         block.
     */
    position_type position(size_type pos);

    /**
     * Given the logical position of an element, get the iterator of the block
     * where the element is located, and its offset from the first element of
     * that block.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the specified position is outside the current container range, except
     * when the specified position is the position immediately after the last
     * valid position, it will return a valid position object representing
     * the end position.</p>
     *
     * @param pos_hint iterator used as a block position hint, to specify
     *                 which block to start when searching for the element
     *                 position.
     * @param pos logical position of the element.
     * @return position object that stores an iterator referencing the element
     *         block where the element resides, and its offset within that
     *         block.
     */
    position_type position(const iterator& pos_hint, size_type pos);

    /**
     * Given the logical position of an element, get an iterator referencing
     * the element block where the element is located, and its offset from the
     * first element of that block.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the specified position is outside the current container range.</p>
     *
     * @param pos position of the element.
     * @return position object that stores an iterator referencing the element
     *         block where the element resides, and its offset within that
     *         block.
     */
    const_position_type position(size_type pos) const;

    /**
     * Given the logical position of an element, get an iterator referencing
     * the element block where the element is located, and its offset from the
     * first element of that block.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the specified position is outside the current container range.</p>
     *
     * @param pos_hint iterator used as a block position hint, to specify
     *                 which block to start when searching for the element
     *                 position.
     * @param pos logical position of the element.
     * @return position object that stores an iterator referencing the element
     *         block where the element resides, and its offset within the
     *         block.
     */
    const_position_type position(const const_iterator& pos_hint, size_type pos) const;

    /**
     * Move elements from one container to another. After the move, the
     * segment where the elements were in the source container becomes empty.
     * When transferring managed elements, this call transfers ownership of
     * the moved elements to the destination container.  The moved elements
     * will overwrite any existing elements in the destination range of the
     * receiving container. Transfer of elements within the same container is
     * not allowed.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * either the starting or the ending position is greater than or equal to
     * the source container size, or the destination container is not
     * large enough to accommodate the transferred elements.</p>
     *
     * @param start_pos starting position
     * @param end_pos ending position, inclusive.
     * @param dest destination container to which the elements are to be
     *             moved.
     * @param dest_pos position in the destination container to which the
     *                 elements are to be moved.
     *
     * @return iterator referencing the block where the moved elements were
     *         prior to the transfer.
     */
    iterator transfer(size_type start_pos, size_type end_pos, multi_type_vector& dest, size_type dest_pos);

    /**
     * Move elements from one container to another. After the move, the
     * segment where the elements were in the source container becomes empty.
     * When transferring managed elements, this call transfers ownership of
     * the moved elements to the new container.  The moved elements will
     * overwrite any existing elements in the destination range of the
     * receiving container. Transfer of elements within the same container is
     * not allowed.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * either the starting or the ending position is greater than or equal to
     * the source container size, or the destination container is not large
     * enough to accommodate the transferred elements.</p>
     *
     * @param pos_hint iterator used as a block position hint, to specify
     *                 which block to start when searching for the blocks
     *                 where the elements to be transferred reside.
     * @param start_pos starting position
     * @param end_pos ending position, inclusive.
     * @param dest destination container to which the elements are to be
     *             moved.
     * @param dest_pos position in the destination container to which the
     *                 elements are to be moved.
     *
     * @return iterator referencing the block where the moved elements were
     *         prior to the transfer.
     */
    iterator transfer(
        const iterator& pos_hint, size_type start_pos, size_type end_pos, multi_type_vector& dest, size_type dest_pos);

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
    template<typename T>
    iterator set(size_type pos, const T& value);

    /**
     * Set a value of an arbitrary type to a specified position.  The type of
     * the value is inferred from the value passed to this method.  The new
     * value will overwrite an existing value at the specified position
     * position if any.
     *
     * <p>This variant takes an iterator as an additional parameter, which is
     * used as a block position hint to speed up the lookup of the
     * right block to insert the value into.  The other variant that doesn't
     * take an iterator always starts the block lookup from the first block,
     * which does not scale well as the block size grows.</p>
     *
     * <p>This position hint iterator must <b>precede</b> the insertion
     * position to yield any performance benefit.</p>
     *
     * <p>The caller is responsible for ensuring that the passed iterator is
     * valid.  The behavior of this method when passing an invalid iterator is
     * undefined.</p>
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception
     * if the specified position is outside the current container range.</p>
     *
     * <p>Calling this method will not change the size of the container.</p>
     *
     * @param pos_hint iterator used as a block position hint, to specify
     *                 which block to start when searching for the right block
     *                 to insert the value into.
     * @param pos position to insert the value to.
     * @param value value to insert.
     * @return iterator position pointing to the block where the value is
     *         inserted.
     */
    template<typename T>
    iterator set(const iterator& pos_hint, size_type pos, const T& value);

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
    template<typename T>
    iterator set(size_type pos, const T& it_begin, const T& it_end);

    /**
     * Set multiple values of identical type to a range of elements starting
     * at specified position.  Any existing values will be overwritten by the
     * new values.
     *
     * <p>This variant takes an iterator as an additional parameter, which is
     * used as a block position hint to speed up the lookup of the first
     * insertion block.  The other variant that doesn't take an iterator
     * always starts the block lookup from the first block, which does not
     * scale well as the block size grows.</p>
     *
     * <p>This position hint iterator must <b>precede</b> the insertion
     * position to yield any performance benefit.</p>
     *
     * <p>The caller is responsible for ensuring that the passed iterator is
     * valid.  The behavior of this method when passing an invalid iterator is
     * undefined.</p>
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the range of new values would fall outside the current container
     * range.</p>
     *
     * <p>Calling this method will not change the size of the container.</p>
     *
     * @param pos_hint iterator used as a block position hint, to specify
     *                 which block to start when searching for the right block
     *                 to insert the value into.
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
    template<typename T>
    iterator set(const iterator& pos_hint, size_type pos, const T& it_begin, const T& it_end);

    /**
     * Append a new value to the end of the container.
     *
     * This method takes the value either as an lvalue or rvalue reference and
     * moves the passed value into the destination element block if the value is
     * passed as an rvalue reference.  Note, however, that the value may still
     * be copied even if it is passed as an rvalue reference if the insertion
     * triggers reallocation of the buffer in the destination block and the
     * value type does not have a move constructor marked as noexcept.
     *
     * @param value New value to be appended to the end of the container.
     *
     * @return iterator position pointing to the block where the value is
     *         appended, which in this case is always the last block of the
     *         container.
     */
    template<typename T>
    iterator push_back(T&& value);

    /**
     * Append a new empty element to the end of the container.
     *
     * @return iterator position pointing to the block where the new empty
     *         element is appended, which in this case is always the last
     *         block of the container.
     */
    iterator push_back_empty();

    /**
     * Append a new element to the end of the container.
     *
     * The new element is constructed by using the `emplace_back()` method of
     * the destination element block, and the arguments `args` are forwarded to
     * it via `std::forward<Args>(args)...`.
     *
     * @param args Arguments to forward to the `emplace_back()` method of the
     *             destination element block.
     *
     * @return Iterator position pointing to the block where the value is
     *         appended, which in this case is always the last block of the
     *         container.
     *
     * @note Due to the limitation of the current implementation, this method
     *       requires the value type `T` to be default-constructible.
     */
    template<typename T, typename... Args>
    iterator emplace_back(Args&&... args);

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
     * @return iterator position pointing to the block where the value is
     *         inserted.  When no value insertion occurs because the value set
     *         is empty, the end iterator position is returned.
     */
    template<typename T>
    iterator insert(size_type pos, const T& it_begin, const T& it_end);

    /**
     * Insert multiple values of identical type to a specified position.
     * Existing values that occur at or below the specified position will get
     * shifted after the insertion.  No existing values will be overwritten by
     * the inserted values.
     *
     * <p>This variant takes an iterator as an additional parameter, which is
     * used as a block position hint to speed up the lookup of the first
     * insertion block.  The other variant that doesn't take an iterator
     * always starts the block lookup from the first block, which does not
     * scale well as the block size grows.</p>
     *
     * <p>This position hint iterator must <b>precede</b> the insertion
     * position to yield any performance benefit.</p>
     *
     * <p>The caller is responsible for ensuring that the passed iterator is
     * valid.  The behavior of this method when passing an invalid iterator is
     * undefined.</p>
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception
     * if the specified position is outside the current container range.</p>
     *
     * <p>Calling this method will increase the size of the container by
     * the length of the new values inserted.</p>
     *
     * @param pos_hint iterator used as a block position hint, to specify
     *                 which block to start when searching for the right block
     *                 to insert the value into.
     * @param pos position at which the new values are to be inserted.
     * @param it_begin iterator that points to the begin position of the
     *                 values being inserted.
     * @param it_end iterator that points to the end position of the values
     *               being inserted.
     * @return iterator position pointing to the block where the value is
     *         inserted.  When no value insertion occurs because the value set
     *         is empty, the end iterator position is returned.
     */
    template<typename T>
    iterator insert(const iterator& pos_hint, size_type pos, const T& it_begin, const T& it_end);

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
     * Set specified range of elements to be empty.  Any existing values will
     * be overwritten.
     *
     * <p>This variant takes an iterator as an additional parameter, which is
     * used as a block position hint to speed up the lookup of the first block
     * to empty.  The other variant that doesn't take an iterator always
     * starts the block lookup from the first block, which does not
     * scale well as the block size grows.</p>
     *
     * <p>This position hint iterator must <b>precede</b> the start
     * position to yield any performance benefit.</p>
     *
     * <p>The caller is responsible for ensuring that the passed iterator is
     * valid.  The behavior of this method when passing an invalid iterator is
     * undefined.</p>
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * either the starting or the ending position is outside the current
     * container size.</p>
     *
     * @param pos_hint iterator used as a block position hint, to specify
     *                 which block to start when searching for the right
     *                 blocks to empty.
     * @param start_pos starting position
     * @param end_pos ending position, inclusive.
     * @return iterator position pointing to the block where the elements are
     *         emptied.
     */
    iterator set_empty(const iterator& pos_hint, size_type start_pos, size_type end_pos);

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
     * Insert a range of empty elements at specified position.  Those elements
     * originally located after the insertion position will get shifted down
     * after the insertion.
     *
     * <p>This variant takes an iterator as an additional parameter, which is
     * used as a block position hint to speed up the lookup of the block in
     * which to insert the new empty segment.  The other variant that doesn't
     * take an iterator always starts the block lookup from the first block,
     * which does not scale well as the block size grows.</p>
     *
     * <p>This position hint iterator must <b>precede</b> the start
     * position to yield any performance benefit.</p>
     *
     * <p>The caller is responsible for ensuring that the passed iterator is
     * valid.  The behavior of this method when passing an invalid iterator is
     * undefined.</p>
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * either the specified position is outside the current container
     * range.</p>
     *
     * <p>Calling this method will increase the size of the container by
     * the length of the inserted empty elements.</p>
     *
     * @param pos_hint iterator used as a block position hint, to specify
     *                 which block to start when searching for the right block
     *                 in which to insert the empty segment.
     * @param pos position at which to insert a range of empty elements.
     * @param length number of empty elements to insert.
     * @return iterator position pointing to the block where the empty range
     *         is inserted. When no insertion occurs because the length is
     *         zero, the end iterator position is returned.
     */
    iterator insert_empty(const iterator& pos_hint, size_type pos, size_type length);

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
    template<typename T>
    void get(size_type pos, T& value) const;

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
    template<typename T>
    T get(size_type pos) const;

    /**
     * Return the value of an element at specified position and set that
     * position empty.  If the element resides in a managed element block,
     * this call will release that element from that block.  If the element is
     * on a non-managed element block, this call is equivalent to
     * set_empty(pos, pos).
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the specified position is outside the current container range.</p>
     *
     * @param pos position of the element to release.
     *
     * @return element value.
     */
    template<typename T>
    T release(size_type pos);

    /**
     * Retrieve the value of an element at specified position and set that
     * position empty.  If the element resides in a managed element block,
     * this call will release that element from that block.  If the element is
     * on a non-managed element block, this call is equivalent to
     * set_empty(pos, pos).
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the specified position is outside the current container range.</p>
     *
     * @param pos position of the element to release.
     * @param value element value.
     *
     * @return iterator referencing the block where the position of the
     *         released element is.
     */
    template<typename T>
    iterator release(size_type pos, T& value);

    /**
     * Retrieve the value of an element at specified position and set that
     * position empty.  If the element resides in a managed element block,
     * this call will release that element from that block.  If the element is
     * on a non-managed element block, this call is equivalent to
     * set_empty(pos, pos).
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * the specified position is outside the current container range.</p>
     *
     * @param pos_hint iterator used as a block position hint, to specify
     *                 which block to start when searching for the block where
     *                 the element resides.
     * @param pos position of the element to release.
     * @param value element value.
     *
     * @return iterator referencing the block where the position of the
     *         released element is.
     */
    template<typename T>
    iterator release(const iterator& pos_hint, size_type pos, T& value);

    /**
     * Release all its elements, and empties its content.  Calling this method
     * relinquishes the ownership of all elements stored in managed element
     * blocks if any.
     *
     * <p>This call is equivalent of clear() if the container consists of no
     * managed element blocks.</p>
     */
    void release();

    /**
     * Make all elements within specified range empty, and relinquish the
     * ownership of the elements in that range.  All elements in the managed
     * element blocks within the range will be released and the container will
     * no longer manage their life cycles after the call.
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * either the starting or the ending position is outside the current
     * container size.</p>
     *
     * @param start_pos starting position
     * @param end_pos ending position, inclusive.
     * @return iterator position pointing to the block where the elements are
     *         released.
     */
    iterator release_range(size_type start_pos, size_type end_pos);

    /**
     * Make all elements within specified range empty, and relinquish the
     * ownership of the elements in that range.  All elements in the managed
     * element blocks within the range will be released and the container will
     * no longer manage their life cycles after the call.
     *
     * <p>This variant takes an iterator as an additional parameter, which is
     * used as a block position hint to speed up the lookup of the first block
     * to empty.  The other variant that doesn't take an iterator always
     * starts the block lookup from the first block, which does not
     * scale well as the block size grows.</p>
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception if
     * either the starting or the ending position is outside the current
     * container size.</p>
     *
     * @param pos_hint iterator used as a block position hint, to specify
     *                 which block to start when searching for the right
     *                 blocks in which elements are to be released.
     * @param start_pos starting position
     * @param end_pos ending position, inclusive.
     * @return iterator position pointing to the block where the elements are
     *         released.
     */
    iterator release_range(const iterator& pos_hint, size_type start_pos, size_type end_pos);

    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

    const_iterator cbegin() const;
    const_iterator cend() const;

    reverse_iterator rbegin();
    reverse_iterator rend();

    const_reverse_iterator rbegin() const;
    const_reverse_iterator rend() const;

    const_reverse_iterator crbegin() const;
    const_reverse_iterator crend() const;

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

    /**
     * Swap a part of the content with another instance.
     *
     * @param start_pos starting position
     * @param end_pos ending position, inclusive.
     * @param other another instance to swap the content with.
     * @param other_pos insertion position in the other instance.
     */
    void swap(size_type start_pos, size_type end_pos, multi_type_vector& other, size_type other_pos);

    /**
     * Trim excess capacity from all non-empty blocks.
     */
    void shrink_to_fit();

    bool operator==(const multi_type_vector& other) const;
    bool operator!=(const multi_type_vector& other) const;

    multi_type_vector& operator=(const multi_type_vector& other);
    multi_type_vector& operator=(multi_type_vector&& other);

    /**
     * Return the numerical identifier that represents passed element.
     *
     * @param elem element value.
     *
     * @return numerical identifier representing the element.
     */
    template<typename T>
    static mtv::element_t get_element_type(const T& elem);

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    void dump_blocks(std::ostream& os) const;

    void check_block_integrity() const;
#endif

private:
    void delete_element_block(size_type block_index);

    void delete_element_blocks(size_type start, size_type end);

    template<typename T>
    void get_impl(size_type pos, T& value) const;

    template<typename T>
    bool set_cells_precheck(size_type row, const T& it_begin, const T& it_end, size_type& end_pos);

    template<typename T>
    iterator set_impl(size_type pos, size_type block_index, const T& value);

    template<typename T>
    iterator release_impl(size_type pos, size_type block_index, T& value);

    void swap_impl(
        multi_type_vector& other, size_type start_pos, size_type end_pos, size_type other_pos, size_type block_index1,
        size_type block_index2, size_type dblock_index1, size_type dblock_index2);

    void swap_single_block(
        multi_type_vector& other, size_type start_pos, size_type end_pos, size_type other_pos, size_type block_index,
        size_type other_block_index);

    void swap_single_to_multi_blocks(
        multi_type_vector& other, size_type start_pos, size_type end_pos, size_type other_pos, size_type block_index,
        size_type dst_block_index1, size_type dst_block_index2);

    void swap_multi_to_multi_blocks(
        multi_type_vector& other, size_type start_pos, size_type end_pos, size_type other_pos, size_type block_index1,
        size_type block_index2, size_type dblock_index1, size_type dblock_index2);

    template<typename T>
    iterator insert_cells_impl(size_type row, size_type block_index, const T& it_begin, const T& it_end);

    void resize_impl(size_type new_size);

    /**
     * Elements to transfer to the other container span across multiple
     * blocks.
     */
    iterator transfer_multi_blocks(
        size_type start_pos, size_type end_pos, size_type block_index1, size_type block_index2, multi_type_vector& dest,
        size_type dest_pos);

    /**
     * @param start_pos logical start position.
     * @param end_pos logical end position.
     * @param block_index1 index of the first block
     * @param overwrite when true, and when the stored values are pointers to
     *                  heap objects, objects pointed to by the overwritten
     *                  pointers should be freed from memory.
     */
    iterator set_empty_impl(size_type start_pos, size_type end_pos, size_type block_index1, bool overwrite);

    iterator set_empty_in_single_block(size_type start_row, size_type end_row, size_type block_index, bool overwrite);

    /**
     * @param start_row logical start position.
     * @param end_row logical end position.
     * @param block_index1 index of the first block.
     * @param block_index2 index of the last block.
     * @param overwrite when true, and when the stored values are pointers to
     *                  heap objects, objects pointed to by the overwritten
     *                  pointers should be freed from memory.
     */
    iterator set_empty_in_multi_blocks(
        size_type start_row, size_type end_row, size_type block_index1, size_type block_index2, bool overwrite);

    void erase_impl(size_type start_pos, size_type end_pos);
    void erase_in_single_block(size_type start_pos, size_type end_pos, size_type block_index);

    /**
     * @param pos logical position at which to insert an empty segment.
     * @param block_index index of the block.
     * @param length length of the emtpy segment to insert.
     */
    iterator insert_empty_impl(size_type pos, size_type block_index, size_type length);

    void insert_blocks_at(size_type position, size_type insert_pos, blocks_type& new_blocks);

    void prepare_blocks_to_transfer(
        blocks_to_transfer& bucket, size_type block_index1, size_type offset1, size_type block_index2,
        size_type offset2);

    iterator set_whole_block_empty(size_type block_index, bool overwrite);

    template<typename T>
    iterator push_back_impl(T&& value);

    template<typename T, typename... Args>
    iterator emplace_back_impl(Args&&... args);

    template<typename T>
    iterator set_cells_impl(
        size_type row, size_type end_row, size_type block_index1, const T& it_begin, const T& it_end);

    template<typename T>
    iterator set_cells_to_single_block(
        size_type start_row, size_type end_row, size_type block_index, const T& it_begin, const T& it_end);

    template<typename T>
    iterator set_cells_to_multi_blocks(
        size_type start_row, size_type end_row, size_type block_index1, size_type block_index2, const T& it_begin,
        const T& it_end);

    template<typename T>
    iterator set_cells_to_multi_blocks_block1_non_equal(
        size_type start_row, size_type end_row, size_type block_index1, size_type block_index2, const T& it_begin,
        const T& it_end);

    template<typename T>
    iterator set_cells_to_multi_blocks_block1_non_empty(
        size_type start_row, size_type end_row, size_type block_index1, size_type block_index2, const T& it_begin,
        const T& it_end);

    template<typename T>
    iterator set_cell_to_empty_block(size_type block_index, size_type pos_in_block, const T& cell);

    template<typename T>
    iterator set_cell_to_non_empty_block_of_size_one(size_type block_index, const T& cell);

    /**
     * Find the correct block position for a given logical row ID.
     *
     * @param row logical position of an element.
     * @param start_block_index index of the first block to start the search
     *                          from.
     *
     * @return index of the block that contains the specified logical row ID.
     */
    size_type get_block_position(size_type row, size_type start_block_index = 0) const;

    /**
     * Same as above, but try to infer block position from the private position
     * data stored in the iterator first before trying full search.
     */
    size_type get_block_position(const typename value_type::private_data& pos_data, size_type row) const;

    template<typename T>
    void create_new_block_with_new_cell(size_type block_index, T&& cell);

    template<typename T, typename... Args>
    void create_new_block_with_emplace_back(size_type block_index, const T& t, Args&&... args);

    template<typename T>
    void append_cell_to_block(size_type block_index, const T& cell);

    /**
     * Try to append a sequence of values to the previous block if the previous
     * block exists and is of the same type as the new values.
     *
     * @return true if the values have been appended successfully, otherwise
     *         false.
     */
    template<typename T>
    bool append_to_prev_block(
        size_type block_index, element_category_type cat, size_type length, const T& it_begin, const T& it_end);

    template<typename T>
    void insert_cells_to_middle(size_type row, size_type block_index, const T& it_begin, const T& it_end);

    template<typename T>
    iterator set_cell_to_middle_of_block(size_type block_index, size_type pos_in_block, const T& cell);

    /**
     * Set a new value to the top of specified non-empty block. The block is
     * expected to be of size greater than one, and the previous block is not of
     * the same type as the value being inserted.
     */
    template<typename T>
    void set_cell_to_top_of_data_block(size_type block_index, const T& cell);

    template<typename T>
    void set_cell_to_bottom_of_data_block(size_type block_index, const T& cell);

    iterator transfer_impl(
        size_type start_pos, size_type end_pos, size_type block_index1, multi_type_vector& dest, size_type dest_pos);

    /**
     * All elements to transfer to the other instance is in the same block.
     */
    iterator transfer_single_block(
        size_type start_pos, size_type end_pos, size_type block_index1, multi_type_vector& dest, size_type dest_pos);

    /**
     * Merge with previous or next block as needed.
     *
     * @param block_index index of the block that may need merging.
     *
     * @return size of previous block if the block is merged with the previous
     *         block, or 0 if it didn't merge with the previous block.
     */
    size_type merge_with_adjacent_blocks(size_type block_index);

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

    /**
     * Send elements from a source block to place them in a destination block.
     * In return, the method returns the elements in the destination block
     * that have been replaced by the elements sent by the caller.  The caller
     * needs to manage the life cycle of the returned block.
     *
     * Note that the destination block is expected to be non-empty. This also
     * implies that the returned block is never null and always contains
     * elements.
     *
     * @param src_data source data block from which the elements are sent.
     * @param src_offset position of the first element in the source block.
     * @param dst_index destination block index.
     * @param dst_offset position in the destination block where the sent
     *                   elements are to be placed.
     * @param len length of elements.
     *
     * @return heap allocated block that contains the overwritten elements
     *         originally in the destination block. The caller needs to manage
     *         its life cycle.
     */
    element_block_type* exchange_elements(
        const element_block_type& src_data, size_type src_offset, size_type dst_index, size_type dst_offset,
        size_type len);

    void exchange_elements(
        const element_block_type& src_blk, size_type src_offset, size_type dst_index1, size_type dst_offset1,
        size_type dst_index2, size_type dst_offset2, size_type len, blocks_type& new_blocks);

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
            {iter_pos, iter_size, iter_elem},
            {m_block_store.positions.end(), m_block_store.sizes.end(), m_block_store.element_blocks.end()}, this,
            block_index);
    }

    inline const_iterator get_const_iterator(size_type block_index) const
    {
        auto iter_pos = m_block_store.positions.cbegin();
        std::advance(iter_pos, block_index);
        auto iter_size = m_block_store.sizes.cbegin();
        std::advance(iter_size, block_index);
        auto iter_elem = m_block_store.element_blocks.cbegin();
        std::advance(iter_elem, block_index);

        return const_iterator(
            {iter_pos, iter_size, iter_elem},
            {m_block_store.positions.cend(), m_block_store.sizes.cend(), m_block_store.element_blocks.cend()}, this,
            block_index);
    }

private:
    using adjust_block_positions_func = detail::adjust_block_positions<blocks_type, Traits::loop_unrolling>;

    event_func m_hdl_event;
    blocks_type m_block_store;
    size_type m_cur_size;

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    mutable int m_trace_call_depth = 0;
#endif
};

}}} // namespace mdds::mtv::soa

#include "main_def.inl"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
