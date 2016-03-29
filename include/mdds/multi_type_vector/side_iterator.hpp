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

#ifndef INCLUDED_MDDS_SIDE_ITERATOR_HPP
#define INCLUDED_MDDS_SIDE_ITERATOR_HPP

#include "mdds/multi_type_vector_types.hpp"

#include <type_traits>
#include <vector>
#include <memory>

namespace mdds { namespace mtv {

/**
 * Special-purpose iterator to enable multiple multi_type_vector instances
 * to be traversed "sideways".  All involved multi_type_vector instances
 * must be of the same type and length.
 */
template<typename _MtvT>
class side_iterator
{
    typedef _MtvT mtv_type;
    typedef typename mtv_type::size_type size_type;
    typedef typename mtv_type::const_iterator const_iterator;
    typedef typename mtv_type::const_position_type const_position_type;

    /** meta-data about each mtv instance.  */
    struct mtv_item
    {
        const_iterator block_pos;
        const_iterator block_end;

        mtv_item(const const_iterator& bp, const const_iterator& be) :
            block_pos(bp), block_end(be) {}
    };

    std::vector<mtv_item> m_vectors;

public:
    struct value_type
    {
        mdds::mtv::element_t type;
        size_type index;
        const_position_type position;
    };

    template<typename _T>
    side_iterator(const _T& begin, const _T& end);

private:
    void init_insert_vector(const std::unique_ptr<mtv_type>& p);

    void init_insert_vector(const std::shared_ptr<mtv_type>& p);

    template<typename _T>
    void init_insert_vector(const _T& t,  typename std::enable_if<std::is_pointer<_T>::value>::type* = 0);

    template<typename _T>
    void init_insert_vector(const _T& t,  typename std::enable_if<!std::is_pointer<_T>::value>::type* = 0);
};

}}

#include "side_iterator_def.inl"

#endif
