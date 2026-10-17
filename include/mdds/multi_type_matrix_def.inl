// SPDX-FileCopyrightText: 2012 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

namespace mdds {

template<mtm::matrix_traits Traits>
multi_type_matrix<Traits>::element_block_node_type::element_block_node_type() noexcept(std::is_fundamental_v<size_type>)
    : type(mtm::element_empty), offset(0), size(0), data(nullptr)
{}

template<mtm::matrix_traits Traits>
multi_type_matrix<Traits>::element_block_node_type::element_block_node_type(
    const element_block_node_type& other) noexcept(std::is_fundamental_v<size_type>)
    : type(other.type), offset(other.offset), size(other.size), data(other.data)
{}

template<mtm::matrix_traits Traits>
void multi_type_matrix<Traits>::element_block_node_type::assign(const const_position_type& pos, size_type section_size)
{
    assert(section_size <= pos.first->size - pos.second);

    type = pos.first->type;
    offset = pos.second;
    size = section_size;
    data = pos.first->data;
}

template<mtm::matrix_traits Traits>
template<typename _Blk>
typename _Blk::const_iterator multi_type_matrix<Traits>::element_block_node_type::begin() const
{
    typename _Blk::const_iterator it = _Blk::begin(*data);
    std::advance(it, offset);
    return it;
}

template<mtm::matrix_traits Traits>
template<typename _Blk>
typename _Blk::const_iterator multi_type_matrix<Traits>::element_block_node_type::end() const
{
    typename _Blk::const_iterator it = _Blk::begin(*data);
    std::advance(it, offset + size);
    return it;
}

template<mtm::matrix_traits Traits>
typename multi_type_matrix<Traits>::position_type multi_type_matrix<Traits>::next_position(const position_type& pos)
{
    return store_type::next_position(pos);
}

template<mtm::matrix_traits Traits>
typename multi_type_matrix<Traits>::const_position_type multi_type_matrix<Traits>::next_position(
    const const_position_type& pos)
{
    return store_type::next_position(pos);
}

template<mtm::matrix_traits Traits>
multi_type_matrix<Traits>::multi_type_matrix() noexcept(nothrow_default_constructible_v)
{}

template<mtm::matrix_traits Traits>
multi_type_matrix<Traits>::multi_type_matrix(size_type rows, size_type cols) : m_store(rows * cols), m_size(rows, cols)
{}

template<mtm::matrix_traits Traits>
template<typename _T>
multi_type_matrix<Traits>::multi_type_matrix(size_type rows, size_type cols, const _T& value)
    : m_store(rows * cols, value), m_size(rows, cols)
{}

template<mtm::matrix_traits Traits>
template<typename _T>
multi_type_matrix<Traits>::multi_type_matrix(size_type rows, size_type cols, const _T& it_begin, const _T& it_end)
    : m_store(create_store(rows * cols, it_begin, it_end)), m_size(rows, cols)
{}

template<mtm::matrix_traits Traits>
multi_type_matrix<Traits>::multi_type_matrix(const multi_type_matrix& other)
    : m_store(other.m_store), m_size(other.m_size)
{}

template<mtm::matrix_traits Traits>
bool multi_type_matrix<Traits>::operator==(const multi_type_matrix& r) const noexcept(nothrow_eq_comparable_v)
{
    return m_size == r.m_size && m_store == r.m_store;
}

template<mtm::matrix_traits Traits>
typename multi_type_matrix<Traits>::position_type multi_type_matrix<Traits>::position(size_type row, size_type col)
{
    return m_store.position(get_pos(row, col));
}

template<mtm::matrix_traits Traits>
typename multi_type_matrix<Traits>::position_type multi_type_matrix<Traits>::position(
    const position_type& pos_hint, size_type row, size_type col)
{
    return m_store.position(pos_hint.first, get_pos(row, col));
}

template<mtm::matrix_traits Traits>
typename multi_type_matrix<Traits>::const_position_type multi_type_matrix<Traits>::position(
    size_type row, size_type col) const
{
    return m_store.position(get_pos(row, col));
}

template<mtm::matrix_traits Traits>
typename multi_type_matrix<Traits>::const_position_type multi_type_matrix<Traits>::position(
    const const_position_type& pos_hint, size_type row, size_type col) const
{
    return m_store.position(pos_hint.first, get_pos(row, col));
}

template<mtm::matrix_traits Traits>
typename multi_type_matrix<Traits>::size_pair_type multi_type_matrix<Traits>::matrix_position(
    const const_position_type& pos) const
{
    size_type mtv_pos = store_type::logical_position(pos);
    size_type col = mtv_pos / m_size.row;
    size_type row = mtv_pos - m_size.row * col;
    return size_pair_type(row, col);
}

template<mtm::matrix_traits Traits>
typename multi_type_matrix<Traits>::position_type multi_type_matrix<Traits>::end_position()
{
    return position_type(m_store.end(), 0);
}

template<mtm::matrix_traits Traits>
typename multi_type_matrix<Traits>::const_position_type multi_type_matrix<Traits>::end_position() const
{
    return const_position_type(m_store.end(), 0);
}

template<mtm::matrix_traits Traits>
mtm::element_t multi_type_matrix<Traits>::get_type(const const_position_type& pos) const
{
    return pos.first->type;
}

template<mtm::matrix_traits Traits>
mtm::element_t multi_type_matrix<Traits>::get_type(size_type row, size_type col) const
{
    return m_store.get_type(get_pos(row, col));
}

template<mtm::matrix_traits Traits>
typename multi_type_matrix<Traits>::numeric_type multi_type_matrix<Traits>::get_numeric(
    size_type row, size_type col) const
{
    return get_numeric(m_store.position(get_pos(row, col)));
}

template<mtm::matrix_traits Traits>
typename multi_type_matrix<Traits>::numeric_type multi_type_matrix<Traits>::get_numeric(
    const const_position_type& pos) const
{
    if (pos.first->type == element_numeric)
        return store_type::template get<numeric_block_type>(pos);

    if (pos.first->type == element_empty)
        return numeric_type(0);

    throw type_error("multi_type_matrix: element is not of numeric type.");
}

template<mtm::matrix_traits Traits>
template<typename T>
T multi_type_matrix<Traits>::get(size_type row, size_type col) const
{
    return get<T>(m_store.position(get_pos(row, col)));
}

template<mtm::matrix_traits Traits>
template<typename T>
T multi_type_matrix<Traits>::get(const const_position_type& pos) const
{
    // The element callbacks of the standard value types are in mdds::mtv.
    using mtv::mdds_mtv_get_empty_value;
    using mtv::mdds_mtv_get_value;

    T val{};

    if (pos.first->type == element_empty)
    {
        mdds_mtv_get_empty_value(val);
        return val;
    }

    if (pos.first->type != store_type::get_element_type(val))
        throw type_error("multi_type_matrix: element is not of the requested type.");

    mdds_mtv_get_value(*pos.first->data, pos.second, val);
    return val;
}

template<mtm::matrix_traits Traits>
template<mtm::detail::referenceable_value_type<typename mtm::detail::block_list_of<Traits>::type> T>
const T& multi_type_matrix<Traits>::get_ref(size_type row, size_type col) const
{
    return get_ref<T>(m_store.position(get_pos(row, col)));
}

template<mtm::matrix_traits Traits>
template<mtm::detail::referenceable_value_type<typename mtm::detail::block_list_of<Traits>::type> T>
const T& multi_type_matrix<Traits>::get_ref(const const_position_type& pos) const
{
    using block_type = typename mtm::detail::block_of<T, block_list_type>::type;

    if (pos.first->type != block_type::block_type)
        throw type_error("multi_type_matrix: element is empty or not of the requested type.");

    return block_type::at(*pos.first->data, pos.second);
}

template<mtm::matrix_traits Traits>
void multi_type_matrix<Traits>::set_empty(size_type row, size_type col)
{
    m_store.set_empty(get_pos(row, col), get_pos(row, col));
}

template<mtm::matrix_traits Traits>
void multi_type_matrix<Traits>::set_empty(size_type row, size_type col, size_type length)
{
    if (length == 0)
        throw general_error("multi_type_matrix::set_empty: length of zero is not permitted.");

    size_type pos1 = get_pos(row, col);
    m_store.set_empty(pos1, pos1 + length - 1);
}

template<mtm::matrix_traits Traits>
typename multi_type_matrix<Traits>::position_type multi_type_matrix<Traits>::set_empty(const position_type& pos)
{
    size_type store_pos = get_pos(pos);
    typename store_type::iterator it = m_store.set_empty(pos.first, store_pos, store_pos);
    return position_type(it, store_pos - it->position);
}

template<mtm::matrix_traits Traits>
void multi_type_matrix<Traits>::set_column_empty(size_type col)
{
    m_store.set_empty(get_pos(0, col), get_pos(m_size.row - 1, col));
}

template<mtm::matrix_traits Traits>
void multi_type_matrix<Traits>::set_row_empty(size_type row)
{
    for (size_type col = 0; col < m_size.column; ++col)
    {
        size_type pos = get_pos(row, col);
        m_store.set_empty(pos, pos);
    }
}

template<mtm::matrix_traits Traits>
void multi_type_matrix<Traits>::set(size_type row, size_type col, numeric_type val)
{
    m_store.set(get_pos(row, col), val);
}

template<mtm::matrix_traits Traits>
typename multi_type_matrix<Traits>::position_type multi_type_matrix<Traits>::set(
    const position_type& pos, numeric_type val)
{
    size_type store_pos = get_pos(pos);
    typename store_type::iterator it = m_store.set(pos.first, store_pos, val);
    return position_type(it, store_pos - it->position);
}

template<mtm::matrix_traits Traits>
template<mtm::detail::listed_value_type<typename mtm::detail::element_blocks_of<Traits>::type> T>
void multi_type_matrix<Traits>::set(size_type row, size_type col, const T& val)
{
    m_store.set(get_pos(row, col), val);
}

template<mtm::matrix_traits Traits>
template<mtm::detail::listed_value_type<typename mtm::detail::element_blocks_of<Traits>::type> T>
typename multi_type_matrix<Traits>::position_type multi_type_matrix<Traits>::set(const position_type& pos, const T& val)
{
    size_type store_pos = get_pos(pos);
    typename store_type::iterator it = m_store.set(pos.first, store_pos, val);
    return position_type(it, store_pos - it->position);
}

template<mtm::matrix_traits Traits>
template<typename _T>
void multi_type_matrix<Traits>::set(size_type row, size_type col, const _T& it_begin, const _T& it_end)
{
    m_store.set(get_pos(row, col), it_begin, it_end);
}

template<mtm::matrix_traits Traits>
template<typename _T>
typename multi_type_matrix<Traits>::position_type multi_type_matrix<Traits>::set(
    const position_type& pos, const _T& it_begin, const _T& it_end)
{
    size_type store_pos = get_pos(pos);
    typename store_type::iterator it = m_store.set(pos.first, store_pos, it_begin, it_end);
    return position_type(it, store_pos - it->position);
}

template<mtm::matrix_traits Traits>
template<typename _T>
void multi_type_matrix<Traits>::set_column(size_type col, const _T& it_begin, const _T& it_end)
{
    size_type pos = get_pos(0, col);
    size_type len = std::distance(it_begin, it_end);

    if (len <= m_size.row)
    {
        m_store.set(pos, it_begin, it_end);
        return;
    }

    _T it_end2 = it_begin;
    std::advance(it_end2, m_size.row);
    m_store.set(pos, it_begin, it_end2);
}

template<mtm::matrix_traits Traits>
typename multi_type_matrix<Traits>::size_pair_type multi_type_matrix<Traits>::size() const
{
    return m_size;
}

template<mtm::matrix_traits Traits>
multi_type_matrix<Traits>& multi_type_matrix<Traits>::transpose()
{
    multi_type_matrix tmp(m_size.column, m_size.row);

    for (auto it_blk = m_store.cbegin(); it_blk != m_store.cend(); ++it_blk)
    {
        if (it_blk->type == element_empty)
            continue;

        dispatch_block(it_blk->type, [&]<typename Blk>() {
            size_type store_pos = it_blk->position;
            auto it = Blk::cbegin(*it_blk->data);
            auto ite = Blk::cend(*it_blk->data);

            for (; it != ite; ++it, ++store_pos)
            {
                size_type old_col_new_row = store_pos / m_size.row;
                size_type old_row_new_col = store_pos - m_size.row * old_col_new_row;
                tmp.m_store.set(tmp.get_pos(old_col_new_row, old_row_new_col), *it);
            }
        });
    }

    swap(tmp);
    return *this;
}

template<mtm::matrix_traits Traits>
void multi_type_matrix<Traits>::copy(const multi_type_matrix& src)
{
    if (&src == this)
        // Self assignment.
        return;

    if (empty() || src.empty())
        return;

    size_type rows = std::min(m_size.row, src.m_size.row);
    size_type cols = std::min(m_size.column, src.m_size.column);

    position_type pos_dest = position(0, 0);
    const_position_type pos_src = src.position(0, 0);

    element_block_node_type src_node;

    for (size_t col = 0; col < cols; ++col)
    {
        pos_dest = position(pos_dest, 0, col);
        pos_src = src.position(pos_src, 0, col);

        size_t remaining_rows = rows;

        do
        {
            size_type src_blk_left = pos_src.first->size - pos_src.second;
            size_type section_size = std::min(src_blk_left, remaining_rows);
            src_node.assign(pos_src, section_size);

            size_type logical_pos_dest = store_type::logical_position(pos_dest);

            typename store_type::iterator blk_pos;

            if (pos_src.first->type == element_empty)
            {
                size_type end = logical_pos_dest + section_size - 1;
                blk_pos = m_store.set_empty(pos_dest.first, logical_pos_dest, end);
            }
            else
            {
                dispatch_block(pos_src.first->type, [&]<typename Blk>() {
                    auto it = src_node.template begin<Blk>();
                    auto ite = src_node.template end<Blk>();
                    blk_pos = m_store.set(pos_dest.first, logical_pos_dest, it, ite);
                });
            }

            remaining_rows -= section_size;

            size_type logical_pos_next = logical_pos_dest + section_size;
            if (logical_pos_next >= m_store.size())
                // No more room left in the destination store.  Bail out.
                return;

            pos_dest = m_store.position(blk_pos, logical_pos_next);

            // Move source to the head of the next block in the column.
            pos_src = const_position_type(++pos_src.first, 0);
        } while (remaining_rows);
    }
}

template<mtm::matrix_traits Traits>
template<typename _T>
void multi_type_matrix<Traits>::copy(size_type rows, size_type cols, const _T& it_begin, const _T& it_end)
{
    size_t n = std::distance(it_begin, it_end);
    if (!n || empty())
        return;

    if (n != rows * cols)
        throw size_error("multi_type_matrix: size of the array does not match the destination size.");

    if (rows > m_size.row || cols > m_size.column)
        throw size_error("multi_type_matrix: specified destination size is larger than the current matrix.");

    // Ensure that the passed array is supported by this matrix.
    validate_type(store_type::get_element_type(*it_begin));

    auto it = it_begin;
    position_type pos_dest = position(0, 0);

    for (size_t col = 0; col < cols; ++col)
    {
        pos_dest = position(pos_dest, 0, col);

        auto it_this_end = it;
        std::advance(it_this_end, rows);

        pos_dest.first = m_store.set(pos_dest.first, get_pos(0, col), it, it_this_end);
        it = it_this_end;
    }
}

template<mtm::matrix_traits Traits>
void multi_type_matrix<Traits>::resize(size_type rows, size_type cols)
{
    if (!rows || !cols)
    {
        m_size.row = 0;
        m_size.column = 0;
        m_store.clear();
        return;
    }

    multi_type_matrix temp(rows, cols);
    temp.copy(*this);
    temp.swap(*this);
}

template<mtm::matrix_traits Traits>
template<typename _T>
void multi_type_matrix<Traits>::resize(size_type rows, size_type cols, const _T& value)
{
    if (!rows || !cols)
    {
        m_size.row = 0;
        m_size.column = 0;
        m_store.clear();
        return;
    }

    multi_type_matrix temp(rows, cols, value);
    temp.copy(*this);
    temp.swap(*this);
}

template<mtm::matrix_traits Traits>
void multi_type_matrix<Traits>::clear()
{
    m_store.clear();
    m_size.row = 0;
    m_size.column = 0;
}

template<mtm::matrix_traits Traits>
bool multi_type_matrix<Traits>::numeric() const
{
    return all_of<numeric_block_type>();
}

template<mtm::matrix_traits Traits>
template<typename... Blks>
bool multi_type_matrix<Traits>::all_of() const
{
    static_assert(
        (mtm::detail::is_block_in_list<Blks, block_list_type>::value && ...),
        "Every block type must be an element block of this matrix.");

    if (m_store.empty())
        return false;

    for (auto it = m_store.cbegin(); it != m_store.cend(); ++it)
    {
        bool matched = ((it->type == Blks::block_type) || ...);
        if (!matched)
            return false;
    }

    return true;
}

template<mtm::matrix_traits Traits>
bool multi_type_matrix<Traits>::empty() const
{
    return m_store.empty();
}

template<mtm::matrix_traits Traits>
void multi_type_matrix<Traits>::swap(multi_type_matrix& r)
{
    m_store.swap(r.m_store);
    std::swap(m_size.row, r.m_size.row);
    std::swap(m_size.column, r.m_size.column);
}

template<mtm::matrix_traits Traits>
template<typename FuncT>
FuncT multi_type_matrix<Traits>::walk(FuncT func) const
{
    walk_func<FuncT> wf(func);
    std::for_each(m_store.begin(), m_store.end(), wf);
    return func;
}

template<mtm::matrix_traits Traits>
template<typename FuncT>
FuncT multi_type_matrix<Traits>::walk(FuncT func, const size_pair_type& start, const size_pair_type& end) const
{
    if (end.row < start.row || end.column < start.column)
    {
        std::ostringstream os;
        os << "multi_type_matrix: invalid start/end position pair: (row=" << start.row << "; column=" << start.column
           << ") - (row=" << end.row << "; column=" << end.column << ")";
        throw size_error(os.str());
    }

    if (end.row > m_size.row || end.column > m_size.column)
        throw size_error("multi_type_matrix: end position is out-of-bound.");

    size_t rows = end.row - start.row + 1;
    element_block_node_type mtm_node;
    const_position_type pos = position(0, 0);

    // we need to handle columns manually, as the columns are continuously in memory.
    // To go from one column to the next we need to jump in the memory.
    for (size_t col = start.column; col <= end.column; ++col)
    {
        pos = position(pos, start.row, col);
        size_t remaining_rows = rows;

        do
        {
            size_type remaining_blk = pos.first->size - pos.second;

            // handle the two possible cases:
            // 1.) the current block is completely contained in our selection
            // 2.) the current block contains the end of the selection

            size_type section_size = std::min(remaining_blk, remaining_rows);
            mtm_node.assign(pos, section_size);

            remaining_rows -= section_size;
            func(mtm_node);

            // Move to the head of the next block in the column.
            pos = const_position_type(++pos.first, 0);
        } while (remaining_rows != 0);
    }

    return func;
}

template<mtm::matrix_traits Traits>
template<typename FuncT>
FuncT multi_type_matrix<Traits>::walk(FuncT func, const multi_type_matrix& right) const
{
    if (size() != right.size())
        throw size_error("multi_type_matrix: left and right matrices must have the same geometry.");

    if (m_store.empty())
        return func;

    size_t remaining_size = m_store.size();

    typename store_type::const_iterator it1 = m_store.begin();
    typename store_type::const_iterator it2 = right.m_store.begin();
    const_position_type pos1(it1, 0), pos2(it2, 0);
    element_block_node_type node1, node2;

    while (remaining_size)
    {
        size_t section_size = std::min(pos1.first->size - pos1.second, pos2.first->size - pos2.second);

        node1.assign(pos1, section_size);
        node2.assign(pos2, section_size);

        func(node1, node2);

        pos1 = store_type::advance_position(pos1, section_size);
        pos2 = store_type::advance_position(pos2, section_size);

        remaining_size -= section_size;
    }

    return func;
}

template<mtm::matrix_traits Traits>
template<typename FuncT>
FuncT multi_type_matrix<Traits>::walk(
    FuncT func, const multi_type_matrix& right, const size_pair_type& start, const size_pair_type& end) const
{
    if (end.row < start.row || end.column < start.column)
    {
        std::ostringstream os;
        os << "multi_type_matrix: invalid start/end position pair: (row=" << start.row << "; column=" << start.column
           << ") - (row=" << end.row << "; column=" << end.column << ")";
        throw size_error(os.str());
    }

    if (end.row > m_size.row || end.column > m_size.column || end.row > right.size().row ||
        end.column > right.size().column)
        throw size_error("multi_type_matrix: end position is out-of-bound.");

    size_t rows = end.row - start.row + 1;

    element_block_node_type node1, node2;
    const_position_type pos1 = position(0, 0), pos2 = right.position(0, 0);

    for (size_t col = start.column; col <= end.column; ++col)
    {
        pos1 = position(pos1, start.row, col);
        pos2 = right.position(pos2, start.row, col);

        size_t remaining_rows = rows;

        do
        {
            size_type blk1_left = pos1.first->size - pos1.second;
            size_type blk2_left = pos2.first->size - pos2.second;

            // Section size should be the smallest of blk1_left, blk2_left and remaining_rows.
            size_type section_size = std::min(blk1_left, blk2_left);
            section_size = std::min(section_size, remaining_rows);

            node1.assign(pos1, section_size);
            node2.assign(pos2, section_size);

            func(node1, node2);

            pos1 = store_type::advance_position(pos1, section_size);
            pos2 = store_type::advance_position(pos2, section_size);

            remaining_rows -= section_size;
        } while (remaining_rows);
    }

    return func;
}

template<mtm::matrix_traits Traits>
void multi_type_matrix<Traits>::validate_type(mtm::element_t type)
{
    auto is_listed = [type]<typename... Blks>(std::tuple<Blks...>*) { return ((Blks::block_type == type) || ...); };

    if (!is_listed(static_cast<block_list_type*>(nullptr)))
        throw type_error("multi_type_matrix: unknown element type.");
}

template<mtm::matrix_traits Traits>
template<typename FuncT>
void multi_type_matrix<Traits>::dispatch_block(mtm::element_t type, FuncT&& func)
{
    auto call_matching = [type, &func]<typename... Blks>(std::tuple<Blks...>*) {
        return ((Blks::block_type == type ? (func.template operator()<Blks>(), true) : false) || ...);
    };

    if (!call_matching(static_cast<block_list_type*>(nullptr)))
        throw type_error("multi_type_matrix: unknown element type.");
}

template<mtm::matrix_traits Traits>
template<typename T>
typename multi_type_matrix<Traits>::store_type multi_type_matrix<Traits>::create_store(
    size_type init_size, const T& it_begin, const T& it_end)
{
    if (it_begin != it_end)
        validate_type(store_type::get_element_type(*it_begin));

    return store_type(init_size, it_begin, it_end);
}

} // namespace mdds
