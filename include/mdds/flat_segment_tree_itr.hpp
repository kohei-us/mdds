/*************************************************************************
 *
 * Copyright (c) 2010-2023 Kohei Yoshida
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

#pragma once

#include "./ref_pair.hpp"
#include <type_traits>

namespace mdds { namespace fst { namespace detail {

/**
 * Handler for forward iterator
 */
template<typename FstType>
struct forward_itr_handler
{
    using fst_type = FstType;

    static const typename fst_type::node* init_pos(const fst_type* _db, bool _end)
    {
        return _end ? _db->m_right_leaf.get() : _db->m_left_leaf.get();
    }

    static void inc(const fst_type* _db, const typename fst_type::node*& p, bool& end)
    {
        if (p == _db->m_right_leaf.get())
            end = true;
        else
            p = p->next.get();
    }

    static void dec(const typename fst_type::node*& p, bool& end)
    {
        if (end)
            end = false;
        else
            p = p->prev.get();
    }
};

/**
 * Handler for reverse iterator
 */
template<typename FstType>
struct reverse_itr_handler
{
    using fst_type = FstType;

    static const typename fst_type::node* init_pos(const fst_type* _db, bool _end)
    {
        return _end ? _db->m_left_leaf.get() : _db->m_right_leaf.get();
    }

    static void inc(const fst_type* _db, const typename fst_type::node*& p, bool& end)
    {
        if (p == _db->m_left_leaf.get())
            end = true;
        else
            p = p->prev.get();
    }

    static void dec(const typename fst_type::node*& p, bool& end)
    {
        if (end)
            end = false;
        else
            p = p->next.get();
    }
};

template<typename FstType, typename Hdl>
class const_iterator_base
{
    typedef Hdl handler_type;

public:
    typedef FstType fst_type;

    // iterator traits
    using value_type = mdds::detail::ref_pair<
        std::add_const_t<typename fst_type::key_type>, std::add_const_t<typename fst_type::value_type>>;
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = ptrdiff_t;
    using iterator_category = ::std::bidirectional_iterator_tag;

    explicit const_iterator_base(const fst_type* _db, bool _end) : m_db(_db), m_end_pos(_end)
    {
        if (!_db)
            return;

        m_pos = handler_type::init_pos(_db, _end);
    }

    explicit const_iterator_base(const fst_type* _db, const typename fst_type::node* pos) : m_db(_db), m_pos(pos)
    {}

    const_iterator_base(const const_iterator_base& r) : m_db(r.m_db), m_pos(r.m_pos), m_end_pos(r.m_end_pos)
    {}

    const_iterator_base& operator=(const const_iterator_base& r)
    {
        m_db = r.m_db;
        m_pos = r.m_pos;
        m_end_pos = r.m_end_pos;
        return *this;
    }

    const_iterator_base& operator++()
    {
        assert(m_pos);
        handler_type::inc(m_db, m_pos, m_end_pos);
        return *this;
    }

    const_iterator_base& operator--()
    {
        assert(m_pos);
        handler_type::dec(m_pos, m_end_pos);
        return *this;
    }

    bool operator==(const const_iterator_base& r) const
    {
        if (m_db != r.m_db)
            return false;

        return (m_pos == r.m_pos) && (m_end_pos == r.m_end_pos);
    }

    bool operator!=(const const_iterator_base& r) const
    {
        return !operator==(r);
    }

    value_type operator*()
    {
        return value_type(m_pos->key, m_pos->value_leaf.value);
    }

    value_type operator->()
    {
        return value_type(m_pos->key, m_pos->value_leaf.value);
    }

protected:
    const typename fst_type::node* get_pos() const
    {
        return m_pos;
    }

    const fst_type* get_parent() const
    {
        return m_db;
    }

    bool is_end_pos() const
    {
        return m_end_pos;
    }

private:
    const fst_type* m_db = nullptr;
    const typename fst_type::node* m_pos = nullptr;
    bool m_end_pos = false;
};

template<typename FstType>
class const_segment_iterator
{
    typedef FstType fst_type;
    friend fst_type;

    const_segment_iterator(const typename fst_type::node* start, const typename fst_type::node* end)
        : m_start(start), m_end(end)
    {
        update_node();
    }

public:
    struct value_type
    {
        typename fst_type::key_type start;
        typename fst_type::key_type end;
        typename fst_type::value_type value;

        value_type() : start(), end(), value()
        {}

        value_type(
            typename fst_type::key_type _start, typename fst_type::key_type _end, typename fst_type::value_type _value)
            : start(std::move(_start)), end(std::move(_end)), value(std::move(_value))
        {}

        bool operator==(const value_type& other) const
        {
            return start == other.start && end == other.end && value == other.value;
        }

        bool operator!=(const value_type& other) const
        {
            return !operator==(other);
        }
    };

    const_segment_iterator() : m_start(nullptr), m_end(nullptr)
    {}
    const_segment_iterator(const const_segment_iterator& other) : m_start(other.m_start), m_end(other.m_end)
    {
        if (m_start)
            update_node();
    }
    const_segment_iterator(const_segment_iterator&& other)
        : m_start(std::move(other.m_start)), m_end(std::move(other.m_end))
    {
        if (m_start)
            update_node();
    }

    ~const_segment_iterator()
    {}

    bool operator==(const const_segment_iterator& other) const
    {
        return m_start == other.m_start && m_end == other.m_end;
    }

    bool operator!=(const const_segment_iterator& other) const
    {
        return !operator==(other);
    }

    const_segment_iterator& operator=(const const_segment_iterator& other)
    {
        m_start = other.m_start;
        m_end = other.m_end;
        if (m_start)
            update_node();
        return *this;
    }

    const_segment_iterator& operator=(const_segment_iterator&& other)
    {
        m_start = std::move(other.m_start);
        m_end = std::move(other.m_end);
        if (m_start)
            update_node();
        return *this;
    }

    const value_type& operator*()
    {
        return m_node;
    }

    const value_type* operator->()
    {
        return &m_node;
    }

    const_segment_iterator& operator++()
    {
        assert(m_start);
        m_start = m_start->next.get();
        m_end = m_start->next.get();
        update_node();
        return *this;
    }

    const_segment_iterator operator++(int)
    {
        assert(m_start);
        const_segment_iterator ret = *this;
        m_start = m_start->next.get();
        m_end = m_start->next.get();
        update_node();
        return ret;
    }

    const_segment_iterator& operator--()
    {
        assert(m_start);
        m_start = m_start->prev.get();
        m_end = m_start->next.get();
        update_node();
        return *this;
    }

    const_segment_iterator operator--(int)
    {
        assert(m_start);
        const_segment_iterator ret = *this;
        m_start = m_start->prev.get();
        m_end = m_start->next.get();
        update_node();
        return ret;
    }

private:
    void update_node()
    {
        if (!m_end)
            // The iterator is at its end position. Nothing to do.
            return;

        m_node.start = m_start->key;
        m_node.end = m_end->key;
        m_node.value = m_start->value_leaf.value;
    }

private:
    const typename fst_type::node* m_start;
    const typename fst_type::node* m_end;
    value_type m_node;
};

}}} // namespace mdds::fst::detail
