/*************************************************************************
 *
 * Copyright (c) 2008-2014 Kohei Yoshida
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

#ifndef __MDDS_NODE_HXX__
#define __MDDS_NODE_HXX__

#include "mdds/global.hpp"

#include <iostream>
#include <vector>
#include <cassert>
#include <sstream>

#include <boost/intrusive_ptr.hpp>

namespace mdds { namespace st { namespace detail {

#ifdef MDDS_DEBUG_NODE_BASE

inline std::size_t node_instance_count = 0;

#endif

struct node_base
{
    node_base* parent; /// parent nonleaf_node
    bool is_leaf;

    node_base(bool _is_leaf) : parent(nullptr), is_leaf(_is_leaf)
    {}
    node_base(const node_base& r) : parent(nullptr), is_leaf(r.is_leaf)
    {}
};

/**
 * Represents a non-leaf node in a segment-tree like structure.
 */
template<typename KeyT, typename ValueT>
struct nonleaf_node : public node_base
{
    using key_type = KeyT;
    using nonleaf_value_type = ValueT;

    nonleaf_value_type value_nonleaf;

    key_type low = {}; /// low range value (inclusive)
    key_type high = {}; /// high range value (non-inclusive)

    node_base* left = nullptr; /// left child nonleaf_node
    node_base* right = nullptr; /// right child nonleaf_node

public:
    nonleaf_node() : node_base(false), value_nonleaf()
    {}

    /**
     * When copying nonleaf_node, only the stored values should be copied.
     * Connections to the parent, left and right nodes must not be copied.
     */
    nonleaf_node(const nonleaf_node& r) : node_base(r), value_nonleaf(r.value_nonleaf), low(r.low), high(r.high)
    {}

    /**
     * Like the copy constructor, only the stored values should be copied.
     */
    nonleaf_node& operator=(const nonleaf_node& r)
    {
        if (this == &r)
            // assignment to self.
            return *this;

        value_nonleaf = r.value_nonleaf;
        return *this;
    }

    bool operator==(const nonleaf_node& r) const
    {
        return low == r.low && high == r.high && value_nonleaf == r.value_nonleaf;
    }

    bool operator!=(const nonleaf_node& r) const
    {
        return !operator==(r);
    }

    std::string to_string() const
    {
        std::ostringstream os;
        os << "[" << low << "-" << high << ")";
        return os.str();
    }
};

/**
 * Represents a leaf node in a segment-tree like structure.
 */
template<typename KeyT, typename ValueT>
struct node : node_base
{
    using key_type = KeyT;
    using leaf_value_type = ValueT;
    using node_ptr = boost::intrusive_ptr<node>;

    static size_t get_instance_count()
    {
#ifdef MDDS_DEBUG_NODE_BASE
        return node_instance_count;
#else
        return 0;
#endif
    }

    leaf_value_type value_leaf;

    key_type key = {};

    node_ptr prev; /// previous sibling leaf node.
    node_ptr next; /// next sibling leaf node.

    std::size_t refcount = 0;

public:
    node() : node_base(true)
    {
#ifdef MDDS_DEBUG_NODE_BASE
        ++node_instance_count;
#endif
    }

    /**
     * When copying node, only the stored values should be copied.
     * Connections to the parent, left and right nodes must not be copied.
     */
    node(const node& r) : node_base(r), key(r.key)
    {
#ifdef MDDS_DEBUG_NODE_BASE
        ++node_instance_count;
#endif
        value_leaf = r.value_leaf;
    }

    /**
     * Like the copy constructor, only the stored values should be copied.
     */
    node& operator=(const node& r)
    {
        if (this == &r)
            // assignment to self.
            return *this;

        value_leaf = r.value_leaf;
        return *this;
    }

    ~node()
    {
#ifdef MDDS_DEBUG_NODE_BASE
        --node_instance_count;
#endif
    }

    bool operator==(const node& r) const
    {
        return key == r.key && value_leaf == r.value_leaf;
    }

    bool operator!=(const node& r) const
    {
        return !operator==(r);
    }

    std::string to_string() const
    {
        std::ostringstream os;
        os << "[" << key << "]";
        return os.str();
    }
};

