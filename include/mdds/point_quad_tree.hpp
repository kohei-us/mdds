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

#ifndef INCLUDED_MDDS_POINT_QUAD_TREE_HPP
#define INCLUDED_MDDS_POINT_QUAD_TREE_HPP

#include "mdds/quad_node.hpp"

#include <cstdlib>
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

#define DEBUG_POINT_QUAD_TREE 0

namespace mdds {

template<typename _PairType>
void ensure_order(_PairType& v)
{
    if (v.first > v.second)
        ::std::swap(v.first, v.second);
}

template<typename _Key, typename _NodeType, typename _Inserter>
void search_region_node(
    const _NodeType* p, _Key x1, _Key y1, _Key x2, _Key y2, _Inserter& result)
{
    using namespace std;

    if (!p)
        return;

    search_region_space_t region = ::mdds::get_search_region_space(p, x1, y1, x2, y2);
    
    switch (region)
    {
        case region_center:
            result(p);
            search_region_node(p->northeast.get(), x1, y1, x2, y2, result);
            search_region_node(p->northwest.get(), x1, y1, x2, y2, result);
            search_region_node(p->southeast.get(), x1, y1, x2, y2, result);
            search_region_node(p->southwest.get(), x1, y1, x2, y2, result);
            break;
        case region_east:
            search_region_node(p->northwest.get(), x1, y1, x2, y2, result);
            search_region_node(p->southwest.get(), x1, y1, x2, y2, result);
            break;
        case region_north:
            search_region_node(p->southeast.get(), x1, y1, x2, y2, result);
            search_region_node(p->southwest.get(), x1, y1, x2, y2, result);
            break;
        case region_northeast:
            search_region_node(p->southwest.get(), x1, y1, x2, y2, result);
            break;
        case region_northwest:
            search_region_node(p->southeast.get(), x1, y1, x2, y2, result);
            break;
        case region_south:
            search_region_node(p->northeast.get(), x1, y1, x2, y2, result);
            search_region_node(p->northwest.get(), x1, y1, x2, y2, result);
            break;
        case region_southeast:
            search_region_node(p->northwest.get(), x1, y1, x2, y2, result);
            break;
        case region_southwest:
            search_region_node(p->northeast.get(), x1, y1, x2, y2, result);
            break;
        case region_west:
            search_region_node(p->northeast.get(), x1, y1, x2, y2, result);
            search_region_node(p->southeast.get(), x1, y1, x2, y2, result);
            break;
        default:
            throw general_error("unknown search region");
    }
}


template<typename _Key, typename _Value>
class point_quad_tree
{
private:
    class search_result_inserter;

public:
    typedef _Key    key_type;
    typedef _Value  value_type;
    typedef size_t  size_type;
    typedef ::std::vector<value_type> data_array_type;

    class data_not_found : public ::std::exception {};

private:
    struct node;
    typedef ::boost::intrusive_ptr<node> node_ptr;

    struct node : quad_node_base<node_ptr, node, key_type>
    {
        value_type data;
        node(key_type _x, key_type _y, value_type _data) :
            quad_node_base<node_ptr, node, key_type>(_x, _y),
            data(_data) {}

        node(const node& r) :
            quad_node_base<node_ptr, node, key_type>(r),
            data(r.data) {}

        void dispose() {}

        bool operator== (const node& r) const
        {
            return quad_node_base<node_ptr, node, key_type>::operator ==(r) && data == r.data;
        }

        node& operator= (const node& r)
        {
            quad_node_base<node_ptr, node, key_type>::operator=(r);
            data = r.data;
            return *this;
        }
    };

    typedef ::std::vector<node_ptr> reinsert_tree_array_type;
    typedef ::std::pair<key_type, key_type> key_range_type;

public:

    /**
     * Node wrapper to allow read-only access to the internal quad node
     * structure.
     */
    class node_access
    {
        friend class point_quad_tree<_Key,_Value>;
    public:
        node_access northeast() const { return node_access(mp->northeast.get()); }
        node_access northwest() const { return node_access(mp->northwest.get()); }
        node_access southeast() const { return node_access(mp->southeast.get()); }
        node_access southwest() const { return node_access(mp->southwest.get()); }

        value_type data() const { return mp->data; }
        key_type x() const { return mp->x; }
        key_type y() const { return mp->y; }

        operator bool() const { return mp != nullptr; }
        bool operator== (const node_access& r) const { return mp == r.mp; }

        node_access& operator= (const node_access& r)
        {
            mp = r.mp;
            return *this;
        }

        node_access() : mp(nullptr) {}
        node_access(const node_access& r) : mp(r.mp) {}
        ~node_access() {}

    private:
        node_access(const node* p) : mp(p) {}

    private:
        const node* mp;
    };

    struct point
    {
        key_type x;
        key_type y;
        point(key_type _x, key_type _y) : x(_x), y(_y) {}
        point() : x(0), y(0) {}
    };

    class search_results
    {
        friend class search_result_inserter;

        typedef std::vector<const node*>        res_nodes_type;
        typedef std::shared_ptr<res_nodes_type> res_nodes_ptr;
    public:

        class const_iterator
        {
            friend class point_quad_tree<_Key,_Value>::search_results;
            typedef typename point_quad_tree<_Key,_Value>::point point;
            typedef typename point_quad_tree<_Key,_Value>::value_type parent_value_type;

        public:
            // Iterator traits
            typedef std::pair<point, parent_value_type> value_type;
            typedef value_type*     pointer;
            typedef value_type&     reference;
            typedef ptrdiff_t       difference_type;
            typedef ::std::bidirectional_iterator_tag iterator_category;

            const_iterator(res_nodes_ptr& ptr) : mp_res_nodes(ptr), m_end_pos(false) {}

