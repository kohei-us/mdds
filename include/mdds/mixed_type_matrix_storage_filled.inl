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

/**
 * This storage creates instance for every single element, even for the
 * empty elements. 
 */
template<typename _MatrixType>
class storage_filled_nested_array : public ::mdds::storage_base<_MatrixType>
{
    typedef _MatrixType matrix_type;
    typedef typename matrix_type::string_type string_type;

public:
    typedef typename matrix_type::element element;
    typedef ::std::vector<element*> row_type;
    typedef ::std::vector<row_type*> rows_type;

    struct elem_wrap
    {
        const element& operator() (const typename row_type::const_iterator& itr) const 
        { 
            return *(*itr); 
        }
    };
    struct rows_wrap
    {
        const row_type& operator() (const typename rows_type::const_iterator& itr) const
        { 
            return *(*itr);
        }
    };
    typedef ::mdds::const_itr_access<storage_filled_nested_array, elem_wrap, rows_wrap> const_itr_access;

    storage_filled_nested_array(size_t _rows, size_t _cols, matrix_init_element_t init_type) :
        storage_base<matrix_type>(matrix_storage_filled, init_type),
        m_row_pool(new ::boost::object_pool<row_type>),
        m_element_pool(new ::boost::object_pool<element>),
        m_numeric(false),
        m_valid(false)
    {
        if (init_type == matrix_init_element_zero)
            m_init_elem.set_numeric(0.0);

        m_rows.reserve(_rows);
        for (size_t i = 0; i < _rows; ++i)
            m_rows.push_back(m_row_pool->construct(_cols, &m_init_elem));
    }

    storage_filled_nested_array(const storage_filled_nested_array& r) :
        storage_base<matrix_type>(r),
        m_row_pool(new ::boost::object_pool<row_type>),
        m_element_pool(new ::boost::object_pool<element>),
        m_init_elem(r.m_init_elem),
        m_numeric(r.m_numeric),
        m_valid(r.m_valid)
    {
        size_t n = r.m_rows.size();
        m_rows.reserve(n);
        for (size_t i = 0; i < n; ++i)
        {
            const row_type& row_other = *r.m_rows[i];
            size_t col_size = row_other.size();
            m_rows.push_back(m_row_pool->construct(col_size, &m_init_elem));
            row_type& row = *m_rows.back();
            for (size_t j = 0; j < col_size; ++j)
            {
                if (row_other[j] != &r.m_init_elem)
                    row[j] = m_element_pool->construct(*row_other[j]);
            }
        }
    }

    virtual ~storage_filled_nested_array()
    {
        delete m_row_pool;
        delete m_element_pool;
    }

    const_itr_access* get_const_itr_access() const
    {
        return new const_itr_access(*this);
    }

    element& get_element(size_t row, size_t col)
    {
        m_valid = false;
        if (m_rows.at(row)->at(col) == &m_init_elem)
        {
            // Initial element.  Instantiate a new element to take its place.
            matrix_init_element_t init_type = storage_base<matrix_type>::get_init_type();
            switch (init_type)
            {
                case matrix_init_element_zero:
                    (*m_rows[row])[col] = m_element_pool->construct(static_cast<double>(0.0));
                break;
                case matrix_init_element_empty:
                    (*m_rows[row])[col] = m_element_pool->construct();
                break;
                default:
                    throw matrix_storage_error("unknown init type.");
            }
        }
        return *(*m_rows[row])[col];
    }

    matrix_element_t get_type(size_t row, size_t col) const
    {
        return m_rows.at(row)->at(col)->m_type;
    }

    double get_numeric(size_t row, size_t col) const
    {
        const element& elem = *m_rows.at(row)->at(col);
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
        const element& elem = *m_rows.at(row)->at(col);
        if (elem.m_type != element_string)
            throw matrix_storage_error("element type is not string.");

        return elem.mp_string;
    }

    bool get_boolean(size_t row, size_t col) const
    {
        const element& elem = *m_rows.at(row)->at(col);
        if (elem.m_type != element_boolean)
            throw matrix_storage_error("element type is not boolean.");

        return elem.m_boolean;
    }

    size_t rows() const
    {
        return m_rows.size();
    }

    size_t cols() const
    {
        return m_rows.empty() ? 0 : m_rows[0]->size();
    }

