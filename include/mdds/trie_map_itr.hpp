/*************************************************************************
 *
 * Copyright (c) 2016-2020 Kohei Yoshida
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
#ifdef MDDS_TRIE_MAP_DEBUG
#include <sstream>
#endif

#include "mdds/global.hpp"
#include "mdds/ref_pair.hpp"

namespace mdds { namespace trie { namespace detail {

enum class iterator_type
{
    /**
     * Normal iterator is expected to have at least one element on the node
     * stack i.e. root.
     */
    normal,
    /**
     * End iterator is the same as a normal iterator except that it is
     * positioned past the last node position.  A normal iterator becomes an
     * end iterator when incrementing past the last node position.
     */
    end,
    /**
     * Empty iterator doesn't reference any node in the tree but still is a
     * valid iterator (therefore differs from a singular iterator).  Its node
     * stack is empty.
     */
    empty
};

enum empty_iterator_type
{
    empty_iterator
};

template<typename _TrieType, typename _C>
struct get_node_stack_type;

template<typename _TrieType>
struct get_node_stack_type<_TrieType, std::true_type>
{
    using type = typename _TrieType::const_node_stack_type;
};

template<typename _TrieType>
struct get_node_stack_type<_TrieType, std::false_type>
{
    using type = typename _TrieType::node_stack_type;
};

template<typename _TrieType>
class search_results;

template<typename _TrieType, bool IsConst>
class iterator_base
{
protected:
    using trie_type = _TrieType;

    using _is_const = std::bool_constant<IsConst>;

    friend trie_type;
    friend search_results<trie_type>;

    using node_stack_type = typename get_node_stack_type<trie_type, _is_const>::type;
    using trie_node_type = mdds::detail::const_t<typename trie_type::trie_node, IsConst>;
    using trie_node_child_pos_type = typename mdds::detail::get_iterator_type<
        typename std::remove_const<trie_node_type>::type::children_type, _is_const>::type;

    using key_traits_type = typename trie_type::key_traits_type;
    using key_type = typename key_traits_type::key_type;
    using key_buffer_type = typename key_traits_type::key_buffer_type;
    using trie_value_type = typename mdds::detail::const_or_not<typename trie_type::value_type, _is_const>::type;

public:
    // iterator traits
    using value_type = mdds::detail::ref_pair<typename std::add_const<key_type>::type, trie_value_type>;
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

protected:
    node_stack_type m_node_stack;
    key_buffer_type m_buffer;
    key_type m_current_key;
    trie_value_type* m_current_value_ptr;
    iterator_type m_type;

    static trie_node_type* push_child_node_to_stack(
        node_stack_type& node_stack, key_buffer_type& buf, trie_node_child_pos_type& child_pos)
    {
        using ktt = key_traits_type;

        trie_node_type* node = &child_pos->second;
        ktt::push_back(buf, child_pos->first);
        node_stack.emplace_back(node, node->children.begin());

        return node;
    }

    /**
     * From the current node, move to its previous child node and descend all
     * the way to the leaf node.
     */
    static trie_node_type* descend_to_previus_leaf_node(node_stack_type& node_stack, key_buffer_type& buf)
    {
        using ktt = key_traits_type;

        trie_node_type* cur_node = nullptr;

        do
        {
            // Keep moving down on the right-most child nodes until the
            // leaf node is reached.

            auto& si = node_stack.back();

            --si.child_pos;
            ktt::push_back(buf, si.child_pos->first);
            cur_node = &si.child_pos->second;
            node_stack.emplace_back(cur_node, cur_node->children.end());
        } while (!cur_node->children.empty());

        return cur_node;
    }

    iterator_base(empty_iterator_type) : m_current_value_ptr(nullptr), m_type(iterator_type::empty)
    {}

public:
    iterator_base() : m_current_value_ptr(nullptr), m_type(iterator_type::normal)
    {}

    iterator_base(node_stack_type&& node_stack, key_buffer_type&& buf, iterator_type type)
        : m_node_stack(std::move(node_stack)), m_buffer(std::move(buf)),
          m_current_key(key_traits_type::to_key(m_buffer)), m_current_value_ptr(&m_node_stack.back().node->value),
          m_type(type)
    {}