            const_iterator(const const_iterator& r) :
                mp_res_nodes(r.mp_res_nodes),
                m_cur_pos(r.m_cur_pos),
                m_cur_value(r.m_cur_value),
                m_end_pos(r.m_end_pos) {}

            const_iterator& operator= (const const_iterator& r)
            {
                mp_res_nodes = r.mp_res_nodes;
                m_cur_pos = r.m_cur_pos;
                m_cur_value = r.m_cur_value;
                m_end_pos = r.m_end_pos;
                return *this;
            }

            bool operator== (const const_iterator& r) const
            {
                if (mp_res_nodes)
                {
                    // Non-empty result set.
                    return mp_res_nodes.get() == r.mp_res_nodes.get() &&
                        m_cur_pos == r.m_cur_pos && m_end_pos == r.m_end_pos;
                }

                // Empty result set.
                if (r.mp_res_nodes)
                    return false;

                return m_end_pos == r.m_end_pos;
            }

            bool operator!= (const const_iterator& r) const
            {
                return !operator==(r);
            }

            const value_type& operator*() const
            {
                return m_cur_value;
            }

            const value_type* operator->() const
            {
                return get_current_value();
            }

            const value_type* operator++()
            {
                // The only difference between the last data position and the 
                // end iterator position must be the value of m_end_pos;
                // m_cur_pos needs to point to the last data position even
                // when the iterator is at the end-of-iterator position.

                typename res_nodes_type::const_iterator cur_pos = m_cur_pos;
                if (++cur_pos == mp_res_nodes->end())
                {
                    m_end_pos = true;
                    return nullptr;
                }
                m_cur_pos = cur_pos;
                update_current_value();
                return operator->();
            }

            const value_type* operator--()
            {
                if (m_end_pos)
                {
                    m_end_pos = false;
                    return get_current_value();
                }
                --m_cur_pos;
                update_current_value();
                return get_current_value();
            }

        private:
            void move_to_front()
            {
                if (!mp_res_nodes)
                {
                    // Empty data set.
                    m_end_pos = true;
                    return;
                }

                m_cur_pos = mp_res_nodes->begin();
                m_end_pos = false;
                update_current_value();
            }

            void move_to_end()
            {
                m_end_pos = true;
                if (!mp_res_nodes)
                    // Empty data set.
                    return;

                m_cur_pos = mp_res_nodes->end();
                --m_cur_pos; // Keep the position at the last data position.
            }

            void update_current_value()
            {
                const node* p = *m_cur_pos;
                m_cur_value.first = point(p->x, p->y);
                m_cur_value.second = p->data;
            }

            const value_type* get_current_value() const
            {
                return &m_cur_value;
            }

        private:
            res_nodes_ptr mp_res_nodes;
            typename res_nodes_type::const_iterator m_cur_pos;
            value_type m_cur_value;
            bool m_end_pos:1;
        };

        search_results() : mp_res_nodes(static_cast<res_nodes_type*>(nullptr)) {}
        search_results(const search_results& r) : mp_res_nodes(r.mp_res_nodes) {}

        typename search_results::const_iterator begin()
        {
            typename search_results::const_iterator itr(mp_res_nodes);
            itr.move_to_front();
            return itr;
        }
    
        typename search_results::const_iterator end()
        {
            typename search_results::const_iterator itr(mp_res_nodes);
            itr.move_to_end();
            return itr;
        }

    private:
        void push_back(const node* p)
        {
            if (!mp_res_nodes)
                mp_res_nodes.reset(new res_nodes_type);
            mp_res_nodes->push_back(p);
        }

    private:
        res_nodes_ptr mp_res_nodes;
    };

    point_quad_tree();
    point_quad_tree(const point_quad_tree& r);
    ~point_quad_tree();

    /**
     * Insert a new data at specified coordinates.  It overwrites existing
     * data in case one exists at the specified coordinates.
     * 
     * @param x x coordinate of new data position
     * @param y y coordinate of new data position
     * @param data data being inserted at the specified coordinates.
     */
    void insert(key_type x, key_type y, value_type data);

    /**
     * Perform region search (aka window search), that is, find all points 
     * that fall within specified rectangular region.  The boundaries are 
     * inclusive.
     *  
     * @param x1 left coordinate of the search region 
     * @param y1 top coordinate of the search region 
     * @param x2 right coordinate of the search region 
     * @param y2 bottom coordinate of the search region 
     * @param result this array will contain all data found without specified 
     *               region.
     */
    void search_region(key_type x1, key_type y1, key_type x2, key_type y2, data_array_type& result) const;

    /**
     * Perform region search (aka window search), that is, find all points 
     * that fall within specified rectangular region.  The boundaries are 
     * inclusive.
     *  
     * @param x1 left coordinate of the search region 
     * @param y1 top coordinate of the search region 
     * @param x2 right coordinate of the search region 
     * @param y2 bottom coordinate of the search region 
     *  
     * @return search result object containing all data found within the 
     *         specified region.
     */
    search_results search_region(key_type x1, key_type y1, key_type x2, key_type y2) const;

    /**
     * Find data at specified coordinates.  If no data exists at the specified 
     * coordinates, this method throws a 
     * <code>point_quad_tree::data_not_found</code> exception.
     *  
     * @param x x coordinate 
     * @param y y coordinate 
     * 
     * @return data found at the specified coordinates.
     */
    value_type find(key_type x, key_type y) const;

    /**
     * Remove data from specified coordinates.  This method does nothing if no 
     * data exists at the specified coordinates. 
     * 
     * @param x x coordinate
     * @param y y coordinate
     */
    void remove(key_type x, key_type y);

    /**
     * Swap the internal state with another instance.
     * 
     * @param r another instance to swap internals with.
     */
    void swap(point_quad_tree& r);

    /**
     * Remove all stored data.
     */
    void clear();

    /**
     * Check whether or not the container is empty. 
     * 
     * @return bool true if empty, false otherwise.
     */
    bool empty() const;

