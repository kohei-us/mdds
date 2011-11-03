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

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/pool/object_pool.hpp>

namespace mdds {

enum matrix_storage_t
{
    matrix_storage_filled,
    matrix_storage_filled_zero,
    matrix_storage_sparse
};

enum matrix_init_element_t
{
    matrix_init_element_zero,
    matrix_init_element_empty
};

class matrix_storage_error : public ::mdds::general_error
{
public:
    matrix_storage_error(const ::std::string& msg) : general_error(msg) {}
};

/**
 * Wrapper class that provides access to the storage internals.  This is
 * used by storage_base::const_iterator to traverse data in different
 * storage backends.
 */
template<typename _StoreType, typename _ElemWrap, typename _RowsWrap>
class const_itr_access
{
    typedef _StoreType  store_type;
    typedef _ElemWrap   element_wrap_type;
    typedef _RowsWrap   rows_wrap_type;
public:
    typedef typename _StoreType::element element;

    const_itr_access(const store_type& db) :
        m_db(db),
        m_rows_itr(db.get_rows().begin()),
        m_rows_itr_end(db.get_rows().end())
    {
        // create iterators for the first row.
        if (!empty())
            update_row_itr();
    }

    const_itr_access(const const_itr_access& r) :
        m_db(r.m_db),
        m_rows_itr(r.m_rows_itr),
        m_rows_itr_end(r.m_rows_itr_end),
        m_row_itr(r.m_row_itr),
        m_row_itr_end(r.m_row_itr_end) {}

    bool operator== (const const_itr_access& r) const
    {
        if (&m_db != &r.m_db)
            // different storage instances.
            return false;

        if (empty())
            return r.empty();

        if (m_rows_itr != r.m_rows_itr)
            return false;

        // If the rows iterators are equal, the end positions should be equal
        // too.  No need to check it.
        assert(m_rows_itr_end == r.m_rows_itr_end);

        if (m_row_itr != r.m_row_itr)
            return false;

        // Same assumption holds here too. See above.
        assert(m_row_itr_end == r.m_row_itr_end);
        return true;
    }

    bool empty() const { return m_db.get_rows().begin() == m_rows_itr_end; }

    const element& get() const { return m_wrap(m_row_itr); }

    bool inc()
    {
        if (m_row_itr == m_row_itr_end)
            return false;

        ++m_row_itr;
        if (m_row_itr == m_row_itr_end)
        {
            // Move to the next row.
            if (m_rows_itr != m_rows_itr_end)
            {
                ++m_rows_itr;
                if (m_rows_itr == m_rows_itr_end)
                    // no more rows.
                    return false;
                update_row_itr();
            }
        }
        return true;
    }

    bool dec()
    {
        if (m_rows_itr == m_rows_itr_end)
        {
            --m_rows_itr;
            assert(m_row_itr == m_row_itr_end);
            --m_row_itr;
            return true;
        }

        if (m_row_itr == m_rows_wrap(m_rows_itr).begin())
        {
            // On the first element of a row.
            if (m_rows_itr == m_db.get_rows().begin())
                // already on the first row.
                return false;

            // Move up to the previous row, and select its last element.
            --m_rows_itr;
            assert(!m_rows_wrap(m_rows_itr).empty());
            m_row_itr_end = m_rows_wrap(m_rows_itr).end();
            m_row_itr = m_row_itr_end;
            --m_row_itr;
            return true;
        }

        // Not on the first element of a row.
        --m_row_itr;
        return true;
    }

    /**
     * Set the current iterator position to the end position.
     */
    void set_to_end()
    {
        if (empty())
            return;

        m_rows_itr = m_rows_itr_end;
        typename store_type::rows_type::const_iterator itr = m_rows_itr_end;
        --itr; // Move to the last row.

        // They both need to be at the end position of the last row.
        m_row_itr = m_row_itr_end = m_rows_wrap(itr).end();
    }

private:

    void update_row_itr()
    {
        m_row_itr = m_rows_wrap(m_rows_itr).begin();
        m_row_itr_end = m_rows_wrap(m_rows_itr).end();
    }

private:
    const store_type& m_db;
    typename store_type::rows_type::const_iterator m_rows_itr;
    typename store_type::rows_type::const_iterator m_rows_itr_end;
    typename store_type::row_type::const_iterator m_row_itr;
    typename store_type::row_type::const_iterator m_row_itr_end;
    element_wrap_type m_wrap;
    rows_wrap_type m_rows_wrap;
};

template<typename _MatrixType>
class storage_base
{
public:
    typedef _MatrixType matrix_type;