template<typename KeyT, typename ValueT>
inline void intrusive_ptr_add_ref(node<KeyT, ValueT>* p)
{
    ++p->refcount;
}

template<typename KeyT, typename ValueT>
inline void intrusive_ptr_release(node<KeyT, ValueT>* p)
{
    --p->refcount;
    if (!p->refcount)
        delete p;
}

template<typename NodePtrT>
void link_nodes(NodePtrT& left, NodePtrT& right)
{
    left->next = right;
    right->prev = left;
}

template<typename T>
class tree_builder
{
public:
    typedef typename T::node leaf_node;
    typedef typename leaf_node::node_ptr leaf_node_ptr;
    typedef typename T::nonleaf_node nonleaf_node;
    typedef std::vector<nonleaf_node> nonleaf_node_pool_type;

    tree_builder(nonleaf_node_pool_type& pool) : m_pool(pool), m_pool_pos(pool.begin()), m_pool_pos_end(pool.end())
    {}

    nonleaf_node* build(const leaf_node_ptr& left_leaf_node)
    {
        if (!left_leaf_node)
            // The left leaf node is empty.  Nothing to build.
            return nullptr;

        leaf_node_ptr node1 = left_leaf_node;

        std::vector<nonleaf_node*> node_list;
        while (true)
        {
            leaf_node_ptr node2 = node1->next;
            nonleaf_node* parent_node = make_parent_node(node1.get(), node2.get());
            node_list.push_back(parent_node);

            if (!node2 || !node2->next)
                // no more nodes.  Break out of the loop.
                break;

            node1 = node2->next;
        }

        return build_tree_non_leaf(node_list);
    }

private:
    nonleaf_node* make_parent_node(node_base* node1, node_base* node2)
    {
        assert(m_pool_pos != m_pool_pos_end);

        nonleaf_node* parent_node = &(*m_pool_pos);
        ++m_pool_pos;
        node1->parent = parent_node;
        parent_node->left = node1;
        if (node2)
        {
            node2->parent = parent_node;
            parent_node->right = node2;
        }

        fill_nonleaf_parent_node(parent_node, node1, node2);
        return parent_node;
    }

    void fill_nonleaf_parent_node(nonleaf_node* parent_node, const node_base* left_node, const node_base* right_node)
    {
        // Parent node should carry the range of all of its child nodes.
        if (left_node)
        {
            parent_node->low = left_node->is_leaf ? static_cast<const leaf_node*>(left_node)->key
                                                  : static_cast<const nonleaf_node*>(left_node)->low;
        }
        else
        {
            // Having a left node is prerequisite.
            throw general_error("fill_nonleaf_parent_node: Having a left node is prerequisite.");
        }

        if (right_node)
        {
            if (right_node->is_leaf)
            {
                // When the child nodes are leaf nodes, the upper bound
                // must be the value of the node that comes after the
                // right leaf node (if such node exists).

                const auto* p = static_cast<const leaf_node*>(right_node);
                if (p->next)
                    parent_node->high = p->next->key;
                else
                    parent_node->high = p->key;
            }
            else
            {
                parent_node->high = static_cast<const nonleaf_node*>(right_node)->high;
            }
        }
        else
        {
            parent_node->high = left_node->is_leaf ? static_cast<const leaf_node*>(left_node)->key
                                                   : static_cast<const nonleaf_node*>(left_node)->high;
        }
    }

    nonleaf_node* build_tree_non_leaf(const std::vector<nonleaf_node*>& node_list)
    {
        size_t node_count = node_list.size();
        if (node_count == 1)
        {
            return node_list.front();
        }
        else if (node_count == 0)
            return nullptr;

        std::vector<nonleaf_node*> new_node_list;
        nonleaf_node* node1 = nullptr;
        typename std::vector<nonleaf_node*>::const_iterator it = node_list.begin();
        typename std::vector<nonleaf_node*>::const_iterator it_end = node_list.end();
        for (bool even_itr = false; it != it_end; ++it, even_itr = !even_itr)
        {
            if (even_itr)
            {
                nonleaf_node* node2 = *it;
                nonleaf_node* parent_node = make_parent_node(node1, node2);
                new_node_list.push_back(parent_node);
                node1 = nullptr;
                node2 = nullptr;
            }
            else
                node1 = *it;
        }

        if (node1)
        {
            // Un-paired node still needs a parent...
            nonleaf_node* parent_node = make_parent_node(node1, nullptr);
            new_node_list.push_back(parent_node);
        }

        // Move up one level, and do the same procedure until the root node is reached.
        return build_tree_non_leaf(new_node_list);
    }

