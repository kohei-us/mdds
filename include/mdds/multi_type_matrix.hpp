/*************************************************************************
 *
 * Copyright (c) 2012 Kohei Yoshida
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

#ifndef __MDDS_MULTI_TYPE_MATRIX_HPP__
#define __MDDS_MULTI_TYPE_MATRIX_HPP__

#include "multi_type_vector.hpp"
#include "multi_type_matrix_block_func.hpp"

namespace mdds {

namespace mtm {

/**
 * Element type for multi_type_matrix.
 */
enum element_t { element_empty, element_boolean, element_string, element_numeric };

}

namespace __mtm {

inline mdds::mtm::element_t to_mtm_type(mdds::mtv::element_t mtv_type)
{
    switch (mtv_type)
    {
        case mdds::mtv::element_type_numeric:
            return mdds::mtm::element_numeric;
        case mdds::mtv::element_type_string:
        case mdds::__mtm::element_type_custom_string:
            return mdds::mtm::element_string;
        case mdds::mtv::element_type_boolean:
            return mdds::mtm::element_boolean;
        case mdds::mtv::element_type_empty:
            return mdds::mtm::element_empty;
        default:
            throw general_error("multi_type_matrix: unknown element type.");
    }
}

template<typename _MtvNode, typename _MtmNode, typename _Func>
struct walk_func : std::unary_function<_MtvNode, void>
{
    _Func& m_func;
    walk_func(_Func& func) : m_func(func) {}

    void operator() (const _MtvNode& mtv_node)
    {
        _MtmNode mtm_node;
        mtm_node.type = to_mtm_type(mtv_node.type);
        mtm_node.size = mtv_node.size;
        mtm_node.data = mtv_node.data;
        m_func(mtm_node);
    }
};

}

template<typename _String>
class multi_type_matrix
{
public:
    typedef _String     string_type;
    typedef size_t      size_type;

private:
    typedef __mtm::trait<string_type> string_trait;
    typedef mdds::multi_type_vector<typename string_trait::elem_block_func> store_type;

public:
    typedef typename store_type::element_block_type element_block_type;

    struct size_pair_type
    {
        size_type row;
        size_type column;
        size_pair_type() : row(0), column(0) {}
        size_pair_type(size_type _row, size_type _column) : row(_row), column(_column) {}
    };

    struct element_block_node_type
    {
        mtm::element_t type;
        size_type size;
        const element_block_type* data;

        element_block_node_type() : type(mtm::element_empty), size(0), data(NULL) {}
        element_block_node_type(const element_block_node_type& other) :
            type(other.type), size(other.size), data(other.data) {}
    };

    /**
     * Default constructor.
     */
    multi_type_matrix();

    /**
     * Construct a matrix of specified size with specified density type.
     */
    multi_type_matrix(size_type rows, size_type cols);

    template<typename _T>
    multi_type_matrix(size_type rows, size_type cols, const _T& value);

    multi_type_matrix(const multi_type_matrix& r);
    ~multi_type_matrix();

    multi_type_matrix& operator= (const multi_type_matrix& r);

    /**
     * Get the type of element specified by its position.  The type can be one
     * of empty, string, numeric, or boolean.
     *
     * @return element type.
     */
    mtm::element_t get_type(size_type row, size_type col) const;

    /**
     * Get a numeric representation of the element.  If the element is of
     * numeric type, its value is returned.  If it's of boolean type, either 1
     * or 0 is returned depending on whether it's true or false.  If it's of
     * empty or string type, 0 is returned.
     *
     * @param row row position of the element.
     * @param col column position of the element.
     *
     * @return numeric representation of the element.
     */
    double get_numeric(size_type row, size_type col) const;

    /**
     * Get a boolean representation of the element.  If the element is of
     * numeric type, true is returned if it's non-zero, otherwise false is
     * returned. If it's of boolean type, its value is returned.  If it's of
     * empty or string type, false is returned.
     *
     * @param row row position of the element.
     * @param col column position of the element.
     *
     * @return boolean representation of the element.
     */
    bool get_boolean(size_type row, size_type col) const;

