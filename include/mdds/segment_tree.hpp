/*************************************************************************
 *
 * Copyright (c) 2010-2015 Kohei Yoshida
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

#ifndef INCLUDED_MDDS_SEGMENTTREE_HPP
#define INCLUDED_MDDS_SEGMENTTREE_HPP

#include "mdds/node.hpp"
#include "mdds/global.hpp"

#include <vector>
#include <deque>
#include <iostream>
#include <map>
#include <unordered_map>
#include <memory>
#include <sstream>

namespace mdds {

template<typename KeyT, typename ValueT>
class segment_tree
{
public:
    typedef KeyT key_type;
    typedef ValueT value_type;
    typedef std::size_t size_type;
    typedef std::vector<value_type> search_results_type;

private:
    struct segment_type
    {
        key_type start;
        key_type end;
        value_type value;

        segment_type();
        segment_type(key_type _start, key_type _end, value_type _value);
        segment_type(const segment_type&) = default;
        segment_type(segment_type&&) = default;

        segment_type& operator=(const segment_type& r) = default;
        bool operator<(const segment_type& r) const;
        bool operator==(const segment_type& r) const;
    };

    using segment_store_type = std::deque<segment_type>;
    using value_pos_type = typename segment_store_type::size_type;
    using data_chain_type = std::vector<value_pos_type>;

    using node_list_type = std::vector<st::detail::node_base*>;
    using data_node_map_type = std::map<value_pos_type, std::unique_ptr<node_list_type>>;

    struct nonleaf_value_type
    {
        std::unique_ptr<data_chain_type> data_chain;

        nonleaf_value_type()
        {}
        nonleaf_value_type(const nonleaf_value_type& r)
        {
            if (r.data_chain)
                data_chain = std::make_unique<data_chain_type>(*r.data_chain);
        }

        bool operator==(const nonleaf_value_type& r) const
        {
            return data_chain == r.data_chain;
        }

        ~nonleaf_value_type()
        {}
    };

    struct leaf_value_type
    {
        std::unique_ptr<data_chain_type> data_chain;

        leaf_value_type()
        {}
        leaf_value_type(const leaf_value_type& r)
        {
            if (r.data_chain)
                data_chain = std::make_unique<data_chain_type>(*r.data_chain);
        }

        bool operator==(const leaf_value_type& r) const
        {
            return data_chain == r.data_chain;
        }

        ~leaf_value_type()
        {}
    };

public:
    using node = st::detail::node<key_type, leaf_value_type>;
    using node_ptr = typename node::node_ptr;
    using nonleaf_node = typename st::detail::nonleaf_node<key_type, nonleaf_value_type>;

private:
    class search_result_inserter;

    /**
     * This base class takes care of collecting data chain pointers during
     * tree descend for search.
     */
    class search_results_base
    {
        friend class search_result_inserter;

    public:
        typedef std::vector<data_chain_type*> res_chains_type;
        typedef std::shared_ptr<res_chains_type> res_chains_ptr;

    protected:
        search_results_base(const segment_store_type& segment_store) : m_segment_store(&segment_store)
        {}

        search_results_base(const search_results_base& r)
            : m_segment_store(r.m_segment_store), mp_res_chains(r.mp_res_chains)
        {}

        bool empty() const
        {
            if (!mp_res_chains)
                return true;

            // mp_res_chains only contains non-empty chain, that is, if it's not
            // empty, it does contain one or more results.
            return mp_res_chains->empty();
        }

        size_type size() const
        {
            size_type combined = 0;
            if (!mp_res_chains)
                return combined;

            for (const data_chain_type* p : *mp_res_chains)
                combined += p->size();
            return combined;
        }

        void push_back_chain(data_chain_type* chain)
        {
            if (!chain || chain->empty())
                return;

            if (!mp_res_chains)
                mp_res_chains.reset(new res_chains_type);
            mp_res_chains->push_back(chain);
        }

        const segment_store_type* get_segment_store() const
        {
            return m_segment_store;
        }

        const res_chains_ptr& get_res_chains() const
        {
            return mp_res_chains;
        }

    private:
        const segment_store_type* m_segment_store = nullptr;
        res_chains_ptr mp_res_chains;
    };

    class const_iterator_base
    {
    protected:
        typedef typename search_results_base::res_chains_type res_chains_type;
        typedef typename search_results_base::res_chains_ptr res_chains_ptr;

        const_iterator_base(const segment_store_type* segment_store, const res_chains_ptr& p)
            : m_segment_store(segment_store), mp_res_chains(p), m_end_pos(true)
        {}

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = const segment_tree::segment_type;
        using pointer = value_type*;
        using reference = value_type&;
        using difference_type = std::ptrdiff_t;

        const_iterator_base()
        {}

        const_iterator_base(const const_iterator_base& r)
            : m_segment_store(r.m_segment_store), mp_res_chains(r.mp_res_chains), m_cur_chain(r.m_cur_chain),
              m_cur_pos_in_chain(r.m_cur_pos_in_chain), m_end_pos(r.m_end_pos)
        {}

        const_iterator_base& operator=(const const_iterator_base& r)
        {
            m_segment_store = r.m_segment_store;
            mp_res_chains = r.mp_res_chains;
            m_cur_chain = r.m_cur_chain;
            m_cur_pos_in_chain = r.m_cur_pos_in_chain;
            m_end_pos = r.m_end_pos;
            return *this;
        }

        value_type* operator++()
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

        value_type* operator--()
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

        bool operator==(const const_iterator_base& r) const
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

        bool operator!=(const const_iterator_base& r) const
        {
            return !operator==(r);
        }

        value_type& operator*()
        {
            return cur_value();
        }

        const value_type* operator->()
        {
            return &cur_value();
        }

    protected:
        void move_to_front()
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

        void move_to_end()
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

    private:
        value_type& cur_value()
        {
            auto pos = *m_cur_pos_in_chain;
            return (*m_segment_store)[pos];
        }

        const segment_store_type* m_segment_store = nullptr;
        res_chains_ptr mp_res_chains;
        typename res_chains_type::const_iterator m_cur_chain;
        typename data_chain_type::const_iterator m_cur_pos_in_chain;
        bool m_end_pos = true;
    };

    class search_result_inserter
    {
    public:
        search_result_inserter(search_results_base& results) : m_results(results)
        {}
        void operator()(data_chain_type* node_data)
        {
            if (!node_data)
                return;

            m_results.push_back_chain(node_data);
        }

    private:
        search_results_base& m_results;
    };

