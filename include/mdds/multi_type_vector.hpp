/*************************************************************************
 *
 * Copyright (c) 2011-2018 Kohei Yoshida
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

#ifndef INCLUDED_MDDS_MULTI_TYPE_VECTOR_HPP
#define INCLUDED_MDDS_MULTI_TYPE_VECTOR_HPP

#include "global.hpp"
#include "multi_type_vector_types.hpp"
#include "multi_type_vector_itr.hpp"

#include <vector>
#include <algorithm>
#include <cassert>
#include <sstream>

#if defined(MDDS_UNIT_TEST) || defined (MDDS_MULTI_TYPE_VECTOR_DEBUG)
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#endif

namespace mdds {

namespace detail { namespace mtv {

/**
 * Empty event function handler structure, used when no custom function
 * handler is specified.
 *
 * @see mdds::multi_type_vector
 */
struct event_func
{
    void element_block_acquired(const mdds::mtv::base_element_block* /*block*/) {}

    void element_block_released(const mdds::mtv::base_element_block* /*block*/) {}
};

template<typename T>
T advance_position(const T& pos, int steps);

}}

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
 * @see mdds::multi_type_vector::value_type
 */
template<typename _ElemBlockFunc, typename _EventFunc = detail::mtv::event_func>
class multi_type_vector
{
public:
    typedef size_t size_type;

    typedef typename mdds::mtv::base_element_block element_block_type;
    typedef typename mdds::mtv::element_t element_category_type;
    typedef _ElemBlockFunc element_block_func;

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
    typedef _EventFunc event_func;

private:

    struct block
    {
        size_type m_size;
        element_block_type* mp_data;

        block();
        block(size_type _size);
        block(size_type _size, element_block_type* _data);
        block(const block& other);
        block(block&& other);
        ~block();
        void swap(block& other);
        void clone_to(block& other) const;

        block& operator=(block);
    };

    struct element_block_deleter
    {
        void operator() (const element_block_type* p)
        {
            element_block_func::delete_block(p);
        }
    };

    typedef std::vector<block> blocks_type;

    struct blocks_to_transfer
    {
        blocks_type blocks;
        size_type insert_index;

        blocks_to_transfer();
    };

    struct iterator_trait
    {
        typedef multi_type_vector parent;
        typedef blocks_type blocks;
        typedef typename blocks_type::iterator base_iterator;
    };

    struct reverse_iterator_trait
    {
        typedef multi_type_vector parent;
        typedef blocks_type blocks;
        typedef typename blocks_type::reverse_iterator base_iterator;
    };

    struct const_iterator_trait
    {
        typedef multi_type_vector parent;
        typedef blocks_type blocks;
        typedef typename blocks_type::const_iterator base_iterator;
    };

    struct const_reverse_iterator_trait
    {
        typedef multi_type_vector parent;
        typedef blocks_type blocks;
        typedef typename blocks_type::const_reverse_iterator base_iterator;
    };

    typedef detail::mtv::iterator_value_node<size_type, element_block_type> itr_node;
    typedef detail::mtv::private_data_forward_update<itr_node> itr_forward_update;
    typedef detail::mtv::private_data_no_update<itr_node> itr_no_update;

public:

    typedef detail::mtv::iterator_base<iterator_trait, itr_forward_update> iterator;
    typedef detail::mtv::iterator_base<reverse_iterator_trait, itr_no_update> reverse_iterator;

    typedef detail::mtv::const_iterator_base<const_iterator_trait, itr_forward_update, iterator> const_iterator;
    typedef detail::mtv::const_iterator_base<const_reverse_iterator_trait, itr_no_update, reverse_iterator> const_reverse_iterator;

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
    typedef itr_node value_type;

    typedef std::pair<iterator, size_type> position_type;
    typedef std::pair<const_iterator, size_type> const_position_type;

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
     * @return iterator position pointing to the block where the value is
     *         inserted.
     */
    template<typename _T>
    iterator set(size_type pos, const _T& value);

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
    template<typename _T>
    iterator set(const iterator& pos_hint, size_type pos, const _T& value);

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
    template<typename _T>
    iterator set(const iterator& pos_hint, size_type pos, const _T& it_begin, const _T& it_end);

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
    template<typename _T>
    iterator insert(size_type pos, const _T& it_begin, const _T& it_end);

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
    template<typename _T>
    iterator insert(const iterator& pos_hint, size_type pos, const _T& it_begin, const _T& it_end);

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
    template<typename _T>
    _T release(size_type pos);

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
    template<typename _T>
    iterator release(size_type pos, _T& value);

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
    template<typename _T>
    iterator release(const iterator& pos_hint, size_type pos, _T& value);

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
    iterator transfer(const iterator& pos_hint, size_type start_pos, size_type end_pos, multi_type_vector& dest, size_type dest_pos);

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
     * contiguous region in memory (element block) and the number of elements
     * it stores.  An empty block only stores its logical size and does not
     * store an actual element block.
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
     * Swap a part of the content with another container.
     *
     * @param start_pos starting position
     * @param end_pos ending position, inclusive.
     * @param other another container to swap the content with.
     * @param other_pos insertion position in the other container.
     */
    void swap(size_type start_pos, size_type end_pos, multi_type_vector& other, size_type other_pos);

