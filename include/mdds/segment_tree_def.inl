/*************************************************************************
 *
 * Copyright (c) 2015 Kohei Yoshida
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

#include <algorithm>
#include <limits>
#include <iterator>
#include <unordered_set>

namespace mdds {

namespace st { namespace detail {

template<typename T, typename InserterT>
void descend_tree_for_search(typename T::key_type point, const st::detail::node_base* pnode, InserterT& result)
{
    assert(pnode);

    typedef typename T::node leaf_node;
    typedef typename T::nonleaf_node nonleaf_node;

    if (pnode->is_leaf)
    {
        result(static_cast<const leaf_node*>(pnode)->value_leaf.data_chain.get());
        return;
    }

    const nonleaf_node* pnonleaf = static_cast<const nonleaf_node*>(pnode);
    if (point < pnonleaf->low || pnonleaf->high <= point)
        // Query point is out-of-range.
        return;

    result(pnonleaf->value_nonleaf.data_chain.get());

    // Check the left child node first, then the right one.
    st::detail::node_base* pchild = pnonleaf->left;
    if (!pchild)
        return;

    assert(pnonleaf->right ? pchild->is_leaf == pnonleaf->right->is_leaf : true);

    if (pchild->is_leaf)
    {
        // The child node are leaf nodes.
        if (point < static_cast<const leaf_node*>(pchild)->key)
        {
            // Out-of-range.  Nothing more to do.
            return;
        }

        if (pnonleaf->right)
        {
            assert(pnonleaf->right->is_leaf);
            if (static_cast<const leaf_node*>(pnonleaf->right)->key <= point)
                // Follow the right node.
                pchild = pnonleaf->right;
        }
    }
    else
    {
        // This child nodes are non-leaf nodes.

        const auto* pleaf_left = static_cast<const nonleaf_node*>(pchild);

        if (point < pleaf_left->low)
        {
            // Out-of-range.  Nothing more to do.
            return;
        }
        if (pleaf_left->high <= point && pnonleaf->right)
            // Follow the right child.
            pchild = pnonleaf->right;

        assert(
            static_cast<const nonleaf_node*>(pchild)->low <= point &&
            point < static_cast<const nonleaf_node*>(pchild)->high);
    }

    descend_tree_for_search<T, InserterT>(point, pchild, result);
}

template<typename SegStoreT>
void erase_deleted_segments(SegStoreT& segments)
{
    using segment_type = typename SegStoreT::value_type;

    SegStoreT compacted;
    for (auto&& v : segments)
    {
        if (v == segment_type{})
            continue;

        compacted.push_back(std::move(v));
    }

    segments.swap(compacted);
}

}} // namespace st::detail

template<typename KeyT, typename ValueT>
segment_tree<KeyT, ValueT>::segment_type::segment_type(key_type _start, key_type _end, value_type _value)
    : start(std::move(_start)), end(std::move(_end)), value(std::move(_value))
{}

template<typename KeyT, typename ValueT>
bool segment_tree<KeyT, ValueT>::segment_type::operator<(const segment_type& r) const
{
    if (start != r.start)
        return start < r.start;

    if (end != r.end)
        return end < r.end;

    return value < r.value;
}

template<typename KeyT, typename ValueT>
bool segment_tree<KeyT, ValueT>::segment_type::operator==(const segment_type& r) const
{
    if (start != r.start)
        return false;

    if (end != r.end)
        return false;

    return value == r.value;
}

template<typename KeyT, typename ValueT>
bool segment_tree<KeyT, ValueT>::segment_type::operator!=(const segment_type& r) const
{
    return !operator==(r);
}

template<typename KeyT, typename ValueT>
bool segment_tree<KeyT, ValueT>::search_results_base::empty() const
{
    if (!mp_res_chains)
        return true;

    // mp_res_chains only contains non-empty chain, that is, if it's not
    // empty, it does contain one or more results.
    return mp_res_chains->empty();
}

template<typename KeyT, typename ValueT>
typename segment_tree<KeyT, ValueT>::size_type segment_tree<KeyT, ValueT>::search_results_base::size() const
{
    size_type combined = 0;
    if (!mp_res_chains)
        return combined;

    for (const value_chain_type* p : *mp_res_chains)
        combined += p->size();
    return combined;
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::search_results_base::push_back_chain(value_chain_type* chain)
{
    if (!chain || chain->empty())
        return;

    if (!mp_res_chains)
        mp_res_chains.reset(new res_chains_type);
    mp_res_chains->push_back(chain);
}

template<typename KeyT, typename ValueT>
auto segment_tree<KeyT, ValueT>::const_iterator_base::operator++() -> value_type*
{
    // We don't check for end position flag for performance reasons.
    // The caller is responsible for making sure not to increment past
    // end position.

    // When reaching the end position, the internal iterators still
    // need to be pointing at the last item before the end position.
    // This is why we need to make copies of the iterators, and copy
    // them back once done.

    auto cur_pos_in_chain = m_cur_pos_in_chain;

    if (++cur_pos_in_chain == (*m_cur_chain)->end())
    {
        // End of current chain.  Inspect the next chain if exists.
        auto cur_chain = m_cur_chain;
        ++cur_chain;
        if (cur_chain == mp_res_chains->end())
        {
            m_end_pos = true;
            return nullptr;
        }
        m_cur_chain = cur_chain;
        m_cur_pos_in_chain = (*m_cur_chain)->begin();
    }
    else
        ++m_cur_pos_in_chain;

    return operator->();
}

template<typename KeyT, typename ValueT>
auto segment_tree<KeyT, ValueT>::const_iterator_base::operator--() -> value_type*
{
    if (!mp_res_chains)
        return nullptr;

    if (m_end_pos)
    {
        m_end_pos = false;
        return &cur_value();
    }

    if (m_cur_pos_in_chain == (*m_cur_chain)->begin())
    {
        if (m_cur_chain == mp_res_chains->begin())
        {
            // Already at the first data chain.  Don't move the iterator position.
            return nullptr;
        }
        --m_cur_chain;
        m_cur_pos_in_chain = (*m_cur_chain)->end();
    }
    --m_cur_pos_in_chain;
    return operator->();
}

template<typename KeyT, typename ValueT>
bool segment_tree<KeyT, ValueT>::const_iterator_base::operator==(const const_iterator_base& r) const
{
    if (mp_res_chains.get())
    {
        // non-empty result set.
        return mp_res_chains.get() == r.mp_res_chains.get() && m_cur_chain == r.m_cur_chain &&
               m_cur_pos_in_chain == r.m_cur_pos_in_chain && m_end_pos == r.m_end_pos;
    }

    // empty result set.
    if (r.mp_res_chains.get())
        return false;
    return m_end_pos == r.m_end_pos;
}

template<typename KeyT, typename ValueT>
bool segment_tree<KeyT, ValueT>::const_iterator_base::operator!=(const const_iterator_base& r) const
{
    return !operator==(r);
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::const_iterator_base::move_to_front()
{
    if (!mp_res_chains)
    {
        // Empty data set.
        m_end_pos = true;
        return;
    }

    // We assume that there is at least one chain list, and no
    // empty chain list exists.  So, skip the check.
    m_cur_chain = mp_res_chains->begin();
    m_cur_pos_in_chain = (*m_cur_chain)->begin();
    m_end_pos = false;
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::const_iterator_base::move_to_end()
{
    m_end_pos = true;
    if (!mp_res_chains)
        // Empty data set.
        return;

    m_cur_chain = mp_res_chains->end();
    --m_cur_chain;
    m_cur_pos_in_chain = (*m_cur_chain)->end();
    --m_cur_pos_in_chain;
}

template<typename KeyT, typename ValueT>
segment_tree<KeyT, ValueT>::segment_tree() : m_root_node(nullptr), m_valid_tree(false)
{}

template<typename KeyT, typename ValueT>
segment_tree<KeyT, ValueT>::segment_tree(const segment_tree& r)
    : m_segment_store(r.m_segment_store), m_root_node(nullptr), m_valid_tree(r.m_valid_tree)
{
    if (m_valid_tree)
        build_tree();
}

template<typename KeyT, typename ValueT>
segment_tree<KeyT, ValueT>::~segment_tree()
{
    clear_all_nodes();
}

template<typename KeyT, typename ValueT>
segment_tree<KeyT, ValueT>& segment_tree<KeyT, ValueT>::operator=(const segment_tree& r)
{
    segment_tree tmp(r);
    swap(tmp);
    return *this;
}

template<typename KeyT, typename ValueT>
segment_tree<KeyT, ValueT>& segment_tree<KeyT, ValueT>::operator=(segment_tree&& r) noexcept(
    std::is_nothrow_move_constructible_v<segment_tree>)
{
    segment_tree tmp(std::move(r));
    swap(tmp);
    return *this;
}

template<typename KeyT, typename ValueT>
bool segment_tree<KeyT, ValueT>::operator==(const segment_tree& r) const
{
    if (m_valid_tree != r.m_valid_tree)
        return false;

    std::unordered_set<const segment_type*> rhs;

    for (const segment_type& seg : r.m_segment_store)
    {
        if (seg == segment_type{})
            continue;

        rhs.insert(&seg);
    }

    std::size_t n_lhs = 0;

    for (const segment_type& seg : m_segment_store)
    {
        if (seg == segment_type{})
            continue;

        ++n_lhs;
        const auto* src = &seg;

        auto it = std::find_if(rhs.begin(), rhs.end(), [src](const segment_type* p) { return *src == *p; });

        if (it == rhs.end())
            return false;
    }

    return n_lhs == rhs.size();
}

template<typename KeyT, typename ValueT>
bool segment_tree<KeyT, ValueT>::operator!=(const segment_tree& r) const
{
    return !operator==(r);
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::build_tree()
{
    m_valid_tree = false;

    // Remove deleted entries first
    st::detail::erase_deleted_segments(m_segment_store);

    build_leaf_nodes();
    m_nonleaf_node_pool.clear();
    m_root_node = nullptr;

    if (!m_left_leaf)
    {
        assert(!m_right_leaf);
        return;
    }

    // Count the number of leaf nodes.
    auto leaf_count = st::detail::count_leaf_nodes<size_type>(m_left_leaf.get(), m_right_leaf.get());

    // Determine the total number of non-leaf nodes needed to build the whole tree.
    size_t nonleaf_count = st::detail::count_needed_nonleaf_nodes(leaf_count);

    m_nonleaf_node_pool.resize(nonleaf_count);

    mdds::st::detail::tree_builder<segment_tree> builder(m_nonleaf_node_pool);
    m_root_node = builder.build(m_left_leaf);

    // Start "inserting" all segments from the root.

    value_to_nodes_type tagged_node_map;

    for (value_pos_type i = 0; i < m_segment_store.size(); ++i)
    {
        const auto& v = m_segment_store[i];
        auto r = tagged_node_map.emplace(std::make_pair(i, node_chain_type{}));
        assert(r.second);

        node_chain_type& nodelist = r.first->second;
        descend_tree_and_mark(m_root_node, i, v.start, v.end, nodelist);
    }

    m_tagged_nodes_map.swap(tagged_node_map);
    m_valid_tree = true;
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::descend_tree_and_mark(
    st::detail::node_base* pnode, value_pos_type value, const key_type& start_key, const key_type& end_key,
    node_chain_type& nodelist)
{
    if (!pnode)
        return;

    if (pnode->is_leaf)
    {
        // This is a leaf node.
        node* pleaf = static_cast<node*>(pnode);
        if (start_key <= pleaf->key && pleaf->key < end_key)
        {
            leaf_value_type& v = pleaf->value_leaf;
            if (!v.data_chain)
                v.data_chain = std::make_unique<value_chain_type>();
            v.data_chain->push_back(value);
            nodelist.push_back(pnode);
        }
        return;
    }

    nonleaf_node* pnonleaf = static_cast<nonleaf_node*>(pnode);
    if (end_key < pnonleaf->low || pnonleaf->high <= start_key)
        return;

    if (start_key <= pnonleaf->low && pnonleaf->high < end_key)
    {
        // mark this non-leaf node and stop.
        nonleaf_value_type& v = pnonleaf->value_nonleaf;

        if (!v.data_chain)
            v.data_chain = std::make_unique<value_chain_type>();
        v.data_chain->push_back(value);
        nodelist.push_back(pnode);
        return;
    }

    descend_tree_and_mark(pnonleaf->left, value, start_key, end_key, nodelist);
    descend_tree_and_mark(pnonleaf->right, value, start_key, end_key, nodelist);
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::build_leaf_nodes()
{
    // NB: m_segment_store must not contain deleted segments!

    disconnect_leaf_nodes(m_left_leaf.get(), m_right_leaf.get());

    // Collect all boundary keys.
    std::vector<key_type> keys;
    keys.reserve(m_segment_store.size() * 2);

    for (const auto& v : m_segment_store)
    {
        keys.push_back(v.start);
        keys.push_back(v.end);
    }

    create_leaf_node_instances(std::move(keys), m_left_leaf, m_right_leaf);
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::create_leaf_node_instances(std::vector<key_type> keys, node_ptr& left, node_ptr& right)
{
    left.reset();
    right.reset();

    if (keys.empty() || keys.size() < 2)
        // We need at least two keys in order to build tree.
        return;

    // sort and remove duplicates.
    std::sort(keys.begin(), keys.end());
    keys.erase(std::unique(keys.begin(), keys.end()), keys.end());

    auto it = keys.cbegin();

    // left-most node
    left.reset(new node);
    left->key = *it;

    // move on to next.
    left->next.reset(new node);
    node_ptr prev_node = left;
    node_ptr cur_node = left->next;
    cur_node->prev = prev_node;

    for (++it; it != keys.cend(); ++it)
    {
        cur_node->key = *it;

        // move on to next
        cur_node->next.reset(new node);
        prev_node = cur_node;
        cur_node = cur_node->next;
        cur_node->prev = prev_node;
    }

    // Remove the excess node.
    prev_node->next.reset();
    right = prev_node;
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::insert(key_type start_key, key_type end_key, value_type value)
{
    if (start_key >= end_key)
        throw std::invalid_argument("start key cannot be greater than or equal to the end key");

    m_segment_store.emplace_back(std::move(start_key), std::move(end_key), std::move(value));
    m_valid_tree = false;
}

template<typename KeyT, typename ValueT>
typename segment_tree<KeyT, ValueT>::search_results segment_tree<KeyT, ValueT>::search(const key_type& point) const
{
    search_results result(m_segment_store);
    if (!m_valid_tree || !m_root_node)
        return result;

    search_result_inserter result_inserter(result);
    typedef segment_tree<KeyT, ValueT> tree_type;
    st::detail::descend_tree_for_search<tree_type, search_result_inserter>(point, m_root_node, result_inserter);

    return result;
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::erase(const typename search_results::const_iterator& pos)
{
    remove_value_pos(pos.cur_pos());
}

template<typename KeyT, typename ValueT>
template<typename Pred>
typename segment_tree<KeyT, ValueT>::size_type segment_tree<KeyT, ValueT>::erase_if(Pred pred)
{
    size_type n_erased = 0;

    for (size_type pos = 0; pos < m_segment_store.size(); ++pos)
    {
        const auto& seg = m_segment_store[pos];
        if (seg == segment_type())
            continue; // skip deleted segments

        if (!pred(seg.start, seg.end, seg.value))
            continue;

        remove_value_pos(pos);
        ++n_erased;
    }

    return n_erased;
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::swap(segment_tree& r) noexcept
{
    m_nonleaf_node_pool.swap(r.m_nonleaf_node_pool);
    m_segment_store.swap(r.m_segment_store);
    m_tagged_nodes_map.swap(r.m_tagged_nodes_map);
    std::swap(m_root_node, r.m_root_node);
    m_left_leaf.swap(r.m_left_leaf);
    m_right_leaf.swap(r.m_right_leaf);
    std::swap(m_valid_tree, r.m_valid_tree);
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::clear()
{
    m_tagged_nodes_map.clear();
    m_segment_store.clear();
    clear_all_nodes();
    m_valid_tree = false;
}

template<typename KeyT, typename ValueT>
typename segment_tree<KeyT, ValueT>::size_type segment_tree<KeyT, ValueT>::size() const
{
    return std::count_if(
        m_segment_store.cbegin(), m_segment_store.cend(), [](const auto& v) { return v != segment_type(); });
}

template<typename KeyT, typename ValueT>
bool segment_tree<KeyT, ValueT>::empty() const
{
    if (m_segment_store.empty())
        return true;

    // NB: take deleted segments into account
    return size() == 0;
}

template<typename KeyT, typename ValueT>
typename segment_tree<KeyT, ValueT>::size_type segment_tree<KeyT, ValueT>::leaf_size() const
{
    return st::detail::count_leaf_nodes<size_type>(m_left_leaf.get(), m_right_leaf.get());
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::remove_data_from_nodes(node_chain_type& nodelist, value_pos_type value)
{
    for (st::detail::node_base* p : nodelist)
    {
        value_chain_type* chain = nullptr;
        if (p->is_leaf)
            chain = static_cast<node*>(p)->value_leaf.data_chain.get();
        else
            chain = static_cast<nonleaf_node*>(p)->value_nonleaf.data_chain.get();

        if (!chain)
            continue;

        remove_data_from_chain(*chain, value);
    }
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::remove_data_from_chain(value_chain_type& chain, value_pos_type value)
{
    if (auto it = std::find(chain.begin(), chain.end(), value); it != chain.end())
    {
        *it = chain.back();
        chain.pop_back();
    }
}
template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::remove_value_pos(size_type pos)
{
#ifdef MDDS_SEGMENT_TREE_DEBUG
    if (pos >= m_segment_store.size())
    {
        std::ostringstream os;
        os << "specified segment position is out-of-bound: (pos=" << pos
           << "; segment-store-count=" << m_segment_store.size() << ")";
        throw std::runtime_error(os.str());
    }
#endif

    m_segment_store[pos] = segment_type();

    if (!m_valid_tree)
        return;

    if (auto it = m_tagged_nodes_map.find(pos); it != m_tagged_nodes_map.end())
    {
        // Tagged node list found.  Remove all the tags from the tree nodes.
        remove_data_from_nodes(it->second, pos);

        // Remove the tags associated with this pointer from the data set.
        m_tagged_nodes_map.erase(it);
    }
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::clear_all_nodes()
{
    disconnect_leaf_nodes(m_left_leaf.get(), m_right_leaf.get());
    m_nonleaf_node_pool.clear();
    m_left_leaf.reset();
    m_right_leaf.reset();
    m_root_node = nullptr;
}

template<typename KeyT, typename ValueT>
segment_tree<KeyT, ValueT>::tree_dumper_traits::to_string::to_string(const tree_type& _tree) : tree(_tree)
{}

template<typename KeyT, typename ValueT>
std::string segment_tree<KeyT, ValueT>::tree_dumper_traits::to_string::operator()(const leaf_type& leaf) const
{
    std::ostringstream os;
    os << leaf.to_string();

    if (leaf.value_leaf.data_chain)
    {
        os << ":{";
        for (const auto pos : *leaf.value_leaf.data_chain)
            os << tree.m_segment_store[pos].value << ',';
        os << '}';
    }
    else
        os << ":{}";

    return os.str();
}

template<typename KeyT, typename ValueT>
std::string segment_tree<KeyT, ValueT>::tree_dumper_traits::to_string::operator()(const nonleaf_type& nonleaf) const
{
    std::ostringstream os;
    os << nonleaf.to_string();

    if (nonleaf.value_nonleaf.data_chain)
    {
        os << ":{";
        for (const auto pos : *nonleaf.value_nonleaf.data_chain)
            os << tree.m_segment_store[pos].value << ',';
        os << '}';
    }
    else
        os << ":{}";

    return os.str();
}

template<typename KeyT, typename ValueT>
std::string segment_tree<KeyT, ValueT>::to_string() const
{
    std::ostringstream os;

    os << "valid tree: " << (m_valid_tree ? "true" : "false") << std::endl;
    os << "stored segments:" << std::endl;

    for (const segment_type& v : m_segment_store)
    {
        if (v == segment_type())
            // skip deleted segment
            continue;

        os << "  - range: '[" << v.start << '-' << v.end << ")'" << std::endl;
        os << "    value: " << v.value << std::endl;
    }

    if (!m_valid_tree)
        return os.str();

    std::size_t node_count = mdds::st::detail::tree_dumper<tree_dumper_traits>::dump(os, *this, m_root_node);
    os << "tree node count: " << node_count;

    return os.str();
}

template<typename KeyT, typename ValueT>
auto segment_tree<KeyT, ValueT>::boundary_keys() const -> std::vector<key_type>
{
    std::vector<key_type> keys;

    for (const auto& seg : m_segment_store)
    {
        if (seg == segment_type{})
            continue;

        keys.push_back(seg.start);
        keys.push_back(seg.end);
    }

    std::sort(keys.begin(), keys.end());
    auto last = std::unique(keys.begin(), keys.end());
    keys.erase(last, keys.end());

    return keys;
}

template<typename KeyT, typename ValueT>
void segment_tree<KeyT, ValueT>::check_integrity(const integrity_check_properties& props) const
{
    node* cur_node = m_left_leaf.get();

    const auto& segstore = m_segment_store;
    auto to_string = [&segstore](const value_chain_type& chain) -> std::string {
        std::ostringstream os;
        for (auto pos : chain)
            os << segstore[pos].value << ", ";
        return os.str();
    };

    auto has_value_index = [](const st::detail::node_base* pnode, value_pos_type value_index) {
        const value_chain_type* chain = nullptr;

        if (pnode->is_leaf)
            chain = static_cast<const node*>(pnode)->value_leaf.data_chain.get();
        else
            chain = static_cast<const nonleaf_node*>(pnode)->value_nonleaf.data_chain.get();

        if (!chain)
            return false;

        if (std::find(chain->begin(), chain->end(), value_index) == chain->end())
            return false;

        return true;
    };

    // Check the integrity of the leaf nodes first.

    for (const auto& node_check : props.leaf_nodes)
    {
        value_chain_type chain_expected;

        for (const auto& v : node_check.value_chain)
        {
            auto it = std::find_if(
                m_segment_store.cbegin(), m_segment_store.cend(), [&v](const auto& seg) { return seg.value == v; });

            if (it == m_segment_store.cend())
            {
                std::ostringstream os;
                os << "value '" << v << "' is not stored in this instance";
                throw std::invalid_argument(os.str());
            }

            chain_expected.push_back(std::distance(m_segment_store.cbegin(), it));
        }

        assert(chain_expected.size() == node_check.value_chain.size());

        if (!cur_node)
            throw integrity_error("current node position is past the right-most node which is unexpected");

        if (cur_node->key != node_check.key)
        {
            std::ostringstream os;
            os << "key values differ: expected='" << node_check.key << "'; actual='" << cur_node->key << "'";
            throw integrity_error(os.str());
        }

        if (chain_expected.empty())
        {
            if (cur_node->value_leaf.data_chain)
                throw integrity_error("data chain should be empty i.e. its pointer value should be nullptr");
        }
        else
        {
            if (!cur_node->value_leaf.data_chain)
                throw integrity_error("node has empty data chain but it should have a non-empty data chain");

            value_chain_type chain_actual = *cur_node->value_leaf.data_chain; // copy

            // Sort both arrays before comparing them.
            std::sort(chain_expected.begin(), chain_expected.end());
            std::sort(chain_actual.begin(), chain_actual.end());

            if (chain_expected != chain_actual)
            {
                std::ostringstream os;
                os << "data chain of leaf node with key '" << cur_node->key << "' is not as expected:" << std::endl;
                os << "  expected: " << to_string(chain_expected) << std::endl;
                os << "  actual: " << to_string(chain_actual);
                throw integrity_error(os.str());
            }
        }

        cur_node = cur_node->next.get();
    }

    if (cur_node)
        // At this point, we expect the current node to be at the position
        // past the right-most node, which is nullptr.
        throw integrity_error("current node position was expected to be past the right-most node but it isn't");

    // Check the integrity of the tagged-node map.

    for (const auto& [value_index, nodes] : m_tagged_nodes_map)
    {
        for (const st::detail::node_base* this_node : nodes)
        {
            if (!has_value_index(this_node, value_index))
            {
                std::ostringstream os;
                os << "invalid tagged node map: node (" << this_node << ") does not contain a value index of "
                   << value_index;
                throw integrity_error(os.str());
            }
        }
    }
}

} // namespace mdds
