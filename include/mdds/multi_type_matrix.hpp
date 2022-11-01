/*************************************************************************
 *
 * Copyright (c) 2012-2021 Kohei Yoshida
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

#ifdef MDDS_MULTI_TYPE_MATRIX_DEBUG
#ifndef MDDS_MULTI_TYPE_VECTOR_DEBUG
#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#endif
#endif

#include "multi_type_vector.hpp"

namespace mdds {

namespace mtm {

/**
 * Element type for multi_type_matrix.
 */
enum element_t
{
    element_empty = mdds::mtv::element_type_empty,
    element_boolean = mdds::mtv::element_type_boolean,
    element_string = mdds::mtv::element_type_string,
    element_numeric = mdds::mtv::element_type_double,
    element_integer = mdds::mtv::element_type_int32
};

/**
 * Default matrix trait that uses std::string as its string type.
 */
struct std_string_traits
{
    typedef mdds::mtv::int32_element_block integer_element_block;
    typedef mdds::mtv::string_element_block string_element_block;
};

} // namespace mtm

/**
 * Matrix that can store numeric, integer, boolean, empty and string types.
 * The string and integer types can be specified in the matrix trait
 * template parameter. To use std::string as the string type and int as the
 * integer type, use mdds::mtm::std_string_traits.
 *
 * Internally it uses mdds::multi_type_vector as its value store.  The
 * element values are linearly stored in column-major order.
 */
template<typename Traits>
class multi_type_matrix
{
    typedef Traits traits_type;

public:
    typedef typename traits_type::string_element_block string_block_type;
    typedef typename traits_type::integer_element_block integer_block_type;

    typedef typename string_block_type::value_type string_type;
    typedef typename integer_block_type::value_type integer_type;
    typedef size_t size_type;

private:
    struct mtv_trait : public mdds::mtv::default_traits
    {
        using block_funcs = mdds::mtv::element_block_funcs<
            mdds::mtv::boolean_element_block, mdds::mtv::int8_element_block, mdds::mtv::double_element_block,
            typename traits_type::string_element_block, typename traits_type::integer_element_block>;
    };

    using store_type = mdds::multi_type_vector<mtv_trait>;

public:
    typedef typename store_type::position_type position_type;
    typedef typename store_type::const_position_type const_position_type;

    typedef typename store_type::element_block_type element_block_type;

    typedef typename mtv::boolean_element_block boolean_block_type;
    typedef typename mtv::double_element_block numeric_block_type;

    struct size_pair_type
    {
        size_type row;
        size_type column;
        size_pair_type() : row(0), column(0)
        {}
        size_pair_type(size_type _row, size_type _column) : row(_row), column(_column)
        {}
        size_pair_type(std::initializer_list<size_type> vs)
        {
            if (vs.size() != 2)
                throw invalid_arg_error("size_pair_type requires exactly 2 elements.");

            size_type* ptrs[2] = {&row, &column};
            size_type** p = ptrs;

            for (size_type v : vs)
                **p++ = v;
        }

        bool operator==(const size_pair_type& r) const
        {
            return row == r.row && column == r.column;
        }
        bool operator!=(const size_pair_type& r) const
        {
            return !operator==(r);
        }
    };

    struct element_block_node_type
    {
        friend class multi_type_matrix;

        mtm::element_t type;
        size_type offset;
        size_type size;
        const element_block_type* data;

        element_block_node_type();
        element_block_node_type(const element_block_node_type& other);

        template<typename _Blk>
        typename _Blk::const_iterator begin() const;

        template<typename _Blk>
        typename _Blk::const_iterator end() const;

    private:
        void assign(const const_position_type& pos, size_type section_size);
    };

    static mtm::element_t to_mtm_type(mdds::mtv::element_t mtv_type)
    {
        switch (mtv_type)
        {
            case string_block_type::block_type:
                return mdds::mtm::element_string;
            case integer_block_type::block_type:
                return mdds::mtm::element_integer;
            case mdds::mtv::element_type_double:
            case mdds::mtv::element_type_boolean:
            case mdds::mtv::element_type_empty:
                // These types share the same numeric values.
                return static_cast<mtm::element_t>(mtv_type);
            default:
                throw type_error("multi_type_matrix: unknown element type.");
        }
    }

private:
    template<typename FuncT>
    struct walk_func
    {
        FuncT& m_func;
        walk_func(FuncT& func) : m_func(func)
        {}

