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

#include "mdds/default_deleter.hpp"
#include "mdds/compat/unique_ptr.hpp"

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
    typedef typename _Trait::cell_type cell_type;
    typedef typename _Trait::cell_category_type cell_category_type;
    typedef typename _Trait::row_key_type row_key_type;

private:
    typedef typename _Trait::cell_delete_handler cell_delete_handler;

    /**
     * Data for non-empty block.  Cells are stored here.
     */
    struct block_data : boost::noncopyable
    {
        cell_category_type m_type;
        std::vector<cell_type*> m_cells;

        block_data(cell_category_type _type, size_t _init_size = 0);
        ~block_data();
    };

    /**
     * Empty or non-empty block.
     */
    struct block : boost::noncopyable
    {
        row_key_type m_size;
        block_data* mp_data;
        bool m_empty;

        block();
        block(row_key_type _size);
        ~block();
    };

    column(); // disabled
public:
    column(row_key_type max_row);
    ~column();

    void set_cell(row_key_type row, cell_category_type cat, cell_type* cell);
    const cell_type* get_cell(row_key_type row) const;

private:
    std::vector<block*> m_blocks;
    row_key_type m_max_row;
};

template<typename _Trait>
column<_Trait>::block_data::block_data(cell_category_type _type, size_t _init_size) :
    m_type(_type), m_cells(_init_size, NULL) {}

template<typename _Trait>
column<_Trait>::block_data::~block_data()
{
    std::for_each(m_cells.begin(), m_cells.end(), cell_delete_handler());
}

template<typename _Trait>
column<_Trait>::block::block() : m_size(0), m_empty(true), mp_data(NULL) {}

template<typename _Trait>
column<_Trait>::block::block(row_key_type _size) : m_size(_size), mp_data(NULL), m_empty(true) {}

template<typename _Trait>
column<_Trait>::block::~block()
{
    delete mp_data;
}

template<typename _Trait>
column<_Trait>::column(row_key_type max_row) : m_max_row(max_row)
{
    // Initialize with an empty block that spans from 0 to max.
    m_blocks.push_back(new block(max_row));
}

template<typename _Trait>
column<_Trait>::~column()
{
    std::for_each(m_blocks.begin(), m_blocks.end(), default_deleter<block>());
}

template<typename _Trait>
void column<_Trait>::set_cell(row_key_type row, cell_category_type cat, cell_type* cell)
{
    unique_ptr<cell_type, cell_delete_handler> p(cell);

    // TODO: implement cell insertion...
}

template<typename _Trait>
const typename column<_Trait>::cell_type*
column<_Trait>::get_cell(row_key_type row) const
{
    row_key_type cur_index = 0;
    for (size_t i = 0, n = m_blocks.size(); i < n; ++i)
    {
        const block& blk = *m_blocks[i];
        if (row >= cur_index + blk.m_size)
        {
            // Specified row is not in this block.
            cur_index += blk.m_size;
            continue;
        }

        if (blk.m_empty)
            // empty cell block.
            return NULL;

        assert(blk.mp_data); // data for non-empty blocks should never be NULL.
        assert(blk.m_size == static_cast<row_key_type>(blk.mp_data->m_cells.size()));
        row_key_type idx = row - cur_index;
        return blk.mp_data->m_cells[idx];
    }
    return NULL;
}

}}

#endif
