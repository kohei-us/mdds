/*************************************************************************
 *
 * Copyright (c) 2012-2018 Kohei Yoshida
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

template<typename _MtxTrait>
multi_type_matrix<_MtxTrait>::element_block_node_type::element_block_node_type() :
    type(mtm::element_empty), offset(0), size(0), data(nullptr) {}

template<typename _MtxTrait>
multi_type_matrix<_MtxTrait>::element_block_node_type::element_block_node_type(const element_block_node_type& other) :
    type(other.type), offset(other.offset), size(other.size), data(other.data) {}

template<typename _MtxTrait>
void multi_type_matrix<_MtxTrait>::element_block_node_type::assign(
    const const_position_type& pos, size_type section_size)
{
    assert(section_size <= pos.first->size - pos.second);

    type = to_mtm_type(pos.first->type);
    offset = pos.second;
    size = section_size;
    data = pos.first->data;
}

template<typename _MtxTrait>
template<typename _Blk>
typename _Blk::const_iterator
multi_type_matrix<_MtxTrait>::element_block_node_type::begin() const
{
    typename _Blk::const_iterator it = _Blk::begin(*data);
    std::advance(it, offset);
    return it;
}

template<typename _MtxTrait>
template<typename _Blk>
typename _Blk::const_iterator
multi_type_matrix<_MtxTrait>::element_block_node_type::end() const
{
    typename _Blk::const_iterator it = _Blk::begin(*data);
    std::advance(it, offset+size);
    return it;
}

template<typename _MtxTrait>
typename multi_type_matrix<_MtxTrait>::position_type
multi_type_matrix<_MtxTrait>::next_position(const position_type& pos)
{
    return store_type::next_position(pos);
}

template<typename _MtxTrait>
typename multi_type_matrix<_MtxTrait>::const_position_type
multi_type_matrix<_MtxTrait>::next_position(const const_position_type& pos)
{
    return store_type::next_position(pos);
}

template<typename _MtxTrait>
multi_type_matrix<_MtxTrait>::multi_type_matrix() : m_size(0, 0) {}

template<typename _MtxTrait>
multi_type_matrix<_MtxTrait>::multi_type_matrix(size_type rows, size_type cols) :
    m_store(rows*cols), m_size(rows, cols) {}

template<typename _MtxTrait>
template<typename _T>
multi_type_matrix<_MtxTrait>::multi_type_matrix(size_type rows, size_type cols, const _T& value) :
    m_store(rows*cols, value), m_size(rows, cols) {}

template<typename _MtxTrait>
template<typename _T>
multi_type_matrix<_MtxTrait>::multi_type_matrix(
    size_type rows, size_type cols, const _T& it_begin, const _T& it_end) :
    m_store(rows*cols, it_begin, it_end), m_size(rows, cols)
{
    if (m_store.empty())
        return;

    // Throw an exception when trying to construct with data that the matrix doesn't support.
    typename store_type::iterator it = m_store.begin();
    to_mtm_type(it->type);
}

template<typename _MtxTrait>
multi_type_matrix<_MtxTrait>::multi_type_matrix(const multi_type_matrix& r) :
    m_store(r.m_store), m_size(r.m_size) {}

template<typename _MtxTrait>
multi_type_matrix<_MtxTrait>::~multi_type_matrix() {}

template<typename _MtxTrait>
bool multi_type_matrix<_MtxTrait>::operator== (const multi_type_matrix& r) const
{
    return m_size == r.m_size && m_store == r.m_store;
}

template<typename _MtxTrait>
bool multi_type_matrix<_MtxTrait>::operator!= (const multi_type_matrix& r) const
{
    return !operator== (r);
}

template<typename _MtxTrait>
multi_type_matrix<_MtxTrait>&
multi_type_matrix<_MtxTrait>::operator= (const multi_type_matrix& r)
{
    if (this == &r)
        return *this;

    store_type tmp(r.m_store);
    m_store.swap(tmp);
    m_size = r.m_size;
    return *this;
}

template<typename _MtxTrait>
typename multi_type_matrix<_MtxTrait>::position_type
multi_type_matrix<_MtxTrait>::position(size_type row, size_type col)
{
    return m_store.position(get_pos(row,col));
}

template<typename _MtxTrait>
typename multi_type_matrix<_MtxTrait>::position_type
multi_type_matrix<_MtxTrait>::position(const position_type& pos_hint, size_type row, size_type col)
{
    return m_store.position(pos_hint.first, get_pos(row,col));
}

template<typename _MtxTrait>
typename multi_type_matrix<_MtxTrait>::const_position_type
multi_type_matrix<_MtxTrait>::position(size_type row, size_type col) const
{
    return m_store.position(get_pos(row,col));
}

template<typename _MtxTrait>
typename multi_type_matrix<_MtxTrait>::const_position_type
multi_type_matrix<_MtxTrait>::position(const const_position_type& pos_hint, size_type row, size_type col) const
{
    return m_store.position(pos_hint.first, get_pos(row,col));
}

template<typename _MtxTrait>
typename multi_type_matrix<_MtxTrait>::size_pair_type
multi_type_matrix<_MtxTrait>::matrix_position(const const_position_type& pos) const
{
    size_type mtv_pos = store_type::logical_position(pos);
    size_type col = mtv_pos / m_size.row;
    size_type row = mtv_pos - m_size.row * col;
    return size_pair_type(row, col);
}

template<typename _MtxTrait>
typename multi_type_matrix<_MtxTrait>::position_type
multi_type_matrix<_MtxTrait>::end_position()
{
    return position_type(m_store.end(), 0);
}

template<typename _MtxTrait>
typename multi_type_matrix<_MtxTrait>::const_position_type
multi_type_matrix<_MtxTrait>::end_position() const
{
    return const_position_type(m_store.end(), 0);
}

template<typename _MtxTrait>
mtm::element_t
multi_type_matrix<_MtxTrait>::get_type(const const_position_type& pos) const
{
    return to_mtm_type(pos.first->type);
}

template<typename _MtxTrait>
mtm::element_t
multi_type_matrix<_MtxTrait>::get_type(size_type row, size_type col) const
{
    return to_mtm_type(m_store.get_type(get_pos(row,col)));
}

template<typename _MtxTrait>
double multi_type_matrix<_MtxTrait>::get_numeric(size_type row, size_type col) const
{
    return get_numeric(m_store.position(get_pos(row,col)));
}

template<typename _MtxTrait>
double multi_type_matrix<_MtxTrait>::get_numeric(const const_position_type& pos) const
{
    switch (pos.first->type)
    {
        case mtv::element_type_double:
            return mtv::double_element_block::at(*pos.first->data, pos.second);
        case integer_block_type::block_type:
            return integer_block_type::at(*pos.first->data, pos.second);
        case mtv::element_type_boolean:
        {
            // vector<bool> cannot return reference i.e. we can't use at() here.
            typename mtv::boolean_element_block::const_iterator it =
                mtv::boolean_element_block::begin(*pos.first->data);
            std::advance(it, pos.second);
            return *it;
        }
        case string_block_type::block_type:
        case mtv::element_type_empty:
            return 0.0;
        default:
            throw general_error("multi_type_matrix: unknown element type.");
    }
}

template<typename _MtxTrait>
typename multi_type_matrix<_MtxTrait>::integer_type
multi_type_matrix<_MtxTrait>::get_integer(size_type row, size_type col) const
{
    return get_integer(m_store.position(get_pos(row,col)));
}

template<typename _MtxTrait>
typename multi_type_matrix<_MtxTrait>::integer_type
multi_type_matrix<_MtxTrait>::get_integer(const const_position_type& pos) const
{
    return static_cast<integer_type>(get_numeric(pos));
}

template<typename _MtxTrait>
bool multi_type_matrix<_MtxTrait>::get_boolean(size_type row, size_type col) const
{
    return static_cast<bool>(get_numeric(row, col));
}

template<typename _MtxTrait>
bool multi_type_matrix<_MtxTrait>::get_boolean(const const_position_type& pos) const
{
    return static_cast<bool>(get_numeric(pos));
}

template<typename _MtxTrait>
const typename multi_type_matrix<_MtxTrait>::string_type&
multi_type_matrix<_MtxTrait>::get_string(size_type row, size_type col) const
{
    return get_string(m_store.position(get_pos(row,col)));
}

template<typename _MtxTrait>
const typename multi_type_matrix<_MtxTrait>::string_type&
multi_type_matrix<_MtxTrait>::get_string(const const_position_type& pos) const
{
    if (pos.first->type != string_block_type::block_type)
        throw general_error("multi_type_matrix: unknown element type.");

    return string_block_type::at(*pos.first->data, pos.second);
}

template<typename _MtxTrait>
template<typename _T>
_T
multi_type_matrix<_MtxTrait>::get(size_type row, size_type col) const
{
    _T val;
    m_store.get(get_pos(row,col), val);
    return val;
}

template<typename _MtxTrait>
void multi_type_matrix<_MtxTrait>::set_empty(size_type row, size_type col)
{
    m_store.set_empty(get_pos(row, col), get_pos(row, col));
}

template<typename _MtxTrait>
void multi_type_matrix<_MtxTrait>::set_empty(size_type row, size_type col, size_type length)
{
    if (length == 0)
        throw general_error("multi_type_matrix::set_empty: length of zero is not permitted.");

    size_type pos1 = get_pos(row, col);
    m_store.set_empty(pos1, pos1+length-1);
}

template<typename _MtxTrait>
typename multi_type_matrix<_MtxTrait>::position_type
multi_type_matrix<_MtxTrait>::set_empty(const position_type& pos)
{
    size_type store_pos = get_pos(pos);
    typename store_type::iterator it = m_store.set_empty(pos.first, store_pos, store_pos);
    return position_type(it, store_pos - it->position);
}

template<typename _MtxTrait>
void multi_type_matrix<_MtxTrait>::set_column_empty(size_type col)
{
    m_store.set_empty(get_pos(0, col), get_pos(m_size.row-1, col));
}

template<typename _MtxTrait>
void multi_type_matrix<_MtxTrait>::set_row_empty(size_type row)
{
    for (size_type col = 0; col < m_size.column; ++col)
    {
        size_type pos = get_pos(row, col);
        m_store.set_empty(pos, pos);
    }
}

template<typename _MtxTrait>
void multi_type_matrix<_MtxTrait>::set(size_type row, size_type col, double val)
{
    m_store.set(get_pos(row,col), val);
}

template<typename _MtxTrait>
typename multi_type_matrix<_MtxTrait>::position_type
multi_type_matrix<_MtxTrait>::set(const position_type& pos, double val)
{
    size_type store_pos = get_pos(pos);
    typename store_type::iterator it = m_store.set(pos.first, store_pos, val);
    return position_type(it, store_pos - it->position);
}

template<typename _MtxTrait>
void multi_type_matrix<_MtxTrait>::set(size_type row, size_type col, bool val)
{
    m_store.set(get_pos(row,col), val);
}

template<typename _MtxTrait>
typename multi_type_matrix<_MtxTrait>::position_type
multi_type_matrix<_MtxTrait>::set(const position_type& pos, bool val)
{
    size_type store_pos = get_pos(pos);
    typename store_type::iterator it = m_store.set(pos.first, store_pos, val);
    return position_type(it, store_pos - it->position);
}

template<typename _MtxTrait>
void multi_type_matrix<_MtxTrait>::set(size_type row, size_type col, const string_type& str)
{
    m_store.set(get_pos(row,col), str);
}

template<typename _MtxTrait>
typename multi_type_matrix<_MtxTrait>::position_type
multi_type_matrix<_MtxTrait>::set(const position_type& pos, const string_type& str)
{
    size_type store_pos = get_pos(pos);
    typename store_type::iterator it = m_store.set(pos.first, store_pos, str);
    return position_type(it, store_pos - it->position);
}

template<typename _MtxTrait>
void multi_type_matrix<_MtxTrait>::set(size_type row, size_type col, integer_type val)
{
    m_store.set(get_pos(row,col), val);
}

template<typename _MtxTrait>
typename multi_type_matrix<_MtxTrait>::position_type
multi_type_matrix<_MtxTrait>::set(const position_type& pos, integer_type val)
{
    size_type store_pos = get_pos(pos);
    typename store_type::iterator it = m_store.set(pos.first, store_pos, val);
    return position_type(it, store_pos - it->position);
}

template<typename _MtxTrait>
template<typename _T>
void multi_type_matrix<_MtxTrait>::set(size_type row, size_type col, const _T& it_begin, const _T& it_end)
{
    m_store.set(get_pos(row,col), it_begin, it_end);
}

template<typename _MtxTrait>
template<typename _T>
typename multi_type_matrix<_MtxTrait>::position_type
multi_type_matrix<_MtxTrait>::set(const position_type& pos, const _T& it_begin, const _T& it_end)
{
    size_type store_pos = get_pos(pos);
    typename store_type::iterator it = m_store.set(pos.first, store_pos, it_begin, it_end);
    return position_type(it, store_pos - it->position);
}

template<typename _MtxTrait>
template<typename _T>
void multi_type_matrix<_MtxTrait>::set_column(size_type col, const _T& it_begin, const _T& it_end)
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

template<typename _MtxTrait>
typename multi_type_matrix<_MtxTrait>::size_pair_type
multi_type_matrix<_MtxTrait>::size() const
{
    return m_size;
}

template<typename _MtxTrait>
multi_type_matrix<_MtxTrait>&
multi_type_matrix<_MtxTrait>::transpose()
{
    multi_type_matrix tmp(m_size.column, m_size.row);
    for (size_type old_row_new_col = 0; old_row_new_col < m_size.row; ++old_row_new_col)
    {
        for (size_type old_col_new_row = 0; old_col_new_row < m_size.column; ++old_col_new_row)
        {
            switch (get_type(old_row_new_col,old_col_new_row))
            {
                case mtm::element_numeric:
                {
                    double val;
                    m_store.get(get_pos(old_row_new_col,old_col_new_row), val);
                    tmp.set(old_col_new_row, old_row_new_col, val);
                }
                break;
                case mtm::element_boolean:
                {
                    bool val;
                    m_store.get(get_pos(old_row_new_col,old_col_new_row), val);
                    tmp.set(old_col_new_row, old_row_new_col, val);
                }
                break;
                case mtm::element_string:
                {
                    string_type val;
                    m_store.get(get_pos(old_row_new_col,old_col_new_row), val);
                    tmp.set(old_col_new_row, old_row_new_col, val);
                }
                break;
                case mtm::element_empty:
                break;
                default:
                    throw general_error("multi_type_matrix: unknown element type.");
            }
        }
    }

    swap(tmp);
    return *this;
}

template<typename _MtxTrait>
void multi_type_matrix<_MtxTrait>::copy(const multi_type_matrix& src)
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

            switch (to_mtm_type(pos_src.first->type))
            {
                case mtm::element_numeric:
                {
                    auto it = src_node.template begin<numeric_block_type>();
                    auto ite = src_node.template end<numeric_block_type>();

                    blk_pos = m_store.set(pos_dest.first, logical_pos_dest, it, ite);
                }
                break;
                case mtm::element_boolean:
                {
                    auto it = src_node.template begin<boolean_block_type>();
                    auto ite = src_node.template end<boolean_block_type>();

                    blk_pos = m_store.set(pos_dest.first, logical_pos_dest, it, ite);
                }
                break;
                case mtm::element_string:
                {
                    auto it = src_node.template begin<string_block_type>();
                    auto ite = src_node.template end<string_block_type>();

                    blk_pos = m_store.set(pos_dest.first, logical_pos_dest, it, ite);
                }
                break;
                case mtm::element_empty:
                {
                    size_type end = logical_pos_dest + section_size - 1;
                    blk_pos = m_store.set_empty(pos_dest.first, logical_pos_dest, end);
                }
                break;
                default:
                    throw general_error("multi_type_matrix: unknown element type.");
            }

            remaining_rows -= section_size;

            size_type logical_pos_next = logical_pos_dest + section_size;
            if (logical_pos_next >= m_store.size())
                // No more room left in the destination store.  Bail out.
                return;

            pos_dest = m_store.position(blk_pos, logical_pos_next);

            // Move source to the head of the next block in the column.
            pos_src = const_position_type(++pos_src.first, 0);
        }
        while (remaining_rows);
    }
}

template<typename _MtxTrait>
template<typename _T>
void multi_type_matrix<_MtxTrait>::copy(
    size_type rows, size_type cols, const _T& it_begin, const _T& it_end)
{
    size_t n = std::distance(it_begin, it_end);
    if (!n || empty())
        return;

    if (n != rows*cols)
        throw size_error(
            "multi_type_matrix: size of the array does not match the destination size.");

    if (rows > m_size.row || cols > m_size.column)
        throw size_error(
            "multi_type_matrix: specified destination size is larger than the current matrix.");

    // Ensure that the passed array is supported by this matrix.
    to_mtm_type(mdds_mtv_get_element_type(*it_begin));

    auto it = it_begin;
    position_type pos_dest = position(0, 0);

    for (size_t col = 0; col < cols; ++col)
    {
        pos_dest = position(pos_dest, 0, col);

        auto it_this_end = it;
        std::advance(it_this_end, rows);

        pos_dest.first = m_store.set(pos_dest.first, get_pos(0,col), it, it_this_end);
        it = it_this_end;
    }
}

template<typename _MtxTrait>
void multi_type_matrix<_MtxTrait>::resize(size_type rows, size_type cols)
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

template<typename _MtxTrait>
template<typename _T>
void multi_type_matrix<_MtxTrait>::resize(size_type rows, size_type cols, const _T& value)
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

template<typename _MtxTrait>
void multi_type_matrix<_MtxTrait>::clear()
{
    m_store.clear();
    m_size.row = 0;
    m_size.column = 0;
}

template<typename _MtxTrait>
bool multi_type_matrix<_MtxTrait>::numeric() const
{
    if (m_store.empty())
        return false;

    typename store_type::const_iterator i = m_store.begin(), iend = m_store.end();
    for (; i != iend; ++i)
    {
        mtv::element_t mtv_type = i->type;
        switch (mtv_type)
        {
            case mtv::element_type_double:
            case mtv::element_type_boolean:
            case integer_block_type::block_type:
                // These are numeric types.
                continue;
            case string_block_type::block_type:
            case mtv::element_type_empty:
                // These are not.
                return false;
            default:
                throw general_error("multi_type_matrix: unknown element type.");
        }
    }

    return true;
}

template<typename _MtxTrait>
bool multi_type_matrix<_MtxTrait>::empty() const
{
    return m_store.empty();
}

template<typename _MtxTrait>
void multi_type_matrix<_MtxTrait>::swap(multi_type_matrix& r)
{
    m_store.swap(r.m_store);
    std::swap(m_size.row, r.m_size.row);
    std::swap(m_size.column, r.m_size.column);
}

template<typename _MtxTrait>
template<typename _Func>
_Func multi_type_matrix<_MtxTrait>::walk(_Func func) const
{
    walk_func<_Func> wf(func);
    std::for_each(m_store.begin(), m_store.end(), wf);
    return func;
}

template<typename _MtxTrait>
template<typename _Func>
_Func multi_type_matrix<_MtxTrait>::walk(
    _Func func, const size_pair_type& start, const size_pair_type& end) const
{
    if (end.row < start.row || end.column < start.column)
    {
        std::ostringstream os;
        os << "multi_type_matrix: invalid start/end position pair: (row="
            << start.row << "; column=" << start.column << ") - (row=" << end.row << "; column=" << end.column << ")";
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
        }
        while (remaining_rows != 0);
    }

    return func;
}

template<typename _MtxTrait>
template<typename _Func>
_Func multi_type_matrix<_MtxTrait>::walk(_Func func, const multi_type_matrix& right) const
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
        size_t section_size = std::min(
            pos1.first->size - pos1.second,
            pos2.first->size - pos2.second);

        node1.assign(pos1, section_size);
        node2.assign(pos2, section_size);

        func(node1, node2);

        pos1 = store_type::advance_position(pos1, section_size);
        pos2 = store_type::advance_position(pos2, section_size);

        remaining_size -= section_size;
    }

    return func;
}

template<typename _MtxTrait>
template<typename _Func>
_Func multi_type_matrix<_MtxTrait>::walk(
    _Func func, const multi_type_matrix& right,
    const size_pair_type& start, const size_pair_type& end) const
{
    if (end.row < start.row || end.column < start.column)
    {
        std::ostringstream os;
        os << "multi_type_matrix: invalid start/end position pair: (row="
            << start.row << "; column=" << start.column << ") - (row=" << end.row << "; column=" << end.column << ")";
        throw size_error(os.str());
    }

    if (end.row > m_size.row || end.column > m_size.column ||
        end.row > right.size().row || end.column > right.size().column)
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
        }
        while (remaining_rows);
    }

    return func;
}

}