    nonleaf_node_pool_type& m_pool;
    typename nonleaf_node_pool_type::iterator m_pool_pos;
    typename nonleaf_node_pool_type::iterator m_pool_pos_end;
};

template<typename KeyT, typename ValueT>
void disconnect_all_nodes(node<KeyT, ValueT>* p)
{
    if (!p)
        return;

    p->prev.reset();
    p->next.reset();
    p->parent = nullptr;
}

template<typename KeyT, typename ValueT>
void disconnect_leaf_nodes(node<KeyT, ValueT>* left_node, node<KeyT, ValueT>* right_node)
{
    if (!left_node || !right_node)
        return;

    // Go through all leaf nodes, and disconnect their links.
    auto* cur_node = left_node;
    do
    {
        auto* next_node = cur_node->next.get();
        disconnect_all_nodes(cur_node);
        cur_node = next_node;
    } while (cur_node != right_node);

    disconnect_all_nodes(right_node);
}

template<typename KeyT, typename ValueT>
size_t count_leaf_nodes(const node<KeyT, ValueT>* left_end, const node<KeyT, ValueT>* right_end)
{
    size_t leaf_count = 1;
    const auto* p = left_end;
    const auto* p_end = right_end;
    for (; p != p_end; p = p->next.get(), ++leaf_count)
        ;

    return leaf_count;
}

inline size_t count_needed_nonleaf_nodes(size_t leaf_count)
{
    size_t nonleaf_count = 0;
    while (true)
    {
        if (leaf_count == 1)
            break;

        if ((leaf_count % 2) == 1)
            // Add one to make it an even number.
            ++leaf_count;

        leaf_count /= 2;
        nonleaf_count += leaf_count;
    }

    return nonleaf_count;
}

template<typename TraitsT>
class tree_dumper
{
    using node_list_type = std::vector<const node_base*>;
    using tree_type = typename TraitsT::tree_type;

public:
    static size_t dump(std::ostream& os, const tree_type& tree, const node_base* root_node)
    {
        if (!root_node)
            return 0;

        node_list_type node_list;
        node_list.push_back(root_node);
        return dump_layer(os, tree, node_list, 0);
    }

private:
    static size_t dump_layer(
        std::ostream& os, const tree_type& tree, const node_list_type& node_list, unsigned int level)
    {
        using leaf_type = typename TraitsT::leaf_type;
        using nonleaf_type = typename TraitsT::nonleaf_type;
        using to_string = typename TraitsT::to_string;

        if (node_list.empty())
            return 0;

        size_t node_count = node_list.size();

        bool is_leaf = node_list.front()->is_leaf;
        os << "level " << level << ':' << std::endl;
        os << "  type: " << (is_leaf ? "leaf" : "non-leaf") << std::endl;
        os << "  nodes:" << std::endl;

        node_list_type new_list;

        for (const node_base* p : node_list)
        {
            os << "    - '";

            if (!p)
            {
                os << "*'" << std::endl;
                continue;
            }

            if (p->is_leaf)
            {
                const auto* pl = static_cast<const leaf_type*>(p);
                os << to_string{tree}(*pl) << "'" << std::endl;
                continue;
            }

            const auto* pnl = static_cast<const nonleaf_type*>(p);
            os << to_string{tree}(*pnl) << "'" << std::endl;

            if (pnl->left)
            {
                new_list.push_back(pnl->left);
                if (pnl->right)
                    new_list.push_back(pnl->right);
            }
        }

        if (!new_list.empty())
            node_count += dump_layer(os, tree, new_list, level + 1);

        return node_count;
    }
};

}}} // namespace mdds::st::detail

#endif
