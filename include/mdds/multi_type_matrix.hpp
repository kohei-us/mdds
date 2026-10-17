// SPDX-FileCopyrightText: 2012 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#ifdef MDDS_MULTI_TYPE_MATRIX_DEBUG
#ifndef MDDS_MULTI_TYPE_VECTOR_DEBUG
#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#endif
#endif

#include "multi_type_vector.hpp"

#include <array>
#include <concepts>
#include <tuple>
#include <type_traits>

namespace mdds {

namespace mtm {

/**
 * Element type for multi_type_matrix.  Its value is the block type id of
 * the element block that stores the element, or mdds::mtm::element_empty
 * for an empty element.
 */
using element_t = mdds::mtv::element_t;

/**
 * Element type value for an empty element.
 */
inline constexpr element_t element_empty = mdds::mtv::element_type_empty;

namespace detail {

template<typename T>
concept element_block_type = std::derived_from<T, mdds::mtv::base_element_block> && requires {
    { T::block_type } -> std::convertible_to<mdds::mtv::element_t>;
    typename T::value_type;
};

template<typename T>
struct is_element_block_list : std::false_type
{
};

template<typename... Ts>
struct is_element_block_list<std::tuple<Ts...>> : std::bool_constant<(element_block_type<Ts> && ...)>
{
};

template<typename Traits>
struct element_blocks_of
{
    using type = std::tuple<>;
};

template<typename Traits>
    requires requires { typename Traits::element_blocks; }
struct element_blocks_of<Traits>
{
    using type = typename Traits::element_blocks;
};

template<typename Blk, typename Tuple>
struct prepend_block;

template<typename Blk, typename... Ts>
struct prepend_block<Blk, std::tuple<Ts...>>
{
    using type = std::tuple<Blk, Ts...>;
};

template<typename Traits>
struct block_list_of
{
    using type =
        typename prepend_block<typename Traits::numeric_element_block, typename element_blocks_of<Traits>::type>::type;
};

template<typename Tuple>
struct make_block_funcs;

template<typename... Ts>
struct make_block_funcs<std::tuple<Ts...>>
{
    using type = mdds::mtv::element_block_funcs<Ts...>;
};

template<typename Blk, typename Tuple>
struct is_block_in_list;

template<typename Blk, typename... Ts>
struct is_block_in_list<Blk, std::tuple<Ts...>> : std::bool_constant<(std::is_same_v<Blk, Ts> || ...)>
{
};

template<typename Tuple>
struct has_distinct_block_ids;

template<typename... Ts>
struct has_distinct_block_ids<std::tuple<Ts...>>
{
    static constexpr bool check()
    {
        constexpr std::array<mdds::mtv::element_t, sizeof...(Ts)> ids = {Ts::block_type...};

        for (std::size_t i = 0; i < ids.size(); ++i)
        {
            for (std::size_t j = i + 1; j < ids.size(); ++j)
            {
                if (ids[i] == ids[j])
                    return false;
            }
        }

        return true;
    }

