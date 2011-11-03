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

namespace mdds { namespace __mtm {

/**
 * This storage stores only non-empty elements.
 */
template<typename _MatrixType>
class storage_sparse : public storage_base<_MatrixType>
{
    typedef _MatrixType matrix_type;

    typedef typename matrix_type::string_type  string_type;

public:
    typedef typename matrix_type::element      element;
    typedef ::boost::ptr_map<size_t, element>  row_type;
    typedef ::boost::ptr_map<size_t, row_type> rows_type;
    struct elem_wrap
    {
        const element& operator() (const typename row_type::const_iterator& itr) const
        {
            return *itr->second;
        }
    };
    struct rows_wrap
    {
        const row_type& operator() (const typename rows_type::const_iterator& itr) const
        {
            return *itr->second;
        }
    };
    typedef ::mdds::const_itr_access<storage_sparse, elem_wrap, rows_wrap> const_itr_access;

    storage_sparse(size_t _rows, size_t _cols, matrix_init_element_t init_type) :
        storage_base<matrix_type>(matrix_storage_sparse, init_type),
        m_row_size(_rows), m_col_size(_cols),
        m_numeric(_rows && _cols), m_valid(true)
    {
        switch (storage_base<matrix_type>::get_init_type())
        {
            case matrix_init_element_zero:
                m_empty_elem.m_type = element_numeric;
                m_empty_elem.m_numeric = 0.0;
            break;
            default:
                m_empty_elem.m_type = element_empty;
                m_numeric = false;
        }
    }

    storage_sparse(const storage_sparse& r) :
        storage_base<matrix_type>(r),
        m_rows(r.m_rows),
        m_empty_elem(r.m_empty_elem),
        m_row_size(r.m_row_size),
        m_col_size(r.m_col_size) {}

    ~storage_sparse() {}

    const_itr_access* get_const_itr_access() const { return new const_itr_access(*this); }

    element & get_element(size_t row, size_t col)
    {
        if (row >= m_row_size || col >= m_col_size)
            throw matrix_storage_error("specified element is out-of-bound.");

        m_valid = false;

        typename rows_type::iterator itr = m_rows.find(row);
        if (itr == m_rows.end())
        {
            // Insert a brand-new row.
            ::std::pair<typename rows_type::iterator, bool> r = m_rows.insert(row, new row_type);
            if (!r.second)
                throw matrix_storage_error("failed to insert a new row instance into storage_sparse.");
            itr = r.first;
        }

        row_type& row_store = *itr->second;
        typename row_type::iterator itr_elem = row_store.find(col);
        if (itr_elem == row_store.end())
        {
            // Insert a new element at this column position.
            ::std::pair<typename row_type::iterator, bool> r = row_store.insert(col, new element);
            if (!r.second)
                throw matrix_storage_error("failed to insert a new element instance.");
            itr_elem = r.first;
        }
        return *itr_elem->second;
    }

    matrix_element_t get_type(size_t row, size_t col) const
    {
        typename rows_type::const_iterator itr = m_rows.find(row);
        if (itr == m_rows.end())
            return m_empty_elem.m_type;

        const row_type& row_store = *itr->second;
        typename row_type::const_iterator itr_elem = row_store.find(col);
        if (itr_elem == row_store.end())
            return m_empty_elem.m_type;

        return itr_elem->second->m_type;
    }

    double get_numeric(size_t row, size_t col) const
    {
        const element& elem = get_non_empty_element(row, col);
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
        matrix_element_t elem_type = get_type(row, col);
        if (elem_type != element_string)
            throw matrix_storage_error("element type is not string.");

        return get_non_empty_element(row, col).mp_string;
    }

    bool get_boolean(size_t row, size_t col) const
    {
        matrix_element_t elem_type = get_type(row, col);
        if (elem_type != element_boolean)
            throw matrix_storage_error("element type is not string.");

        return get_non_empty_element(row, col).m_boolean;
    }

    size_t rows() const
    {
        return m_row_size;
    }

    size_t cols() const
    {
        return m_col_size;
    }

    typedef ::std::pair<size_t, size_t> elem_pos_type;

    struct elem_pos_sorter : public ::std::binary_function<elem_pos_type, elem_pos_type, bool>
    {
        bool operator() (const elem_pos_type& left, const elem_pos_type& right) const
        {
            if (left.first != right.first)
                return left.first < right.first;
            return left.second < right.second;
        }
    };

