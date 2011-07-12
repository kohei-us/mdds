/*************************************************************************
 *
 * Copyright (c) 2011 Kohei Yoshida
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

template<typename _StoreType>
class const_itr_access_linear
{
    typedef _StoreType store_type;
public:
    typedef typename store_type::element element;

    const_itr_access_linear(const store_type& db) : 
        m_db(db), 
        m_itr(db.get_array().begin()),
        m_itr_end(db.get_array().end()) {}

    const_itr_access_linear(const const_itr_access_linear& r) :
        m_db(r.m_db),
        m_itr(r.m_itr),
        m_itr_end(r.m_itr_end) {}

    bool operator== (const const_itr_access_linear& r) const
    {
        if (&m_db != &r.m_db)
            // different storage instances.
            return false;

        return m_itr == r.m_itr;
    }

    bool empty() const { return m_db.get_array().begin() == m_itr_end; }

    const element& get() const
    {
        assert(m_itr != m_itr_end);
        return *(*m_itr);
    }

    bool inc()
    {
        if (m_itr == m_itr_end)
            return false;

        ++m_itr;
        return m_itr != m_itr_end;
    }

    bool dec()
    {
        if (m_itr == m_db.get_array().begin())
            // already on the first element.
            return false;

        --m_itr;
        return true;
    }

    /**
     * Set the current iterator position to the end position.
     */
    void set_to_end()
    {
        if (empty())
            return;

        m_itr = m_itr_end;
    }
private:
    const store_type& m_db;
    typename store_type::array_type::const_iterator m_itr;
    typename store_type::array_type::const_iterator m_itr_end;
};

/**
 * This storage creates instance for every single element, even for the
 * empty elements. 
 */
template<typename _MatrixType>
class storage_filled_linear : public ::mdds::storage_base<_MatrixType>
{
    typedef _MatrixType matrix_type;
    typedef typename matrix_type::string_type string_type;

public:
    typedef typename matrix_type::element element;
    typedef ::std::vector<element*> array_type;
    typedef const_itr_access_linear<storage_filled_linear> const_itr_access;

    storage_filled_linear(size_t _rows, size_t _cols, matrix_init_element_t init_type) :
        storage_base<matrix_type>(matrix_storage_filled, init_type),
        m_element_pool(new ::boost::object_pool<element>),
        m_rows(_rows),
        m_cols(_cols),
        m_numeric(false),
        m_valid(false)
    {
        if (init_type == matrix_init_element_zero)
            m_init_elem.set_numeric(0.0);

        size_t n = m_rows * m_cols;
        m_array.resize(n, &m_init_elem);
    }

    storage_filled_linear(const storage_filled_linear& r) :
        storage_base<matrix_type>(r),
        m_element_pool(new ::boost::object_pool<element>),
        m_init_elem(r.m_init_elem),
        m_rows(r.m_rows),
        m_cols(r.m_cols),
        m_numeric(r.m_numeric),
        m_valid(r.m_valid)
    {
        size_t n = r.m_array.size();
        if (!n)
            return;

        // Populate the array with initial values first.
        m_array.resize(n, &m_init_elem);

        for (size_t i = 0; i < n; ++i)
        {
            const element* p = r.m_array[i];
            if (p != &r.m_init_elem)
                // non-default value.  Duplicate the element instance.
                m_array[i] = m_element_pool->construct(*p);
        }
    }

    virtual ~storage_filled_linear()
    {
        delete m_element_pool;
    }

    const_itr_access* get_const_itr_access() const
    {
        return new const_itr_access(*this);
    }

    element& get_element(size_t row, size_t col)
    {
        m_valid = false;
        size_t pos = get_pos(row, col);
        if (m_array.at(pos) == &m_init_elem)
        {
            // Initial element.  Instantiate a new element to take its place.
            matrix_init_element_t init_type = storage_base<matrix_type>::get_init_type();
            switch (init_type)
            {
                case matrix_init_element_zero:
                    m_array[pos] = m_element_pool->construct(static_cast<double>(0.0));
                break;
                case matrix_init_element_empty:
                    m_array[pos] = m_element_pool->construct();
                break;
                default:
                    throw matrix_storage_error("unknown init type.");
            }
        }
        return *m_array[pos];
    }

    matrix_element_t get_type(size_t row, size_t col) const
    {
        return m_array.at(get_pos(row, col))->m_type;
    }