    typedef typename _MatrixType::element               element;
    typedef typename _MatrixType::flag_storage          flag_storage;
    typedef typename _MatrixType::string_type           string_type;

    typedef typename _MatrixType::filled_storage_type filled_storage_type;
    typedef typename _MatrixType::filled_storage_zero_type filled_storage_zero_type;
    typedef typename _MatrixType::sparse_storage_type sparse_storage_type;

    class const_iterator
    {
        typedef typename filled_storage_type::const_itr_access filled_access_type;
        typedef typename filled_storage_zero_type::const_itr_access filled_zero_access_type;
        typedef typename sparse_storage_type::const_itr_access sparse_access_type;
    public:
        // iterator traits
        typedef element     value_type;
        typedef element*    pointer;
        typedef element&    reference;
        typedef ptrdiff_t   difference_type;
        typedef ::std::bidirectional_iterator_tag   iterator_category;

        const_iterator() :
            m_const_itr_access(NULL), m_type(matrix_storage_filled)
        {}

        const_iterator(void* p, matrix_storage_t type, bool _end = false) :
            m_const_itr_access(p), m_type(type)
        {
            assert(p != NULL);
            if (_end)
            {
                switch (m_type)
                {
                    case matrix_storage_filled:
                        get_filled_itr()->set_to_end();
                    break;
                    case matrix_storage_filled_zero:
                        get_filled_zero_itr()->set_to_end();
                    break;
                    case matrix_storage_sparse:
                        get_sparse_itr()->set_to_end();
                    break;
                    default:
                        assert(!"unknown storage type");
                }
            }
        }

        const_iterator(const const_iterator& r) :
            m_const_itr_access(NULL),
            m_type(r.m_type)
        {
            if (!r.m_const_itr_access)
                return;

            switch (r.m_type)
            {
                case matrix_storage_filled:
                    m_const_itr_access = new filled_access_type(*r.get_filled_itr());
                break;
                case matrix_storage_filled_zero:
                    m_const_itr_access = new filled_zero_access_type(*r.get_filled_zero_itr());
                break;
                case matrix_storage_sparse:
                    m_const_itr_access = new sparse_access_type(*r.get_sparse_itr());
                break;
                default:
                    assert(!"unknown storage type");
            }
        }

        ~const_iterator()
        {
            switch (m_type)
            {
                case matrix_storage_filled:
                    delete get_filled_itr();
                break;
                case matrix_storage_filled_zero:
                    delete get_filled_zero_itr();
                break;
                case matrix_storage_sparse:
                    delete get_sparse_itr();
                break;
                default:
                    assert(!"unknown storage type");
            }
        }

        void swap(const_iterator& r)
        {
            ::std::swap(m_type, r.m_type);
            ::std::swap(m_const_itr_access, r.m_const_itr_access);
        }

        const_iterator& operator=(const const_iterator& r)
        {
            if (this == &r)
                // self assignment.
                return *this;

            const_iterator new_itr(r);
            swap(new_itr);
            return *this;
        }

        const element& operator*() const
        {
            switch (m_type)
            {
                case matrix_storage_filled:
                    return get_filled_itr()->get();
                case matrix_storage_filled_zero:
                    return get_filled_zero_itr()->get();
                case matrix_storage_sparse:
                    return get_sparse_itr()->get();
                default:
                    assert(!"unknown storage type");
            }
            throw matrix_storage_error("unknown storage type");
        }

        const element* operator->() const
        {
            switch (m_type)
            {
                case matrix_storage_filled:
                    return &get_filled_itr()->get();
                case matrix_storage_filled_zero:
                    return &get_filled_zero_itr()->get();
                case matrix_storage_sparse:
                    return &get_sparse_itr()->get();
                default:
                    assert(!"unknown storage type");
            }
            return NULL;
        }

        const element* operator++()
        {
            bool has_next = false;
            switch (m_type)
            {
                case matrix_storage_filled:
                    has_next = get_filled_itr()->inc();
                break;
                case matrix_storage_filled_zero:
                    has_next = get_filled_zero_itr()->inc();
                break;
                case matrix_storage_sparse:
                    has_next = get_sparse_itr()->inc();
                break;
                default:
                    assert(!"unknown storage type");
            }
            return has_next ? operator->() : NULL;
        }

