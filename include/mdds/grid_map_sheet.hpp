/*************************************************************************
 *
 * Copyright (c) 2011 Kohei Yoshida
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

    typedef typename _Trait::cell_type cell_type;
    typedef column<cell_type, row_key_type> column_type;

    sheet();
    ~sheet();

private:
    std::vector<column_type*> m_columns;
};

template<typename _Trait>
sheet<_Trait>::sheet() {}

template<typename _Trait>
sheet<_Trait>::~sheet()
{
    std::for_each(m_columns.begin(), m_columns.end(), default_deleter<column_type>());
}

}}

#endif