    bool operator==(const iterator_base& other) const
    {
        if (m_type != other.m_type)
            return false;

        if (m_type == iterator_type::empty)
            return true;

        return m_node_stack.back() == other.m_node_stack.back();
    }

    bool operator!=(const iterator_base& other) const
    {
        return !operator==(other);
    }

    value_type operator*()
    {
        return value_type(m_current_key, *m_current_value_ptr);
    }

    value_type operator->()
    {
        return value_type(m_current_key, *m_current_value_ptr);
    }

    iterator_base& operator++()
    {
        using ktt = key_traits_type;

        trie_node_type* cur_node = m_node_stack.back().node;

        do
        {
            if (cur_node->children.empty())
            {
                // Current node is a leaf node.  Keep moving up the stack until we
                // reach a parent node with unvisited children.

                while (true)
                {
                    if (m_node_stack.size() == 1)
                    {
#ifdef MDDS_TRIE_MAP_DEBUG
                        if (m_type == iterator_type::end)
                        {
                            std::ostringstream os;
                            os << "iterator_base::operator++#" << __LINE__ << ": moving past the end position!";
                            throw general_error(os.str());
                        }
#endif
                        // We've reached the end position. Bail out.
                        m_type = iterator_type::end;
                        return *this;
                    }

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
                }
            }
            else
            {
                // Current node has child nodes.  Follow the first child node.
                auto child_pos = cur_node->children.begin();
                cur_node = push_child_node_to_stack(m_node_stack, m_buffer, child_pos);
            }
        } while (!cur_node->has_value);

        m_current_key = ktt::to_key(m_buffer);
        m_current_value_ptr = &cur_node->value;
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
        using ktt = key_traits_type;
        trie_node_type* cur_node = m_node_stack.back().node;

        if (m_type == iterator_type::end && cur_node->has_value)
        {
            assert(m_node_stack.size() == 1);
            m_type = iterator_type::normal;
        }
        else if (m_node_stack.size() == 1)
        {
            // This is the end position aka root node.  Move down to the
            // right-most leaf node.
            auto& si = m_node_stack.back();
            assert(si.child_pos == cur_node->children.end());
            cur_node = descend_to_previus_leaf_node(m_node_stack, m_buffer);
            m_type = iterator_type::normal;
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
            } while (!cur_node->has_value);
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
                    // Root node reached.  Left and down.
                    cur_node = descend_to_previus_leaf_node(m_node_stack, m_buffer);
                    assert(cur_node->has_value);
                }
            } while (!cur_node->has_value);
        }

        assert(cur_node->has_value);
        m_current_key = ktt::to_key(m_buffer);
        m_current_value_ptr = &cur_node->value;
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
class const_iterator;

template<typename _TrieType>
class iterator : public iterator_base<_TrieType, false>
{
    using trie_type = _TrieType;

    friend trie_type;
    friend search_results<trie_type>;
    friend const_iterator<trie_type>;

    using base_type = iterator_base<trie_type, false>;
    using node_stack_type = typename base_type::node_stack_type;
    using key_buffer_type = typename base_type::key_buffer_type;

    iterator(empty_iterator_type t) : base_type(t)
    {}

public:
    iterator() : base_type()
    {}

    iterator(node_stack_type&& node_stack, key_buffer_type&& buf, iterator_type type)
        : base_type(std::move(node_stack), std::move(buf), type)
    {}
};

template<typename _TrieType>
class const_iterator : public iterator_base<_TrieType, true>
{
    using trie_type = _TrieType;

    friend trie_type;
    friend search_results<trie_type>;

    using base_type = iterator_base<trie_type, true>;
    using node_stack_type = typename base_type::node_stack_type;
    using key_buffer_type = typename base_type::key_buffer_type;

    using base_type::m_buffer;
    using base_type::m_current_key;
    using base_type::m_current_value_ptr;
    using base_type::m_node_stack;
    using base_type::m_type;

    const_iterator(empty_iterator_type t) : base_type(t)
    {}

public:
    const_iterator() : base_type()
    {}

    const_iterator(node_stack_type&& node_stack, key_buffer_type&& buf, iterator_type type)
        : base_type(std::move(node_stack), std::move(buf), type)
    {}

    const_iterator(const iterator<_TrieType>& it) : base_type()
    {
        m_buffer = it.m_buffer;
        m_current_key = it.m_current_key;
        m_current_value_ptr = it.m_current_value_ptr;
        m_type = it.m_type;

        for (const auto& e : it.m_node_stack)
            m_node_stack.emplace_back(e.node, e.child_pos);
    }
};

