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

template<typename _String>
multi_type_matrix<_String>::multi_type_matrix() {}

template<typename _String>
multi_type_matrix<_String>::~multi_type_matrix() {}

template<typename _String>
multi_type_matrix<_String>::multi_type_matrix(size_type rows, size_type cols) :
    m_store(rows*cols), m_size(rows, cols) {}

template<typename _String>
template<typename _T>
multi_type_matrix<_String>::multi_type_matrix(size_type rows, size_type cols, const _T& value) :
    m_store(rows*cols, value), m_size(rows, cols) {}

template<typename _String>
typename multi_type_matrix<_String>::element_t
multi_type_matrix<_String>::get_type(size_type row, size_type col) const
{
    mtv::element_t mtv_type = m_store.get_type(get_pos(row,col));
    switch (mtv_type)
    {
        case mtv::element_type_numeric:
            return element_numeric;
        case mtv::element_type_string:
        case __mtm::element_type_mtx_custom:
            return element_string;
        case mtv::element_type_boolean:
            return element_boolean;
        case mtv::element_type_empty:
            return element_empty;
        default:
            throw general_error("multi_type_matrix: unknown element type.");
    }
}

template<typename _String>
double multi_type_matrix<_String>::get_numeric(size_type row, size_type col) const
{
    switch (get_type(row,col))
    {
        case element_numeric:
        {
            double val;
            m_store.get(get_pos(row,col), val);
            return val;
        }
        case element_boolean:
        {
            bool val;
            m_store.get(get_pos(row,col), val);
            return val;
        }
        case element_string:
        case element_empty:
            return 0.0;
        default:
            throw general_error("multi_type_matrix: unknown element type.");
    }
}

template<typename _String>
bool multi_type_matrix<_String>::get_boolean(size_type row, size_type col) const
{
    return static_cast<bool>(get_numeric(row, col));
}

template<typename _String>
typename multi_type_matrix<_String>::string_type
multi_type_matrix<_String>::get_string(size_type row, size_type col) const
{
    switch (get_type(row,col))
    {
        case element_string:
        {
            string_type val;
            m_store.get(get_pos(row,col), val);
            return val;
        }
        case element_numeric:
        case element_boolean:
        case element_empty:
        default:
            throw general_error("multi_type_matrix: unknown element type.");
    }
}

template<typename _String>
template<typename _T>
_T
multi_type_matrix<_String>::get(size_type row, size_type col) const
{
    _T val;
    m_store.get(get_pos(row,col), val);
    return val;
}

template<typename _String>
void multi_type_matrix<_String>::set_empty(size_type row, size_type col)
{
    m_store.set_empty(get_pos(row, col), get_pos(row, col));
}

template<typename _String>
void multi_type_matrix<_String>::set_column_empty(size_type col)
{
    m_store.set_empty(get_pos(0, col), get_pos(m_size.row-1, col));
}

template<typename _String>
void multi_type_matrix<_String>::set_row_empty(size_type row)
{
    for (size_type col = 0; col < m_size.column; ++col)
    {
        size_type pos = get_pos(row, col);
        m_store.set_empty(pos, pos);
    }
}

template<typename _String>
void multi_type_matrix<_String>::set(size_type row, size_type col, double val)
{
    m_store.set(get_pos(row,col), val);
}

template<typename _String>
void multi_type_matrix<_String>::set(size_type row, size_type col, bool val)
{
    m_store.set(get_pos(row,col), val);
}

template<typename _String>
void multi_type_matrix<_String>::set(size_type row, size_type col, const string_type& str)
{
    m_store.set(get_pos(row,col), str);
}

template<typename _String>
template<typename _T>
void multi_type_matrix<_String>::set(size_type row, size_type col, const _T& it_begin, const _T& it_end)
{
    m_store.set(get_pos(row,col), it_begin, it_end);
}

template<typename _String>
template<typename _T>
void multi_type_matrix<_String>::set_column(size_type col, const _T& it_begin, const _T& it_end)
{
    size_type pos = get_pos(0, col);
    size_type len = std::distance(it_begin, it_end);

    if (len <= m_size.row)
    {
        m_store.set(pos, it_begin, it_end);
        return;
    }

    _T it_end2 = it_begin;
    std::advance(it_end2, m_size.row);
    m_store.set(pos, it_begin, it_end2);
}

template<typename _String>
typename multi_type_matrix<_String>::size_pair_type
multi_type_matrix<_String>::size() const
{
    return m_size;
}

template<typename _String>
multi_type_matrix<_String>&
multi_type_matrix<_String>::transpose()
{
    multi_type_matrix tmp(m_size.column, m_size.row);
    for (size_type row = 0; row < m_size.row; ++row)
    {
        for (size_type col = 0; col < m_size.column; ++col)
        {
            switch (get_type(row,col))
            {
                case element_numeric:
                {
                    double val;
                    m_store.get(get_pos(row,col), val);
                    tmp.set(col, row, val);
                }
                break;
                case element_boolean:
                {
                    bool val;
                    m_store.get(get_pos(row,col), val);
                    tmp.set(col, row, val);
                }
                break;
                case element_string:
                {
                    string_type val;
                    m_store.get(get_pos(row,col), val);
                    tmp.set(col, row, val);
                }
                break;
                case element_empty:
                break;
                default:
                    throw general_error("multi_type_matrix: unknown element type.");
            }
        }
    }

    swap(tmp);
    return *this;
}

template<typename _String>
void multi_type_matrix<_String>::swap(multi_type_matrix& r)
{
    m_store.swap(r.m_store);
    std::swap(m_size.row, r.m_size.row);
    std::swap(m_size.column, r.m_size.column);
}

}
