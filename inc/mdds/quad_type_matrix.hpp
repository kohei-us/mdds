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

#ifndef __MDDS_QUAD_TYPE_MATRIX_HPP__
#define __MDDS_QUAD_TYPE_MATRIX_HPP__

#include "mdds/global.hpp"

#include <iostream>
#include <cstdlib>
#include <boost/ptr_container/ptr_vector.hpp>

namespace mdds {

/**
 * This data structure represents a matrix where each individual element may
 * be of one of four types: value, boolean, string, or empty.
 */
template<typename _Key, typename _String>
class quad_type_matrix
{
public:
    class matrix_error : public ::mdds::general_error
    {
    public:
        matrix_error(const ::std::string& msg) : general_error(msg) {}
    };

    typedef _Key        key_type;
    typedef _String     string_type;
    typedef size_t      size_type;

    enum data_density_type
    {
        data_density_filled,
        data_density_sparse
    };

    enum element_type
    { 
        element_empty, 
        element_numeric, 
        element_boolean, 
        element_string 
    };

    quad_type_matrix();
    quad_type_matrix(key_type rows, key_type cols);
    ~quad_type_matrix();

    void set_numeric(key_type row, key_type col, double val);
    void set_boolean(key_type row, key_type col, bool val);
    void set_string(key_type row, key_type col, string_type* str);
    void set_empty(key_type row, key_type col);

    size_t size_rows() const;
    size_t size_cols() const;

    void resize(key_type row, key_type col);

#ifdef UNIT_TEST
    void dump() const;
#endif

private:
    struct element
    {
        element_type m_type;

        union
        {
            double       m_numeric;
            bool         m_boolean;
            string_type* mp_string;
        };

        element() : m_type(element_empty) {}
        explicit element(double v) : m_type(element_numeric), m_numeric(v) {}
        explicit element(bool v) : m_type(element_boolean), m_boolean(v) {}
        explicit element(string_type* p) : m_type(element_string), mp_string(p) {}

        ~element()
        {
            if (m_type == element_string)
                delete mp_string;
        }

        void set_empty()
        {
            if (m_type == element_string)
                delete mp_string;
            m_type = element_empty;
        }

        void set_numeric(double val)
        {
            if (m_type == element_string)
                delete mp_string;
            m_type = element_numeric;
            m_numeric = val;
        }

        void set_boolean(bool val)
        {
            if (m_type == element_string)
                delete mp_string;
            m_type = element_boolean;
            m_boolean = val;
        }

        void set_string(string_type* str)
        {
            if (m_type == element_string)
                delete mp_string;
            m_type = element_string;
            mp_string = str;
        }
    };

    class storage_base
    {
    public:
        virtual element& get_element(key_type row, key_type col) = 0;

        virtual element_type get_type(key_type row, key_type col) const = 0;

        virtual double get_numeric(key_type row, key_type col) const = 0;
        virtual string_type get_string(key_type row, key_type col) const = 0;
        virtual bool get_boolean(key_type row, key_type col) const = 0;

        virtual size_t rows() const = 0;
        virtual size_t cols() const = 0;

        virtual void resize(key_type row, key_type col) = 0;
    };

    /**
     * This storage creates instance for every single element, even for the
     * empty elements.  The constructor initializes all elements to numeric 
     * type having a value of 0. 
     */
    class storage_filled : public storage_base
    {
        typedef ::boost::ptr_vector<element>  row_type;
        typedef ::boost::ptr_vector<row_type> rows_type;

    public:
        storage_filled(key_type rows, key_type cols)
        {
            m_rows.reserve(rows);
            for (key_type i = 0; i < rows; ++i)
            {
                m_rows.push_back(new row_type);
                init_row(m_rows.back(), cols);
            }
        }

        virtual element& get_element(key_type row, key_type col)
        {
            return m_rows.at(row).at(col);
        }

        virtual element_type get_type(key_type row, key_type col) const
        {
            return m_rows.at(row).at(col).m_type;
        }

        virtual double get_numeric(key_type row, key_type col) const
        {
            const element& elem = m_rows.at(row).at(col);
            if (elem.m_type != element_numeric)
                throw matrix_error("element type is not numeric.");

            return elem.m_numeric;
        }

        virtual string_type get_string(key_type row, key_type col) const
        {
            const element& elem = m_rows.at(row).at(col);
            if (elem.m_type != element_string)
                throw matrix_error("element type is not string.");

            return *elem.mp_string;
        }

