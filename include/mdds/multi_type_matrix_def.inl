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

namespace mdds {

template<typename _String, typename _Flag>
multi_type_matrix<_String,_Flag>::multi_type_matrix() {}

template<typename _String, typename _Flag>
multi_type_matrix<_String,_Flag>::~multi_type_matrix() {}

template<typename _String, typename _Flag>
multi_type_matrix<_String,_Flag>::multi_type_matrix(size_type rows, size_type cols) :
    m_store(rows*cols), m_flag_store(rows*cols), m_size(rows, cols)
{

}

template<typename _String, typename _Flag>
void multi_type_matrix<_String,_Flag>::set(size_type row, size_type col, double val)
{
    m_store.set(get_pos(row,col), val);
}

template<typename _String, typename _Flag>
typename multi_type_matrix<_String,_Flag>::size_pair_type
multi_type_matrix<_String,_Flag>::size() const
{
    return m_size;
}

}
