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
#include <cassert>
#include <iostream>

namespace mdds { namespace trie {

template<typename _TrieType>
class iterator_base
{
    typedef _TrieType trie_type;
    friend trie_type;

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

    static const trie_node* push_child_node_to_stack(
        node_stack_type& node_stack, buffer_type& buf,
        const typename trie_node::children_type::const_iterator& child_pos)
    {
        using ktt = key_trait_type;

        const trie_node* node = &child_pos->second;
        ktt::push_back(buf, child_pos->first);
        node_stack.emplace_back(node, node->children.begin());

        return node;
    }

    /**
     * From the current node, move to its previous child node and descend all
     * the way to the leaf node.
     */
    static const trie_node* descend_to_previus_leaf_node(
        node_stack_type& node_stack, buffer_type& buf)
    {
        using ktt = key_trait_type;

        const trie_node* cur_node = nullptr;

        do
        {
            // Keep moving down on the right-most child nodes until the
            // leaf node is reached.

            auto& si = node_stack.back();

            --si.child_pos;
            ktt::push_back(buf, si.child_pos->first);
            cur_node = &si.child_pos->second;
            node_stack.emplace_back(cur_node, cur_node->children.end());
        }
        while (!cur_node->children.empty());

        return cur_node;
    }

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
                    auto& si = m_node_stack.back();
                    ++si.child_pos;

                    if (si.child_pos != si.node->children.end())
                    {
                        // Move down to this unvisited child node.
                        cur_node = push_child_node_to_stack(m_node_stack, m_buffer, si.child_pos);
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
                cur_node = push_child_node_to_stack(m_node_stack, m_buffer, child_pos);
            }
        }
        while (!cur_node->has_value);

        m_current_value = value_type(ktt::to_string(m_buffer), cur_node->value);
        return *this;
    }

    iterator_base operator++(int)
    {
        iterator_base tmp(*this);
        operator++();
        return tmp;
    }

    iterator_base& operator--()
    {
        using ktt = key_trait_type;
        const trie_node* cur_node = m_node_stack.back().node;

        if (m_node_stack.size() == 1)
        {
            // This is the end position aka root node.  Move down to the
            // right-most leaf node.
            auto& si = m_node_stack.back();
            assert(si.child_pos == cur_node->children.end());
            cur_node = descend_to_previus_leaf_node(m_node_stack, m_buffer);
        }
        else if (cur_node->children.empty())
        {
            // This is a leaf node.  Keep going up until it finds a parent
            // node with unvisited child nodes on the left side, then descend
            // on that path all the way to its leaf.

            do
            {
                // Go up one node.

                ktt::pop_back(m_buffer);
                m_node_stack.pop_back();
                auto& si = m_node_stack.back();
                cur_node = si.node;

                if (si.child_pos != cur_node->children.begin())
                {
                    // Left and down.
                    cur_node = descend_to_previus_leaf_node(m_node_stack, m_buffer);
                    assert(cur_node->has_value);
                }
            }
            while (!cur_node->has_value);
        }
        else
        {
            // Non-leaf node with value.  Keep going up until either the root
            // node or another node with value is reached.

            assert(cur_node->has_value);
            assert(m_node_stack.back().child_pos == cur_node->children.begin());

            do
            {
                // Go up.
                ktt::pop_back(m_buffer);
                m_node_stack.pop_back();
                auto& si = m_node_stack.back();
                cur_node = si.node;

                if (m_node_stack.size() == 1)
                {
                    // Root node reached.
                    --si.child_pos;
                    cur_node = descend_to_previus_leaf_node(m_node_stack, m_buffer);
                    assert(cur_node->has_value);
                }
            }
            while (!cur_node->has_value);
        }

        assert(cur_node->has_value);
        m_current_value = value_type(ktt::to_string(m_buffer), cur_node->value);
        return *this;
    }

    iterator_base operator--(int)
    {
        iterator_base tmp(*this);
        operator--();
        return tmp;
    }
};

template<typename _TrieType>
class packed_iterator_base
{
    typedef _TrieType trie_type;
    friend trie_type;

    typedef typename trie_type::stack_item stack_item;
    typedef typename trie_type::node_stack_type node_stack_type;

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

    static void push_child_node_to_stack(
        node_stack_type& node_stack, buffer_type& buf, const uintptr_t* child_pos)
    {
        using ktt = key_trait_type;

        const uintptr_t* node_pos = node_stack.back().node_pos;

        char_type c = *child_pos;
        ktt::push_back(buf, c);
        ++child_pos;
        size_t offset = *child_pos;
        node_pos -= offset; // Jump to the head of the child node.
        const uintptr_t* p = node_pos;
        ++p;
        size_t index_size = *p;
        ++p;
        child_pos = p;
        const uintptr_t* child_end = child_pos + index_size;

        // Push it onto the stack.
        node_stack.emplace_back(node_pos, child_pos, child_end);
    }

public:
    packed_iterator_base() {}

    packed_iterator_base(node_stack_type&& node_stack, buffer_type&& buf) :
        m_node_stack(std::move(node_stack)),
        m_buffer(std::move(buf)) {}

    packed_iterator_base(node_stack_type&& node_stack, buffer_type&& buf, const typename trie_type::value_type& v) :
        m_node_stack(std::move(node_stack)),
        m_buffer(std::move(buf)),
        m_current_value(key_trait_type::to_string(m_buffer), v) {}

    bool operator== (const packed_iterator_base& other) const
    {
        return m_node_stack.back().node_pos == other.m_node_stack.back().node_pos;
    }

    bool operator!= (const packed_iterator_base& other) const
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

    packed_iterator_base& operator++()
    {
        using ktt = key_trait_type;

        stack_item* si = &m_node_stack.back();
        const typename trie_type::value_type* pv = nullptr;
        size_t index_size = *(si->node_pos+1);

        do
        {
            if (!index_size)
            {
                // Current node is a leaf node.  Keep moving up the stack until we
                // reach a parent node with unvisited children.

                while (true)
                {
                    // Move up one parent and see if it has an unvisited child node.
                    ktt::pop_back(m_buffer);
                    m_node_stack.pop_back();
                    si = &m_node_stack.back();
                    std::advance(si->child_pos, 2);

                    if (si->child_pos != si->child_end)
                    {
                        // Move down to this unvisited child node.
                        push_child_node_to_stack(m_node_stack, m_buffer, si->child_pos);
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
                push_child_node_to_stack(m_node_stack, m_buffer, si->child_pos);
            }

            pv = reinterpret_cast<const typename trie_type::value_type*>(*m_node_stack.back().node_pos);
        }
        while (!pv);

        assert(pv);
        m_current_value = value_type(ktt::to_string(m_buffer), *pv);

        return *this;
    }
};

}}

#endif
