/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2021 Kohei Yoshida
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

#ifndef INCLUDED_MDDS_MULTI_TYPE_VECTOR_DIR_SOA_ITERATOR_HPP
#define INCLUDED_MDDS_MULTI_TYPE_VECTOR_DIR_SOA_ITERATOR_HPP

#include "../iterator_node.hpp"

#include <ostream>

namespace mdds { namespace mtv { namespace soa { namespace detail {

/**
 * Common base for both const and non-const iterators to handle traversal of
 * the internal source iterators.
 *
 * Its protected inc() and dec() methods have non-const return type, and the
 * derived classes wrap them and return values with their respective const
 * modifiers.
 *
 * The trait struct needs to have the following static member types:
 * <ul>
 *     <li><code>parent</code></li>
 *     <li><code>positions_type</code></li>
 *     <li><code>sizes_type</code></li>
 *     <li><code>element_blocks_type</code></li>
 * </ul>
 */
template<typename Trait>
class iterator_updater
{
protected:
    using parent_type = typename Trait::parent;
    using positions_type = typename Trait::positions_type;
    using sizes_type = typename Trait::sizes_type;
    using element_blocks_type = typename Trait::element_blocks_type;
    using size_type = typename Trait::parent::size_type;

    using node = mdds::detail::mtv::iterator_value_node<parent_type, size_type>;

    using positions_iterator_type = typename Trait::positions_iterator_type;
    using sizes_iterator_type = typename Trait::sizes_iterator_type;
    using element_blocks_iterator_type = typename Trait::element_blocks_iterator_type;

    /**
     * This struct groups together the iterators for the three array types for
     * easy synchronized traversal of the arrays.
     */
    struct grouped_iterator_type
    {
        positions_iterator_type position_iterator;
        sizes_iterator_type size_iterator;
        element_blocks_iterator_type element_block_iterator;

        void inc()
        {
            ++position_iterator;
            ++size_iterator;
            ++element_block_iterator;
        }

        void dec()
        {
            --position_iterator;
            --size_iterator;
            --element_block_iterator;
        }

        bool operator==(const grouped_iterator_type& other) const
        {
            return position_iterator == other.position_iterator && size_iterator == other.size_iterator &&
                   element_block_iterator == other.element_block_iterator;
        }

        bool operator!=(const grouped_iterator_type& other) const
        {
            return !operator==(other);
        }

        grouped_iterator_type() = default;

        grouped_iterator_type(
            const positions_iterator_type& itr_pos, const sizes_iterator_type& itr_size,
            const element_blocks_iterator_type& itr_elem_blocks)
            : position_iterator(itr_pos), size_iterator(itr_size), element_block_iterator(itr_elem_blocks)
        {}
    };

    node m_cur_node;
    grouped_iterator_type m_pos;
    grouped_iterator_type m_end;

    iterator_updater() : m_cur_node(nullptr, 0)
    {}

    iterator_updater(
        const grouped_iterator_type& pos, const grouped_iterator_type& end, const parent_type* parent,
        size_type block_index)
        : m_cur_node(parent, block_index), m_pos(pos), m_end(end)
    {
        if (m_pos != m_end)
            update_node();
    }

    iterator_updater(
        const positions_iterator_type& positions_pos, const sizes_iterator_type& sizes_pos,
        const element_blocks_iterator_type& eb_pos, const positions_iterator_type& positions_end,
        const sizes_iterator_type& sizes_end, const element_blocks_iterator_type& eb_end, const parent_type* parent,
        size_type block_index)
        : iterator_updater({positions_pos, sizes_pos, eb_pos}, {positions_end, sizes_end, eb_end}, parent, block_index)
    {}

    iterator_updater(const iterator_updater& other)
        : m_cur_node(other.m_cur_node), m_pos(other.m_pos), m_end(other.m_end)
    {}

    void update_node()
    {
#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
        if (m_pos == m_end)
            throw general_error("Current node position should never equal the end position during node update.");
#endif

        m_cur_node.position = *m_pos.position_iterator;
        m_cur_node.size = *m_pos.size_iterator;
        m_cur_node.data = *m_pos.element_block_iterator;

        if (m_cur_node.data)
            m_cur_node.type = mdds::mtv::get_block_type(*m_cur_node.data);
        else
            m_cur_node.type = mdds::mtv::element_type_empty;
    }

