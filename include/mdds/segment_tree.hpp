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
#include <iostream>
#include <map>
#include <unordered_map>
#include <memory>

#ifdef MDDS_UNIT_TEST
#include <sstream>
#endif

namespace mdds {

template<typename _Key, typename _Value>
class rectangle_set;

template<typename _Key, typename _Value>
class segment_tree
{
    friend class rectangle_set<_Key, _Value>;
public:
    typedef _Key        key_type;
    typedef _Value      value_type;
    typedef size_t      size_type;
    typedef ::std::vector<value_type> search_result_type;

#ifdef MDDS_UNIT_TEST
    struct segment_data
    {
        key_type    begin_key;
        key_type    end_key;
        value_type   pdata;

        segment_data(key_type _beg, key_type _end, value_type p) :
            begin_key(_beg), end_key(_end), pdata(p) {}

        bool operator==(const segment_data& r) const
        {
            return begin_key == r.begin_key && end_key == r.end_key && pdata == r.pdata;
        }

        bool operator!=(const segment_data& r) const
        {
            return !operator==(r);
        }
    };

    struct segment_map_printer : public ::std::unary_function< ::std::pair<value_type, ::std::pair<key_type, key_type> >, void>
    {
        void operator() (const ::std::pair<value_type, ::std::pair<key_type, key_type> >& r) const
        {
            using namespace std;
            cout << r.second.first << "-" << r.second.second << ": " << r.first->name << endl;
        }
    };
#endif

public:
    typedef ::std::vector<value_type> data_chain_type;
    typedef std::unordered_map<value_type, ::std::pair<key_type, key_type> > segment_map_type;
    typedef ::std::map<value_type, ::std::pair<key_type, key_type> >               sorted_segment_map_type;

    struct nonleaf_value_type
    {
        key_type low;   /// low range value (inclusive)
        key_type high;  /// high range value (non-inclusive)
        data_chain_type* data_chain;

        bool operator== (const nonleaf_value_type& r) const
        {
            return low == r.low && high == r.high && data_chain == r.data_chain;
        }
    };

    struct leaf_value_type
    {
        key_type key;
        data_chain_type* data_chain;

        bool operator== (const leaf_value_type& r) const
        {
            return key == r.key && data_chain == r.data_chain;
        }
    };

    struct fill_nonleaf_value_handler;
    struct init_handler;
    struct dispose_handler;
#ifdef MDDS_UNIT_TEST
    struct to_string_handler;
#endif

    typedef __st::node<segment_tree> node;
    typedef typename node::node_ptr node_ptr;

    typedef typename __st::nonleaf_node<segment_tree> nonleaf_node;

    struct fill_nonleaf_value_handler
    {
        void operator() (__st::nonleaf_node<segment_tree>& _self, const __st::node_base* left_node, const __st::node_base* right_node)
        {
            // Parent node should carry the range of all of its child nodes.
            if (left_node)
            {
                _self.value_nonleaf.low  = left_node->is_leaf ?
                    static_cast<const node*>(left_node)->value_leaf.key :
                    static_cast<const nonleaf_node*>(left_node)->value_nonleaf.low;
            }
            else
            {
                // Having a left node is prerequisite.
                throw general_error("segment_tree::fill_nonleaf_value_handler: Having a left node is prerequisite.");
            }

            if (right_node)
            {
                if (right_node->is_leaf)
                {
                    // When the child nodes are leaf nodes, the upper bound
                    // must be the value of the node that comes after the
                    // right leaf node (if such node exists).

                    const node* p = static_cast<const node*>(right_node);
                    if (p->next)
                        _self.value_nonleaf.high = p->next->value_leaf.key;
                    else
                        _self.value_nonleaf.high = p->value_leaf.key;
                }
                else
                {
                    _self.value_nonleaf.high = static_cast<const nonleaf_node*>(right_node)->value_nonleaf.high;
                }
            }
            else
            {
                _self.value_nonleaf.high = left_node->is_leaf ?
                    static_cast<const node*>(left_node)->value_leaf.key :
                    static_cast<const nonleaf_node*>(left_node)->value_nonleaf.high;
            }
        }
    };

#ifdef MDDS_UNIT_TEST
    struct to_string_handler
    {
        std::string operator() (const node& _self) const
        {
            std::ostringstream os;
            os << "[" << _self.value_leaf.key << "] ";
            return os.str();
        }