        void operator()(const typename store_type::const_iterator::value_type& mtv_node)
        {
            element_block_node_type mtm_node;
            mtm_node.type = to_mtm_type(mtv_node.type);
            mtm_node.size = mtv_node.size;
            mtm_node.data = mtv_node.data;
            m_func(mtm_node);
        }
    };

public:
    /**
     * Move to the next logical position. The movement is in the top-to-bottom
     * then left-to-right direction.
     *
     * @param pos position object.
     *
     * @return position object that references the element at the next logical
     *         position.
     */
    static position_type next_position(const position_type& pos);

    /**
     * Move to the next logical position. The movement is in the top-to-bottom
     * then left-to-right direction.
     *
     * @param pos position object.
     *
     * @return non-mutable position object that references the element at the
     *         next logical position.
     */
    static const_position_type next_position(const const_position_type& pos);

    /**
     * Default constructor.
     */
    multi_type_matrix();

    /**
     * Construct a matrix of specified size.
     *
     * @param rows size of rows.
     * @param cols size of columns.
     */
    multi_type_matrix(size_type rows, size_type cols);

    /**
     * Construct a matrix of specified size and initialize all its elements
     * with specified value.
     *
     * @param rows size of rows.
     * @param cols size of columns.
     * @param value value to initialize all its elements with.
     */
    template<typename _T>
    multi_type_matrix(size_type rows, size_type cols, const _T& value);

    /**
     * Construct a matrix of specified size and initialize its elements with
     * specified values.  The values are assigned to 2-dimensional matrix
     * layout in column-major order.  The size of the value array must equal
     * <code>rows</code> x <code>cols</code>.
     *
     * @param rows size of rows.
     * @param cols size of columns.
     * @param it_begin iterator that points to the value of the first element.
     * @param it_end iterator that points to the position after the last
     *               element value.
     */
    template<typename _T>
    multi_type_matrix(size_type rows, size_type cols, const _T& it_begin, const _T& it_end);

    /**
     * Copy constructor.
     */
    multi_type_matrix(const multi_type_matrix& r);

    /**
     * Destructor.
     */
    ~multi_type_matrix();

    bool operator==(const multi_type_matrix& other) const;
    bool operator!=(const multi_type_matrix& other) const;

    multi_type_matrix& operator=(const multi_type_matrix& r);

    /**
     * Get a mutable reference of an element (position object) at specified
     * position. The position object can then be passed to an additional
     * method to get the type or value of the element it references, or set a
     * new value to it.
     *
     * @param row row position of the referenced element.
     * @param col column position of the referenced element.
     *
     * @return reference object of element at specified position.
     */
    position_type position(size_type row, size_type col);

    /**
     * Get a mutable reference of an element (position object) at specified
     * position. The position object can then be passed to an additional
     * method to get the type or value of the element it references, or set a
     * new value to it.
     *
     * @param pos_hint position object to be used as a position hint for
     *                 faster lookup.
     * @param row row position of the referenced element.
     * @param col column position of the referenced element.
     *
     * @return reference object of element at specified position.
     */
    position_type position(const position_type& pos_hint, size_type row, size_type col);

    /**
     * Get an immutable reference of an element (position object) at specified
     * position. The position object can then be passed to an additional
     * method to get the type or value of the element it references.
     *
     * @param row row position of the referenced element.
     * @param col column position of the referenced element.
     *
     * @return reference object of element at specified position.
     */
    const_position_type position(size_type row, size_type col) const;

    /**
     * Get an immutable reference of an element (position object) at specified
     * position. The position object can then be passed to an additional
     * method to get the type or value of the element it references.
     *
     * @param pos_hint position object to be used as a position hint for
     *                 faster lookup.
     * @param row row position of the referenced element.
     * @param col column position of the referenced element.
     *
     * @return reference object of element at specified position.
     */
    const_position_type position(const const_position_type& pos_hint, size_type row, size_type col) const;

    /**
     * Get the row and column positions of the current element from a position
     * object.
     *
     * @param pos position object.
     *
     * @return 0-based row and column positions.
     */
    size_pair_type matrix_position(const const_position_type& pos) const;

    /**
     * Return a position type that represents an end position.  This can be
     * used to compare with another position object to see if it is past the
     * last element position.
     *
     * @return end position object.
     */
    position_type end_position();

    /**
     * Return a position type that represents an end position.  This can be
     * used to compare with another position object to see if it is past the
     * last element position.
     *
     * @return end position object.
     */
    const_position_type end_position() const;

    /**
     * Get the type of element from a position object.  The type can be one
     * of empty, string, numeric, or boolean.
     *
     * @param pos position object of an element
     *
     * @return element type.
     */
    mtm::element_t get_type(const const_position_type& pos) const;

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
     * Get a numeric representation of the element from a position object.
     * If the element is of numeric type, its value is returned.  If it's of
     * boolean type, either 1 or 0 is returned depending on whether it's true
     * or false.  If it's of empty or string type, 0 is returned.
     *
     * @param pos position object of an element
     *
     * @return numeric representation of the element.
     */
    double get_numeric(const const_position_type& pos) const;