    node* inc()
    {
        m_pos.inc();
        if (m_pos == m_end)
            return nullptr;

        update_node();
        return &m_cur_node;
    }

    node* dec()
    {
        m_pos.dec();
        update_node();
        return &m_cur_node;
    }

    void _print_state(std::ostream& os) const
    {
        auto prev_flags = os.flags();
        os << "parent=" << std::hex << m_cur_node.__private_data.parent
           << "; block-index=" << m_cur_node.__private_data.block_index << "; position=" << m_cur_node.position
           << "; size=" << m_cur_node.size << "; type=" << m_cur_node.type << "; data=" << m_cur_node.data;
        os.flags(prev_flags);
    }

public:
    bool operator==(const iterator_updater& other) const
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

    bool operator!=(const iterator_updater& other) const
    {
        return !operator==(other);
    }

    iterator_updater& operator=(const iterator_updater& other)
    {
        m_cur_node = other.m_cur_node;
        m_pos = other.m_pos;
        m_end = other.m_end;
        return *this;
    }

    void swap(iterator_updater& other)
    {
        m_cur_node.swap(other.m_cur_node);
        std::swap(m_pos, other.m_pos);
        std::swap(m_end, other.m_end);
    }

    const node& get_node() const
    {
        return m_cur_node;
    }
    const grouped_iterator_type& get_pos() const
    {
        return m_pos;
    }
    const grouped_iterator_type& get_end() const
    {
        return m_end;
    }
};

template<typename Trait>
class iterator_base : public iterator_updater<Trait>
{
    using parent_type = typename Trait::parent;
    using node_update_func = typename Trait::private_data_update;
    using updater = iterator_updater<Trait>;

    using grouped_iterator_type = typename updater::grouped_iterator_type;
    using size_type = typename updater::size_type;

    using updater::dec;
    using updater::inc;
    using updater::m_cur_node;

public:
    using updater::get_end;
    using updater::get_pos;

    // iterator traits
    using value_type = typename updater::node;
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

public:
    iterator_base()
    {}
    iterator_base(
        const grouped_iterator_type& pos, const grouped_iterator_type& end, const parent_type* parent,
        size_type block_index)
        : updater(pos, end, parent, block_index)
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

    void _print_state(std::ostream& os) const
    {
        os << "{iterator: ";
        updater::_print_state(os);
        os << "}";
    }
};

template<typename Trait, typename NonConstItrBase>
class const_iterator_base : public iterator_updater<Trait>
{
    using parent_type = typename Trait::parent;
    using node_update_func = typename Trait::private_data_update;
    using updater = iterator_updater<Trait>;

    using grouped_iterator_type = typename updater::grouped_iterator_type;
    using size_type = typename updater::size_type;

    using updater::dec;
    using updater::inc;
    using updater::m_cur_node;

public:
    using updater::get_end;
    using updater::get_pos;

    using iterator_base = NonConstItrBase;

    // iterator traits
    using value_type = typename updater::node;
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

public:
    const_iterator_base() : updater()
    {}
    const_iterator_base(
        const grouped_iterator_type& pos, const grouped_iterator_type& end, const parent_type* parent,
        size_type block_index)
        : updater(pos, end, parent, block_index)
    {}

    /**
     * Take the non-const iterator counterpart to create a const iterator.
     */
    const_iterator_base(const iterator_base& other)
        : updater(
              other.get_pos().position_iterator, other.get_pos().size_iterator, other.get_pos().element_block_iterator,
              other.get_end().position_iterator, other.get_end().size_iterator, other.get_end().element_block_iterator,
              other.get_node().__private_data.parent, other.get_node().__private_data.block_index)
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
        return updater::operator==(other);
    }

    bool operator!=(const const_iterator_base& other) const
    {
        return updater::operator!=(other);
    }

    void _print_state(std::ostream& os) const
    {
        os << "(const-iterator: ";
        updater::_print_state(os);
        os << ")";
    }
};

template<typename Trait>
std::ostream& operator<<(std::ostream& os, const iterator_base<Trait>& it)
{
    it._print_state(os);
    return os;
}

template<typename Trait, typename NonConstItrBase>
std::ostream& operator<<(std::ostream& os, const const_iterator_base<Trait, NonConstItrBase>& it)
{
    it._print_state(os);
    return os;
}

}}}} // namespace mdds::mtv::soa::detail

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