        std::string operator() (const __st::nonleaf_node<segment_tree>& _self) const
        {
            std::ostringstream os;
            os << "[" << _self.value_nonleaf.low << "-" << _self.value_nonleaf.high << ")";
            if (_self.value_nonleaf.data_chain)
            {
                os << " { ";
                typename data_chain_type::const_iterator
                    itr,
                    itr_beg = _self.value_nonleaf.data_chain->begin(),
                    itr_end = _self.value_nonleaf.data_chain->end();
                for (itr = itr_beg; itr != itr_end; ++itr)
                {
                    if (itr != itr_beg)
                        os << ", ";
                    os << (*itr)->name;
                }
                os << " }";
            }
            os << " ";
            return os.str();
        }
    };
#endif

    struct init_handler
    {
        void operator() (node& _self)
        {
            _self.value_leaf.data_chain = nullptr;
        }

        void operator() (__st::nonleaf_node<segment_tree>& _self)
        {
            _self.value_nonleaf.data_chain = nullptr;
        }
    };

    struct dispose_handler
    {
        void operator() (node& _self)
        {
            delete _self.value_leaf.data_chain;
        }

        void operator() (__st::nonleaf_node<segment_tree>& _self)
        {
            delete _self.value_nonleaf.data_chain;
        }
    };

#ifdef MDDS_UNIT_TEST
    struct node_printer : public ::std::unary_function<const __st::node_base*, void>
    {
        void operator() (const __st::node_base* p) const
        {
            if (p->is_leaf)
                std::cout << static_cast<const node*>(p)->to_string() << " ";
            else
                std::cout << static_cast<const nonleaf_node*>(p)->to_string() << " ";
        }
    };
#endif

private:

    /**
     * This base class takes care of collecting data chain pointers during
     * tree descend for search.
     */
    class search_result_base
    {
    public:
        typedef std::vector<data_chain_type*>       res_chains_type;
        typedef std::shared_ptr<res_chains_type>    res_chains_ptr;
    public:

        search_result_base() :
            mp_res_chains(static_cast<res_chains_type*>(nullptr)) {}

        search_result_base(const search_result_base& r) :
            mp_res_chains(r.mp_res_chains) {}

        size_t size() const
        {
            size_t combined = 0;
            if (!mp_res_chains)
                return combined;

            typename res_chains_type::const_iterator
                itr = mp_res_chains->begin(), itr_end = mp_res_chains->end();
            for (; itr != itr_end; ++itr)
                combined += (*itr)->size();
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

    res_chains_ptr& get_res_chains() { return mp_res_chains; }

    private:
        res_chains_ptr  mp_res_chains;
    };

    class iterator_base
    {
    protected:
        typedef typename search_result_base::res_chains_type res_chains_type;
        typedef typename search_result_base::res_chains_ptr res_chains_ptr;

        iterator_base(const res_chains_ptr& p) :
            mp_res_chains(p), m_end_pos(true) {}

    public:
        typedef ::std::bidirectional_iterator_tag           iterator_category;
        typedef typename data_chain_type::value_type        value_type;
        typedef typename data_chain_type::pointer           pointer;
        typedef typename data_chain_type::reference         reference;
        typedef typename data_chain_type::difference_type   difference_type;

        iterator_base() :
            mp_res_chains(static_cast<res_chains_type*>(nullptr)), m_end_pos(true) {}

        iterator_base(const iterator_base& r) :
            mp_res_chains(r.mp_res_chains),
            m_cur_chain(r.m_cur_chain),
            m_cur_pos_in_chain(r.m_cur_pos_in_chain),
            m_end_pos(r.m_end_pos) {}

        iterator_base& operator= (const iterator_base& r)
        {
            mp_res_chains = r.mp_res_chains;
            m_cur_chain = r.m_cur_chain;
            m_cur_pos_in_chain = r.m_cur_pos_in_chain;
            m_end_pos = r.m_end_pos;
            return *this;
        }

