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

namespace mdds { namespace multi_type_vector { namespace soa {

namespace detail { namespace mtv {

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

}}

template<typename _ElemBlockFunc, typename _EventFunc = detail::mtv::event_func>
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
    struct blocks_type
    {
        std::vector<size_type> positions;
        std::vector<size_type> sizes;
        std::vector<element_block_type*> element_blocks;
    };

    struct element_block_deleter
    {
        void operator() (const element_block_type* p)
        {
            element_block_func::delete_block(p);
        }
    };

public:
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

private:

    /**
     * Find the correct block position for a given logical row ID.
     *
     * @param start_block_index index of the first block to start the search
     *                          from.
     *
     * @return index of the block that contains the specified logical row ID.
     */
    size_type get_block_position(size_type row, size_type start_block_index=0) const;

private:
    event_func m_hdl_event;
    blocks_type m_block_store;
    size_type m_cur_size;
};

}}}

#include "main_def.inl"

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