        const element* operator--()
        {
            bool has_next = false;
            switch (m_type)
            {
                case matrix_storage_filled:
                    has_next = get_filled_itr()->dec();
                break;
                case matrix_storage_filled_zero:
                    has_next = get_filled_zero_itr()->dec();
                break;
                case matrix_storage_sparse:
                    has_next = get_sparse_itr()->dec();
                break;
                default:
                    assert(!"unknown storage type");
            }
            return has_next ? operator->() : NULL;
        }

        bool operator== (const const_iterator& r) const
        {
            if (m_type != r.m_type)
                // Types differ.
                return false;

            if (!m_const_itr_access)
                // This instance has empty access.  The other one must be empty too.
                return r.m_const_itr_access == NULL;

            assert(m_const_itr_access != NULL);
            assert(r.m_const_itr_access != NULL);

            switch (m_type)
            {
                case matrix_storage_filled:
                    return *get_filled_itr() == *r.get_filled_itr();
                case matrix_storage_filled_zero:
                    return *get_filled_zero_itr() == *r.get_filled_zero_itr();
                case matrix_storage_sparse:
                    return *get_sparse_itr() == *r.get_sparse_itr();
                default:
                    assert(!"unknown storage type");
            }
            return false;
        }

        bool operator!= (const const_iterator& r) const
        {
            return !operator==(r);
        }

    private:

        filled_access_type* get_filled_itr()
        {
            return static_cast<filled_access_type*>(m_const_itr_access);
        }

        const filled_access_type* get_filled_itr() const
        {
            return static_cast<const filled_access_type*>(m_const_itr_access);
        }

        filled_zero_access_type* get_filled_zero_itr()
        {
            return static_cast<filled_zero_access_type*>(m_const_itr_access);
        }

        const filled_zero_access_type* get_filled_zero_itr() const
        {
            return static_cast<const filled_zero_access_type*>(m_const_itr_access);
        }

        sparse_access_type* get_sparse_itr()
        {
            return static_cast<sparse_access_type*>(m_const_itr_access);
        }

        const sparse_access_type* get_sparse_itr() const
        {
            return static_cast<const sparse_access_type*>(m_const_itr_access);
        }

        /**
         * Stores new'ed instance of const_itr_access of the respective
         * storage type. TODO: Find out if there is a way to store the
         * const_itr_access instance in a type-safe way.
         */
        void* m_const_itr_access;

        /**
         * Matrix storage type which is either filled or sparse.
         */
        matrix_storage_t m_type;
    };

    storage_base(matrix_storage_t store_type, matrix_init_element_t init) :
        m_store_type(store_type), m_init_type(init) {}

    storage_base(const storage_base& r) :
        m_store_type(r.m_store_type), m_init_type(r.m_init_type), m_flags(r.m_flags) {}

    matrix_storage_t get_storage_type() const { return m_store_type; }

    /**
     * When deleting the storage object, the caller must explicitly specify
     * the concrete class or else memory will leak.
     */
    ~storage_base() {}

