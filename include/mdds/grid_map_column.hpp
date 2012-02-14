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
    typedef size_t size_type;

    typedef typename _Trait::cell_block_type cell_block_type;
    typedef typename _Trait::cell_category_type cell_category_type;
    typedef typename _Trait::row_key_type row_key_type;

private:
    typedef typename _Trait::cell_type_inspector cell_type_inspector;
    typedef typename _Trait::cell_block_type_inspector cell_block_type_inspector;
    typedef typename _Trait::cell_block_modifier cell_block_modifier;

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

    template<typename _T>
    void set_cell(row_key_type row, const _T& cell);

    template<typename _T>
    void set_cells(row_key_type row, const _T& it_begin, const _T& it_end);

    template<typename _T>
    void insert_cells(row_key_type row, const _T& it_begin, const _T& it_end);

    template<typename _T>
    void get_cell(row_key_type row, _T& cell) const;

    bool is_empty(row_key_type row) const;

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

    void erase_impl(size_type start_row, size_type end_row);

    void insert_empty_impl(size_type row, size_type length);

private:
    typedef std::vector<block*> blocks_type;
    blocks_type m_blocks;
    size_type m_cur_size;

    static cell_type_inspector get_type;
    static cell_block_type_inspector get_block_type;
};

}}

#include "grid_map_column_def.inl"

#endif