    static constexpr bool value = check();
};

template<typename T, typename Tuple>
struct is_listed_value : std::false_type
{
};

template<typename T, typename... Ts>
struct is_listed_value<T, std::tuple<Ts...>> : std::bool_constant<(std::is_same_v<T, typename Ts::value_type> || ...)>
{
};

/**
 * Concept satisfied when the type T is the value type of one of the element
 * blocks in the std::tuple Blocks.
 */
template<typename T, typename Blocks>
concept listed_value_type = is_listed_value<T, Blocks>::value;

template<typename T, typename Tuple>
struct block_of;

template<typename T>
struct block_of<T, std::tuple<>>
{
    using type = void;
};

template<typename T, typename Blk, typename... Ts>
struct block_of<T, std::tuple<Blk, Ts...>>
{
    using type = std::conditional_t<
        std::is_same_v<T, typename Blk::value_type>, Blk, typename block_of<T, std::tuple<Ts...>>::type>;
};

/**
 * Concept satisfied when the type T is the value type of one of the element
 * blocks in the std::tuple Blocks, and that block provides references to
 * its stored elements.  A block backed by std::vector<bool> does not.
 */
template<typename T, typename Blocks>
concept referenceable_value_type =
    !std::is_void_v<typename block_of<T, Blocks>::type> &&
    std::is_lvalue_reference_v<typename block_of<T, Blocks>::type::store_type::const_reference>;

} // namespace detail

/**
 * Concept that a matrix traits type must satisfy.  A matrix traits type
 * specifies the element blocks that the matrix can store.  It must have the
 * following members:
 *
 * <ul>
 *   <li><code>numeric_element_block</code> - element block type that stores
 *   the numeric elements.  Its value type must be constructible from 0.
 *   This member is required.</li>
 *   <li><code>element_blocks</code> - std::tuple that lists the element
 *   block types of all the other element types the matrix can store, such
 *   as boolean, integer and string.  The tuple is used only as a list of
 *   types.  This member is optional.  When it is absent, the matrix stores
 *   only numeric and empty elements.</li>
 * </ul>
 *
 * Each block type must be an element block type whose value type has its
 * element callbacks defined.  All block types must have distinct block type
 * ids, and the numeric block type must not also appear in
 * <code>element_blocks</code>.
 */
template<typename T>
concept matrix_traits =
    requires { typename T::numeric_element_block; } && detail::element_block_type<typename T::numeric_element_block> &&
    std::constructible_from<typename T::numeric_element_block::value_type, int> &&
    detail::is_element_block_list<typename detail::element_blocks_of<T>::type>::value;

/**
 * Default matrix traits type that uses double as its numeric type, and
 * stores boolean, 32-bit integer and std::string values in addition to the
 * numeric values.
 */
struct std_string_traits
{
    using numeric_element_block = mdds::mtv::double_element_block;

    using element_blocks =
        std::tuple<mdds::mtv::boolean_element_block, mdds::mtv::int32_element_block, mdds::mtv::string_element_block>;
};

} // namespace mtm

/**
 * Matrix that can store numeric and empty elements, plus elements of any
 * other type specified in the matrix traits template parameter.  The matrix
 * traits type specifies the element block type that stores the numeric
 * elements, and the list of element block types that store all the other
 * element types.  See mdds::mtm::matrix_traits for the details.  To store
 * double, boolean, 32-bit integer and std::string values, use
 * mdds::mtm::std_string_traits.
 *
 * Internally it uses mdds::multi_type_vector as its value store.  The
 * element values are linearly stored in column-major order.
 */
template<mtm::matrix_traits Traits>
class multi_type_matrix
{
    using traits_type = Traits;

    using element_blocks_type = typename mtm::detail::element_blocks_of<traits_type>::type;

public:
    using numeric_block_type = typename traits_type::numeric_element_block;
    using numeric_type = typename numeric_block_type::value_type;
    using size_type = std::size_t;

    /**
     * Element type value for an empty element.
     */
    static constexpr mtm::element_t element_empty = mtm::element_empty;

    /**
     * Element type value for a numeric element.  It is the block type id of
     * the numeric element block.
     */
    static constexpr mtm::element_t element_numeric = numeric_block_type::block_type;

private:
    using block_list_type = typename mtm::detail::block_list_of<traits_type>::type;

    static_assert(
        !mtm::detail::is_block_in_list<numeric_block_type, element_blocks_type>::value,
        "The numeric element block must not also appear in element_blocks.");

    static_assert(
        mtm::detail::has_distinct_block_ids<block_list_type>::value,
        "All element blocks of the matrix must have distinct block type ids.");

    struct mtv_traits : public mdds::mtv::default_traits
    {
        using block_funcs = typename mtm::detail::make_block_funcs<block_list_type>::type;
    };

    using store_type = mdds::multi_type_vector<mtv_traits>;

public:
    using position_type = typename store_type::position_type;
    using const_position_type = typename store_type::const_position_type;

    using element_block_type = typename mtv::base_element_block;

    struct size_pair_type
    {
        size_type row;
        size_type column;
        size_pair_type() noexcept(std::is_fundamental_v<size_type>) : row(0), column(0)
        {}
        size_pair_type(size_type _row, size_type _column) noexcept(std::is_fundamental_v<size_type>)
            : row(_row), column(_column)
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

