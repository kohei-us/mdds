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

#ifndef __MDDS_MIXED_TYPE_MATRIX_STORAGE_HPP__
#define __MDDS_MIXED_TYPE_MATRIX_STORAGE_HPP__

#include <cassert>

namespace mdds {

enum matrix_storage_t
{
    matrix_storage_filled,
    matrix_storage_sparse
};

enum matrix_init_element_t
{
    matrix_init_element_zero,
    matrix_init_element_empty
};

template<typename _MatrixType>
class storage_base
{
public:
    typedef _MatrixType matrix_type;
    typedef typename _MatrixType::element       element;
    typedef typename _MatrixType::flag_storage  flag_storage;
    typedef typename _MatrixType::string_type   string_type;
    typedef typename _MatrixType::filled_storage_type filled_storage_type;
    typedef typename _MatrixType::sparse_storage_type sparse_storage_type;

    storage_base(matrix_storage_t store_type, matrix_init_element_t init) : 
        m_store_type(store_type), m_init_type(init) {}
    storage_base(const storage_base& r) : 
        m_store_type(r.m_store_type), m_init_type(r.m_init_type), m_flags(r.m_flags) {}

    matrix_storage_t get_storage_type() const { return m_store_type; }

    /** 
     * the destructor must remain virtual because the derived classes have 
     * different sizes.  TODO: Figure out a way to remove the virtual-ness 
     * without leaking memory. 
     */ 
    virtual ~storage_base() {}

    element& get_element(size_t row, size_t col)
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<filled_storage_type*>(this)->get_element(row, col);
            case matrix_storage_sparse:
                return static_cast<sparse_storage_type*>(this)->get_element(row, col);
            default:
                assert(!"unknown storage type");
        }
    }

    matrix_element_t get_type(size_t row, size_t col) const
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<const filled_storage_type*>(this)->get_type(row, col);
            case matrix_storage_sparse:
                return static_cast<const sparse_storage_type*>(this)->get_type(row, col);
            default:
                assert(!"unknown storage type");
        }
    }

    double get_numeric(size_t row, size_t col) const
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<const filled_storage_type*>(this)->get_numeric(row, col);
            case matrix_storage_sparse:
                return static_cast<const sparse_storage_type*>(this)->get_numeric(row, col);
            default:
                assert(!"unknown storage type");
        }
    }

    const string_type* get_string(size_t row, size_t col) const
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<const filled_storage_type*>(this)->get_string(row, col);
            case matrix_storage_sparse:
                return static_cast<const sparse_storage_type*>(this)->get_string(row, col);
            default:
                assert(!"unknown storage type");
        }
    }

    bool get_boolean(size_t row, size_t col) const
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<const filled_storage_type*>(this)->get_boolean(row, col);
            case matrix_storage_sparse:
                return static_cast<const sparse_storage_type*>(this)->get_boolean(row, col);
            default:
                assert(!"unknown storage type");
        }
    }

    size_t rows() const
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<const filled_storage_type*>(this)->rows();
            case matrix_storage_sparse:
                return static_cast<const sparse_storage_type*>(this)->rows();
            default:
                assert(!"unknown storage type");
        }
    }

    size_t cols() const
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<const filled_storage_type*>(this)->cols();
            case matrix_storage_sparse:
                return static_cast<const sparse_storage_type*>(this)->cols();
            default:
                assert(!"unknown storage type");
        }
    }

    void transpose()
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                static_cast<filled_storage_type*>(this)->transpose();
            break;
            case matrix_storage_sparse:
                static_cast<sparse_storage_type*>(this)->transpose();
            break;
            default:
                assert(!"unknown storage type");
        }
    }

    void resize(size_t row, size_t col)
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                static_cast<filled_storage_type*>(this)->resize(row, col);
            break;
            case matrix_storage_sparse:
                static_cast<sparse_storage_type*>(this)->resize(row, col);
            break;
            default:
                assert(!"unknown storage type");
        }
    }

    void clear()
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                static_cast<filled_storage_type*>(this)->clear();
            break;
            case matrix_storage_sparse:
                static_cast<sparse_storage_type*>(this)->clear();
            break;
            default:
                assert(!"unknown storage type");
        }
    }

    bool numeric()
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<filled_storage_type*>(this)->numeric();
            case matrix_storage_sparse:
                return static_cast<sparse_storage_type*>(this)->numeric();
            default:
                assert(!"unknown storage type");
        }
    }

    bool empty() const
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<const filled_storage_type*>(this)->empty();
            case matrix_storage_sparse:
                return static_cast<const sparse_storage_type*>(this)->empty();
            default:
                assert(!"unknown storage type");
        }
    }

    storage_base* clone() const
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<const filled_storage_type*>(this)->clone();
            case matrix_storage_sparse:
                return static_cast<const sparse_storage_type*>(this)->clone();
            default:
                assert(!"unknown storage type");
        }
    }

    flag_storage& get_flag_storage() { return m_flags; }

protected:
    matrix_init_element_t get_init_type() const { return m_init_type; }

private:
    matrix_storage_t        m_store_type;
    matrix_init_element_t   m_init_type;
    flag_storage            m_flags;
};

}

#endif
