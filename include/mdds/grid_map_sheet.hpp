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
    void set_cell(row_key_type row, col_key_type col, const _T& cell);

    template<typename _T>
    _T get_cell(row_key_type row, col_key_type col) const;

private:
    std::vector<column_type*> m_columns;

    size_type m_row_size;
    size_type m_col_size;
};

template<typename _Trait>
sheet<_Trait>::sheet() : m_row_size(0), m_col_size(0) {}

template<typename _Trait>
sheet<_Trait>::sheet(size_type row_size, size_type col_size) :
    m_row_size(row_size), m_col_size(col_size)
{
    for (size_type i = 0; i < m_col_size; ++i)
        m_columns.push_back(new column_type(m_row_size));
}

template<typename _Trait>
sheet<_Trait>::~sheet()
{
    std::for_each(m_columns.begin(), m_columns.end(), default_deleter<column_type>());
}

template<typename _Trait>
template<typename _T>
void sheet<_Trait>::set_cell(row_key_type row, col_key_type col, const _T& cell)
{
    m_columns.at(col)->set_cell(row, cell);
}

template<typename _Trait>
template<typename _T>
_T sheet<_Trait>::get_cell(row_key_type row, col_key_type col) const
{
    return m_columns.at(col)->get_cell(row);
}

}}

#endif
