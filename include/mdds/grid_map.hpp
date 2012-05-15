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

#ifndef __MDDS_GRID_MAP_HPP__
#define __MDDS_GRID_MAP_HPP__

#include "mdds/grid_map_sheet.hpp"

#include <vector>
#include <algorithm>

namespace mdds {

/**
 * This container stores grid cells in a 3-dimensional hyperplane.  Cells
 * are accessed via 3-key combinations.
 */
template<typename _Trait>
class grid_map
{
public:
    typedef typename _Trait::sheet_key_type sheet_key_type;
    typedef typename _Trait::row_key_type row_key_type;
    typedef typename _Trait::col_key_type col_key_type;

    typedef typename __gridmap::sheet<_Trait> sheet_type;
    typedef typename sheet_type::column_type column_type;
    typedef typename column_type::size_type size_type;

    grid_map();
    grid_map(size_type sheet_size, size_type row_size, size_type col_size);
    ~grid_map();

    sheet_type& operator[](size_type n) { return *m_sheets[n]; }
    const sheet_type& operator[](size_type n) const { return *m_sheets[n]; }

    sheet_type& at(size_type n) { return *m_sheets.at(n); }
    const sheet_type& at(size_type n) const { return *m_sheets.at(n); }

    void push_back(size_type row_size, size_type col_size);

    size_type size() const { return m_sheets.size(); }

private:
    std::vector<sheet_type*> m_sheets;
};

}

#include "grid_map_def.inl"

#endif
