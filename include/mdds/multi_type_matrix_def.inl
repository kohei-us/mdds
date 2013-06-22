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
multi_type_matrix<_String>::multi_type_matrix(size_type rows, size_type cols) :
    m_store(rows*cols), m_size(rows, cols) {}

template<typename _String>
template<typename _T>
multi_type_matrix<_String>::multi_type_matrix(size_type rows, size_type cols, const _T& value) :
    m_store(rows*cols, value), m_size(rows, cols) {}

template<typename _String>
multi_type_matrix<_String>::multi_type_matrix(const multi_type_matrix& r) :
    m_store(r.m_store), m_size(r.m_size) {}

template<typename _String>
multi_type_matrix<_String>::~multi_type_matrix() {}

template<typename _String>
bool multi_type_matrix<_String>::operator== (const multi_type_matrix& r) const
{
    return m_size == r.m_size && m_store == r.m_store;
}

template<typename _String>
bool multi_type_matrix<_String>::operator!= (const multi_type_matrix& r) const
{
    return !operator== (r);
}

template<typename _String>
multi_type_matrix<_String>&
multi_type_matrix<_String>::operator= (const multi_type_matrix& r)
{
    if (this == &r)
        return *this;

    store_type tmp(r.m_store);
    m_store.swap(tmp);
    m_size = r.m_size;
    return *this;
}

template<typename _String>
typename multi_type_matrix<_String>::position_type
multi_type_matrix<_String>::position(size_type row, size_type col)
{
    return m_store.position(get_pos(row,col));
}

template<typename _String>
typename multi_type_matrix<_String>::const_position_type
multi_type_matrix<_String>::position(size_type row, size_type col) const
{
    return m_store.position(get_pos(row,col));
}

template<typename _String>
mtm::element_t
multi_type_matrix<_String>::get_type(const const_position_type& pos) const
{
    return to_mtm_type(pos.first->type);
}

template<typename _String>
mtm::element_t
multi_type_matrix<_String>::get_type(size_type row, size_type col) const
{
    return to_mtm_type(m_store.get_type(get_pos(row,col)));
}

template<typename _String>
double multi_type_matrix<_String>::get_numeric(size_type row, size_type col) const
{
    return get_numeric(m_store.position(get_pos(row,col)));
}

