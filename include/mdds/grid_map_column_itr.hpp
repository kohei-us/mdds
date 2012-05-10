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

namespace mdds { namespace __gridmap {

template<typename _ColType, typename _BlksType, typename _ItrType>
class column_iterator
{
    typedef _ColType column_type;
    typedef _BlksType blocks_type;
    typedef _ItrType base_iterator_type;

    struct node
    {
        mdds::gridmap::cell_t type;
        typename column_type::size_type size;
        const typename column_type::cell_block_type* data;

        node() : type(mdds::gridmap::celltype_empty), size(0), data(NULL) {}
        node(const node& other) : type(other.type), size(other.size), data(other.data) {}
    };

public:

    // iterator traits
    typedef node value_type;
    typedef value_type* pointer;
    typedef value_type& reference;
    typedef ptrdiff_t   difference_type;
    typedef std::bidirectional_iterator_tag iterator_category;

public:
    column_iterator() {}
    column_iterator(const base_iterator_type& pos, const base_iterator_type& end) : m_pos(pos), m_end(end) {}
    column_iterator(const column_iterator& other) : m_pos(other.m_pos), m_end(other.m_end) {}

    bool operator== (const column_iterator& other) const
    {
        return m_pos == other.m_pos && m_end == other.m_end;
    }

    bool operator!= (const column_iterator& other) const
    {
        return !operator==(other);
    }

    column_iterator& operator= (const column_iterator& other)
    {
        m_pos = other.m_pos;
        m_end = other.m_end;
        return *this;
    }

    const value_type& operator*()
    {
        return get_current_node();
    }

    const value_type* operator->()
    {
        return &get_current_node();
    }

    const value_type* operator++()
    {
        ++m_pos;
        return (m_pos != m_end) ? operator->() : NULL;
    }

    const value_type* operator--()
    {
        --m_pos;
        return operator->();
    }

private:

    const value_type& get_current_node()
    {
        // blocks_type::value_type is a pointer to column_type::block.
        const typename blocks_type::value_type blk = *m_pos;
        if (blk->mp_data)
            m_cur_node.type = mdds::gridmap::get_block_type(*blk->mp_data);
        else
            m_cur_node.type = mdds::gridmap::celltype_empty;

        m_cur_node.size = blk->m_size;
        m_cur_node.data = blk->mp_data;

        return m_cur_node;
    }

private:
    node m_cur_node;
    base_iterator_type m_pos;
    base_iterator_type m_end;
};

}}
