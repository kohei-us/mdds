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

namespace mdds {

template<typename _Trait>
grid_map<_Trait>::grid_map() {}

template<typename _Trait>
grid_map<_Trait>::grid_map(size_type sheet_size, size_type row_size, size_type col_size)
{
    for (size_type i = 0; i < sheet_size; ++i)
        m_sheets.push_back(new sheet_type(row_size, col_size));
}

template<typename _Trait>
grid_map<_Trait>::~grid_map()
{
    std::for_each(m_sheets.begin(), m_sheets.end(), default_deleter<sheet_type>());
}

template<typename _Trait>
void grid_map<_Trait>::push_back(size_type row_size, size_type col_size)
{
    m_sheets.push_back(new sheet_type(row_size, col_size));
}

}
