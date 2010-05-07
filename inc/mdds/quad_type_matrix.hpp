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
    class wrong_element_type : public ::mdds::general_error
    {
    public:
        wrong_element_type(const ::std::string& msg) : general_error(msg) {}
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
    };

    class storage_base
    {
    };

    /**
     * This storage creates instance for every single element, even for the
     * empty elements.
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
                    row.push_back(new element);
            }
        }

        explicit storage_filled(key_type rows, key_type cols, double init_val)
        {
            m_rows.reserve(rows);
            for (key_type i = 0; i < rows; ++i)
            {
                m_rows.push_back(new row_type);
                row_type& row = m_rows.back();
                row.reserve(cols);
                for (key_type j = 0; j < cols; ++j)
                    row.push_back(new element(init_val));
            }
        }

        element_type get_type(key_type row, key_type col) const
        {
            return m_rows.at(row).at(col).m_type;
        }

        double get_numeric(key_type row, key_type col) const
        {
            const element& elem = m_rows.at(row).at(col);
            if (elem.m_type != elem_numeric)
                throw wrong_element_type("element type is not numeric.");

            return elem.m_numeric;
        }

        string_type get_string(key_type row, key_type col) const
        {
            const element& elem = m_rows.at(row).at(col);
            if (elem.m_type != elem_string)
                throw wrong_element_type("element type is not string.");

            return *elem.mp_string;
        }

        bool get_boolean(key_type row, key_type col) const
        {
            const element& elem = m_rows.at(row).at(col);
            if (elem.m_type != elem_boolean)
                throw wrong_element_type("element type is not boolean.");

            return elem.m_boolean;
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
    mp_storage = new storage_filled(rows, cols, 0.0);
}

template<typename _Key, typename _String>
quad_type_matrix<_Key,_String>::~quad_type_matrix()
{
    delete mp_storage;
}

}

#endif