    /**
     * Get an integer representation of the element.  If the element is of
     * integer type, its value is returned.  If it's of boolean type, either 1
     * or 0 is returned depending on whether it's true or false.  If it's of
     * empty or string type, 0 is returned.
     *
     * @param row row position of the element.
     * @param col column position of the element.
     *
     * @return integer representation of the element.
     */
    integer_type get_integer(size_type row, size_type col) const;

    /**
     * Get an integer representation of the element.  If the element is of
     * integer type, its value is returned.  If it's of boolean type, either 1
     * or 0 is returned depending on whether it's true or false.  If it's of
     * empty or string type, 0 is returned.
     *
     * @param pos position object of an element
     *
     * @return integer representation of the element.
     */
    integer_type get_integer(const const_position_type& pos) const;

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
     * Get a boolean representation of the element from a position object.
     * If the element is of numeric type, true is returned if it's non-zero,
     * otherwise false is returned. If it's of boolean type, its value is
     * returned.  If it's of empty or string type, false is returned.
     *
     * @param pos position object of an element
     *
     * @return boolean representation of the element.
     */
    bool get_boolean(const const_position_type& pos) const;

    /**
     * Get the value of a string element.  If the element is not of string
     * type, it throws an exception.
     *
     * @param row row position of the element.
     * @param col column position of the element.
     *
     * @return value of the element.
     */
    const string_type& get_string(size_type row, size_type col) const;

    /**
     * Get the value of a string element from a position object.  If the
     * element is not of string type, it throws an exception.
     *
     * @param pos position object of an element
     *
     * @return value of the element.
     */
    const string_type& get_string(const const_position_type& pos) const;

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

    /**
     * Set specified element position empty.
     *
     * @param row row position of the element.
     * @param col column position of the element.
     */
    void set_empty(size_type row, size_type col);

    /**
     * Set a range of elements empty.  The range starts from the position
     * specified by the <code>row</code> and <code>col</code>, and extends
     * downward first then to the right.
     *
     * @param row row position of the first element.
     * @param col column position of the first element.
     * @param length length of the range to set empty.  When the length is
     *               greater than 1, the range extends downward first then to
     *               the right.
     */
    void set_empty(size_type row, size_type col, size_type length);

    /**
     * Set element referenced by the position object empty.
     *
     * @param pos position object that references element.
     *
     * @return position of the element that has just been made empty.
     */
    position_type set_empty(const position_type& pos);

    /**
     * Set the entire column empty.
     *
     * @param col index of the column to empty.
     */
    void set_column_empty(size_type col);

    /**
     * Set the entire row empty.
     *
     * @param row index of the row to empty.
     */
    void set_row_empty(size_type row);

    /**
     * Set a numeric value to an element at specified position.
     *
     * @param row row index of the element.
     * @param col column index of the element.
     * @param val new value to set.
     */
    void set(size_type row, size_type col, double val);

    /**
     * Set a numeric value to an element at specified position.
     *
     * @param pos position of the element to update.
     * @param val new value to set.
     *
     * @return position of the element block where the new value has been set.
     */
    position_type set(const position_type& pos, double val);

    /**
     * Set a boolean value to an element at specified position.
     *
     * @param row row index of the element.
     * @param col column index of the element.
     * @param val new value to set.
     */
    void set(size_type row, size_type col, bool val);

    /**
     * Set a boolean value to an element at specified position.
     *
     * @param pos position of the element to update.
     * @param val new value to set.
     *
     * @return position of the element where the new value has been set.
     */
    position_type set(const position_type& pos, bool val);

    /**
     * Set a string value to an element at specified position.
     *
     * @param row row index of the element.
     * @param col column index of the element.
     * @param str new value to set.
     */
    void set(size_type row, size_type col, const string_type& str);

    /**
     * Set a string value to an element at specified position.
     *
     * @param pos position of the element to update.
     * @param str new value to set.
     *
     * @return position of the element block where the new value has been set.
     */
    position_type set(const position_type& pos, const string_type& str);

    /**
     * Set an integer value to an element at specified position.
     *
     * @param row row index of the element.
     * @param col column index of the element.
     * @param val new value to set.
     */
    void set(size_type row, size_type col, integer_type val);

    /**
     * Set an integer value to an element at specified position.
     *
     * @param pos position of the element to update.
     * @param val new value to set.
     *
     * @return position of the element block where the new value has been set.
     */
    position_type set(const position_type& pos, integer_type val);

