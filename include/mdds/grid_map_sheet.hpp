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

#include "mdds/multi_type_vector.hpp"

#include <vector>
#include <algorithm>

namespace mdds { namespace __gridmap {

template<typename _Trait>
class sheet
{
public:
    typedef typename _Trait::row_key_type row_key_type;
    typedef typename _Trait::col_key_type col_key_type;

    typedef mdds::multi_type_vector<_Trait> column_type;
    typedef typename column_type::size_type size_type;

    sheet();
    sheet(size_type row_size, size_type col_size);
    ~sheet();

    column_type& operator[](size_type n) { return *m_columns[n]; }
    const column_type& operator[](size_type n) const { return *m_columns[n]; }

    column_type& at(size_type n) { return *m_columns.at(n); }
    const column_type& at(size_type n) const { return *m_columns.at(n); }

    /**
     * Return the number of columns.
     *
     * @return number of columns.
     */
    size_type size() const { return m_columns.size(); }

private:
    std::vector<column_type*> m_columns;
};

}}

#include "grid_map_sheet_def.inl"

#endif