template<typename _String>
double multi_type_matrix<_String>::get_numeric(const const_position_type& pos) const
{
    switch (pos.first->type)
    {
        case mtv::element_type_numeric:
            return mtv::numeric_element_block::at(*pos.first->data, pos.second);
        case mtv::element_type_boolean:
        {
            // vector<bool> cannot return reference i.e. we can't use at() here.
            typename mtv::boolean_element_block::const_iterator it =
                mtv::boolean_element_block::begin(*pos.first->data);
            std::advance(it, pos.second);
            return *it;
        }
        case string_trait::string_type_identifier:
        case mtv::element_type_empty:
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
bool multi_type_matrix<_String>::get_boolean(const const_position_type& pos) const
{
    return static_cast<bool>(get_numeric(pos));
}

template<typename _String>
const typename multi_type_matrix<_String>::string_type&
multi_type_matrix<_String>::get_string(size_type row, size_type col) const
{
    return get_string(m_store.position(get_pos(row,col)));
}

template<typename _String>
const typename multi_type_matrix<_String>::string_type&
multi_type_matrix<_String>::get_string(const const_position_type& pos) const
{
    if (pos.first->type != string_trait::string_type_identifier)
        throw general_error("multi_type_matrix: unknown element type.");

    return string_block_type::at(*pos.first->data, pos.second);
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
                case mtm::element_numeric:
                {
                    double val;
                    m_store.get(get_pos(row,col), val);
                    tmp.set(col, row, val);
                }
                break;
                case mtm::element_boolean:
                {
                    bool val;
                    m_store.get(get_pos(row,col), val);
                    tmp.set(col, row, val);
                }
                break;
                case mtm::element_string:
                {
                    string_type val;
                    m_store.get(get_pos(row,col), val);
                    tmp.set(col, row, val);
                }
                break;
                case mtm::element_empty:
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
void multi_type_matrix<_String>::copy(const multi_type_matrix& r)
{
    if (&r == this)
        // Self assignment.
        return;

    size_type rows = std::min(m_size.row, r.m_size.row);
    size_type cols = std::min(m_size.column, r.m_size.column);

    for (size_type col = 0; col < cols; ++col)
    {
        for (size_type row = 0; row < rows; ++row)
        {
            switch (r.get_type(row, col))
            {
                case mtm::element_numeric:
                    m_store.set(get_pos(row,col), r.get<double>(row,col));
                break;
                case mtm::element_boolean:
                    m_store.set(get_pos(row,col), r.get<bool>(row,col));
                break;
                case mtm::element_string:
                    m_store.set(get_pos(row,col), r.get<string_type>(row,col));
                break;
                case mtm::element_empty:
                    m_store.set_empty(get_pos(row,col), get_pos(row,col));
                break;
                default:
                    throw general_error("multi_type_matrix: unknown element type.");
            }
        }
    }
}

template<typename _String>
void multi_type_matrix<_String>::copy_store(store_type& dest, size_type rows, size_type cols) const
{
    size_type row_count = std::min(rows, m_size.row);
    size_type col_count = std::min(cols, m_size.column);
    for (size_type c = 0; c < col_count; ++c)
    {
        for (size_type r = 0; r < row_count; ++r)
        {
            switch (get_type(r, c))
            {
                case mtm::element_numeric:
                    dest.set(rows*c+r, get<double>(r,c));
                break;
                case mtm::element_boolean:
                    dest.set(rows*c+r, get<bool>(r,c));
                break;
                case mtm::element_string:
                    dest.set(rows*c+r, get<string_type>(r,c));
                break;
                case mtm::element_empty:
                    // Do nothing since the temp store has been initialized with empty elements.
                break;
                default:
                    throw general_error("multi_type_matrix: unknown element type.");
            }
        }
    }
}

template<typename _String>
void multi_type_matrix<_String>::resize(size_type rows, size_type cols)
{
    if (!rows || !cols)
    {
        m_size.row = 0;
        m_size.column = 0;
        m_store.clear();
        return;
    }

    store_type temp_store(rows*cols);
    copy_store(temp_store, rows, cols);

    m_size.row = rows;
    m_size.column = cols;
    m_store.swap(temp_store);
}

template<typename _String>
template<typename _T>
void multi_type_matrix<_String>::resize(size_type rows, size_type cols, const _T& value)
{
    if (!rows || !cols)
    {
        m_size.row = 0;
        m_size.column = 0;
        m_store.clear();
        return;
    }

    store_type temp_store(rows*cols, value);
    copy_store(temp_store, rows, cols);

    m_size.row = rows;
    m_size.column = cols;
    m_store.swap(temp_store);
}

template<typename _String>
void multi_type_matrix<_String>::clear()
{
    m_store.clear();
    m_size.row = 0;
    m_size.column = 0;
}

template<typename _String>
bool multi_type_matrix<_String>::numeric() const
{
    if (m_store.empty())
        return false;

    typename store_type::const_iterator i = m_store.begin(), iend = m_store.end();
    for (; i != iend; ++i)
    {
        mtv::element_t mtv_type = i->type;
        switch (mtv_type)
        {
            case mtv::element_type_numeric:
            case mtv::element_type_boolean:
                // These are numeric types.
                continue;
            case string_trait::string_type_identifier:
            case mtv::element_type_empty:
                // These are not.
                return false;
            default:
                throw general_error("multi_type_matrix: unknown element type.");
        }
    }

    return true;
}

template<typename _String>
bool multi_type_matrix<_String>::empty() const
{
    return m_store.empty();
}

template<typename _String>
void multi_type_matrix<_String>::swap(multi_type_matrix& r)
{
    m_store.swap(r.m_store);
    std::swap(m_size.row, r.m_size.row);
    std::swap(m_size.column, r.m_size.column);
}

template<typename _String>
template<typename _Func>
void multi_type_matrix<_String>::walk(_Func& func) const
{
    walk_func<_Func> wf(func);
    std::for_each(m_store.begin(), m_store.end(), wf);
}

}
