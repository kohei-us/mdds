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
side_iterator<_MtvT>::side_iterator(std::vector<mtv_item>&& vectors, begin_state_type) :
    m_vectors(std::move(vectors)), m_elem_pos(0)
{
    const mtv_item& col1 = m_vectors.front();

    m_cur_node.index = 0;
    m_cur_node.type = col1.block_pos->type;
    m_cur_node.position = typename mtv_type::const_position_type(col1.block_pos, 0);
}

template<typename _MtvT>
side_iterator<_MtvT>::side_iterator(std::vector<mtv_item>&& vectors, end_state_type) :
    m_vectors(std::move(vectors)),
    m_elem_pos(0)
{
    // TODO : to be worked on.
}

template<typename _MtvT>
template<typename _T>
collection<_MtvT>::collection(const _T& begin, const _T& end)
{
    size_type n = std::distance(begin, end);
    m_vectors.reserve(n);

    for (_T it = begin; it != end; ++it)
        init_insert_vector(*it);
}

template<typename _MtvT>
typename collection<_MtvT>::const_iterator
collection<_MtvT>::begin() const
{
    return const_iterator(build_iterator_state(), const_iterator::begin_state);
}

template<typename _MtvT>
typename collection<_MtvT>::const_iterator
collection<_MtvT>::end() const
{
    return const_iterator(build_iterator_state(), const_iterator::end_state);
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
    m_vectors.emplace_back(t);
}

template<typename _MtvT>
void collection<_MtvT>::init_insert_vector(const std::unique_ptr<mtv_type>& p)
{
    std::cout << "unique pointer: " << p.get() << std::endl;
    m_vectors.emplace_back(p.get());
}

template<typename _MtvT>
void collection<_MtvT>::init_insert_vector(const std::shared_ptr<mtv_type>& p)
{
    std::cout << "shared pointer: " << p.get() << std::endl;
    m_vectors.emplace_back(p.get());
}

template<typename _MtvT>
template<typename _T>
void collection<_MtvT>::init_insert_vector(
    const _T& t,  typename std::enable_if<!std::is_pointer<_T>::value>::type*)
{
    std::cout << "non-pointer: " << &t << std::endl;
    m_vectors.emplace_back(&t);
}

}}