        bool operator==(const size_pair_type& r) const noexcept(std::is_fundamental_v<size_type>)
        {
            return row == r.row && column == r.column;
        }
    };

    struct element_block_node_type
    {
        friend class multi_type_matrix;

        mtm::element_t type;
        size_type offset;
        size_type size;
        const element_block_type* data;

        element_block_node_type() noexcept(std::is_fundamental_v<size_type>);
        element_block_node_type(const element_block_node_type& other) noexcept(std::is_fundamental_v<size_type>);

        template<typename _Blk>
        typename _Blk::const_iterator begin() const;

        template<typename _Blk>
        typename _Blk::const_iterator end() const;

    private:
        void assign(const const_position_type& pos, size_type section_size);
    };

private:
    template<typename FuncT>
    struct walk_func
    {
        FuncT& m_func;
        walk_func(FuncT& func) noexcept : m_func(func)
        {}

        void operator()(const typename store_type::const_iterator::value_type& mtv_node)
        {
            element_block_node_type mtm_node;
            mtm_node.type = mtv_node.type;
            mtm_node.size = mtv_node.size;
            mtm_node.data = mtv_node.data;
            m_func(mtm_node);
        }
    };

    /**
     * Throw a mdds::type_error if the type is not the type of one of the
     * element blocks of this matrix.
     *
     * @param type Element type to validate.
     */
    static void validate_type(mtm::element_t type);

    /**
     * Call the templated call operator of the function object, with the
     * element block type whose block type id equals the specified type as
     * its template argument.  Throw a mdds::type_error if no element block
     * of this matrix has that type.
     *
     * @param type Element type of the block.  It must not be empty.
     * @param func Function object to call with the matching block type.
     */
    template<typename FuncT>
    static void dispatch_block(mtm::element_t type, FuncT&& func);

    /**
     * Create a store initialized with the values in the specified range,
     * after validating the type of the values.
     */
    template<typename T>
    static store_type create_store(size_type init_size, const T& it_begin, const T& it_end);

    static constexpr bool nothrow_default_constructible_v =
        std::is_nothrow_default_constructible_v<store_type> && std::is_nothrow_default_constructible_v<size_pair_type>;

    static constexpr bool nothrow_eq_comparable_v =
        noexcept(std::declval<store_type>() == std::declval<store_type>()) &&
        noexcept(std::declval<size_pair_type>() == std::declval<size_pair_type>());

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
    multi_type_matrix() noexcept(nothrow_default_constructible_v);

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
    multi_type_matrix(const multi_type_matrix& other);

    /**
     * Move constructor.
     */
    multi_type_matrix(multi_type_matrix&& other) = default;

    /**
     * Destructor.
     */
    ~multi_type_matrix() = default;

    bool operator==(const multi_type_matrix& other) const noexcept(nothrow_eq_comparable_v);

    multi_type_matrix& operator=(const multi_type_matrix& other) = default;
    multi_type_matrix& operator=(multi_type_matrix&& other) = default;

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
     * Get the type of element from a position object.  The type is either
     * element_empty or the block type id of the element block that stores
     * the element.
     *
     * @param pos Position object of an element.
     *
     * @return Element type.
     */
    mtm::element_t get_type(const const_position_type& pos) const;

    /**
     * Get the type of element specified by its position.  The type is either
     * element_empty or the block type id of the element block that stores
     * the element.
     *
     * @param row Row position of the element.
     * @param col Column position of the element.
     *
     * @return Element type.
     */
    mtm::element_t get_type(size_type row, size_type col) const;

    /**
     * Get the numeric value of the element.  If the element is of numeric
     * type, its value is returned.  If it's empty, 0 is returned.  If it's of
     * any other type, a mdds::type_error exception is thrown.
     *
     * @param row Row position of the element.
     * @param col Column position of the element.
     *
     * @return Numeric value of the element.
     */
    numeric_type get_numeric(size_type row, size_type col) const;

    /**
     * Get the numeric value of the element from a position object.  If the
     * element is of numeric type, its value is returned.  If it's empty, 0 is
     * returned.  If it's of any other type, a mdds::type_error exception is
     * thrown.
     *
     * @param pos Position object of an element.
     *
     * @return Numeric value of the element.
     */
    numeric_type get_numeric(const const_position_type& pos) const;