template<typename _TrieType>
bool operator==(const iterator<_TrieType>& left, const const_iterator<_TrieType>& right)
{
    return const_iterator<_TrieType>(left) == right;
}

template<typename _TrieType>
bool operator!=(const iterator<_TrieType>& left, const const_iterator<_TrieType>& right)
{
    return const_iterator<_TrieType>(left) != right;
}

template<typename _TrieType>
bool operator==(const const_iterator<_TrieType>& left, const iterator<_TrieType>& right)
{
    return left == const_iterator<_TrieType>(right);
}

template<typename _TrieType>
bool operator!=(const const_iterator<_TrieType>& left, const iterator<_TrieType>& right)
{
    return left != const_iterator<_TrieType>(right);
}

template<typename _TrieType>
class search_results
{
    using trie_type = _TrieType;
    friend trie_type;
    using node_stack_type = typename trie_type::const_node_stack_type;

    using trie_node = typename trie_type::trie_node;
    using key_traits_type = typename trie_type::key_traits_type;
    using key_type = typename key_traits_type::key_type;
    using key_buffer_type = typename key_traits_type::key_buffer_type;
    using key_unit_type = typename key_traits_type::key_unit_type;

    const trie_node* m_node;
    key_buffer_type m_buffer;
    node_stack_type m_node_stack;

    search_results(const trie_node* node, key_buffer_type&& buf) : m_node(node), m_buffer(buf)
    {}

public:
    using const_iterator = typename trie_type::const_iterator;

    const_iterator begin() const
    {
        if (!m_node)
            // empty results.
            return const_iterator(empty_iterator);

        // Push the root node.
        key_buffer_type buf(m_buffer);
        node_stack_type node_stack;
        node_stack.emplace_back(m_node, m_node->children.begin());

        while (!node_stack.back().node->has_value)
        {
            // There should always be at least one value node along the
            // left-most branch.

            auto it = node_stack.back().child_pos;
            const_iterator::push_child_node_to_stack(node_stack, buf, it);
        }

        return const_iterator(std::move(node_stack), std::move(buf), iterator_type::normal);
    }

    const_iterator end() const
    {
        if (!m_node)
            // empty results.
            return const_iterator(empty_iterator);

        node_stack_type node_stack;
        node_stack.emplace_back(m_node, m_node->children.end());
        return const_iterator(std::move(node_stack), key_buffer_type(m_buffer), iterator_type::end);
    }
};

template<typename _TrieType>
class packed_search_results;

template<typename _TrieType>
class packed_iterator_base
{
    using trie_type = _TrieType;
    friend trie_type;
    friend packed_search_results<trie_type>;

    using stack_item = typename trie_type::stack_item;
    using node_stack_type = typename trie_type::node_stack_type;

    using key_traits_type = typename trie_type::key_traits_type;
    using key_type = typename key_traits_type::key_type;
    using key_buffer_type = typename key_traits_type::key_buffer_type;
    using key_unit_type = typename key_traits_type::key_unit_type;

public:
    // iterator traits
    using value_type = typename trie_type::key_value_type;
    using pointer = value_type*;
    using reference = value_type&;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::bidirectional_iterator_tag;

private:
    node_stack_type m_node_stack;
    key_buffer_type m_buffer;
    value_type m_current_value;
    iterator_type m_type;

