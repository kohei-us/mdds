/*************************************************************************
 *
 * Copyright (c) 2010 Kohei Yoshida
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
typename mixed_type_matrix<_String,_Flag>::storage_base*
mixed_type_matrix<_String,_Flag>::create_storage(size_t rows, size_t cols, matrix_density_t density)
{
    switch (density)
    {
        case matrix_density_filled_zero:
            return new filled_storage_zero_type(rows, cols, matrix_init_element_zero);
        case matrix_density_filled_empty:
            return new filled_storage_type(rows, cols, matrix_init_element_empty);
        case matrix_density_sparse_zero:
            return new sparse_storage_type(rows, cols, matrix_init_element_zero);
        case matrix_density_sparse_empty:
            return new sparse_storage_type(rows, cols, matrix_init_element_empty);
        default:
            throw matrix_error("unknown density type");
    }
}

template<typename _String, typename _Flag>
mixed_type_matrix<_String,_Flag>::mixed_type_matrix() :
    mp_storage(NULL),
    m_cached_size(0, 0)
{
    mp_storage = create_storage(0, 0, matrix_density_filled_zero);
}

template<typename _String, typename _Flag>
mixed_type_matrix<_String,_Flag>::mixed_type_matrix(matrix_density_t density) :
    mp_storage(NULL),
    m_cached_size(0, 0)
{
    mp_storage = create_storage(0, 0, density);
}

template<typename _String, typename _Flag>
mixed_type_matrix<_String,_Flag>::mixed_type_matrix(size_t rows, size_t cols, matrix_density_t density) :
    mp_storage(NULL),
    m_cached_size(rows, cols)
{
    mp_storage = create_storage(rows, cols, density);
}

template<typename _String, typename _Flag>
mixed_type_matrix<_String,_Flag>::mixed_type_matrix(const mixed_type_matrix& r) :
    mp_storage(r.mp_storage->clone()),
    m_cached_size(r.m_cached_size)
{
}

template<typename _String, typename _Flag>
mixed_type_matrix<_String,_Flag>::~mixed_type_matrix()
{
    delete_storage();
}

template<typename _String, typename _Flag>
typename mixed_type_matrix<_String,_Flag>::const_iterator
mixed_type_matrix<_String,_Flag>::begin() const
{
    return mp_storage->begin();
}

template<typename _String, typename _Flag>
typename mixed_type_matrix<_String,_Flag>::const_iterator
mixed_type_matrix<_String,_Flag>::end() const
{
    return mp_storage->end();
}

template<typename _String, typename _Flag>
mixed_type_matrix<_String,_Flag>&
mixed_type_matrix<_String,_Flag>::operator= (const mixed_type_matrix& r)
{
    if (this == &r)
        // self assignment.
        return *this;

    delete_storage();
    mp_storage = r.mp_storage->clone();
    m_cached_size = r.m_cached_size;
    return *this;
}

template<typename _String, typename _Flag>
matrix_element_t mixed_type_matrix<_String,_Flag>::get_type(size_t row, size_t col) const
{
    return mp_storage->get_type(row, col);
}

template<typename _String, typename _Flag>
double mixed_type_matrix<_String,_Flag>::get_numeric(size_t row, size_t col) const
{
    return mp_storage->get_numeric(row, col);
}

template<typename _String, typename _Flag>
bool mixed_type_matrix<_String,_Flag>::get_boolean(size_t row, size_t col) const
{
    return mp_storage->get_boolean(row, col);
}

template<typename _String, typename _Flag>
const typename mixed_type_matrix<_String,_Flag>::string_type*
mixed_type_matrix<_String,_Flag>::get_string(size_t row, size_t col) const
{
    return mp_storage->get_string(row, col);
}

template<typename _String, typename _Flag>
void mixed_type_matrix<_String,_Flag>::set_numeric(size_t row, size_t col, double val)
{
    mp_storage->get_element(row, col).set_numeric(val);
}

template<typename _String, typename _Flag>
void mixed_type_matrix<_String,_Flag>::set_boolean(size_t row, size_t col, bool val)
{
    mp_storage->get_element(row, col).set_boolean(val);
}

template<typename _String, typename _Flag>
void mixed_type_matrix<_String,_Flag>::set_string(size_t row, size_t col, string_type* str)
{
    mp_storage->get_element(row, col).set_string(str);
}

template<typename _String, typename _Flag>
void mixed_type_matrix<_String,_Flag>::set_flag(size_t row, size_t col, flag_type flag)
{
    mp_storage->get_flag_storage().set_flag(row, col, flag);
}

template<typename _String, typename _Flag>
typename mixed_type_matrix<_String,_Flag>::flag_type
mixed_type_matrix<_String,_Flag>::get_flag(size_t row, size_t col) const
{
    return mp_storage->get_flag_storage().get_flag(row, col);
}

template<typename _String, typename _Flag>
void mixed_type_matrix<_String,_Flag>::clear_flag(size_t row, size_t col)
{
    return mp_storage->get_flag_storage().clear_flag(row, col);
}

template<typename _String, typename _Flag>
void mixed_type_matrix<_String,_Flag>::set_empty(size_t row, size_t col)
{
    mp_storage->get_element(row, col).set_empty();
}

template<typename _String, typename _Flag>
void mixed_type_matrix<_String,_Flag>::set(size_t row, size_t col, double val)
{
    set_numeric(row, col, val);
}

template<typename _String, typename _Flag>
void mixed_type_matrix<_String,_Flag>::set(size_t row, size_t col, bool val)
{
    set_boolean(row, col, val);
}

template<typename _String, typename _Flag>
void mixed_type_matrix<_String,_Flag>::set(size_t row, size_t col, string_type* str)
{
    set_string(row, col, str);
}

template<typename _String, typename _Flag>
typename mixed_type_matrix<_String,_Flag>::size_pair_type
mixed_type_matrix<_String,_Flag>::size() const
{
    return m_cached_size;
}

template<typename _String, typename _Flag>
mixed_type_matrix<_String,_Flag>&
mixed_type_matrix<_String,_Flag>::transpose()
{
    mp_storage->transpose();
    m_cached_size = size_pair_type(mp_storage->rows(), mp_storage->cols());
    return *this;
}

template<typename _String, typename _Flag>
void mixed_type_matrix<_String,_Flag>::assign(const mixed_type_matrix& r)
{
    if (this == &r)
        // assignment to self.
        return;

    size_t row_count = ::std::min(mp_storage->rows(), r.mp_storage->rows());
    size_t col_count = ::std::min(mp_storage->cols(), r.mp_storage->cols());
    for (size_t i = 0; i < row_count; ++i)
        for (size_t j = 0; j < col_count; ++j)
            mp_storage->get_element(i, j) = r.mp_storage->get_element(i, j);
}

template<typename _String, typename _Flag>
void mixed_type_matrix<_String,_Flag>::resize(size_t row, size_t col)
{
    mp_storage->resize(row, col);
    m_cached_size = size_pair_type(row, col);
}

template<typename _String, typename _Flag>
void mixed_type_matrix<_String,_Flag>::clear()
{
    mp_storage->clear();
    m_cached_size = size_pair_type(0, 0);
}

template<typename _String, typename _Flag>
bool mixed_type_matrix<_String,_Flag>::numeric() const
{
    return mp_storage->numeric();
}

template<typename _String, typename _Flag>
bool mixed_type_matrix<_String,_Flag>::empty() const
{
    return mp_storage->empty();
}

template<typename _String, typename _Flag>
void mixed_type_matrix<_String,_Flag>::swap(mixed_type_matrix& r)
{
    ::std::swap(mp_storage, r.mp_storage);
    size_pair_type temp = m_cached_size;
    r.m_cached_size = m_cached_size;
    m_cached_size = temp;
}

#ifdef MDDS_UNIT_TEST
template<typename _String, typename _Flag>
void mixed_type_matrix<_String,_Flag>::dump() const
{
    using namespace std;
    size_t rows = m_cached_size.first, cols = m_cached_size.second;
    cout << "rows: " << rows << "  cols: " << cols << endl;
    for (size_t i = 0; i < rows; ++i)
    {
        cout << "row " << i << ": ";
        for (size_t j = 0; j < cols; ++j)
        {
            matrix_element_t etype = mp_storage->get_type(i, j);
            if (j > 0)
                cout << ", ";
            cout << "(col " << j << ": ";
            switch (etype)
            {
                case element_boolean:
                    cout << boolalpha << mp_storage->get_boolean(i, j) << noboolalpha;
                    break;
                case element_empty:
                    cout << "-";
                    break;
                case element_numeric:
                    cout << mp_storage->get_numeric(i, j);
                    break;
                case element_string:
                    cout << "'" << mp_storage->get_string(i, j) << "'";
                    break;
                default:
                    ;
            }
            cout << ")";
        }
        cout << endl;
    }
}

template<typename _String, typename _Flag>
void mixed_type_matrix<_String,_Flag>::dump_flags() const
{
    mp_storage->get_flag_storage().dump();
}

template<typename _String, typename _Flag>
typename mixed_type_matrix<_String,_Flag>::size_pair_type
mixed_type_matrix<_String,_Flag>::get_storage_size() const
{
    size_pair_type size_pair(mp_storage->rows(), mp_storage->cols());
    return size_pair;
}
#endif

template<typename _String, typename _Flag>
void mixed_type_matrix<_String,_Flag>::delete_storage()
{
    switch (mp_storage->get_storage_type())
    {
        case matrix_storage_filled:
            delete static_cast<filled_storage_type*>(mp_storage);
        break;
        case matrix_storage_filled_zero:
            delete static_cast<filled_storage_zero_type*>(mp_storage);
        break;
        case matrix_storage_sparse:
            delete static_cast<sparse_storage_type*>(mp_storage);
        break;
        default:
            assert(!"unknown storage type!");
    }
}

}
