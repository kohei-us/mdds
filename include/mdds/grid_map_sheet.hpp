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

#ifndef __MDDS_GRID_MAP_SHEET_HPP__
#define __MDDS_GRID_MAP_SHEET_HPP__

#include "mdds/grid_map_column.hpp"

#include <vector>
#include <algorithm>

namespace mdds { namespace __gridmap {

template<typename _Trait>
class sheet
{
public:
    typedef typename _Trait::row_key_type row_key_type;
    typedef typename _Trait::col_key_type col_key_type;

    typedef column<_Trait> column_type;
    typedef typename column_type::size_type size_type;

    sheet();
    sheet(size_type row_size, size_type col_size);
    ~sheet();

    template<typename _T>
    void set_cell(col_key_type col, row_key_type row, const _T& cell);

    template<typename _T>
    _T get_cell(col_key_type col, row_key_type row) const;

    mdds::gridmap::cell_t get_type(col_key_type col, row_key_type row) const;

    bool is_empty(col_key_type col, row_key_type row) const;

    void set_empty(col_key_type col, row_key_type start_row, row_key_type end_row);

private:
    std::vector<column_type*> m_columns;

    size_type m_row_size;
    size_type m_col_size;
};

}}

#include "grid_map_sheet_def.inl"

#endif