    void transpose()
    {
        using namespace std;

        rows_type trans;

        // First, pick up the positions of all non-empty elements.
        vector<elem_pos_type> filled_elems;
        {
            typename rows_type::const_iterator itr_row = m_rows.begin(), itr_row_end = m_rows.end();
            for (; itr_row != itr_row_end; ++itr_row)
            {
                size_t row_idx = itr_row->first;
                const row_type& row = *itr_row->second;
                typename row_type::const_iterator itr_col = row.begin(), itr_col_end = row.end();
                for (; itr_col != itr_col_end; ++itr_col)
                {
                    // Be sure to swap the row and column indices.
                    filled_elems.push_back(elem_pos_type(itr_col->first, row_idx));
                }
            }
        }
        // Sort by row index first, then by column index.
        sort(filled_elems.begin(), filled_elems.end(), elem_pos_sorter());

        // Iterate through the non-empty element positions and perform
        // transposition.
        typename vector<elem_pos_type>::const_iterator
            itr_pos = filled_elems.begin(), itr_pos_end = filled_elems.end();
        while (itr_pos != itr_pos_end)
        {
            // First item of the new row.
            size_t row_idx = itr_pos->first;
            size_t col_idx = itr_pos->second;
            pair<typename rows_type::iterator, bool> r = trans.insert(row_idx, new row_type);
            if (!r.second)
                throw matrix_storage_error("failed to insert a new row instance during transposition.");

            typename rows_type::iterator itr_row = r.first;
            row_type& row = *itr_row->second;
            pair<typename row_type::iterator, bool> r2 =
                row.insert(col_idx, new element(m_rows[col_idx][row_idx]));
            if (!r2.second)
                throw matrix_storage_error("afiled to insert a new element instance during transposition.");

            // Keep iterating until we get a different row index.
            for (++itr_pos; itr_pos != itr_pos_end && itr_pos->first == row_idx; ++itr_pos)
            {
                col_idx = itr_pos->second;
                r2 = row.insert(col_idx, new element(m_rows[col_idx][row_idx]));
                if (!r2.second)
                    throw matrix_storage_error("afiled to insert a new element instance during transposition.");
            }
        }

        m_rows.swap(trans);
        ::std::swap(m_row_size, m_col_size);
    }

    void resize(size_t row, size_t col)
    {
        m_valid = false;

        if (!row || !col)
        {
            clear();
            return;
        }

        // Resizing a sparse matrix need to modify the data only when
        // shrinking.

        if (m_row_size > row)
        {
            // Remove all rows where the row index is greater than or
            // equal to 'row'.
            typename rows_type::iterator itr = m_rows.lower_bound(row);
            m_rows.erase(itr, m_rows.end());
        }

        if (m_col_size > col)
        {
            typename rows_type::iterator itr = m_rows.begin(), itr_end = m_rows.end();
            for (; itr != itr_end; ++itr)
            {
                // Now, remove all columns where the column index is
                // greater than or equal to 'col'.
                row_type& row_container = *itr->second;
                typename row_type::iterator itr_elem = row_container.lower_bound(col);
                row_container.erase(itr_elem, row_container.end());
            }
        }

        m_row_size = row;
        m_col_size = col;
    }

    void clear()
    {
        m_rows.clear();
        m_row_size = 0;
        m_col_size = 0;
        m_valid = true;
        m_numeric = false;
    }

    bool numeric()
    {
        using namespace std;

        if (m_valid)
            return m_numeric;

        size_t non_empty_count = 0;
        typename rows_type::const_iterator itr_row = m_rows.begin(), itr_row_end = m_rows.end();
        for (; itr_row != itr_row_end; ++itr_row)
        {
            const row_type& row = *itr_row->second;
            non_empty_count += row.size();
            assert(row.size() <= m_col_size);
            typename row_type::const_iterator itr_col = row.begin(), itr_col_end = row.end();
            for (; itr_col != itr_col_end; ++itr_col)
            {
                const element& elem = *itr_col->second;
                if (elem.m_type != element_numeric && elem.m_type != element_boolean)
                {
                    m_valid = true;
                    m_numeric = false;
                    return m_numeric;
                }
            }
        }

        // All non-empty elements are numeric.

        matrix_init_element_t init_type = storage_base<matrix_type>::get_init_type();
        if (init_type == matrix_init_element_zero)
            m_numeric = true;
        else
        {
            size_t total_elem_count = m_row_size * m_col_size;
            assert(non_empty_count <= total_elem_count);
            m_numeric = total_elem_count == non_empty_count;
        }

        m_valid = true;
        return m_numeric;
    }

    bool empty() const
    {
        // If one of row and column sizes are zero, the other size must be
        // zero, and vise versa.
        assert((!m_row_size && !m_col_size) || (m_row_size && m_col_size));

        return m_row_size == 0 || m_col_size == 0;
    }

    storage_base<matrix_type>* clone() const
    {
        return new storage_sparse(*this);
    }

    const rows_type& get_rows() const { return m_rows; }

private:
    const element& get_non_empty_element(size_t row, size_t col) const
    {
        typename rows_type::const_iterator itr = m_rows.find(row);
        if (itr == m_rows.end())
            return m_empty_elem;

        const row_type& row_store = *itr->second;
        typename row_type::const_iterator itr_elem = row_store.find(col);
        if (itr_elem == row_store.end())
            return m_empty_elem;
        return *itr_elem->second;
    }

private:
    rows_type   m_rows;
    element     m_empty_elem;
    size_t      m_row_size;
    size_t      m_col_size;
    bool        m_numeric:1;
    bool        m_valid:1;
};

}}