    /**
     * Trim excess capacity from all non-empty blocks.
     */
    void shrink_to_fit();

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

#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
    void dump_blocks(std::ostream& os) const;

    bool check_block_integrity() const;
#endif

private:

    /**
     * Delete only the element block owned by an outer block.
     *
     * @param blk reference to the instance of an outer block that may own an
     *          element block instance.
     */
    void delete_element_block(block& blk);

    /**
     * Delete the element block(s) owned by one or more outer blocks in the
     * specified iterator ranges.
     *
     * @param it start position.
     * @param it_end end position (not inclusive).
     */
    void delete_element_blocks(typename blocks_type::iterator it, typename blocks_type::iterator it_end);

    template<typename _T>
    iterator set_impl(size_type pos, size_type start_row, size_type block_index, const _T& value);

    template<typename _T>
    iterator release_impl(size_type pos, size_type start_pos, size_type block_index, _T& value);

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

    /**
     * Same as above, but try to infer block position from the iterator first
     * before trying full search.
     */
    void get_block_position(const const_iterator& pos_hint, size_type pos, size_type& start_pos, size_type& block_index) const;

    template<typename _T>
    void create_new_block_with_new_cell(element_block_type*& data, const _T& cell);

    template<typename _T>
    iterator set_cell_to_middle_of_block(
        size_type start_row, size_type block_index, size_type pos_in_block, const _T& cell);

    template<typename _T>
    void append_cell_to_block(size_type block_index, const _T& cell);

    template<typename _T>
    iterator set_cell_to_empty_block(
        size_type start_row, size_type block_index, size_type pos_in_block, const _T& cell);

    template<typename _T>
    iterator set_cell_to_block_of_size_one(
        size_type start_row, size_type block_index, const _T& cell);

    template<typename _T>
    void set_cell_to_top_of_data_block(
        size_type block_index, const _T& cell);

    template<typename _T>
    void set_cell_to_bottom_of_data_block(
        size_type block_index, const _T& cell);

    iterator transfer_impl(
        size_type start_pos, size_type end_pos, size_type start_pos_in_block1, size_type block_index1,
        multi_type_vector& dest, size_type dest_pos);

    /**
     * All elements to transfer to the other container is in the same block.
     */
    iterator transfer_single_block(
        size_type start_pos, size_type end_pos, size_type start_pos_in_block1, size_type block_index1,
        multi_type_vector& dest, size_type dest_pos);

    /**
     * Elements to transfer to the other container span across multiple
     * blocks.
     */
    iterator transfer_multi_blocks(
        size_type start_pos, size_type end_pos, size_type start_pos_in_block1, size_type block_index1,
        size_type start_pos_in_block2, size_type block_index2,
        multi_type_vector& dest, size_type dest_pos);

    iterator set_empty_impl(
        size_type start_pos, size_type end_pos, size_type start_pos_in_block1, size_type block_index1,
        bool overwrite);

    void swap_impl(
        multi_type_vector& other, size_type start_pos, size_type end_pos, size_type other_pos,
        size_type start_pos_in_block1, size_type block_index1, size_type start_pos_in_block2, size_type block_index2,
        size_type start_pos_in_dblock1, size_type dblock_index1, size_type start_pos_in_dblock2, size_type dblock_index2);

    void swap_single_block(
        multi_type_vector& other, size_type start_pos, size_type end_pos, size_type other_pos,
        size_type start_pos_in_block, size_type block_index, size_type start_pos_in_other_block, size_type other_block_index);

    void swap_single_to_multi_blocks(
        multi_type_vector& other, size_type start_pos, size_type end_pos, size_type other_pos,
        size_type start_pos_in_block, size_type block_index, size_type dst_start_pos_in_block1, size_type dst_block_index1,
        size_type dst_start_pos_in_block2, size_type dst_block_index2);

    void swap_multi_to_multi_blocks(
        multi_type_vector& other, size_type start_pos, size_type end_pos, size_type other_pos,
        size_type start_pos_in_block1, size_type block_index1, size_type start_pos_in_block2, size_type block_index2,
        size_type start_pos_in_dblock1, size_type dblock_index1, size_type start_pos_in_dblock2, size_type dblock_index2);

