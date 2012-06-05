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
    m_store(rows*cols), m_flag_store(rows*cols), m_size(rows, cols) {}

template<typename _String, typename _Flag>
template<typename _T>
multi_type_matrix<_String,_Flag>::multi_type_matrix(size_type rows, size_type cols, const _T& value) :
    m_store(rows*cols, value), m_flag_store(rows*cols), m_size(rows, cols) {}

template<typename _String, typename _Flag>
typename multi_type_matrix<_String,_Flag>::element_t
multi_type_matrix<_String,_Flag>::get_type(size_type row, size_type col) const
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

template<typename _String, typename _Flag>
double multi_type_matrix<_String,_Flag>::get_numeric(size_type row, size_type col) const
{
    switch (get_type(row,col))
    {
        case element_numeric:
        {
            double val;
            return m_store.get(get_pos(row,col), val);
            return val;
        }
        case element_boolean:
        {
            bool val;
            return m_store.get(get_pos(row,col), val);
            return val;
        }
        case element_string:
        case element_empty:
            return 0.0;
        default:
            throw general_error("multi_type_matrix: unknown element type.");
    }
}

template<typename _String, typename _Flag>
bool multi_type_matrix<_String,_Flag>::get_boolean(size_type row, size_type col) const
{
    switch (get_type(row,col))
    {
        case element_numeric:
        {
            double val;
            return m_store.get(get_pos(row,col), val);
            return val;
        }
        case element_boolean:
        {
            bool val;
            return m_store.get(get_pos(row,col), val);
            return val;
        }
        case element_string:
        case element_empty:
            return false;
        default:
            throw general_error("multi_type_matrix: unknown element type.");
    }
}

template<typename _String, typename _Flag>
typename multi_type_matrix<_String,_Flag>::string_type
multi_type_matrix<_String,_Flag>::get_string(size_type row, size_type col) const
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

template<typename _String, typename _Flag>
void multi_type_matrix<_String,_Flag>::set(size_type row, size_type col, double val)
{
    m_store.set(get_pos(row,col), val);
}

template<typename _String, typename _Flag>
template<typename _T>
void multi_type_matrix<_String,_Flag>::set(size_type row, size_type col, const _T& it_begin, const _T& it_end)
{
    m_store.set(get_pos(row,col), it_begin, it_end);
}

template<typename _String, typename _Flag>
typename multi_type_matrix<_String,_Flag>::size_pair_type
multi_type_matrix<_String,_Flag>::size() const
{
    return m_size;
}

}