public:
    class search_results : public search_results_base
    {
        friend class segment_tree;

        typedef typename search_results_base::res_chains_type res_chains_type;
        typedef typename search_results_base::res_chains_ptr res_chains_ptr;

        search_results(const segment_store_type& segment_tree) : search_results_base(segment_tree)
        {}

    public:
        class const_iterator : public const_iterator_base
        {
            friend class segment_tree<KeyT, ValueT>::search_results;

        private:
            const_iterator(const segment_store_type* segment_store, const res_chains_ptr& p)
                : const_iterator_base(segment_store, p)
            {}

        public:
            const_iterator() : const_iterator_base()
            {}

            const_iterator(const const_iterator& r) : const_iterator_base(r)
            {}

            const_iterator& operator=(const const_iterator& r)
            {
                const_iterator_base::operator=(r);
                return *this;
            }
        };

        /**
         * Check if this results pool is empty or not.
         *
         * @return true if this results pool is empty, otherwise false.
         */
        bool empty() const
        {
            return search_results_base::empty();
        }

        /**
         * Count the number of results in this results pool.
         *
         * @return number of results in this results pool.
         */
        size_type size() const
        {
            return search_results_base::size();
        }

        typename search_results::const_iterator begin() const
        {
            typename search_results::const_iterator it(
                search_results_base::get_segment_store(), search_results_base::get_res_chains());
            it.move_to_front();
            return it;
        }

        typename search_results::const_iterator end() const
        {
            typename search_results::const_iterator it(
                search_results_base::get_segment_store(), search_results_base::get_res_chains());
            it.move_to_end();
            return it;
        }
    };

    segment_tree();
    segment_tree(const segment_tree& r);
    ~segment_tree();

    /**
     * Check equality with another instance.
     *
     * @note Equality between two segment_tree instances is evaluated
     * by comparing the logically stored segments only; the tree parts of the
     * structures are not compared.
     */
    bool operator==(const segment_tree& r) const;

    bool operator!=(const segment_tree& r) const
    {
        return !operator==(r);
    }

    /**
     * Check whether or not the internal tree is in a valid state.  The tree
     * must be valid in order to perform searches.
     *
     * @return true if the tree is valid, false otherwise.
     */
    bool is_tree_valid() const
    {
        return m_valid_tree;
    }

    /**
     * Build or re-build tree based on the current set of segments.
     */
    void build_tree();

    /**
     * Insert a new segment.  Duplicate segments are allowed.
     *
     * @param start_key start key of a segment.  The value is inclusive.
     * @param end_key end key of a segment.  The value is non-inclusive.
     *                It must be greater than the start key.
     * @param value value to associate with this segment.
     */
    void insert(key_type start_key, key_type end_key, value_type value);

    /**
     * Search the tree and collect all segments that include a specified
     * point.
     *
     * @param point specified point value
     *
     * @return object containing the result of the search, which can be
     *         accessed via iterator.
     */
    search_results search(const key_type& point) const;

    /**
     * Remove a segment that matches by the value.  This will <i>not</i>
     * invalidate the tree; however, if you have removed a large amount of
     * of segments, you might want to re-build the tree to compact its size.
     *
     * @param value Value to remove a segment by.
     *
     * @todo This needs to be replaced with better alternatives.
     */
    void remove(const value_type& value);

    /**
     * Remove all segments data.
     */
    void clear();

    /**
     * Return the number of segments currently stored in this container.
     */
    size_t size() const;

    /**
     * Return whether or not the container stores any segments or none at all.
     */
    bool empty() const;

    /**
     * Return the number of leaf nodes.
     *
     * @return number of leaf nodes.
     */
    size_t leaf_size() const;

    /**
     * Create a string representation of the internal state of a tree.
     *
     * @return String representation of the internal state of a tree.
     */
    std::string to_string() const;

    struct integrity_check_properties
    {
        struct leaf_node
        {
            key_type key = {};
            std::vector<value_type> value_chain;
        };

        std::vector<leaf_node> leaf_nodes;
    };

    void check_integrity(const integrity_check_properties& props) const;

