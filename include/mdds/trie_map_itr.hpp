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
#include <iostream>

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
        return m_node_stack.back().node == other.m_node_stack.back().node;
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

    iterator_base& operator++()
    {
        using ktt = key_trait_type;

        const trie_node* cur_node = m_node_stack.back().node;

        do
        {
            if (cur_node->children.empty())
            {
                // Current node is a leaf node.  Keep moving up the stack until we
                // reach a parent node with unvisited children.

                while (true)
                {
                    // Move up one parent and see if it has an unvisited child node.
                    ktt::pop_back(m_buffer);
                    m_node_stack.pop_back();
                    typename node_stack_type::value_type& si = m_node_stack.back();
                    ++si.child_pos;

                    if (si.child_pos != si.node->children.end())
                    {
                        // Move down to this unvisited child node.
                        cur_node = &si.child_pos->second;
                        ktt::push_back(m_buffer, si.child_pos->first);
                        m_node_stack.emplace_back(cur_node, cur_node->children.begin());
                        break;
                    }

                    if (m_node_stack.size() == 1)
                    {
                        // We've reached the end position. Bail out.
                        return *this;
                    }
                }
            }
            else
            {
                // Current node has child nodes.  Follow the first child node.
                auto child_pos = cur_node->children.begin();
                cur_node = &child_pos->second;
                ktt::push_back(m_buffer, child_pos->first);
                m_node_stack.emplace_back(cur_node, cur_node->children.begin());
            }
        }
        while (!cur_node->has_value);

        m_current_value = value_type(ktt::to_string(m_buffer), cur_node->value);
        return *this;
    }
};

}}

#endif
