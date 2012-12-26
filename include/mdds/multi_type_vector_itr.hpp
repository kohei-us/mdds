/*************************************************************************
 *
 * Copyright (c) 2012 Kohei Yoshida
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

namespace mdds { namespace __mtv {

template<typename _ParentType, typename _BlksType, typename _BaseItrType>
class iterator_common_base
{
protected:
    typedef _ParentType parent_type;
    typedef _BlksType blocks_type;
    typedef _BaseItrType base_iterator_type;

    struct node
    {
        mdds::mtv::element_t type;
        typename parent_type::size_type size;
        typename parent_type::element_block_type* data;

        node() : type(mdds::mtv::element_type_empty), size(0), data(NULL) {}
        node(const node& other) : type(other.type), size(other.size), data(other.data) {}

        void swap(node& other)
        {
            std::swap(type, other.type);
            std::swap(size, other.size);
            std::swap(data, other.data);
        }
    };

    iterator_common_base() {}

    iterator_common_base(const base_iterator_type& pos, const base_iterator_type& end) :
        m_pos(pos), m_end(end)
    {
        if (m_pos != m_end)
            update_node();
    }

    iterator_common_base(const iterator_common_base& other) :
        m_pos(other.m_pos), m_end(other.m_end)
    {
        if (m_pos != m_end)
            update_node();
    }

    void update_node()
    {
        // blocks_type::value_type is a pointer to parent_type::block.
        typename blocks_type::value_type blk = *m_pos;
        if (blk->mp_data)
            m_cur_node.type = mdds::mtv::get_block_type(*blk->mp_data);
        else
            m_cur_node.type = mdds::mtv::element_type_empty;

        m_cur_node.size = blk->m_size;
        m_cur_node.data = blk->mp_data;
    }

    node m_cur_node;
    base_iterator_type m_pos;
    base_iterator_type m_end;

public:
    bool operator== (const iterator_common_base& other) const
    {
        return m_pos == other.m_pos && m_end == other.m_end;
    }

    bool operator!= (const iterator_common_base& other) const
    {
        return !operator==(other);
    }

    iterator_common_base& operator= (const iterator_common_base& other)
    {
        iterator_common_base assigned(other);
        swap(assigned);
        return *this;
    }

    void swap(iterator_common_base& other)
    {
        m_cur_node.swap(other.m_cur_node);
        std::swap(m_pos, other.m_pos);
        std::swap(m_end, other.m_end);
    }
};

template<typename _ParentType, typename _BlksType, typename _BaseItrType>
class iterator_base : public iterator_common_base<_ParentType,_BlksType,_BaseItrType>
{
    typedef iterator_common_base<_ParentType,_BlksType,_BaseItrType> common_base;
    typedef _BaseItrType base_iterator_type;

    using common_base::update_node;
    using common_base::m_cur_node;
    using common_base::m_pos;
    using common_base::m_end;

public:

    // iterator traits
    typedef typename common_base::node value_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    typedef ptrdiff_t   difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;

public:
    iterator_base() {}
    iterator_base(const base_iterator_type& pos, const base_iterator_type& end) :
        common_base(pos, end) {}

    iterator_base(const iterator_base& other) :
        common_base(other) {}

    value_type& operator*()
    {
        return m_cur_node;
    }

    value_type* operator->()
    {
        return &m_cur_node;
    }

    value_type* operator++()
    {
        ++m_pos;
        if (m_pos == m_end)
            return NULL;

        update_node();
        return &m_cur_node;
    }

    value_type* operator--()
    {
        --m_pos;
        update_node();
        return &m_cur_node;
    }

    /**
     * This method is public only to allow const_iterator_base to instantiate
     * from iterator_base.
     */
    const base_iterator_type& get_pos() const { return m_pos; }

    /**
     * This method is public only to allow const_iterator_base to instantiate
     * from iterator_base.
     */
    const base_iterator_type& get_end() const { return m_end; }
};

template<typename _ParentType, typename _BlksType, typename _BaseItrType, typename _NonConstItrBase>
class const_iterator_base : public iterator_common_base<_ParentType,_BlksType,_BaseItrType>
{
    typedef iterator_common_base<_ParentType,_BlksType,_BaseItrType> common_base;
    typedef _BaseItrType base_iterator_type;

    using common_base::update_node;
    using common_base::m_cur_node;
    using common_base::m_pos;
    using common_base::m_end;

public:

    typedef _NonConstItrBase iterator_base;

    // iterator traits
    typedef typename common_base::node value_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    typedef ptrdiff_t   difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;

public:
    const_iterator_base() : common_base() {}
    const_iterator_base(const base_iterator_type& pos, const base_iterator_type& end) :
        common_base(pos, end) {}

    const_iterator_base(const const_iterator_base& other) :
        common_base(other) {}

    /**
     * Take the non-const iterator counterpart to create a const iterator.
     */
    const_iterator_base(const iterator_base& other) :
        common_base(other.get_pos(), other.get_end()) {}

    const value_type& operator*() const
    {
        return m_cur_node;
    }

    const value_type* operator->() const
    {
        return &m_cur_node;
    }

    const value_type* operator++()
    {
        ++m_pos;
        if (m_pos == m_end)
            return NULL;

        update_node();
        return &m_cur_node;
    }

    const value_type* operator--()
    {
        --m_pos;
        update_node();
        return &m_cur_node;
    }
};

}}