    /**
     * Given a child offset position (child_pos), jump to the actual child
     * node position and push that onto the stack as stack_item.
     */
    static void push_child_node_to_stack(node_stack_type& node_stack, key_buffer_type& buf, const uintptr_t* child_pos)
    {
        using ktt = key_traits_type;

        const uintptr_t* node_pos = node_stack.back().node_pos;

        key_unit_type c = static_cast<key_unit_type>(*child_pos);
        ktt::push_back(buf, c);
        ++child_pos;
        size_t offset = static_cast<size_t>(*child_pos);
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

    static const void descend_to_previus_leaf_node(node_stack_type& node_stack, key_buffer_type& buf)
    {
        using ktt = key_traits_type;

        const uintptr_t* node_pos = nullptr;
        size_t index_size = 0;

        do
        {
            // Keep moving down on the right-most child nodes until the
            // leaf node is reached.

            stack_item* si = &node_stack.back();
            node_pos = si->node_pos;
            --si->child_pos;
            size_t offset = *si->child_pos;
            --si->child_pos;
            key_unit_type c = *si->child_pos;
            node_pos -= offset; // Jump to the head of the child node.
            ktt::push_back(buf, c);

            const uintptr_t* p = node_pos;
            ++p;
            index_size = *p;
            ++p;
            const uintptr_t* child_pos = p;
            const uintptr_t* child_end = child_pos + index_size;
            node_stack.emplace_back(node_pos, child_end, child_end);
        } while (index_size);
    }

    packed_iterator_base(empty_iterator_type) : m_type(iterator_type::empty)
    {}

public:
    packed_iterator_base() : m_type(iterator_type::normal)
    {}

    packed_iterator_base(node_stack_type&& node_stack, key_buffer_type&& buf, const typename trie_type::value_type& v)
        : m_node_stack(std::move(node_stack)), m_buffer(std::move(buf)),
          m_current_value(key_traits_type::to_key(m_buffer), v), m_type(iterator_type::normal)
    {}

    packed_iterator_base(node_stack_type&& node_stack, key_buffer_type&& buf)
        : m_node_stack(std::move(node_stack)), m_buffer(std::move(buf)), m_type(iterator_type::end)
    {}

    bool operator==(const packed_iterator_base& other) const
    {
        if (m_type != other.m_type)
            return false;

        if (m_type == iterator_type::empty)
            return true;

        return m_node_stack.back() == other.m_node_stack.back();
    }

    bool operator!=(const packed_iterator_base& other) const
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
        using ktt = key_traits_type;

        stack_item* si = &m_node_stack.back();
        const typename trie_type::value_type* pv = nullptr;
        size_t index_size = *(si->node_pos + 1);

        do
        {
            if (!index_size)
            {
                // Current node is a leaf node.  Keep moving up the stack until we
                // reach a parent node with unvisited children.

                while (true)
                {
                    if (m_node_stack.size() == 1)
                    {
#ifdef MDDS_TRIE_MAP_DEBUG
                        if (m_type == iterator_type::end)
                        {
                            std::ostringstream os;
                            os << "packed_iterator_base::operator++#" << __LINE__ << ": moving past the end position!";
                            throw general_error(os.str());
                        }
#endif
                        // We've reached the end position. Bail out.
                        m_type = iterator_type::end;
                        return *this;
                    }

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
                }
            }
            else
            {
                // Current node has child nodes.  Follow the first child node.
                push_child_node_to_stack(m_node_stack, m_buffer, si->child_pos);
            }

            si = &m_node_stack.back();
            pv = reinterpret_cast<const typename trie_type::value_type*>(*si->node_pos);
            index_size = *(si->node_pos + 1);
        } while (!pv);

        assert(pv);
        m_current_value = value_type(ktt::to_key(m_buffer), *pv);

        return *this;
    }

    packed_iterator_base operator++(int)
    {
        packed_iterator_base tmp(*this);
        operator++();
        return tmp;
    }

