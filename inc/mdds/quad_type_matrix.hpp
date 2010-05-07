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
        data_filled,
        data_sparse
    };

    enum element_type { elem_empty, elem_numeric, elem_boolean, elem_string };

    quad_type_matrix();
    quad_type_matrix(key_type rows, key_type cols);
    ~quad_type_matrix();

    void set_numeric(key_type row, key_type col, double val);
    void set_boolean(key_type row, key_type col, bool val);
    void set_string(key_type row, key_type col, string_type* str);
    void set_empty(key_type row, key_type col);

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

        element() : m_type(elem_empty) {}
        explicit element(double v) : m_type(elem_numeric), m_numeric(v) {}
        explicit element(bool v) : m_type(elem_boolean), m_boolean(v) {}
        explicit element(string_type* p) : m_type(elem_string), mp_string(p) {}

        ~element()
        {
            if (m_type == elem_string)
                delete mp_string;
        }

        void set_empty()
        {
            if (m_type == elem_string)
                delete mp_string;
            m_type = elem_empty;
        }

        void set_numeric(double val)
        {
            if (m_type == elem_string)
                delete mp_string;
            m_type = elem_numeric;
            m_numeric = val;
        }

        void set_boolean(bool val)
        {
            if (m_type == elem_string)
                delete mp_string;
            m_type = elem_boolean;
            m_boolean = val;
        }

        void set_string(string_type* str)
        {
            if (m_type == elem_string)
                delete mp_string;
            m_type = elem_string;
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
    };

    /**
     * This storage creates instance for every single element, even for the
     * empty elements.  The constructor initializes all elements to numeric 
     * type having a value of 0. 
     */
    class storage_filled : public storage_base
    {
    public:
        storage_filled(key_type rows, key_type cols)
        {
            m_rows.reserve(rows);
            for (key_type i = 0; i < rows; ++i)
            {
                m_rows.push_back(new row_type);
                row_type& row = m_rows.back();
                row.reserve(cols);
                for (key_type j = 0; j < cols; ++j)
                    row.push_back(new element(static_cast<double>(0.0)));
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
            if (elem.m_type != elem_numeric)
                throw matrix_error("element type is not numeric.");

            return elem.m_numeric;
        }

        virtual string_type get_string(key_type row, key_type col) const
        {
            const element& elem = m_rows.at(row).at(col);
            if (elem.m_type != elem_string)
                throw matrix_error("element type is not string.");

            return *elem.mp_string;
        }

        virtual bool get_boolean(key_type row, key_type col) const
        {
            const element& elem = m_rows.at(row).at(col);
            if (elem.m_type != elem_boolean)
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

    private:
        typedef ::boost::ptr_vector<element>  row_type;
        typedef ::boost::ptr_vector<row_type> rows_type;
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
                case elem_boolean:
                    cout << boolalpha << mp_storage->get_boolean(i, j) << noboolalpha;
                    break;
                case elem_empty:
                    cout << "-";
                    break;
                case elem_numeric:
                    cout << mp_storage->get_numeric(i, j);
                    break;
                case elem_string:
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
