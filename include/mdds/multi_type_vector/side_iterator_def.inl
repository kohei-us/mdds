/*************************************************************************
 *
 * Copyright (c) 2016 Kohei Yoshida
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

#include <iostream>

namespace mdds { namespace mtv {

template<typename _MtvT>
side_iterator<_MtvT>::side_iterator(
    std::vector<mtv_item>&& vectors, size_type mtv_size, uintptr_t identity, begin_state_type) :
    m_vectors(std::move(vectors)),
    m_elem_pos(0),
    m_mtv_size(mtv_size),
    m_identity(identity)
{
    assert(m_mtv_size);
    const mtv_item& col1 = m_vectors.front();

    m_cur_node.index = 0;
    m_cur_node.type = col1.block_pos->type;
    m_cur_node.position = typename mtv_type::const_position_type(col1.block_pos, 0);
}

template<typename _MtvT>
side_iterator<_MtvT>::side_iterator(
    std::vector<mtv_item>&& vectors, size_type mtv_size, uintptr_t identity, end_state_type) :
    m_vectors(std::move(vectors)),
    m_elem_pos(0),
    m_mtv_size(mtv_size),
    m_identity(identity)
{
    assert(m_mtv_size);

    m_elem_pos = m_mtv_size;
    m_cur_node.index = 0;

    // We can leave the position and type uninitialized since this is an end
    // position which doesn't reference an actual element.
}

template<typename _MtvT>
typename side_iterator<_MtvT>::side_iterator&
side_iterator<_MtvT>::operator++()
{
    ++m_cur_node.index;
    if (m_cur_node.index >= m_vectors.size())
    {
        // Move to the next element position.
        m_cur_node.index = 0;
        ++m_elem_pos;
        if (m_elem_pos >= m_mtv_size)
            // End position has been reached.  Don't update the current node.
            return *this;
    }

    // Get the current vector.
    assert(m_cur_node.index < m_vectors.size());
    mtv_item& col = m_vectors[m_cur_node.index];

    // Update the current node.
    m_cur_node.position = col.vector->position(col.block_pos, m_elem_pos);
    col.block_pos = m_cur_node.position.first;
    m_cur_node.type = col.block_pos->type;

    return *this;
}

template<typename _MtvT>
bool side_iterator<_MtvT>::operator== (const side_iterator& other) const
{
    if (m_identity != other.m_identity)
        return false;

    if (m_mtv_size != other.m_mtv_size)
        return false;

    return m_cur_node.index == other.m_cur_node.index && m_elem_pos == other.m_elem_pos;
}

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
}

template<typename _MtvT>
typename collection<_MtvT>::const_iterator
collection<_MtvT>::begin() const
{
    return const_iterator(
        build_iterator_state(), m_mtv_size, m_identity, const_iterator::begin_state);
}

template<typename _MtvT>
typename collection<_MtvT>::const_iterator
collection<_MtvT>::end() const
{
    return const_iterator(
        build_iterator_state(), m_mtv_size, m_identity, const_iterator::end_state);
}

template<typename _MtvT>
std::vector<typename collection<_MtvT>::const_iterator::mtv_item>
collection<_MtvT>::build_iterator_state() const
{
    std::vector<typename const_iterator::mtv_item> cols;
    cols.reserve(m_vectors.size());

    std::for_each(m_vectors.begin(), m_vectors.end(),
        [&](const mtv_type* p)
        {
            cols.emplace_back(p, p->begin(), p->end());
        }
    );

    return cols;
}

template<typename _MtvT>
template<typename _T>
void collection<_MtvT>::init_insert_vector(
    const _T& t,  typename std::enable_if<std::is_pointer<_T>::value>::type*)
{
    std::cout << "pointer: " << t << std::endl;
    check_vector_size(*t);
    m_vectors.emplace_back(t);
}

template<typename _MtvT>
void collection<_MtvT>::init_insert_vector(const std::unique_ptr<mtv_type>& p)
{
    std::cout << "unique pointer: " << p.get() << std::endl;
    check_vector_size(*p);
    m_vectors.emplace_back(p.get());
}

template<typename _MtvT>
void collection<_MtvT>::init_insert_vector(const std::shared_ptr<mtv_type>& p)
{
    std::cout << "shared pointer: " << p.get() << std::endl;
    check_vector_size(*p);
    m_vectors.emplace_back(p.get());
}

template<typename _MtvT>
template<typename _T>
void collection<_MtvT>::init_insert_vector(
    const _T& t,  typename std::enable_if<!std::is_pointer<_T>::value>::type*)
{
    std::cout << "non-pointer: " << &t << std::endl;
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