    double get_numeric(size_t row, size_t col) const
    {
        const element& elem = *m_array.at(get_pos(row, col));
        switch (elem.m_type)
        {
            case element_numeric:
                return elem.m_numeric;
            case element_boolean:
                return static_cast<double>(elem.m_boolean);
            case element_empty:
            default:
                ;
        }
        return 0.0;
    }

    const string_type* get_string(size_t row, size_t col) const
    {
        const element& elem = *m_array.at(get_pos(row, col));
        if (elem.m_type != element_string)
            throw matrix_storage_error("element type is not string.");

        return elem.mp_string;
    }

    bool get_boolean(size_t row, size_t col) const
    {
        const element& elem = *m_array.at(get_pos(row, col));
        if (elem.m_type != element_boolean)
            throw matrix_storage_error("element type is not boolean.");

        return elem.m_boolean;
    }

    size_t rows() const
    {
        return m_rows;
    }

    size_t cols() const
    {
        return m_cols;
    }

    void transpose()
    {
        array_type trans_array(m_array.size(), &m_init_elem);
        for (size_t i = 0; i < m_rows; ++i)
            for (size_t j = 0; j < m_cols; ++j)
                trans_array[m_rows*j+i] = m_array[get_pos(i,j)];

        m_array.swap(trans_array);
        ::std::swap(m_rows, m_cols);
    }

    void resize(size_t row, size_t col)
    {
        m_valid = false;
        if (!row || !col)
        {
            // Empty the matrix.
            clear();
            m_rows = row;
            m_cols = col;
            return;
        }

        size_t new_size = row * col;
        if (m_array.empty())
        {
            // Current matrix is empty.
            m_array.resize(new_size, &m_init_elem);
            m_rows = row;
            m_cols = col;
            return;
        }

        array_type new_array(new_size, &m_init_elem);
        size_t min_rows = ::std::min(row, m_rows);
        size_t min_cols = ::std::min(col, m_cols);
        for (size_t i = 0; i < min_rows; ++i)
        {
            for (size_t j = 0; j < min_cols; ++j)
                new_array[col*i+j] = m_array[get_pos(i, j)];
        }

        if (row < m_rows)
        {
            // Delete all element instances that are in the rows being stripped off.
            for (size_t i = row; i < m_rows; ++i)
            {
                for (size_t j = 0; j < m_cols; ++j)
                {
                    element* p = m_array[get_pos(i, j)];
                    if (p != &m_init_elem)
                        m_element_pool->destroy(p);
                }
            }
        }

        if (col < m_cols)
        {
            // Delete all elements in the columns that are being stripped off.
            for (size_t i = 0; i < min_rows; ++i)
            {
                for (size_t j = col; j < m_cols; ++j)
                {
                    element* p = m_array[get_pos(i, j)];
                    if (p != &m_init_elem)
                        m_element_pool->destroy(p);
                }
            }
        }

        m_array.swap(new_array);
        m_rows = row;
        m_cols = col;
    }

    void clear()
    {
        delete m_element_pool;
        m_element_pool = new ::boost::object_pool<element>;
        m_array.clear();
        m_valid = true;
        m_numeric = false;
    }

    bool numeric()
    {
        if (m_valid)
            return m_numeric;

        typename array_type::const_iterator itr = m_array.begin(), itr_end = m_array.end();
        for (; itr != itr_end; ++itr)
        {
            const element* p = *itr;
            matrix_element_t elem_type = p->m_type;
            if (elem_type != element_numeric && elem_type != element_boolean)
            {
                m_numeric = false;
                m_valid = true;
                return m_numeric;
            }
        }

        m_numeric = true;
        m_valid = true;
        return m_numeric;
    }

    bool empty() const
    {
        return m_array.empty();
    }

    ::mdds::storage_base<matrix_type>* clone() const
    {
        return new storage_filled_linear(*this);
    }

    const array_type& get_array() const { return m_array; }

private:

    /**
     * Get an array position of the data referenced by the row and column 
     * indices.  The array consists of multiple rows, the content of row 0 
     * followded by the content of row 1, and so on.  <b>Note that no 
     * boundary check is performed in this method.</b> 
     *  
     * @param row 0-based row index.
     * @param col 0-based column index.
     * @return position in the data array. 
     */
    size_t get_pos(size_t row, size_t col) const
    {
        return m_cols * row + col;
    }

private:
    ::boost::object_pool<element>* m_element_pool;
    array_type m_array;
    element m_init_elem;
    size_t m_rows;
    size_t m_cols;
    bool m_numeric:1;
    bool m_valid:1;
};

}
