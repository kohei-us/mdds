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
#include "mdds/global.hpp"

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
    typedef typename _Trait::cell_block_type cell_block_type;
    typedef typename _Trait::cell_category_type cell_category_type;
    typedef typename _Trait::row_key_type row_key_type;

private:
    typedef typename _Trait::cell_type_inspector cell_type_inspector;
    typedef typename _Trait::cell_block_type_inspector cell_block_type_inspector;

    typedef typename _Trait::cell_block_modifier cell_block_modifier;

    struct block : boost::noncopyable
    {
        row_key_type m_size;
        cell_block_type* mp_data;

        block();
        block(row_key_type _size);
        ~block();
    };

    column(); // disabled
public:
    column(row_key_type max_row_size);
    ~column();

    template<typename _T>
    void set_cell(row_key_type row, const _T& cell);

    template<typename _T>
    void get_cell(row_key_type row, _T& cell) const;

private:
    template<typename _T>
    void create_new_block_with_new_cell(cell_block_type*& data, const _T& cell);

    template<typename _T>
    void set_cell_to_empty_block(
        size_t block_index, row_key_type pos_in_block, const _T& cell);

private:
    std::vector<block*> m_blocks;
    row_key_type m_max_row_size;

    static cell_type_inspector get_type;
    static cell_block_type_inspector get_block_type;
};

}}

#include "grid_map_column_def.inl"

#endif