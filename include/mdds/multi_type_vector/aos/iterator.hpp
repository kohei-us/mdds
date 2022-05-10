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

#ifndef INCLUDED_MDDS_MULTI_TYPE_VECTOR_DIR_AOS_ITERATOR_HPP
#define INCLUDED_MDDS_MULTI_TYPE_VECTOR_DIR_AOS_ITERATOR_HPP

#include "../iterator_node.hpp"

#include <cstddef>

namespace mdds { namespace mtv { namespace aos { namespace detail {

/**
 * Common base for both const and non-const iterators.  Its protected inc()
 * and dec() methods have non-const return type, and the derived classes
 * wrap them and return values with their respective const modifiers.
 */
template<typename Trait>
class iterator_common_base
{
protected:
    typedef typename Trait::parent parent_type;
    typedef typename Trait::blocks blocks_type;
    typedef typename Trait::base_iterator base_iterator_type;

    typedef typename parent_type::size_type size_type;
    typedef mdds::detail::mtv::iterator_value_node<parent_type, size_type> node;

    iterator_common_base() : m_cur_node(nullptr, 0)
    {}

    iterator_common_base(
        const base_iterator_type& pos, const base_iterator_type& end, const parent_type* parent, size_type block_index)
        : m_cur_node(parent, block_index), m_pos(pos), m_end(end)
    {
        if (m_pos != m_end)
            update_node();
    }

    iterator_common_base(const iterator_common_base& other)
        : m_cur_node(other.m_cur_node), m_pos(other.m_pos), m_end(other.m_end)
    {}

    void update_node()
    {
#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
        if (m_pos == m_end)
            throw general_error("Current node position should never equal the end position during node update.");
#endif
        // blocks_type::value_type is a pointer to multi_type_vector::block.
        const typename blocks_type::value_type& blk = *m_pos;
        if (blk.data)
            m_cur_node.type = mdds::mtv::get_block_type(*blk.data);
        else
            m_cur_node.type = mdds::mtv::element_type_empty;

        m_cur_node.position = blk.position;
        m_cur_node.size = blk.size;
        m_cur_node.data = blk.data;
    }

    node* inc()
    {
        ++m_pos;
        if (m_pos == m_end)
            return nullptr;

        update_node();
        return &m_cur_node;
    }

    node* dec()
    {
        --m_pos;
        update_node();
        return &m_cur_node;
    }

    node m_cur_node;
    base_iterator_type m_pos;
    base_iterator_type m_end;

public:
    bool operator==(const iterator_common_base& other) const
    {
        if (m_pos != m_end && other.m_pos != other.m_end)
        {
            // TODO: Set hard-coded values to the current node for the end
            // position nodes to remove this if block.
            if (m_cur_node != other.m_cur_node)
                return false;
        }
        return m_pos == other.m_pos && m_end == other.m_end;
    }

    bool operator!=(const iterator_common_base& other) const
    {
        return !operator==(other);
    }

    iterator_common_base& operator=(const iterator_common_base& other)
    {
        m_cur_node = other.m_cur_node;
        m_pos = other.m_pos;
        m_end = other.m_end;
        return *this;
    }

    void swap(iterator_common_base& other)
    {
        m_cur_node.swap(other.m_cur_node);
        std::swap(m_pos, other.m_pos);
        std::swap(m_end, other.m_end);
    }

    const node& get_node() const
    {
        return m_cur_node;
    }
    const base_iterator_type& get_pos() const
    {
        return m_pos;
    }
    const base_iterator_type& get_end() const
    {
        return m_end;
    }
};

template<typename Trait, typename NodeUpdateFunc>
class iterator_base : public iterator_common_base<Trait>
{
    using parent_type = typename Trait::parent;
    typedef NodeUpdateFunc node_update_func;
    typedef iterator_common_base<Trait> common_base;

    typedef typename Trait::base_iterator base_iterator_type;
    typedef typename common_base::size_type size_type;

    using common_base::dec;
    using common_base::inc;
    using common_base::m_cur_node;

public:
    using common_base::get_end;
    using common_base::get_pos;

    // iterator traits
    typedef typename common_base::node value_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    typedef ptrdiff_t difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;

public:
    iterator_base()
    {}
    iterator_base(
        const base_iterator_type& pos, const base_iterator_type& end, const parent_type* parent, size_type block_index)
        : common_base(pos, end, parent, block_index)
    {}

    value_type& operator*()
    {
        return m_cur_node;
    }

    const value_type& operator*() const
    {
        return m_cur_node;
    }

    value_type* operator->()
    {
        return &m_cur_node;
    }

    const value_type* operator->() const
    {
        return &m_cur_node;
    }

    iterator_base& operator++()
    {
        node_update_func::inc(m_cur_node);
        inc();
        return *this;
    }

    iterator_base& operator--()
    {
        dec();
        node_update_func::dec(m_cur_node);
        return *this;
    }
};

template<typename Trait, typename NodeUpdateFunc, typename NonConstItrBase>
class const_iterator_base : public iterator_common_base<Trait>
{
    using parent_type = typename Trait::parent;
    typedef NodeUpdateFunc node_update_func;
    typedef iterator_common_base<Trait> common_base;

    typedef typename Trait::base_iterator base_iterator_type;
    typedef typename common_base::size_type size_type;

    using common_base::dec;
    using common_base::inc;
    using common_base::m_cur_node;

public:
    using common_base::get_end;
    using common_base::get_pos;

    typedef NonConstItrBase iterator_base;

    // iterator traits
    typedef typename common_base::node value_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    typedef ptrdiff_t difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;

public:
    const_iterator_base() : common_base()
    {}
    const_iterator_base(
        const base_iterator_type& pos, const base_iterator_type& end, const parent_type* parent, size_type block_index)
        : common_base(pos, end, parent, block_index)
    {}

    /**
     * Take the non-const iterator counterpart to create a const iterator.
     */
    const_iterator_base(const iterator_base& other)
        : common_base(
              other.get_pos(), other.get_end(), other.get_node().__private_data.parent,
              other.get_node().__private_data.block_index)
    {}

    const value_type& operator*() const
    {
        return m_cur_node;
    }

    const value_type* operator->() const
    {
        return &m_cur_node;
    }

    const_iterator_base& operator++()
    {
        node_update_func::inc(m_cur_node);
        inc();
        return *this;
    }

    const_iterator_base& operator--()
    {
        dec();
        node_update_func::dec(m_cur_node);
        return *this;
    }

    bool operator==(const const_iterator_base& other) const
    {
        return iterator_common_base<Trait>::operator==(other);
    }

    bool operator!=(const const_iterator_base& other) const
    {
        return iterator_common_base<Trait>::operator!=(other);
    }
};

}}}} // namespace mdds::mtv::aos::detail

#endif