    /**
     * Get the number of stored data. 
     * 
     * @return the number of data currently stored in the container.
     */
    size_t size() const;

    /**
     * Get read-only access to the internal quad node tree. 
     * 
     * @return root node
     */
    node_access get_node_access() const;

    point_quad_tree& operator= (const point_quad_tree& r);

    bool operator== (const point_quad_tree& r) const;

    bool operator!= (const point_quad_tree& r) const { return !operator== (r); }

#ifdef MDDS_UNIT_TEST
public:
#else
private:
#endif
    /**
     * Data stored in each node.  Used for verification of data stored in tree 
     * during unit testing. 
     */
    struct node_data
    {
        key_type    x;
        key_type    y;
        value_type  data;
        node_data(key_type _x, key_type _y, value_type _data) :
            x(_x), y(_y), data(_data) {}
        node_data(const node_data& r) : 
            x(r.x), y(r.y), data(r.data) {}

        bool operator== (const node_data& r) const
        {
            return (x == r.x) && (y == r.y) && (data == r.data);
        }

        bool operator!= (const node_data& r) const
        {
            return !operator==(r);
        }

        struct sorter : public ::std::binary_function<node_data, node_data, bool>
        {
            bool operator() (const node_data& left, const node_data& right) const
            {
                if (left.x != right.x)
                    return left.x < right.x;
                if (left.y != right.y)
                    return left.y < right.y;
                return left.data < right.data;
            }
        };
    };

    static bool equals(::std::vector<node_data>& v1, ::std::vector<node_data>& v2);

    bool verify_data(::std::vector<node_data>& expected) const;

    bool verify_node_iterator(const node_access& nac) const;
    static bool verify_node_iterator(const node_access& nac, const node* p);

    void get_all_stored_data(::std::vector<node_data>& stored_data) const;

    void dump_tree_svg(const ::std::string& fpath) const;

private:
    class array_inserter : public ::std::unary_function<const node*, void>
    {
    public:
        array_inserter(data_array_type& result) : m_result(result) {}

        void operator() (const node* p)
        {
            m_result.push_back(p->data);
        }
    private:
        data_array_type& m_result;
    };

    class search_result_inserter : public ::std::unary_function<const node*, void>
    {
    public:
        search_result_inserter(search_results& result) : m_result(result) {}

        void operator() (const node* p)
        {
            m_result.push_back(p);
        }
    private:
        search_results& m_result;
    };

    class data_inserter : public ::std::unary_function<node_data, void>
    {
    public:
        data_inserter(point_quad_tree& db) : m_db(db) {}

        void operator() (const node_data& v)
        {
            m_db.insert(v.x, v.y, v.data);
        }
    private:
        point_quad_tree& m_db;
    };

    struct node_distance
    {
        node_quadrant_t quad;
        key_type        dist;
        node_ptr        node;

        node_distance() : quad(quad_unspecified), dist(0), node(nullptr) {}
        node_distance(node_quadrant_t _quad, key_type _dist, const node_ptr& _node) : 
            quad(_quad), dist(_dist), node(_node) {}
    };

    node_ptr find_node(key_type x, key_type y) const;
    const node* find_node_ptr(key_type x, key_type y) const;
    node_ptr find_replacement_node(key_type x, key_type y, const node_ptr& delete_node) const;

    void find_candidate_in_quad(key_type x, key_type y, 
             node_distance& dx_node, node_distance& dy_node, node_distance& min_city_block_node,
             const node_ptr& delete_node, node_quadrant_t quad) const;

    void adjust_quad(const key_range_type& hatched_xrange, const key_range_type& hatched_yrange,
                     node_ptr quad_root, direction_t dir, reinsert_tree_array_type& insert_list);

    void set_new_root(const key_range_type& hatched_xrange, const key_range_type& hatched_yrange,
                      node_ptr& quad_root, node_quadrant_t dir, reinsert_tree_array_type& insert_list);

    void insert_node(node_ptr& dest, node_ptr& node);
    void reinsert_tree(node_ptr& dest, node_ptr& root);
    void reinsert_tree(node_ptr& dest, node_quadrant_t quad, node_ptr& root);
    void clear_all_nodes();
    void dump_node_svg(const node* p, ::std::ofstream& file) const;
    void count_all_nodes(const node* p, size_t& node_count) const;
    void insert_data_from(const point_quad_tree& r);
    void get_all_stored_data(const node* p, ::std::vector<node_data>& stored_data) const;

private:
    node_ptr    m_root;