    void transpose()
    {
        rows_type trans_mx;
        size_t row_size = rows(), col_size = cols();
        trans_mx.reserve(col_size);
        for (size_t col = 0; col < col_size; ++col)
        {
            trans_mx.push_back(m_row_pool->construct());
            row_type& trans_row = *trans_mx.back();
            trans_row.reserve(row_size);
            for (size_t row = 0; row < row_size; ++row)
                trans_row.push_back((*m_rows[row])[col]);
        }
        m_rows.swap(trans_mx);

        // Delete the row instances in the old container.
        delete_rows(trans_mx, 0);
    }

    void resize(size_t row, size_t col)
    {
        m_valid = false;
        if (!row || !col)
        {
            // Empty the matrix.
            clear();
            return;
        }

        size_t cur_rows = rows(), cur_cols = cols();

        if (!cur_rows || !cur_cols)
        {
            // current matrix is empty.
            rows_type new_rows;
            new_rows.reserve(row);
            for (size_t i = 0; i < row; ++i)
                new_rows.push_back(m_row_pool->construct(col, &m_init_elem));

            m_rows.swap(new_rows);
            return;
        }

        if (row > cur_rows)
        {
            // Insert extra rows...
            size_t new_row_count = row - cur_rows;
            m_rows.reserve(row);
            for (size_t i = 0; i < new_row_count; ++i)
                m_rows.push_back(m_row_pool->construct(col, &m_init_elem));

            resize_rows(cur_rows-1, cur_cols, col);
        }
        else if (cur_rows > row)
        {
            // Remove rows to new size.  Delete the row instances that are to
            // be stripped off.
            delete_rows(m_rows, row);
            m_rows.resize(row);
            resize_rows(row-1, cur_cols, col);
        }
        else
        {
            assert(cur_rows == row);
            resize_rows(cur_rows-1, cur_cols, col);
        }
    }

    void clear()
    {
        delete m_row_pool;
        delete m_element_pool;
        m_row_pool = new ::boost::object_pool<row_type>;
        m_element_pool = new ::boost::object_pool<element>;
        m_rows.clear();
        m_valid = true;
        m_numeric = false;
    }

    bool numeric()
    {
        if (m_valid)
            return m_numeric;

        typename rows_type::const_iterator itr_row = m_rows.begin(), itr_row_end = m_rows.end();
        for (; itr_row != itr_row_end; ++itr_row)
        {
            typename row_type::const_iterator itr_col = (*itr_row)->begin(), itr_col_end = (*itr_row)->end();
            for (; itr_col != itr_col_end; ++itr_col)
            {
                const element* p = *itr_col;
                matrix_element_t elem_type = p->m_type;
                if (elem_type != element_numeric && elem_type != element_boolean)
                {
                    m_numeric = false;
                    m_valid = true;
                    return m_numeric;
                }
            }
        }

        m_numeric = true;
        m_valid = true;
        return m_numeric;
    }

    bool empty() const
    {
        return m_rows.empty();
    }

    ::mdds::storage_base<matrix_type>* clone() const
    {
        return new storage_filled_nested_array(*this);
    }

    const rows_type& get_rows() const { return m_rows; }

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
                    append_new_elem(*m_rows[i]);
            }
            else if (new_cols < cur_cols)
            {
                // Delete the instances being cast out.
                delete_elems_from_row(*m_rows[i], new_cols);
                m_rows[i]->resize(new_cols);
            }
        }
    }

    void delete_rows(rows_type& row_array, size_t first_row)
    {
        typename rows_type::iterator itr = row_array.begin(), itr_end = row_array.end();
        ::std::advance(itr, first_row);
        for (; itr != itr_end; ++itr)
        {
            row_type* p = *itr;
            m_row_pool->destroy(p);
        }
    }

    void delete_elems_from_row(row_type& row, size_t new_cols)
    {
        typename row_type::iterator itr = row.begin(), itr_end = row.end();
        ::std::advance(itr, new_cols);
        for (; itr != itr_end; ++itr)
        {
            element* p = *itr;
            if (p != &m_init_elem)
                m_element_pool->destroy(p);
        }
    }

    void append_new_elem(row_type& row)
    {
        matrix_init_element_t init_type = storage_base<matrix_type>::get_init_type();
        switch (init_type)
        {
            case matrix_init_element_zero:
                row.push_back(m_element_pool->construct(static_cast<double>(0.0)));
            break;
            case matrix_init_element_empty:
                row.push_back(m_element_pool->construct());
            break;
            default:
                throw matrix_storage_error("unknown init type.");
        }
    }

private:
    ::boost::object_pool<row_type>* m_row_pool;
    ::boost::object_pool<element>* m_element_pool;
    rows_type m_rows;
    element m_init_elem;
    bool m_numeric:1;
    bool m_valid:1;
};

}
