/*************************************************************************
 *
 * Copyright (c) 2016-2018 Kohei Yoshida
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

#include <sstream>

namespace mdds { namespace mtv {

namespace detail {

template<typename _MtvT>
side_iterator<_MtvT>::side_iterator() :
    m_elem_pos(0), m_elem_pos_end(0), m_index_offset(0), m_identity(0) {}

template<typename _MtvT>
side_iterator<_MtvT>::side_iterator(
    std::vector<mtv_item>&& vectors, size_type elem_pos, size_type elem_size,
    size_type index_offset, uintptr_t identity, begin_state_type) :
    m_vectors(std::move(vectors)),
    m_elem_pos(elem_pos),
    m_elem_pos_end(elem_pos+elem_size),
    m_index_offset(index_offset),
    m_identity(identity)
{
    m_cur_node.index = index_offset;

    if (m_vectors.empty())
        return;

    mtv_item& col1 = m_vectors.front();

    m_cur_node.__position = col1.vector->position(col1.block_pos, m_elem_pos);
    col1.block_pos = m_cur_node.__position.first;
    m_cur_node.type = col1.block_pos->type;
    m_cur_node.position = m_elem_pos;
}

template<typename _MtvT>
side_iterator<_MtvT>::side_iterator(
    std::vector<mtv_item>&& vectors, size_type elem_pos, size_type elem_size,
    size_type index_offset, uintptr_t identity, end_state_type) :
    m_vectors(std::move(vectors)),
    m_elem_pos(elem_pos),
    m_elem_pos_end(elem_pos+elem_size),
    m_index_offset(index_offset),
    m_identity(identity)
{
    m_elem_pos = m_elem_pos_end;
    m_cur_node.index = index_offset;

    // We can leave the position and type uninitialized since this is an end
    // position which doesn't reference an actual element.
}

template<typename _MtvT>
side_iterator<_MtvT>& side_iterator<_MtvT>::operator++()
{
    ++m_cur_node.index;
    size_type pos = m_cur_node.index - m_index_offset;
    if (pos >= m_vectors.size())
    {
        // Move to the next element position.
        m_cur_node.index = m_index_offset;
        ++m_elem_pos;
        if (m_elem_pos >= m_elem_pos_end)
            // End position has been reached.  Don't update the current node.
            return *this;
    }

    pos = m_cur_node.index - m_index_offset;
    // Get the current vector.
    assert(pos < m_vectors.size());
    mtv_item& col = m_vectors[pos];

    // Update the current node.
    m_cur_node.__position = col.vector->position(col.block_pos, m_elem_pos);
    m_cur_node.position = m_elem_pos;
    col.block_pos = m_cur_node.__position.first;
    m_cur_node.type = col.block_pos->type;

    return *this;
}

template<typename _MtvT>
side_iterator<_MtvT> side_iterator<_MtvT>::operator++(int)
{
    side_iterator tmp(*this);
    operator++();
    return tmp;
}

template<typename _MtvT>
bool side_iterator<_MtvT>::operator== (const side_iterator& other) const
{
    if (m_identity != other.m_identity)
        return false;

    if (m_elem_pos_end != other.m_elem_pos_end)
        return false;

    if (m_elem_pos != other.m_elem_pos || m_elem_pos_end != other.m_elem_pos_end)
        return false;

    return m_cur_node.index == other.m_cur_node.index;
}

template<typename _MtvT>
bool side_iterator<_MtvT>::operator!= (const side_iterator& other) const
{
    return !operator==(other);
}

}

template<typename _MtvT>
collection<_MtvT>::collection() :
    m_mtv_size(0), m_identity(0)
{}

template<typename _MtvT>
template<typename _T>
collection<_MtvT>::collection(const _T& begin, const _T& end) :
    m_mtv_size(0), m_identity(0)
{
    size_type n = std::distance(begin, end);
    m_vectors.reserve(n);

    for (_T it = begin; it != end; ++it)
        init_insert_vector(*it);

    // Create a single value that identifies the whole collection.
    auto it = m_vectors.begin();
    uintptr_t identity = reinterpret_cast<uintptr_t>(*it);
    ++it;
    std::for_each(it, m_vectors.end(),
        [&](const mtv_type* p0)
        {
            uintptr_t p = reinterpret_cast<uintptr_t>(p0);
            identity = identity << 1;
            identity ^= p;
        }
    );
    m_identity = identity;

    assert(m_mtv_size); // This should have been set by this point.
    m_elem_range.start = 0;
    m_elem_range.size = m_mtv_size;

    m_col_range.start = 0;
    m_col_range.size = n;
}

template<typename _MtvT>
typename collection<_MtvT>::const_iterator
collection<_MtvT>::begin() const
{
    return const_iterator(
        build_iterator_state(), m_elem_range.start, m_elem_range.size,
        m_col_range.start, m_identity, const_iterator::begin_state);
}

template<typename _MtvT>
typename collection<_MtvT>::const_iterator
collection<_MtvT>::end() const
{
    return const_iterator(
        build_iterator_state(), m_elem_range.start, m_elem_range.size,
        m_col_range.start, m_identity, const_iterator::end_state);
}

template<typename _MtvT>
typename collection<_MtvT>::size_type
collection<_MtvT>::size() const
{
    return m_mtv_size;
}

template<typename _MtvT>
void collection<_MtvT>::swap(collection& other)
{
    m_vectors.swap(other.m_vectors);
    std::swap(m_mtv_size, other.m_mtv_size);
    std::swap(m_identity, other.m_identity);
    std::swap(m_elem_range, other.m_elem_range);
    std::swap(m_col_range, other.m_col_range);
}

template<typename _MtvT>
void collection<_MtvT>::set_collection_range(size_type start, size_type size)
{
    check_collection_range(start, size);
    m_col_range.start = start;
    m_col_range.size = size;
}

template<typename _MtvT>
void collection<_MtvT>::set_element_range(size_type start, size_type size)
{
    check_element_range(start, size);
    m_elem_range.start = start;
    m_elem_range.size = size;
}

template<typename _MtvT>
void collection<_MtvT>::check_collection_range(size_type start, size_type size) const
{
    if (start >= m_vectors.size())
    {
        std::ostringstream os;
        os << "range start position must be less than " << m_vectors.size();
        throw invalid_arg_error(os.str());
    }

    if (!size)
        throw invalid_arg_error("size of 0 is not allowed.");

    if ((start+size) > m_vectors.size())
        throw invalid_arg_error("size is too large.");
}

template<typename _MtvT>
void collection<_MtvT>::check_element_range(size_type start, size_type size) const
{
    if (start >= m_mtv_size)
    {
        std::ostringstream os;
        os << "range start position must be less than " << m_mtv_size;
        throw invalid_arg_error(os.str());
    }

    if (!size)
        throw invalid_arg_error("size of 0 is not allowed.");

    if ((start+size) > m_mtv_size)
        throw invalid_arg_error("size is too large.");
}

template<typename _MtvT>
std::vector<typename collection<_MtvT>::const_iterator::mtv_item>
collection<_MtvT>::build_iterator_state() const
{
    std::vector<typename const_iterator::mtv_item> cols;
    cols.reserve(m_col_range.size);

    auto it = m_vectors.begin();
    std::advance(it, m_col_range.start);
    auto it_end = it;
    std::advance(it_end, m_col_range.size);

    std::for_each(it, it_end,
        [&](const mtv_type* p)
        {
            cols.emplace_back(p, p->cbegin(), p->cend());
        }
    );

    return cols;
}

template<typename _MtvT>
template<typename _T>
void collection<_MtvT>::init_insert_vector(
    const _T& t, typename std::enable_if<std::is_pointer<_T>::value>::type*)
{
    check_vector_size(*t);
    m_vectors.emplace_back(t);
}

template<typename _MtvT>
void collection<_MtvT>::init_insert_vector(const std::unique_ptr<mtv_type>& p)
{
    check_vector_size(*p);
    m_vectors.emplace_back(p.get());
}

template<typename _MtvT>
void collection<_MtvT>::init_insert_vector(const std::shared_ptr<mtv_type>& p)
{
    check_vector_size(*p);
    m_vectors.emplace_back(p.get());
}

template<typename _MtvT>
template<typename _T>
void collection<_MtvT>::init_insert_vector(
    const _T& t, typename std::enable_if<!std::is_pointer<_T>::value>::type*)
{
    check_vector_size(t);
    m_vectors.emplace_back(&t);
}

template<typename _MtvT>
void collection<_MtvT>::check_vector_size(const mtv_type& t)
{
    if (t.empty())
        throw invalid_arg_error("Empty multi_type_vector instance is not allowed.");

    if (!m_mtv_size)
        m_mtv_size = t.size();
    else if (m_mtv_size != t.size())
        throw invalid_arg_error("All multi_type_vector instances must be of the same length.");
}

}}