private:
    /**
     * To be called from rectangle_set.
     */
    void search(key_type point, search_results_base& result) const;

    static void create_leaf_node_instances(std::vector<key_type> keys, node_ptr& left, node_ptr& right);

    /**
     * Descend the tree from the root node, and mark appropriate nodes, both
     * leaf and non-leaf, based on segment's end points.  When marking nodes,
     * record their positions as a list of node pointers.
     */
    void descend_tree_and_mark(
        st::detail::node_base* pnode, value_pos_type value, key_type start_key, key_type end_key,
        node_list_type* plist);

    void build_leaf_nodes();

    /**
     * Go through the list of nodes, and remove the specified data pointer
     * value from the nodes.
     */
    void remove_data_from_nodes(node_list_type* plist, value_pos_type value);
    void remove_data_from_chain(data_chain_type& chain, value_pos_type value);

    void clear_all_nodes();

private:
    struct tree_dumper_traits
    {
        using leaf_type = node;
        using nonleaf_type = nonleaf_node;
        using tree_type = segment_tree;

        struct to_string
        {
            const tree_type& tree;

            to_string(const tree_type& _tree);
            std::string operator()(const leaf_type& leaf) const;
            std::string operator()(const nonleaf_type& nonleaf) const;
        };
    };

    std::vector<nonleaf_node> m_nonleaf_node_pool;

    /**
     * Storage for inserted segments.  Note that real values are only stored
     * here. The rest of the structure only stores indices into this storage to
     * find the values when needed.
     */
    segment_store_type m_segment_store;

    /**
     * For each segment index, it keeps track of all nodes, leaf or non-leaf,
     * that are marked with that index.  This is used for removing segments by
     * value.
     */
    data_node_map_type m_tagged_node_map;

    nonleaf_node* m_root_node;
    node_ptr m_left_leaf;
    node_ptr m_right_leaf;
    bool m_valid_tree;
};

} // namespace mdds

#include "segment_tree_def.inl"

#endif