    /**
     * Set values of multiple elements at once, starting at specified element
     * position following the direction of columns.  When the new value series
     * does not fit in the first column, it gets wrapped into the next
     * column(s).
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
     * Set values of multiple elements at once, starting at specified element
     * position following the direction of columns.  When the new value series
     * does not fit in the first column, it gets wrapped into the next
     * column(s).
     *
     * @param pos position of the first element.
     * @param it_begin iterator that points to the begin position of the
     *                 values being set.
     * @param it_end iterator that points to the end position of the values
     *               being set.
     *
     * @return position of the first element that has been modified.
     */
    template<typename _T>
    position_type set(const position_type& pos, const _T& it_begin, const _T& it_end);

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
     * @param src passed matrix object to copy element values from.
     */
    void copy(const multi_type_matrix& src);

    /**
     * Copy values from an array or array-like container, to a specified
     * sub-matrix range.  The length of the array must match the number of
     * elements in the destination range, else it will throw a
     * mdds::size_error.
     *
     * @param rows row size of the destination range.
     * @param cols column size of the destination range.
     * @param it_begin iterator pointing to the beginning of the input array.
     * @param it_end iterator pointing to the end position of the input array.
     */
    template<typename _T>
    void copy(size_type rows, size_type cols, const _T& it_begin, const _T& it_end);

    /**
     * Resize the matrix to specified size.  This method supports resizing to
     * zero-sized matrix; however, either specifying the row or column size to
     * zero will resize the matrix to 0 x 0.  When resizing the matrix larger,
     * empty elements will be inserted in the region where no elements
     * existed prior to the call.
     *
     * @param rows new row size
     * @param cols new column size
     */
    void resize(size_type rows, size_type cols);

    /**
     * Resize the matrix to specified size and initial value.  The initial
     * value will be applied to new elements created when the specified size
     * is larger than the current one.
     *
     * @param rows new row size
     * @param cols new column size
     * @param value initial value for new elements
     */
    template<typename _T>
    void resize(size_type rows, size_type cols, const _T& value);

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
     *
     * @return function object passed to this method.
     */
    template<typename FuncT>
    FuncT walk(FuncT func) const;

    /**
     * Walk through the element blocks in a sub-matrix range defined by start
     * and end positions passed to this method.
     *
     * @param func function object whose operator() gets called on the
     *              element block.
     *
     * @param start the column/row position of the upper-left corner of the
     *              sub-matrix.
     *
     * @param end the column/row position of the lower-right corner of the
     *          sub-matrix.  Both column and row must be greater or equal to
     *          those of the start position.
     *
     * @return function object passed to this method.
     */
    template<typename FuncT>
    FuncT walk(FuncT func, const size_pair_type& start, const size_pair_type& end) const;

    /**
     * Walk through all element blocks in parallel with another matrix
     * instance.  It stops at the block boundaries of both matrix instances
     * during the walk.
     *
     * @param func function object whose operator() gets called on each
     *             element block.
     *
     * @param right another matrix instance to parallel-walk with.
     */
    template<typename FuncT>
    FuncT walk(FuncT func, const multi_type_matrix& right) const;

    /**
     * Walk through the element blocks in a sub-matrix range in parallel with
     * another matrix instance. It stops at the block boundaries of both
     * matrix instances during the walk.  The sub-matrix range is defined by
     * start and end positions passed to this method.
     *
     * @param func function object whose operator() gets called on each
     *             element block.
     *
     * @param right another matrix instance to parallel-walk with.
     *
     * @param start the column/row position of the upper-left corner of the
     *              sub-matrix.
     *
     * @param end the column/row position of the lower-right corner of the
     *          sub-matrix.  Both column and row must be greater or equal to
     *          those of the start position.
     */
    template<typename FuncT>
    FuncT walk(
        FuncT func, const multi_type_matrix& right, const size_pair_type& start, const size_pair_type& end) const;

#ifdef MDDS_MULTI_TYPE_MATRIX_DEBUG
    void dump() const
    {
        m_store.dump_blocks(std::cout);
    }
#endif

private:
    /**
     * Get an array position of the data referenced by the row and column
     * indices.  The array consists of multiple columns, the content of column
     * 0 followed by the content of column 1, and so on.  <b>Note that no
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

    inline size_type get_pos(const const_position_type& pos) const
    {
        return pos.first->position + pos.second;
    }

private:
    store_type m_store;
    size_pair_type m_size;
};

} // namespace mdds

#include "multi_type_matrix_def.inl"

#endif
