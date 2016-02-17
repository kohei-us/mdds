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

#ifndef INCLUDED_MDDS_TRIE_MAP_ITR_HPP
#define INCLUDED_MDDS_TRIE_MAP_ITR_HPP

#include <utility>

namespace mdds { namespace trie {

template<typename _TrieType>
class iterator_base
{
    typedef _TrieType trie_type;
    typedef typename trie_type::node_stack_type node_stack_type;

    typedef typename trie_type::trie_node trie_node;
    typedef typename trie_type::key_trait_type key_trait_type;
    typedef typename key_trait_type::string_type string_type;
    typedef typename key_trait_type::buffer_type buffer_type;
    typedef typename key_trait_type::char_type   char_type;

    // iterator traits
    typedef typename trie_type::key_value_type value_type;
    typedef value_type*     pointer;
    typedef value_type&     reference;
    typedef std::ptrdiff_t  difference_type;

    node_stack_type m_node_stack;
    buffer_type m_buffer;
    value_type m_current_value;

public:
    iterator_base() {}

    iterator_base(node_stack_type&& node_stack, buffer_type&& buf) :
        m_node_stack(std::move(node_stack)),
        m_buffer(std::move(buf)),
        m_current_value(key_trait_type::to_string(m_buffer), m_node_stack.back().node->value)
    {}

    bool operator== (const iterator_base& other) const
    {
        if (m_node_stack.size() != other.m_node_stack.size())
            return false;

        auto it = m_node_stack.begin();
        auto ite = m_node_stack.end();
        auto it2 = other.m_node_stack.begin();

        for (; it != ite; ++it, ++it2)
        {
            if (it->node != it2->node)
                return false;
        }

        return true;
    }

    bool operator!= (const iterator_base& other) const
    {
        return !operator==(other);
    }

    const value_type& operator*()
    {
        return m_current_value;
    }

    const value_type* operator->()
    {
        return &m_current_value;
    }
};

}}

#endif
