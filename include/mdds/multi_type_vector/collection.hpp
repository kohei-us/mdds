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

#ifndef INCLUDED_MDDS_COLLECTION_HPP
#define INCLUDED_MDDS_COLLECTION_HPP

#include "mdds/multi_type_vector_types.hpp"

#include <type_traits>
#include <vector>
#include <memory>

namespace mdds { namespace mtv {

template<typename _MtvT>
class collection;

namespace detail {

template<typename _MtvT>
class side_iterator
{
    typedef _MtvT mtv_type;
    friend collection<mtv_type>;

    typedef typename mtv_type::size_type size_type;
    typedef typename mtv_type::const_iterator const_iterator;
    typedef typename mtv_type::const_position_type const_position_type;

    /** meta-data about each mtv instance.  */
    struct mtv_item
    {
        const mtv_type* vector;
        const_iterator block_pos;
        const_iterator block_end;

        mtv_item(const mtv_type* v, const const_iterator& bp, const const_iterator& be) :
            vector(v), block_pos(bp), block_end(be) {}
    };

    /** single element value. */
    struct node
    {
        friend class side_iterator;

        /** type of current element */
        mdds::mtv::element_t type;

        /** index of current mtv instance */
        size_type index;

        /** logical position of the current element within the mtv.  */
        size_type position;

        template<typename _Blk>
        typename _Blk::value_type get() const
        {
            return _Blk::at(*__position.first->data, __position.second);
        }

    private:
        /** position object of current element within the mtv instance.  */
        const_position_type __position;
    };

    enum begin_state_type { begin_state };
    enum end_state_type { end_state };

    std::vector<mtv_item> m_vectors;
    node m_cur_node;
    size_type m_elem_pos;
    size_type m_elem_pos_end;
    size_type m_index_offset;
    uintptr_t m_identity;

    side_iterator(
        std::vector<mtv_item>&& vectors, size_type elem_pos, size_type elem_size,
        size_type index_offset, uintptr_t identity, begin_state_type);

    side_iterator(
        std::vector<mtv_item>&& vectors, size_type elem_pos, size_type elem_size,
        size_type index_offset, uintptr_t identity, end_state_type);

public:
    typedef node value_type;

    side_iterator();

    template<typename _T>
    side_iterator(const _T& begin, const _T& end);

    const value_type& operator*() const
    {
        return m_cur_node;
    }

    const value_type* operator->() const
    {
        return &m_cur_node;
    }

    side_iterator& operator++();

    side_iterator operator++(int);

    bool operator== (const side_iterator& other) const;
};

}

/**
 * Special-purpose collection of multiple multi_type_vector instances to
 * allow them to be traversed "sideways". All involved multi_type_vector
 * instances must be of the same type and length.
 */
template<typename _MtvT>
class collection
{
public:
    typedef _MtvT mtv_type;
    typedef typename mtv_type::size_type size_type;

private:

    struct range
    {
        size_type start;
        size_type size;

        range() : start(0), size(0) {}
    };

    std::vector<const mtv_type*> m_vectors;
    size_type m_mtv_size;
    uintptr_t m_identity;

    range m_elem_range; /// element range.
    range m_col_range;  /// collection range.

public:

    typedef detail::side_iterator<mtv_type> const_iterator;

    collection();

    template<typename _T>
    collection(const _T& begin, const _T& end);

    const_iterator begin() const;

    const_iterator end() const;

    size_type size() const;

    void swap(collection& other);

    /**
     * Set the sub-range of the collection to iterate.
     *
     * @param start start position.
     * @param size length of the collection range.
     */
    void set_collection_range(size_type start, size_type size);

    /**
     * Set the sub element range to iterate.
     *
     *
     * @param start start element position.
     * @param size length of the element range.
     */
    void set_element_range(size_type start, size_type size);

private:

    void check_collection_range(size_type start, size_type size) const;
    void check_element_range(size_type start, size_type size) const;

    std::vector<typename const_iterator::mtv_item> build_iterator_state() const;

    void init_insert_vector(const std::unique_ptr<mtv_type>& p);

    void init_insert_vector(const std::shared_ptr<mtv_type>& p);

    template<typename _T>
    void init_insert_vector(const _T& t, typename std::enable_if<std::is_pointer<_T>::value>::type* = 0);

    template<typename _T>
    void init_insert_vector(const _T& t, typename std::enable_if<!std::is_pointer<_T>::value>::type* = 0);

    void check_vector_size(const mtv_type& t);
};

}}

#include "collection_def.inl"

#endif