        virtual bool get_boolean(key_type row, key_type col) const
        {
            const element& elem = m_rows.at(row).at(col);
            if (elem.m_type != element_boolean)
                throw matrix_error("element type is not boolean.");

            return elem.m_boolean;
        }

        virtual size_t rows() const
        {
            return m_rows.size();
        }

        virtual size_t cols() const
        {
            return m_rows.empty() ? 0 : m_rows[0].size();
        }

        virtual void resize(key_type row, key_type col)
        {
            size_t cur_rows = rows(), cur_cols = cols();

            if (row > cur_rows)
            {
                // Insert extra rows...
                size_t new_row_count = row - cur_rows;
                m_rows.reserve(row);
                for (size_t i = 0; i < new_row_count; ++i)
                {
                    m_rows.push_back(new row_type);
                    init_row(m_rows.back(), col);
                }

                resize_rows(cur_rows-1, cur_cols, col);
            }
            else if (cur_rows > row)
            {
                // Remove rows to new size.
                m_rows.resize(row);
                resize_rows(row-1, cur_cols, col);
            }
            else
            {
                assert(cur_rows == row);
                resize_rows(cur_rows-1, cur_cols, col);
            }
        }

    private:

        /**
         * Resize rows to a new column size, from row 0 up to specified upper 
         * row. 
         */
        void resize_rows(size_t upper_row, size_t cur_cols, size_t new_cols)
        {
            for (size_t i = 0; i <= upper_row; ++i)
            {
                // Resize pre-existing rows to new column size.
                if (new_cols > cur_cols)
                {
                    size_t new_col_count = new_cols - cur_cols;
                    for (size_t j = 0; j < new_col_count; ++j)
                        m_rows[i].push_back(new element(static_cast<double>(0.0)));
                }
                else if (new_cols < cur_cols)
                    m_rows[i].resize(new_cols);
            }
        }

        static void init_row(row_type& row, size_t col_size)
        {
            row.reserve(col_size);
            for (key_type j = 0; j < col_size; ++j)
                row.push_back(new element(static_cast<double>(0.0)));
        }

    private:
        rows_type m_rows;
    };

private:
    storage_filled* mp_storage;
};

template<typename _Key, typename _String>
quad_type_matrix<_Key,_String>::quad_type_matrix() :
    mp_storage(NULL)
{
}

template<typename _Key, typename _String>
quad_type_matrix<_Key,_String>::quad_type_matrix(key_type rows, key_type cols) :
    mp_storage(NULL)
{
    mp_storage = new storage_filled(rows, cols);
}

template<typename _Key, typename _String>
quad_type_matrix<_Key,_String>::~quad_type_matrix()
{
    delete mp_storage;
}

template<typename _Key, typename _String>
void quad_type_matrix<_Key,_String>::set_numeric(key_type row, key_type col, double val)
{
    mp_storage->get_element(row, col).set_numeric(val);
}

template<typename _Key, typename _String>
void quad_type_matrix<_Key,_String>::set_boolean(key_type row, key_type col, bool val)
{
    mp_storage->get_element(row, col).set_boolean(val);
}

template<typename _Key, typename _String>
void quad_type_matrix<_Key,_String>::set_string(key_type row, key_type col, string_type* str)
{
    mp_storage->get_element(row, col).set_string(str);
}

template<typename _Key, typename _String>
void quad_type_matrix<_Key,_String>::set_empty(key_type row, key_type col)
{
    mp_storage->get_element(row, col).set_empty();
}

template<typename _Key, typename _String>
size_t quad_type_matrix<_Key,_String>::size_rows() const
{
    return mp_storage->rows();
}

template<typename _Key, typename _String>
size_t quad_type_matrix<_Key,_String>::size_cols() const
{
    return mp_storage->cols();
}

template<typename _Key, typename _String>
void quad_type_matrix<_Key,_String>::resize(key_type row, key_type col)
{
    mp_storage->resize(row, col);
}

#ifdef UNIT_TEST
template<typename _Key, typename _String>
void quad_type_matrix<_Key,_String>::dump() const
{
    using namespace std;
    size_t rows = mp_storage->rows(), cols = mp_storage->cols();
    cout << "rows: " << mp_storage->rows() << "  cols: " << mp_storage->cols() << endl;
    for (size_t i = 0; i < rows; ++i)
    {
        cout << "row " << i << ": ";
        for (size_t j = 0; j < cols; ++j)
        {
            element_type etype = mp_storage->get_type(i, j);
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
                    cout << mp_storage->get_string(i, j);
                    break;
                default:
                    ;
            }
            cout << ")";
        }
        cout << endl;
    }
}
#endif

}

#endif