    key_range_type m_xrange;
    key_range_type m_yrange;
};

template<typename _Key, typename _Value>
point_quad_tree<_Key,_Value>::point_quad_tree() :
    m_root(nullptr),
    m_xrange(0,0),
    m_yrange(0,0)
{
}

template<typename _Key, typename _Value>
point_quad_tree<_Key,_Value>::point_quad_tree(const point_quad_tree& r) :
    m_root(nullptr),
    m_xrange(0,0),
    m_yrange(0,0)
{
    insert_data_from(r);
}

template<typename _Key, typename _Value>
point_quad_tree<_Key,_Value>::~point_quad_tree()
{
    clear_all_nodes();
}

template<typename _Key, typename _Value>
void point_quad_tree<_Key,_Value>::insert(key_type x, key_type y, value_type data)
{
    m_xrange.first  = ::std::min(m_xrange.first,  x);
    m_xrange.second = ::std::max(m_xrange.second, x);
    m_yrange.first  = ::std::min(m_yrange.first,  y);
    m_yrange.second = ::std::max(m_yrange.second, y);

    if (!m_root)
    {
        // The very first node.
        m_root.reset(new node(x, y, data));
        return;
    }

    node_ptr cur_node = m_root;
    while (true)
    {
        if (cur_node->x == x && cur_node->y == y)
        {
            // Replace the current data with this, and we are done!
            cur_node->data = data;
            return;
        }

        node_quadrant_t quad = cur_node->get_quadrant(x, y);
        switch (quad)
        {
            case quad_northeast:
                if (cur_node->northeast)
                    cur_node = cur_node->northeast;
                else
                {
                    cur_node->northeast.reset(new node(x, y, data));
                    cur_node->northeast->parent = cur_node;
                    return;
                }
                break;
            case quad_northwest:
                if (cur_node->northwest)
                    cur_node = cur_node->northwest;
                else
                {
                    cur_node->northwest.reset(new node(x, y, data));
                    cur_node->northwest->parent = cur_node;
                    return;
                }
                break;
            case quad_southeast:
                if (cur_node->southeast)
                    cur_node = cur_node->southeast;
                else
                {
                    cur_node->southeast.reset(new node(x, y, data));
                    cur_node->southeast->parent = cur_node;
                    return;
                }
                break;
            case quad_southwest:
                if (cur_node->southwest)
                    cur_node = cur_node->southwest;
                else
                {
                    cur_node->southwest.reset(new node(x, y, data));
                    cur_node->southwest->parent = cur_node;
                    return;
                }
                break;
            default:
                throw general_error("unknown quadrant");
        }
    }
    assert(!"This should never be reached.");
}

template<typename _Key, typename _Value>
void point_quad_tree<_Key,_Value>::search_region(key_type x1, key_type y1, key_type x2, key_type y2, data_array_type& result) const
{
    using namespace std;
    const node* p = m_root.get();
    array_inserter _inserter(result);
    ::mdds::search_region_node(p, x1, y1, x2, y2, _inserter);
}

template<typename _Key, typename _Value>
typename point_quad_tree<_Key,_Value>::search_results
point_quad_tree<_Key,_Value>::search_region(key_type x1, key_type y1, key_type x2, key_type y2) const
{
    using namespace std;
    search_results result;
    const node* p = m_root.get();
    search_result_inserter _inserter(result);
    ::mdds::search_region_node(p, x1, y1, x2, y2, _inserter);
    return result;
}

template<typename _Key, typename _Value>
typename point_quad_tree<_Key,_Value>::value_type
point_quad_tree<_Key,_Value>::find(key_type x, key_type y) const
{
    const node* p = find_node_ptr(x, y);
    if (!p)
        throw data_not_found();
    return p->data;
}

template<typename _Key, typename _Value>
void point_quad_tree<_Key,_Value>::remove(key_type x, key_type y)
{
    using namespace std;
    node_ptr delete_node = find_node(x, y);
    if (!delete_node)
        // No node exists at this coordinate.
        return;

#if DEBUG_POINT_QUAD_TREE
    cout << "found the node to be removed at " << delete_node->x << "," << delete_node->y << " (" << *delete_node->data << ")" << endl;
#endif

    // Check if this is a leaf node, in which case we can just delete it 
    // without further processing.
    if (delete_node->leaf())
    {
#if DEBUG_POINT_QUAD_TREE
        cout << "deleting a leaf node." << endl;
#endif
        if (delete_node.get() == m_root.get())
            m_root.reset();
        else
            disconnect_node_from_parent(delete_node);
        delete_node.reset();
        return;
    }

    node_ptr repl_node = find_replacement_node(x, y, delete_node);
    if (!repl_node)
        // Non-leaf node should have at least one replacement candidate.
        throw general_error("failed to find a replacement candidate node.");

    node_quadrant_t repl_quad = delete_node->get_quadrant(repl_node->x, repl_node->y);
    
    key_range_type xrange(delete_node->x, repl_node->x);
    key_range_type yrange(delete_node->y, repl_node->y);
    ensure_order(xrange);
    ensure_order(yrange);
    reinsert_tree_array_type insert_list;

    // Call the quadrant where the replacement node is quadrant I.  Adjust the
    // quadrants adjacent to quadrant I first, then adjust quadrant I
    // afterwards.
    switch (repl_quad)
    {
        case quad_northeast:
            adjust_quad(xrange, yrange, delete_node->northwest, dir_south, insert_list);
            adjust_quad(xrange, yrange, delete_node->southeast, dir_west, insert_list);
            set_new_root(xrange, yrange, delete_node->northeast, quad_southwest, insert_list);
            break;
        case quad_northwest:
            adjust_quad(xrange, yrange, delete_node->northeast, dir_south, insert_list);
            adjust_quad(xrange, yrange, delete_node->southwest, dir_east, insert_list);
            set_new_root(xrange, yrange, delete_node->northwest, quad_southeast, insert_list);
            break;
        case quad_southeast:
            adjust_quad(xrange, yrange, delete_node->northeast, dir_west, insert_list);
            adjust_quad(xrange, yrange, delete_node->southwest, dir_north, insert_list);
            set_new_root(xrange, yrange, delete_node->southeast, quad_northwest, insert_list);
            break;
        case quad_southwest:
            adjust_quad(xrange, yrange, delete_node->northwest, dir_east, insert_list);
            adjust_quad(xrange, yrange, delete_node->southeast, dir_north, insert_list);
            set_new_root(xrange, yrange, delete_node->southwest, quad_northeast, insert_list);
            break;
        default:
            throw general_error("quadrant for the replacement node is unspecified.");
    }

    // Reinsert all child nodes from the replacement node into the node to be 
    // "deleted".    
    switch (repl_quad)
    {
        case quad_northeast:
        case quad_southwest:
        {
            node_ptr root = repl_node->northwest;
            repl_node->northwest.reset();
            reinsert_tree(delete_node, quad_northwest, root);
            
            root = repl_node->southeast;
            repl_node->southeast.reset();
            reinsert_tree(delete_node, quad_southeast, root);
        }
        break;
        case quad_northwest:
        case quad_southeast:
        {
            node_ptr root = repl_node->northeast;
            repl_node->northeast.reset();
            reinsert_tree(delete_node, quad_northeast, root);
            
            root = repl_node->southwest;
            repl_node->southwest.reset();
            reinsert_tree(delete_node, quad_southwest, root);
        }
        break;
        default:
            throw general_error("quadrant for the replacement node is unspecified.");
    }

    // Finally, replace the node to be removed with the replacement node.
    delete_node->x = repl_node->x;
    delete_node->y = repl_node->y;
    delete_node->data = repl_node->data;

    // Reset the parent node.
    delete_node->parent = repl_node->parent;
    repl_node->parent.reset();

    switch (repl_quad)
    {
        case quad_northeast:
            delete_node->northeast = repl_node->northeast;
            repl_node->northeast.reset();
            break;
        case quad_northwest:
            delete_node->northwest = repl_node->northwest;
            repl_node->northwest.reset();
            break;
        case quad_southeast:
            delete_node->southeast = repl_node->southeast;
            repl_node->southeast.reset();
            break;
        case quad_southwest:
            delete_node->southwest = repl_node->southwest;
            repl_node->southwest.reset();
            break;
        default:
            throw general_error("quadrant for the replacement node is unspecified.");
    }

    // Lastly, re-insert all those trees that have been cut during the quad 
    // adjustment into the new root.
    typename reinsert_tree_array_type::iterator 
        itr = insert_list.begin(), itr_end = insert_list.end();
    for (; itr != itr_end; ++itr)
        reinsert_tree(delete_node, *itr);
}

template<typename _Key, typename _Value>
void point_quad_tree<_Key,_Value>::swap(point_quad_tree& r)
{
    m_root.swap(r.m_root);
    ::std::swap(m_xrange, r.m_xrange);
    ::std::swap(m_yrange, r.m_yrange);
}

template<typename _Key, typename _Value>
void point_quad_tree<_Key,_Value>::clear()
{
    clear_all_nodes();
}

template<typename _Key, typename _Value>
bool point_quad_tree<_Key,_Value>::empty() const
{
    return (m_root.get() == nullptr);
}

template<typename _Key, typename _Value>
size_t point_quad_tree<_Key,_Value>::size() const
{
    size_t node_count = 0;
    count_all_nodes(m_root.get(), node_count);
    return node_count;
}

template<typename _Key, typename _Value>
typename point_quad_tree<_Key,_Value>::node_access
point_quad_tree<_Key,_Value>::get_node_access() const
{
    return node_access(m_root.get());
}

template<typename _Key, typename _Value>
point_quad_tree<_Key,_Value>& point_quad_tree<_Key,_Value>::operator= (const point_quad_tree& r)
{
    m_xrange = key_range_type(0, 0);
    m_yrange = key_range_type(0, 0);
    clear_all_nodes();
    insert_data_from(r);
    return *this;
}

template<typename _Key, typename _Value>
bool point_quad_tree<_Key,_Value>::operator== (const point_quad_tree& r) const
{
    ::std::vector<node_data> v1, v2;
    get_all_stored_data(v1);
    r.get_all_stored_data(v2);
    return equals(v1, v2);
}

template<typename _Key, typename _Value>
void point_quad_tree<_Key,_Value>::dump_tree_svg(const ::std::string& fpath) const
{
    using namespace std;
    ofstream file(fpath.c_str());
    file << "<svg width=\"60cm\" height=\"60cm\" viewBox=\"-2 -2 202 202\" xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << endl;
    file << "<defs>"
         << "  <marker id=\"Triangle\""
         << "    viewBox=\"0 0 10 10\" refX=\"10\" refY=\"5\" "
         << "    markerUnits=\"strokeWidth\""
         << "    markerWidth=\"9\" markerHeight=\"6\""
         << "    orient=\"auto\">"
         << "    <path d=\"M 0 0 L 10 5 L 0 10 z\" />"
         << "  </marker>"
         << "</defs>" << endl;
    
    file << "<path d=\"M 0 0 L 0 " << m_yrange.second + 1 << "\" stroke=\"blue\" stroke-width=\"0.2\" marker-end=\"url(#Triangle)\"/>" << endl;
    file << "<path d=\"M 0 0 L " << m_xrange.second + 1 << " 0\" stroke=\"blue\" stroke-width=\"0.2\" marker-end=\"url(#Triangle)\"/>" << endl;
    dump_node_svg(m_root.get(), file);
    file << "</svg>" << endl;
}

template<typename _NodePtr>
void draw_svg_arrow(::std::ofstream& file, const _NodePtr start, const _NodePtr end)
{
    using namespace std;
    file << "<g stroke=\"red\" marker-end=\"url(#Triangle)\">" << endl;
    file << "<line x1=\"" << start->x << "\" y1=\"" << start->y << "\" x2=\"" 
        << end->x << "\" y2=\"" << end->y << "\" stroke-width=\"0.2\"/>" << endl;
    file << "</g>" << endl;
}

template<typename _Key, typename _Value>
void point_quad_tree<_Key,_Value>::dump_node_svg(const node* p, ::std::ofstream& file) const
{
    using namespace std;

    if (!p)
        return;

    file << "<circle cx=\"" << p->x << "\" cy=\"" << p->y << "\" r=\"0.1\""
        << " fill=\"black\" stroke=\"black\"/>" << endl;
    file << "<text x=\"" << p->x + 1 << "\" y=\"" << p->y + 2 << "\" font-size=\"1.2\" fill=\"black\">"
        << *p->data << " (" << p->x << "," << p->y << ")</text>" << endl;

    if (p->northwest)
        draw_svg_arrow<const node*>(file, p, p->northwest.get());

    if (p->northeast)
        draw_svg_arrow<const node*>(file, p, p->northeast.get());

    if (p->southwest)
        draw_svg_arrow<const node*>(file, p, p->southwest.get());

    if (p->southeast)
        draw_svg_arrow<const node*>(file, p, p->southeast.get());

    dump_node_svg(p->northeast.get(), file);
    dump_node_svg(p->northwest.get(), file);
    dump_node_svg(p->southeast.get(), file);
    dump_node_svg(p->southwest.get(), file);
}

template<typename _Key, typename _Value>
bool point_quad_tree<_Key,_Value>::equals(::std::vector<node_data>& v1, ::std::vector<node_data>& v2)
{
    using namespace std;

    if (v1.size() != v2.size())
        return false;

    sort(v1.begin(), v1.end(), typename node_data::sorter());
    sort(v2.begin(), v2.end(), typename node_data::sorter());
    
    typename vector<node_data>::const_iterator 
        itr1 = v1.begin(), itr1_end = v1.end(), itr2 = v2.begin(), itr2_end = v2.end();

    for (; itr1 != itr1_end; ++itr1, ++itr2)
    {
        if (itr2 == itr2_end)
            return false;

        if (*itr1 != *itr2)
            return false;
    }
    if (itr2 != itr2_end)
        return false;

    return true;
}

template<typename _Key, typename _Value>
void point_quad_tree<_Key,_Value>::get_all_stored_data(::std::vector<node_data>& stored_data) const
{
    stored_data.clear();
    if (!m_root)
        return;

    get_all_stored_data(m_root.get(), stored_data);
}

template<typename _Key, typename _Value>
void point_quad_tree<_Key,_Value>::count_all_nodes(const node* p, size_t& node_count) const
{
    if (!p)
        return;

    ++node_count;

    count_all_nodes(p->northeast.get(), node_count);
    count_all_nodes(p->northwest.get(), node_count);
    count_all_nodes(p->southeast.get(), node_count);
    count_all_nodes(p->southwest.get(), node_count);
}

template<typename _Key, typename _Value>
void point_quad_tree<_Key,_Value>::insert_data_from(const point_quad_tree& r)
{
    using namespace std;
    vector<node_data> all_data;
    r.get_all_stored_data(all_data);
    for_each(all_data.begin(), all_data.end(), data_inserter(*this));
}

template<typename _Key, typename _Value>
bool point_quad_tree<_Key,_Value>::verify_data(::std::vector<node_data>& expected) const
{
    ::std::vector<node_data> stored;
    get_all_stored_data(stored);
    return equals(stored, expected);
}

template<typename _Key, typename _Value>
bool point_quad_tree<_Key,_Value>::verify_node_iterator(const node_access& nac) const
{
    return verify_node_iterator(nac, m_root.get());
}

template<typename _Key, typename _Value>
bool point_quad_tree<_Key,_Value>::verify_node_iterator(const node_access& nac, const node* p)
{
    if (!nac)
        return (p == nullptr);

    if (!p)
        return false;

    if (!verify_node_iterator(nac.northeast(), p->northeast.get()))
        return false;
    if (!verify_node_iterator(nac.northwest(), p->northwest.get()))
        return false;
    if (!verify_node_iterator(nac.southeast(), p->southeast.get()))
        return false;
    if (!verify_node_iterator(nac.southwest(), p->southwest.get()))
        return false;

    return true;
}

template<typename _Key, typename _Value>
void point_quad_tree<_Key,_Value>::get_all_stored_data(const node* p, ::std::vector<node_data>& stored_data) const
{
    if (!p)
        return;

    stored_data.push_back(node_data(p->x, p->y, p->data));

    get_all_stored_data(p->northeast.get(), stored_data);
    get_all_stored_data(p->northwest.get(), stored_data);
    get_all_stored_data(p->southeast.get(), stored_data);
    get_all_stored_data(p->southwest.get(), stored_data);
}

template<typename _Key, typename _Value>
typename point_quad_tree<_Key,_Value>::node_ptr
point_quad_tree<_Key,_Value>::find_node(key_type x, key_type y) const
{
    node_ptr cur_node = m_root;
    while (cur_node)
    {
        if (cur_node->x == x && cur_node->y == y)
        {
            // Found the node.
            return cur_node;
        }

        node_quadrant_t quad = cur_node->get_quadrant(x, y);
        switch (quad)
        {
            case quad_northeast:
                if (!cur_node->northeast)
                    return node_ptr();
                cur_node = cur_node->northeast;
                break;
            case quad_northwest:
                if (!cur_node->northwest)
                    return node_ptr();
                cur_node = cur_node->northwest;
                break;
            case quad_southeast:
                if (!cur_node->southeast)
                    return node_ptr();
                cur_node = cur_node->southeast;
                break;
            case quad_southwest:
                if (!cur_node->southwest)
                    return node_ptr();
                cur_node = cur_node->southwest;
                break;
            default:
                throw general_error("unknown quadrant");
        }
    }
    return node_ptr();
}

template<typename _Key, typename _Value>
const typename point_quad_tree<_Key,_Value>::node*
point_quad_tree<_Key,_Value>::find_node_ptr(key_type x, key_type y) const
{
    const node* cur_node = m_root.get();
    while (cur_node)
    {
        if (cur_node->x == x && cur_node->y == y)
        {
            // Found the node.
            return cur_node;
        }

        node_quadrant_t quad = cur_node->get_quadrant(x, y);
        switch (quad)
        {
            case quad_northeast:
                if (!cur_node->northeast)
                    return nullptr;
                cur_node = cur_node->northeast.get();
                break;
            case quad_northwest:
                if (!cur_node->northwest)
                    return nullptr;
                cur_node = cur_node->northwest.get();
                break;
            case quad_southeast:
                if (!cur_node->southeast)
                    return nullptr;
                cur_node = cur_node->southeast.get();
                break;
            case quad_southwest:
                if (!cur_node->southwest)
                    return nullptr;
                cur_node = cur_node->southwest.get();
                break;
            default:
                throw general_error("unknown quadrant");
        }
    }
    return nullptr;
}

template<typename _Key, typename _Value>
typename point_quad_tree<_Key,_Value>::node_ptr
point_quad_tree<_Key,_Value>::find_replacement_node(key_type x, key_type y, const node_ptr& delete_node) const
{
    using namespace std;

    // Try to get a replacement candidate in each quadrant.
    node_distance dx_node, dy_node, min_city_block_node;

#if DEBUG_POINT_QUAD_TREE
    cout << "northeast" << endl;
#endif        
    find_candidate_in_quad(
        x, y, dx_node, dy_node, min_city_block_node, delete_node, quad_northeast);

#if DEBUG_POINT_QUAD_TREE
    cout << "northwest" << endl;
#endif        
    find_candidate_in_quad(
        x, y, dx_node, dy_node, min_city_block_node, delete_node, quad_northwest);

#if DEBUG_POINT_QUAD_TREE
    cout << "southwest" << endl;
#endif        
    find_candidate_in_quad(
        x, y, dx_node, dy_node, min_city_block_node, delete_node, quad_southwest);

#if DEBUG_POINT_QUAD_TREE
    cout << "southeast" << endl;
#endif        
    find_candidate_in_quad(
        x, y, dx_node, dy_node, min_city_block_node, delete_node, quad_southeast);

    // Check Criterion 1.

#if DEBUG_POINT_QUAD_TREE
    if (dx_node.node)
        cout << "node closest to x axis: " << *dx_node.node->data << " (dx=" << dx_node.dist << ")" << endl;

    if (dy_node.node)
        cout << "node closest to y axis: " << *dy_node.node->data << " (dy=" << dy_node.dist << ")" << endl;
#endif        

    if (dx_node.node == dy_node.node && ((dx_node.quad == quad_northwest) || (dx_node.quad == quad_southeast)))
    {
#if DEBUG_POINT_QUAD_TREE
        cout << "node that satisfies Criterion 1: " << *dx_node.node->data << endl;
#endif                
        return dx_node.node;
    }
    else
    {
#if DEBUG_POINT_QUAD_TREE
        cout << "unable to find node that satisfies Criterion 1." << endl;
#endif                
    }

    // Move on to Criterion 2.

    if (min_city_block_node.node)
    {
#if DEBUG_POINT_QUAD_TREE
        cout << "node that satisfies Criterion 2: " << *min_city_block_node.node->data << " (dist=" << min_city_block_node.dist << ")" << endl;
#endif                
        return min_city_block_node.node;
    }

    return node_ptr();
}

template<typename _Key, typename _Value>
void point_quad_tree<_Key,_Value>::find_candidate_in_quad(
    key_type x, key_type y, 
    node_distance& dx_node, node_distance& dy_node, node_distance& min_city_block_node,
    const node_ptr& delete_node, node_quadrant_t quad) const
{
    using namespace std;

    node_ptr repl_node = delete_node->get_quadrant_node(quad);
    if (!repl_node)
    {
        // No candidate in this quadrant.
#if DEBUG_POINT_QUAD_TREE
        cout << "  no candidate in this quadrant" << endl;
#endif                
        return;
    }

    node_quadrant_t oppo_quad = opposite(quad);
    while (repl_node->has_quadrant_node(oppo_quad))
        repl_node = repl_node->get_quadrant_node(oppo_quad);

#if DEBUG_POINT_QUAD_TREE
    cout << "  candidate: " << repl_node->x << "," << repl_node->y << " (" << *repl_node->data << ")" << endl;
#endif        

    // Calculate its distance to each of the borders.
    key_type dx = repl_node->x > x ? repl_node->x - x : x - repl_node->x;
    key_type dy = repl_node->y > y ? repl_node->y - y : y - repl_node->y;
#if DEBUG_POINT_QUAD_TREE
    cout << "  dx = " << dx << ", dy = " << dy << endl;
#endif        

    if (!dx_node.node || dx_node.dist > dx)
        dx_node = node_distance(quad, dx, repl_node);
    if (!dy_node.node || dy_node.dist > dy)
        dy_node = node_distance(quad, dy, repl_node);

    if (!min_city_block_node.node || min_city_block_node.dist > (dx + dy))
        min_city_block_node = node_distance(quad_unspecified, dx+dy, repl_node);
}

template<typename _Key, typename _Value>
void point_quad_tree<_Key,_Value>::adjust_quad(
    const key_range_type& hatched_xrange, const key_range_type& hatched_yrange, 
    node_ptr quad_root, direction_t dir, reinsert_tree_array_type& insert_list)
{
    using namespace std;

    if (!quad_root)
        return;

#if DEBUG_POINT_QUAD_TREE
    cout << "adjust_quad: checking " << *quad_root->data << " (" << quad_root->x << "," << quad_root->y << ")" << endl;
#endif        

    if ((hatched_xrange.first <= quad_root->x && quad_root->x <= hatched_xrange.second) ||
        (hatched_yrange.first <= quad_root->y && quad_root->y <= hatched_yrange.second))
    {
#if DEBUG_POINT_QUAD_TREE
        cout << "  " << *quad_root->data << " is in the hatched region" << endl;
#endif                
        // Insert the whole tree, including the root, into the insert list.
        disconnect_node_from_parent(quad_root);
        quad_root->parent.reset();
        insert_list.push_back(quad_root);
        return;
    }

    switch (dir)
    {
        case dir_east:
            adjust_quad(hatched_xrange, hatched_yrange, quad_root->northeast, dir_east, insert_list);
            adjust_quad(hatched_xrange, hatched_yrange, quad_root->southeast, dir_east, insert_list);
            break;
        case dir_north:
            adjust_quad(hatched_xrange, hatched_yrange, quad_root->northeast, dir_north, insert_list);
            adjust_quad(hatched_xrange, hatched_yrange, quad_root->northwest, dir_north, insert_list);
            break;
        case dir_south:
            adjust_quad(hatched_xrange, hatched_yrange, quad_root->southeast, dir_south, insert_list);
            adjust_quad(hatched_xrange, hatched_yrange, quad_root->southwest, dir_south, insert_list);
            break;
        case dir_west:
            adjust_quad(hatched_xrange, hatched_yrange, quad_root->northwest, dir_west, insert_list);
            adjust_quad(hatched_xrange, hatched_yrange, quad_root->southwest, dir_west, insert_list);
            break;
        default:
            ;
    }
}

template<typename _Key, typename _Value>
void point_quad_tree<_Key,_Value>::set_new_root(
    const key_range_type& hatched_xrange, const key_range_type& hatched_yrange, 
    node_ptr& quad_root, node_quadrant_t dir, reinsert_tree_array_type& insert_list)
{
    node_ptr cur_node = quad_root;
    while (cur_node)
    {
        switch (dir)
        {
            case quad_northeast:
                adjust_quad(hatched_xrange, hatched_yrange, cur_node->southeast, dir_east, insert_list);
                adjust_quad(hatched_xrange, hatched_yrange, cur_node->northwest, dir_north, insert_list);
                break;
            case quad_northwest:
                adjust_quad(hatched_xrange, hatched_yrange, cur_node->northeast, dir_north, insert_list);
                adjust_quad(hatched_xrange, hatched_yrange, cur_node->southwest, dir_west, insert_list);
                break;
            case quad_southeast:
                adjust_quad(hatched_xrange, hatched_yrange, cur_node->northeast, dir_east, insert_list);
                adjust_quad(hatched_xrange, hatched_yrange, cur_node->southwest, dir_south, insert_list);
                break;
            case quad_southwest:
                adjust_quad(hatched_xrange, hatched_yrange, cur_node->northwest, dir_west, insert_list);
                adjust_quad(hatched_xrange, hatched_yrange, cur_node->southeast, dir_south, insert_list);
                break;
            default:
                throw general_error("unspecified quadrant");
        }
        cur_node = cur_node->get_quadrant_node(dir);
    }
}

template<typename _Key, typename _Value>
void point_quad_tree<_Key,_Value>::insert_node(node_ptr& dest, node_ptr& this_node)
{
    node_ptr cur_node = dest;
    while (true)
    {
        if (cur_node->x == this_node->x && cur_node->y == this_node->y)
        {
            // When inserting a node instance directly (likely as part of tree
            // re-insertion), we are not supposed to have another node at
            // identical position.
            throw general_error("node with identical position encountered.");
        }

        node_quadrant_t quad = cur_node->get_quadrant(this_node->x, this_node->y);
        switch (quad)
        {
            case quad_northeast:
                if (cur_node->northeast)
                    cur_node = cur_node->northeast;
                else
                {
                    cur_node->northeast = this_node;
                    this_node->parent = cur_node;
                    return;
                }
                break;
            case quad_northwest:
                if (cur_node->northwest)
                    cur_node = cur_node->northwest;
                else
                {
                    cur_node->northwest = this_node;
                    this_node->parent = cur_node;
                    return;
                }
                break;
            case quad_southeast:
                if (cur_node->southeast)
                    cur_node = cur_node->southeast;
                else
                {
                    cur_node->southeast = this_node;
                    this_node->parent = cur_node;
                    return;
                }
                break;
            case quad_southwest:
                if (cur_node->southwest)
                    cur_node = cur_node->southwest;
                else
                {
                    cur_node->southwest = this_node;
                    this_node->parent = cur_node;
                    return;
                }
                break;
            default:
                throw general_error("unknown quadrant");
        }
    }
    assert(!"This should never be reached.");
}

template<typename _Key, typename _Value>
void point_quad_tree<_Key,_Value>::reinsert_tree(node_ptr& dest, node_ptr& root)
{
    assert(dest); // Destination node should not be null.

    if (!root)
        // Nothing to re-insert.  Bail out.
        return;

    if (root->northeast)
    {
        reinsert_tree(dest, root->northeast);
        root->northeast.reset();
    }
    if (root->northwest)
    {
        reinsert_tree(dest, root->northwest);
        root->northwest.reset();
    }
    if (root->southeast)
    {
        reinsert_tree(dest, root->southeast);
        root->southeast.reset();
    }
    if (root->southwest)
    {
        reinsert_tree(dest, root->southwest);
        root->southwest.reset();
    }

    root->parent.reset();
    insert_node(dest, root);
}

template<typename _Key, typename _Value>
void point_quad_tree<_Key,_Value>::reinsert_tree(node_ptr& dest, node_quadrant_t quad, node_ptr& root)
{
    if (!root)
        // Nothing to re-insert.  Bail out.
        return;
    
    switch (quad)
    {
        case quad_northeast:
            if (dest->northeast)
                reinsert_tree(dest->northeast, root);
            else
            {
                dest->northeast = root;
                root->parent = dest;
            }
            break;
        case quad_northwest:
            if (dest->northwest)
                reinsert_tree(dest->northwest, root);
            else
            {
                dest->northwest = root;
                root->parent = dest;
            }
            break;
        case quad_southeast:
            if (dest->southeast)
                reinsert_tree(dest->southeast, root);
            else
            {
                dest->southeast = root;
                root->parent = dest;
            }
            break;
        case quad_southwest:
            if (dest->southwest)
                reinsert_tree(dest->southwest, root);
            else
            {
                dest->southwest = root;
                root->parent = dest;
            }
            break;
        default:
            throw general_error("reinsert_tree: quadrant unspecified");
    }
}

template<typename _Key, typename _Value>
void point_quad_tree<_Key,_Value>::clear_all_nodes()
{
    ::mdds::disconnect_all_nodes(m_root);
    m_root.reset();
}

}

#endif
