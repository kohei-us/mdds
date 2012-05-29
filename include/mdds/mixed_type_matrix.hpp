/*************************************************************************
 *
 * Copyright (c) 2010, 2011 Kohei Yoshida
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

#ifndef __MDDS_MIXED_TYPE_MATRIX_HPP__
#define __MDDS_MIXED_TYPE_MATRIX_HPP__

#include "mdds/global.hpp"
#include "mdds/mixed_type_matrix_element.hpp"
#include "mdds/mixed_type_matrix_storage.hpp"
#include "mdds/mixed_type_matrix_flag_storage.hpp"

#include <iostream>
#include <cstdlib>

namespace mdds {

enum matrix_density_t
{
    matrix_density_filled_zero,
    matrix_density_filled_empty,
    matrix_density_sparse_zero,
    matrix_density_sparse_empty
};

class matrix_error : public ::mdds::general_error
{
public:
    matrix_error(const ::std::string& msg) : general_error(msg) {}
};

/**
 * This data structure represents a matrix where each individual element may
 * be of one of four types: value, boolean, string, or empty.
 */
template<typename _String, typename _Flag>
class mixed_type_matrix
{
public:
    typedef _String     string_type;
    typedef _Flag       flag_type;
    typedef size_t      size_type;
    typedef ::std::pair<size_type, size_type> size_pair_type;
    typedef ::mdds::element<string_type> element;

private:
    struct size_pair_type_hash
    {
        size_t operator() (const size_pair_type& val) const
        {
            size_t n = val.first + (val.second << 8);
            return n;
        }
    };
    typedef ::mdds::storage_base<mixed_type_matrix> storage_base;

    static storage_base* create_storage(size_t rows, size_t cols, matrix_density_t density);

public:
    typedef ::mdds::__mtm::flag_storage<flag_type, size_pair_type, size_pair_type_hash> flag_storage;
    typedef ::mdds::__mtm::storage_filled_linear<mixed_type_matrix> filled_storage_type;
    typedef ::mdds::__mtm::storage_filled_linear_zero<mixed_type_matrix> filled_storage_zero_type;
    typedef ::mdds::__mtm::storage_sparse<mixed_type_matrix> sparse_storage_type;

    typedef typename storage_base::const_iterator const_iterator;

    /**
     * Default constructor.
     */
    mixed_type_matrix();

    /**
     * Construct an empty matrix with specified density type.
     */
    mixed_type_matrix(matrix_density_t density);

    /**
     * Construct a matrix of specified size with specified density type.
     */
    mixed_type_matrix(size_t rows, size_t cols, matrix_density_t density);

    mixed_type_matrix(const mixed_type_matrix& r);
    ~mixed_type_matrix();

    const_iterator begin() const;
    const_iterator end() const;

    mixed_type_matrix& operator= (const mixed_type_matrix& r);

    /**
     * Get the type of element specified by its position.  The type can be one
     * of empty, string, numeric, or boolean.
     *
     * @return element type.
     */
    matrix_element_t get_type(size_t row, size_t col) const;

    double get_numeric(size_t row, size_t col) const;
    bool get_boolean(size_t row, size_t col) const;
    const string_type* get_string(size_t row, size_t col) const;

    void set_numeric(size_t row, size_t col, double val);
    void set_boolean(size_t row, size_t col, bool val);
    void set_string(size_t row, size_t col, string_type* str);
    void set_empty(size_t row, size_t col);

    void set(size_t row, size_t col, double val);
    void set(size_t row, size_t col, bool val);
    void set(size_t row, size_t col, string_type* str);

    /**
     * Set flag value at specified position.
     *
     * @param row row position
     * @param col column position
     * @param flag_type flag value
     */
    void set_flag(size_t row, size_t col, flag_type flag);

    /**
     * Get flag value at specified position.
     *
     * @param row row position
     * @param col column position
     *
     * @return flag value stored at specified position
     */
    flag_type get_flag(size_t row, size_t col) const;

    void clear_flag(size_t row, size_t cols);

    /**
     * Return the size of matrix as a pair.  The first value is the row size,
     * while the second value is the column size.
     *
     * @return matrix size as a value pair.
     */
    size_pair_type size() const;

    /**
     * Transpose the stored matrix data.
     *
     * @return reference to this matrix instance.
     */
    mixed_type_matrix& transpose();

    /**
     * Assign values from the passed matrix instance.  If the size of the
     * passed matrix is smaller, then the element values are assigned by their
     * positions, while the rest of the elements that fall outside the size of
     * the passed matrix instance will remain unmodified.  If the size of the
     * pass matrix instance is larger, then only the elements within the size
     * of this matrix instance will get assigned.
     *
     * @param r passed matrix object to assign element values from.
     */
    void assign(const mixed_type_matrix& r);

    /**
     * Resize the matrix to specified size.  This method supports resizing to
     * zero-sized matrix; however, either specifying the row or column size to
     * zero will resize the matrix to 0 x 0.
     *
     * @param row new row size
     * @param col new column size
     */
    void resize(size_t row, size_t col);

    /**
     * Empty the matrix.
     */
    void clear();

    /**
     * Check whether or not this matrix is numeric.  A numeric matrix contains
     * only numeric or boolean elements.
     *
     * @return true if the matrix contains only numeric or boolean elements,
     *         or false otherwise.
     */
    bool numeric() const;

    /**
     * Check whether or not this matrix is empty.
     *
     * @return true if this matrix is empty, or false otherwise.
     */
    bool empty() const;

    /**
     * Swap the content of the matrix with another instance.
     */
    void swap(mixed_type_matrix& r);

#ifdef UNIT_TEST
    void dump() const;
    void dump_flags() const;
    size_pair_type get_storage_size() const;
#endif

private:
    /**
     * Storage classes have no vtable; the concrete class type must be
     * specified when deleting the instance.
     */
    void delete_storage();

private:
    storage_base* mp_storage;
    size_pair_type m_cached_size;
};

}

#include "mixed_type_matrix_def.inl"

#endif
