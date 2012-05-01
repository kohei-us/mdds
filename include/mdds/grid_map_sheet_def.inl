/*************************************************************************
 *
 * Copyright (c) 2012 Kohei Yoshida
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

namespace mdds { namespace __gridmap {

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
typename sheet<_Trait>::column_type&
sheet<_Trait>::get_column(col_key_type col)
{
    return *m_columns.at(col);
}

template<typename _Trait>
const typename sheet<_Trait>::column_type&
sheet<_Trait>::get_column(col_key_type col) const
{
    return *m_columns.at(col);
}

}}