    const_iterator begin() const
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
            {
                void* p = static_cast<const filled_storage_type*>(this)->get_const_itr_access();
                return const_iterator(p, m_store_type);
            }
            break;
            case matrix_storage_filled_zero:
            {
                void* p = static_cast<const filled_storage_zero_type*>(this)->get_const_itr_access();
                return const_iterator(p, m_store_type);
            }
            break;
            case matrix_storage_sparse:
            {
                void* p = static_cast<const sparse_storage_type*>(this)->get_const_itr_access();
                return const_iterator(p, m_store_type);
            }
            break;
            default:
                assert(!"unknown storage type");
        }
        throw matrix_storage_error("unknown storage type");
    }

    const_iterator end() const
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
            {
                void* p = static_cast<const filled_storage_type*>(this)->get_const_itr_access();
                return const_iterator(p, m_store_type, true);
            }
            break;
            case matrix_storage_filled_zero:
            {
                void* p = static_cast<const filled_storage_zero_type*>(this)->get_const_itr_access();
                return const_iterator(p, m_store_type, true);
            }
            break;
            case matrix_storage_sparse:
            {
                void* p = static_cast<const sparse_storage_type*>(this)->get_const_itr_access();
                return const_iterator(p, m_store_type, true);
            }
            break;
            default:
                assert(!"unknown storage type");
        }
        throw matrix_storage_error("unknown storage type");
    }

    element& get_element(size_t row, size_t col)
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<filled_storage_type*>(this)->get_element(row, col);
            case matrix_storage_filled_zero:
                return static_cast<filled_storage_zero_type*>(this)->get_element(row, col);
            case matrix_storage_sparse:
                return static_cast<sparse_storage_type*>(this)->get_element(row, col);
            default:
                assert(!"unknown storage type");
        }
        throw matrix_storage_error("unknown storage type");
    }

    matrix_element_t get_type(size_t row, size_t col) const
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<const filled_storage_type*>(this)->get_type(row, col);
            case matrix_storage_filled_zero:
                return static_cast<const filled_storage_zero_type*>(this)->get_type(row, col);
            case matrix_storage_sparse:
                return static_cast<const sparse_storage_type*>(this)->get_type(row, col);
            default:
                assert(!"unknown storage type");
        }
        throw matrix_storage_error("unknown storage type");
    }

    double get_numeric(size_t row, size_t col) const
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<const filled_storage_type*>(this)->get_numeric(row, col);
            case matrix_storage_filled_zero:
                return static_cast<const filled_storage_zero_type*>(this)->get_numeric(row, col);
            case matrix_storage_sparse:
                return static_cast<const sparse_storage_type*>(this)->get_numeric(row, col);
            default:
                assert(!"unknown storage type");
        }
        throw matrix_storage_error("unknown storage type");
    }

    const string_type* get_string(size_t row, size_t col) const
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<const filled_storage_type*>(this)->get_string(row, col);
            case matrix_storage_filled_zero:
                return static_cast<const filled_storage_zero_type*>(this)->get_string(row, col);
            case matrix_storage_sparse:
                return static_cast<const sparse_storage_type*>(this)->get_string(row, col);
            default:
                assert(!"unknown storage type");
        }
        throw matrix_storage_error("unknown storage type");
    }

    bool get_boolean(size_t row, size_t col) const
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<const filled_storage_type*>(this)->get_boolean(row, col);
            case matrix_storage_filled_zero:
                return static_cast<const filled_storage_zero_type*>(this)->get_boolean(row, col);
            case matrix_storage_sparse:
                return static_cast<const sparse_storage_type*>(this)->get_boolean(row, col);
            default:
                assert(!"unknown storage type");
        }
        throw matrix_storage_error("unknown storage type");
    }

    size_t rows() const
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<const filled_storage_type*>(this)->rows();
            case matrix_storage_filled_zero:
                return static_cast<const filled_storage_zero_type*>(this)->rows();
            case matrix_storage_sparse:
                return static_cast<const sparse_storage_type*>(this)->rows();
            default:
                assert(!"unknown storage type");
        }
        throw matrix_storage_error("unknown storage type");
    }

    size_t cols() const
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<const filled_storage_type*>(this)->cols();
            case matrix_storage_filled_zero:
                return static_cast<const filled_storage_zero_type*>(this)->cols();
            case matrix_storage_sparse:
                return static_cast<const sparse_storage_type*>(this)->cols();
            default:
                assert(!"unknown storage type");
        }
        throw matrix_storage_error("unknown storage type");
    }

    void transpose()
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                static_cast<filled_storage_type*>(this)->transpose();
            break;
            case matrix_storage_filled_zero:
                static_cast<filled_storage_zero_type*>(this)->transpose();
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
            case matrix_storage_filled_zero:
                static_cast<filled_storage_zero_type*>(this)->resize(row, col);
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
            case matrix_storage_filled_zero:
                static_cast<filled_storage_zero_type*>(this)->clear();
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
            case matrix_storage_filled_zero:
                return static_cast<filled_storage_zero_type*>(this)->numeric();
            case matrix_storage_sparse:
                return static_cast<sparse_storage_type*>(this)->numeric();
            default:
                assert(!"unknown storage type");
        }
        throw matrix_storage_error("unknown storage type");
    }

    bool empty() const
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<const filled_storage_type*>(this)->empty();
            case matrix_storage_filled_zero:
                return static_cast<const filled_storage_zero_type*>(this)->empty();
            case matrix_storage_sparse:
                return static_cast<const sparse_storage_type*>(this)->empty();
            default:
                assert(!"unknown storage type");
        }
        throw matrix_storage_error("unknown storage type");
    }

    storage_base* clone() const
    {
        switch (m_store_type)
        {
            case matrix_storage_filled:
                return static_cast<const filled_storage_type*>(this)->clone();
            case matrix_storage_filled_zero:
                return static_cast<const filled_storage_zero_type*>(this)->clone();
            case matrix_storage_sparse:
                return static_cast<const sparse_storage_type*>(this)->clone();
            default:
                assert(!"unknown storage type");
        }
        throw matrix_storage_error("unknown storage type");
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

#include "mixed_type_matrix_storage_filled_linear.inl"
#include "mixed_type_matrix_storage_sparse.inl"

#endif