    packed_iterator_base& operator--()
    {
        using ktt = key_traits_type;

        stack_item* si = &m_node_stack.back();
        const typename trie_type::value_type* pv =
            reinterpret_cast<const typename trie_type::value_type*>(*si->node_pos);
        size_t index_size = *(si->node_pos + 1); // index size for child nodes.

        if (m_type == iterator_type::end && pv)
        {
            assert(m_node_stack.size() == 1);
            m_type = iterator_type::normal;
        }
        else if (m_node_stack.size() == 1)
        {
            // This is the end position aka root node.  Move down to the
            // right-most leaf node.
            assert(si->child_pos == si->child_end);
            descend_to_previus_leaf_node(m_node_stack, m_buffer);
            si = &m_node_stack.back();
            pv = reinterpret_cast<const typename trie_type::value_type*>(*si->node_pos);
            m_type = iterator_type::normal;
        }
        else if (!index_size)
        {
            // This is a leaf node.  Keep going up until it finds a parent
            // node with unvisited child nodes on the left side, then descend
            // on that path all the way to its leaf.

            do
            {
                // Go up one node.

                ktt::pop_back(m_buffer);
                m_node_stack.pop_back();
                si = &m_node_stack.back();
                const uintptr_t* p = si->node_pos;
                pv = reinterpret_cast<const typename trie_type::value_type*>(*p);
                ++p;
                index_size = *p;
                ++p;
                const uintptr_t* first_child = p;

                if (si->child_pos != first_child)
                {
                    // Left and down.
                    descend_to_previus_leaf_node(m_node_stack, m_buffer);
                    si = &m_node_stack.back();
                    p = si->node_pos;
                    pv = reinterpret_cast<const typename trie_type::value_type*>(*p);
                    assert(pv);
                }
            } while (!pv);
        }
        else
        {
            // Non-leaf node with value.  Keep going up until either the root
            // node or another node with value is reached.

            assert(*si->node_pos); // this node should have a value.
            assert(si->child_pos == (si->node_pos + 2));

            do
            {
                // Go up.
                ktt::pop_back(m_buffer);
                m_node_stack.pop_back();
                si = &m_node_stack.back();
                pv = reinterpret_cast<const typename trie_type::value_type*>(*si->node_pos);

                if (m_node_stack.size() == 1)
                {
                    // Root node reached.
                    descend_to_previus_leaf_node(m_node_stack, m_buffer);
                    si = &m_node_stack.back();
                    pv = reinterpret_cast<const typename trie_type::value_type*>(*si->node_pos);
                    assert(pv);
                }
            } while (!pv);
        }

        assert(pv);
        m_current_value = value_type(ktt::to_key(m_buffer), *pv);

        return *this;
    }

    packed_iterator_base operator--(int)
    {
        packed_iterator_base tmp(*this);
        operator--();
        return tmp;
    }
};

template<typename _TrieType>
class packed_search_results
{
    using trie_type = _TrieType;
    friend trie_type;
    using node_stack_type = typename trie_type::node_stack_type;

    using key_traits_type = typename trie_type::key_traits_type;
    using key_type = typename key_traits_type::key_type;
    using key_buffer_type = typename key_traits_type::key_buffer_type;
    using key_unit_type = typename key_traits_type::key_unit_type;

    const uintptr_t* m_node;
    key_buffer_type m_buffer;

    packed_search_results(const uintptr_t* node, key_buffer_type&& buf) : m_node(node), m_buffer(std::move(buf))
    {}

    node_stack_type get_root_node() const
    {
        const uintptr_t* p = m_node;
        ++p;
        size_t index_size = *p;
        ++p;
        const uintptr_t* child_pos = p;
        const uintptr_t* child_end = child_pos + index_size;

        // Push this child node onto the stack.
        node_stack_type node_stack;
        node_stack.emplace_back(m_node, child_pos, child_end);
        return node_stack;
    }

    void swap(packed_search_results& other)
    {
        std::swap(m_node, other.m_node);
        std::swap(m_buffer, other.m_buffer);
    }

public:
    using const_iterator = packed_iterator_base<trie_type>;

    packed_search_results() : m_node(nullptr)
    {}

    packed_search_results(const packed_search_results& other) : m_node(other.m_node), m_buffer(other.m_buffer)
    {}

    packed_search_results(packed_search_results&& other) : m_node(other.m_node), m_buffer(std::move(other.m_buffer))
    {
        other.m_node = nullptr;
    }

    packed_search_results& operator=(packed_search_results other)
    {
        packed_search_results tmp(std::move(other));
        swap(tmp);
        return *this;
    }

    const_iterator begin() const
    {
        if (!m_node)
            // empty results.
            return const_iterator(empty_iterator);

        // Push the root node.
        key_buffer_type buf(m_buffer);
        node_stack_type node_stack = get_root_node();

        while (!node_stack.back().has_value())
        {
            // There should always be at least one value node along the
            // left-most branch.

            const_iterator::push_child_node_to_stack(node_stack, buf, node_stack.back().child_pos);
        }

        return const_iterator(std::move(node_stack), std::move(buf), *node_stack.back().get_value());
    }

    const_iterator end() const
    {
        if (!m_node)
            // empty results.
            return const_iterator(empty_iterator);

        node_stack_type node_stack = get_root_node();
        auto& si = node_stack.back();
        si.child_pos = si.child_end;
        return const_iterator(std::move(node_stack), key_buffer_type(m_buffer));
    }
};

}}} // namespace mdds::trie::detail

#endif