        typename data_chain_type::value_type* operator++ ()
        {
            // We don't check for end position flag for performance reasons.
            // The caller is responsible for making sure not to increment past
            // end position.

            // When reaching the end position, the internal iterators still
            // need to be pointing at the last item before the end position.
            // This is why we need to make copies of the iterators, and copy
            // them back once done.

            typename data_chain_type::iterator cur_pos_in_chain = m_cur_pos_in_chain;

            if (++cur_pos_in_chain == (*m_cur_chain)->end())
            {
                // End of current chain.  Inspect the next chain if exists.
                typename res_chains_type::iterator cur_chain = m_cur_chain;
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

        typename data_chain_type::value_type* operator-- ()
        {
            if (!mp_res_chains)
                return nullptr;

            if (m_end_pos)
            {
                m_end_pos = false;
                return &(*m_cur_pos_in_chain);
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

        bool operator== (const iterator_base& r) const
        {
            if (mp_res_chains.get())
            {
                // non-empty result set.
                return mp_res_chains.get() == r.mp_res_chains.get() &&
                    m_cur_chain == r.m_cur_chain && m_cur_pos_in_chain == r.m_cur_pos_in_chain &&
                    m_end_pos == r.m_end_pos;
            }

            // empty result set.
            if (r.mp_res_chains.get())
                return false;
            return m_end_pos == r.m_end_pos;
        }

        bool operator!= (const iterator_base& r) const { return !operator==(r); }

        typename data_chain_type::value_type& operator*()
        {
            return *m_cur_pos_in_chain;
        }

        typename data_chain_type::value_type* operator->()
        {
            return &(*m_cur_pos_in_chain);
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
        res_chains_ptr mp_res_chains;
        typename res_chains_type::iterator  m_cur_chain;
        typename data_chain_type::iterator  m_cur_pos_in_chain;
        bool m_end_pos:1;
    };

public:

    class search_result : public search_result_base
    {
        typedef typename search_result_base::res_chains_type res_chains_type;
        typedef typename search_result_base::res_chains_ptr res_chains_ptr;
    public:

        class iterator : public iterator_base
        {
            friend class segment_tree<_Key,_Value>::search_result;
        private:
            iterator(const res_chains_ptr& p) : iterator_base(p) {}
        public:
            iterator() : iterator_base() {}
        };

        typename search_result::iterator begin()
        {
            typename search_result::iterator itr(search_result_base::get_res_chains());
            itr.move_to_front();
            return itr;
        }

        typename search_result::iterator end()
        {
            typename search_result::iterator itr(search_result_base::get_res_chains());
            itr.move_to_end();
            return itr;
        }
    };

    class search_result_vector_inserter : public ::std::unary_function<data_chain_type*, void>
    {
    public:
        search_result_vector_inserter(search_result_type& result) : m_result(result) {}
        void operator() (data_chain_type* node_data)
        {
            if (!node_data)
                return;

            typename data_chain_type::const_iterator itr = node_data->begin(), itr_end = node_data->end();
            for (; itr != itr_end; ++itr)
                m_result.push_back(*itr);
        }
    private:
        search_result_type& m_result;
    };

    class search_result_inserter : public ::std::unary_function<data_chain_type*, void>
    {
    public:
        search_result_inserter(search_result_base& result) : m_result(result) {}
        void operator() (data_chain_type* node_data)
        {
            if (!node_data)
                return;

            m_result.push_back_chain(node_data);
        }
    private:
        search_result_base& m_result;
    };

    segment_tree();
    segment_tree(const segment_tree& r);
    ~segment_tree();

    /**
     * Equality between two segment_tree instances is evaluated by comparing
     * the segments that they store.  The trees are not compared.
     */
    bool operator==(const segment_tree& r) const;

    bool operator!=(const segment_tree& r) const { return !operator==(r); }

    /**
     * Check whether or not the internal tree is in a valid state.  The tree
     * must be valid in order to perform searches.
     *
     * @return true if the tree is valid, false otherwise.
     */
    bool is_tree_valid() const { return m_valid_tree; }

    /**
     * Build or re-build tree based on the current set of segments.
     */
    void build_tree();

    /**
     * Insert a new segment.
     *
     * @param begin_key begin point of the segment.  The value is inclusive.
     * @param end_key end point of the segment.  The value is non-inclusive.
     * @param pdata pointer to the data instance associated with this segment.
     *               Note that <i>the caller must manage the life cycle of the
     *               data instance</i>.
     */
    bool insert(key_type begin_key, key_type end_key, value_type pdata);

    /**
     * Search the tree and collect all segments that include a specified
     * point.
     *
     * @param point specified point value
     * @param result doubly-linked list of data instances associated with
     *                   the segments that include the specified point.
     *                   <i>Note that the search result gets appended to the
     *                   list; the list will not get emptied on each
     *                   search.</i>  It is caller's responsibility to empty
     *                   the list before passing it to this method in case the
     *                   caller so desires.
     *
     * @return true if the search is performed successfully, false if the
     *         search has ended prematurely due to error conditions.
     */
    bool search(key_type point, search_result_type& result) const;

    /**
     * Search the tree and collect all segments that include a specified
     * point.
     *
     * @param point specified point value
     *
     * @return object containing the result of the search, which can be
     *         accessed via iterator.
     */
    search_result search(key_type point) const;

    /**
     * Remove a segment that matches by the value.  This will <i>not</i>
     * invalidate the tree; however, if you have removed lots of segments, you
     * might want to re-build the tree to shrink its size.
     *
     * @param value value to remove a segment by.
     */
    void remove(value_type value);

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

#ifdef MDDS_UNIT_TEST
    void dump_tree() const;
    void dump_leaf_nodes() const;
    void dump_segment_data() const;
    bool verify_node_lists() const;

    struct leaf_node_check
    {
        key_type key;
        data_chain_type data_chain;
    };

    bool verify_leaf_nodes(const ::std::vector<leaf_node_check>& checks) const;

    /**
     * Verify the validity of the segment data array.
     *
     * @param checks null-terminated array of expected values.  The last item
     *               must have a nullptr pdata value to terminate the array.
     */
    bool verify_segment_data(const segment_map_type& checks) const;
#endif

private:
    /**
     * To be called from rectangle_set.
     */
    void search(key_type point, search_result_base& result) const;

    typedef std::vector<__st::node_base*> node_list_type;
    typedef std::map<value_type, std::unique_ptr<node_list_type>> data_node_map_type;

    static void create_leaf_node_instances(const ::std::vector<key_type>& keys, node_ptr& left, node_ptr& right);

    /**
     * Descend the tree from the root node, and mark appropriate nodes, both
     * leaf and non-leaf, based on segment's end points.  When marking nodes,
     * record their positions as a list of node pointers.
     */
    void descend_tree_and_mark(
        __st::node_base* pnode, value_type pdata, key_type begin_key, key_type end_key, node_list_type* plist);

    void build_leaf_nodes();

    /**
     * Go through the list of nodes, and remove the specified data pointer
     * value from the nodes.
     */
    void remove_data_from_nodes(node_list_type* plist, const value_type pdata);
    void remove_data_from_chain(data_chain_type& chain, const value_type pdata);

    void clear_all_nodes();

#ifdef MDDS_UNIT_TEST
    static bool has_data_pointer(const node_list_type& node_list, const value_type pdata);
    static void print_leaf_value(const leaf_value_type& v);
#endif

private:
    std::vector<nonleaf_node> m_nonleaf_node_pool;

    segment_map_type m_segment_data;

    /**
     * For each data pointer, it keeps track of all nodes, leaf or non-leaf,
     * that stores the data pointer label.  This data is used when removing
     * segments by the data pointer value.
     */
    data_node_map_type m_tagged_node_map;

    nonleaf_node* m_root_node;
    node_ptr   m_left_leaf;
    node_ptr   m_right_leaf;
    bool m_valid_tree:1;
};

}

#include "segment_tree_def.inl"

#endif