    /**
     * Get the value of a string element.  If the element is not of string
     * type, it throws an exception.
     *
     * @param row row position of the element.
     * @param col column position of the element.
     *
     * @return value of the element.
     */
    string_type get_string(size_type row, size_type col) const;

    /**
     * Get the value of element at specified position.  The caller must
     * explicitly specify the return type.  If the element is not of the
     * specified type, it throws an exception.
     *
     * @param row row position of the element.
     * @param col column position of the element.
     *
     * @return value of the element.
     */
    template<typename _T>
    _T get(size_type row, size_type col) const;

    void set_empty(size_type row, size_type col);
    void set_column_empty(size_type col);
    void set_row_empty(size_type row);

    void set(size_type row, size_type col, double val);
    void set(size_type row, size_type col, bool val);
    void set(size_type row, size_type col, const string_type& str);

    /**
     * Set values of multiple elements at once, starting at specified element
     * position following the direction of columns.  When the new value series
     * does not fit in the column of the start element, it gets wrapped into
     * the next column(s).
     *
     * <p>The method will throw an <code>std::out_of_range</code> exception
     * if the specified position is outside the current container range.</p>
     *
     * @param row row position of the start element.
     * @param col column position of the start element.
     * @param it_begin iterator that points to the begin position of the
     *                 values being set.
     * @param it_end iterator that points to the end position of the values
     *               being set.
     */
    template<typename _T>
    void set(size_type row, size_type col, const _T& it_begin, const _T& it_end);

    /**
     * Set values of multiple elements at once in a single column.  When the
     * length of passed elements exceeds that of the column, it gets truncated
     * to the column size.
     *
     * @param col column position
     * @param it_begin iterator that points to the begin position of the
     *                 values being set.
     * @param it_end iterator that points to the end position of the values
     *               being set.
     */
    template<typename _T>
    void set_column(size_type col, const _T& it_begin, const _T& it_end);

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
    multi_type_matrix& transpose();

    /**
     * Copy values from the passed matrix instance.  If the size of the passed
     * matrix is smaller, then the element values are copied by their
     * positions, while the rest of the elements that fall outside the size of
     * the passed matrix instance will remain unmodified.  If the size of the
     * passed matrix instance is larger, then only the elements within the
     * size of this matrix instance will get copied.
     *
     * @param r passed matrix object to copy element values from.
     */
    void copy(const multi_type_matrix& r);

    /**
     * Resize the matrix to specified size.  This method supports resizing to
     * zero-sized matrix; however, either specifying the row or column size to
     * zero will resize the matrix to 0 x 0.
     *
     * @param rows new row size
     * @param cols new column size
     */
    void resize(size_type rows, size_type cols);

    /**
     * Empty the matrix.
     */
    void clear();

    /**
     * Check whether or not this matrix is numeric.  A numeric matrix contains
     * only numeric or boolean elements.  An empty matrix is not numeric.
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
    void swap(multi_type_matrix& r);

    /**
     * Walk all element blocks that consist of the matrix.
     *
     * @param func function object whose operator() gets called on each
     *             element block.
     */
    template<typename _Func>
    void walk(_Func& func) const;

private:

    /**
     * Get an array position of the data referenced by the row and column
     * indices.  The array consists of multiple columns, the content of column
     * 0 followded by the content of column 1, and so on.  <b>Note that no
     * boundary check is performed in this method.</b>
     *
     * @param row 0-based row index.
     * @param col 0-based column index.
     * @return position in the data array.
     */
    inline size_type get_pos(size_type row, size_type col) const
    {
        return m_size.row * col + row;
    }

private:
    store_type m_store;
    size_pair_type m_size;
};

}

#include "multi_type_matrix_def.inl"

#endif