    /**
     * Get the value of element at specified position.  The caller must
     * explicitly specify the value type.  If the element is empty, the empty
     * value of the specified type is returned.  If the element is of a
     * different type, a mdds::type_error exception is thrown.
     *
     * @param row Row position of the element.
     * @param col Column position of the element.
     *
     * @return Value of the element.
     */
    template<typename T>
    T get(size_type row, size_type col) const;

    /**
     * Get the value of element from a position object.  The caller must
     * explicitly specify the value type.  If the element is empty, the empty
     * value of the specified type is returned.  If the element is of a
     * different type, a mdds::type_error exception is thrown.
     *
     * @param pos Position object of an element.
     *
     * @return Value of the element.
     */
    template<typename T>
    T get(const const_position_type& pos) const;

    /**
     * Get a reference to the value of element at specified position.  The
     * caller must explicitly specify the value type.  Unlike get(), this
     * method throws a mdds::type_error exception when the element is empty,
     * since there is no stored value to reference.  It also throws when the
     * element is of a different type.
     *
     * The reference stays valid until the matrix is next modified.
     *
     * This method is disabled via requires clause for a value type whose
     * element block does not provide references to its elements, such as a
     * block backed by std::vector<bool>.
     *
     * @param row Row position of the element.
     * @param col Column position of the element.
     *
     * @return Reference to the value of the element.
     */
    template<mtm::detail::referenceable_value_type<typename mtm::detail::block_list_of<Traits>::type> T>
    const T& get_ref(size_type row, size_type col) const;

    /**
     * Get a reference to the value of element from a position object.  The
     * caller must explicitly specify the value type.  Unlike get(), this
     * method throws a mdds::type_error exception when the element is empty,
     * since there is no stored value to reference.  It also throws when the
     * element is of a different type.
     *
     * The reference stays valid until the matrix is next modified.
     *
     * This method is disabled via requires clause for a value type whose
     * element block does not provide references to its elements, such as a
     * block backed by std::vector<bool>.
     *
     * @param pos Position object of an element.
     *
     * @return Reference to the value of the element.
     */
    template<mtm::detail::referenceable_value_type<typename mtm::detail::block_list_of<Traits>::type> T>
    const T& get_ref(const const_position_type& pos) const;

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
    void set(size_type row, size_type col, numeric_type val);

    /**
     * Set a numeric value to an element at specified position.
     *
     * @param pos position of the element to update.
     * @param val new value to set.
     *
     * @return position of the element block where the new value has been set.
     */
    position_type set(const position_type& pos, numeric_type val);

    /**
     * Set a value to an element at specified position.  The type of the
     * value must be the value type of one of the element blocks listed in
     * the <code>element_blocks</code> member of the matrix traits type.
     *
     * @param row Row index of the element.
     * @param col Column index of the element.
     * @param val New value to set.
     */
    template<mtm::detail::listed_value_type<typename mtm::detail::element_blocks_of<Traits>::type> T>
    void set(size_type row, size_type col, const T& val);

    /**
     * Set a value to an element at specified position.  The type of the
     * value must be the value type of one of the element blocks listed in
     * the <code>element_blocks</code> member of the matrix traits type.
     *
     * @param pos Position of the element to update.
     * @param val New value to set.
     *
     * @return Position of the element block where the new value has been set.
     */
    template<mtm::detail::listed_value_type<typename mtm::detail::element_blocks_of<Traits>::type> T>
    position_type set(const position_type& pos, const T& val);

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
     * only numeric elements.  An empty matrix is not numeric.
     *
     * @return True if the matrix contains only numeric elements, or false
     *         otherwise.
     */
    bool numeric() const;

    /**
     * Check whether or not every element block of this matrix is one of the
     * specified element block types.  An empty element never matches, so a
     * matrix that contains an empty element returns false.  An empty matrix
     * also returns false.
     *
     * Each specified block type must be an element block of this matrix.
     *
     * @return True if every element block is one of the specified block
     *         types, or false otherwise.
     */
    template<typename... Blks>
    bool all_of() const;

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