    void insert_blocks_at(size_type insert_pos, blocks_type& new_blocks);

    void prepare_blocks_to_transfer(blocks_to_transfer& bucket, size_type block_index1, size_type offset1, size_type block_index2, size_type offset2);

    iterator set_whole_block_empty(size_type block_index, size_type start_pos_in_block, bool overwrite);

    iterator set_empty_in_single_block(
        size_type start_pos, size_type end_pos, size_type block_index, size_type start_pos_in_block,
        bool overwrite);

    iterator set_empty_in_multi_blocks(
        size_type start_pos, size_type end_pos,
        size_type block_index1, size_type start_pos_in_block1,
        size_type block_index2, size_type start_pos_in_block2, bool overwrite);

    void erase_impl(size_type start_pos, size_type end_pos);
    void erase_in_single_block(
        size_type start_pos, size_type end_pos, size_type block_pos, size_type start_pos_in_block);

    iterator insert_empty_impl(size_type row, size_type start_pos, size_type block_index, size_type length);

    template<typename _T>
    bool set_cells_precheck(
        size_type row, const _T& it_begin, const _T& it_end, size_type& end_pos);

    template<typename _T>
    iterator set_cells_impl(
        size_type row, size_type end_row, size_type start_row1, size_type block_index1, const _T& it_begin, const _T& it_end);

    template<typename _T>
    iterator insert_cells_impl(size_type row, size_type start_row, size_type block_index, const _T& it_begin, const _T& it_end);

    template<typename _T>
    iterator set_cells_to_single_block(
        size_type start_pos, size_type end_pos, size_type block_index,
        size_type start_pos_in_block, const _T& it_begin, const _T& it_end);

    template<typename _T>
    iterator set_cells_to_multi_blocks(
        size_type start_pos, size_type end_pos,
        size_type block_index1, size_type start_pos_in_block1,
        size_type block_index2, size_type start_pos_in_block2,
        const _T& it_begin, const _T& it_end);

    template<typename _T>
    iterator set_cells_to_multi_blocks_block1_non_equal(
        size_type start_pos, size_type end_pos,
        size_type block_index1, size_type start_pos_in_block1,
        size_type block_index2, size_type start_pos_in_block2,
        const _T& it_begin, const _T& it_end);

    template<typename _T>
    iterator set_cells_to_multi_blocks_block1_non_empty(
        size_type start_pos, size_type end_pos,
        size_type block_index1, size_type start_pos_in_block1,
        size_type block_index2, size_type start_pos_in_block2,
        const _T& it_begin, const _T& it_end);

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

    template<typename _T>
    bool append_to_prev_block(
        size_type block_index, element_category_type cat, size_type length,
        const _T& it_begin, const _T& it_end);

    template<typename _T>
    void insert_cells_to_middle(
        size_type row, size_type block_index, size_type start_pos,
        const _T& it_begin, const _T& it_end);

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
     * @return reference to the middle block
     */
    block& set_new_block_to_middle(
        size_type block_index, size_type offset, size_type new_block_size, bool overwrite);

    block* get_previous_block_of_type(size_type block_index, element_category_type cat);

    /**
     * @param block_index index of the current block.
     * @param cat desired block type.
     *
     * @return pointer to the next block if the next block exists and it's of
     *         specified type, otherwise nullptr will be returned.
     */
    block* get_next_block_of_type(size_type block_index, element_category_type cat);

    /**
     * Send elements from a source block to place them in a destination block.
     * In return, the method returns the elements in the destination block
     * that have been replaced by the elements sent by the caller.  The caller
     * needs to manage the life cycle of the returned block.
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
        const element_block_type& src_data, size_type src_offset, size_type dst_index, size_type dst_offset, size_type len);

    void exchange_elements(
        const element_block_type& src_data, size_type src_offset,
        size_type dst_index1, size_type dst_offset1, size_type dst_index2, size_type dst_offset2,
        size_type len, blocks_type& new_blocks);

    bool append_empty(size_type len);

    inline iterator get_iterator(size_type block_index, size_type start_row)
    {
        typename blocks_type::iterator block_pos = m_blocks.begin();
        std::advance(block_pos, block_index);
        return iterator(block_pos, m_blocks.end(), start_row, block_index);
    }

    inline const_iterator get_const_iterator(size_type block_index, size_type start_row) const
    {
        typename blocks_type::const_iterator block_pos = m_blocks.begin();
        std::advance(block_pos, block_index);
        return const_iterator(block_pos, m_blocks.end(), start_row, block_index);
    }

private:
    event_func m_hdl_event;
    blocks_type m_blocks;
    size_type m_cur_size;
};

}

#include "multi_type_vector_def.inl"

#endif
