/*************************************************************************
 *
 * Copyright (c) 2010 Kohei Yoshida
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

#ifndef __MDDS_POINT_QUAD_TREE_HPP__
#define __MDDS_POINT_QUAD_TREE_HPP__

#include "mdds/quad_node.hpp"

#include <cstdlib>
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

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


template<typename _Key, typename _Data>
class point_quad_tree
{
private:
    class search_result_inserter;

public:
    typedef _Key    key_type;
    typedef _Data   data_type;
    typedef size_t  size_type;
    typedef ::std::vector<data_type*> data_array_type;

private:
    struct node;
    typedef ::boost::intrusive_ptr<node> node_ptr;

    struct node : quad_node_base<node_ptr, node, key_type>
    {
        data_type* data;
        node(key_type _x, key_type _y, data_type* _data) :
            quad_node_base<node_ptr, node, key_type>(_x, _y),
            data(_data) {}

        node(const node& r) :
            quad_node_base<node_ptr, node, key_type>(r),
            data(r.data) {}

        void dispose()
        {
        }

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
    struct point
    {
        key_type x;
        key_type y;
        point(key_type _x, key_type _y) : x(_x), y(_y) {}
        point() : x(0), y(0) {}
    };

    class search_result
    {
        friend class search_result_inserter;

        typedef ::std::vector<const node*>          res_nodes_type;
        typedef ::boost::shared_ptr<res_nodes_type> res_nodes_ptr;
    public:

        class const_iterator
        {
            friend class point_quad_tree<_Key,_Data>::search_result;
            typedef typename point_quad_tree<_Key,_Data>::point point;
            typedef typename point_quad_tree<_Key,_Data>::data_type data_type;

        public:
            const_iterator(res_nodes_ptr& ptr) : mp_res_nodes(ptr), m_end_pos(false) {}

            const_iterator(const const_iterator& r) :
                mp_res_nodes(r.mp_res_nodes),
                m_cur_pos(r.m_cur_pos),
                m_end_pos(r.m_end_pos) {}

            const_iterator& operator= (const const_iterator& r)
            {
                mp_res_nodes = r.mp_res_nodes;
                m_cur_pos = r.m_cur_pos;
                m_end_pos = r.m_end_pos;
                return *this;
            }

            bool operator== (const const_iterator& r) const
            {
                return mp_res_nodes.get() == r.mp_res_nodes.get() && 
                    m_cur_pos == r.m_cur_pos && m_end_pos == r.m_end_pos;
            }

            bool operator!= (const const_iterator& r) const
            {
                return !operator==(r);
            }

            const ::std::pair<point, const data_type*>& operator*() const
            {
                return m_cur_value;
            }

            const ::std::pair<point, const data_type*>* operator->() const
            {
                return get_current_value();
            }

            const ::std::pair<point, const data_type*>* operator++()
            {
                // The only difference between the last data position and the 
                // end iterator position must be the value of m_end_pos;
                // m_cur_pos needs to point to the last data position even
                // when the iterator is at the end-of-iterator position.

                typename res_nodes_type::const_iterator cur_pos = m_cur_pos;
                if (++cur_pos == mp_res_nodes->end())
                {
                    m_end_pos = true;
                    return NULL;
                }
                m_cur_pos = cur_pos;
                update_current_value();
                return operator->();
            }

            const ::std::pair<point, const data_type*>* operator--()
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

            const ::std::pair<point, const data_type*>* get_current_value() const
            {
                return &m_cur_value;
            }

        private:
            res_nodes_ptr mp_res_nodes;
            typename res_nodes_type::const_iterator m_cur_pos;
            ::std::pair<point, const data_type*> m_cur_value;
            bool m_end_pos:1;
        };

        search_result() : mp_res_nodes(static_cast<res_nodes_type*>(NULL)) {}
        search_result(const search_result& r) : mp_res_nodes(r.mp_res_nodes) {}

        typename search_result::const_iterator begin()
        {
            typename search_result::const_iterator itr(mp_res_nodes);
            itr.move_to_front();
            return itr;
        }
    
        typename search_result::const_iterator end()
        {
            typename search_result::const_iterator itr(mp_res_nodes);
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
    ~point_quad_tree();

    void insert(key_type x, key_type y, data_type* data);

    /**
     * Perform region search (aka window search), that is, find all points 
     * that fall within specified rectangular region. 
     *  
     * @param x1 left coordinate of the search region 
     * @param y1 top coordinate of the search region 
     * @param x2 right coordinate of the search region 
     * @param y2 bottom coordinate of the search region 
     */
    void search_region(key_type x1, key_type y1, key_type x2, key_type y2, data_array_type& result) const;

    search_result search_region(key_type x1, key_type y1, key_type x2, key_type y2) const;

    void remove(key_type x, key_type y);

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
        search_result_inserter(search_result& result) : m_result(result) {}

        void operator() (const node* p)
        {
            m_result.push_back(p);
        }
    private:
        search_result& m_result;
    };

    struct node_distance
    {
        node_quadrant_t quad;
        key_type        dist;
        node_ptr        node;

        node_distance() : quad(quad_unspecified), dist(0), node(NULL) {}
        node_distance(node_quadrant_t _quad, key_type _dist, const node_ptr& _node) : 
            quad(_quad), dist(_dist), node(_node) {}
    };

    node_ptr find_node(key_type x, key_type y) const;
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

private:
    node_ptr    m_root;

    key_range_type m_xrange;
    key_range_type m_yrange;
};

template<typename _Key, typename _Data>
point_quad_tree<_Key,_Data>::point_quad_tree() :
    m_root(NULL),
    m_xrange(0,0),
    m_yrange(0,0)
{
}

template<typename _Key, typename _Data>
point_quad_tree<_Key,_Data>::~point_quad_tree()
{
    clear_all_nodes();
}

template<typename _Key, typename _Data>
void point_quad_tree<_Key,_Data>::insert(key_type x, key_type y, data_type* data)
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

template<typename _Key, typename _Data>
void point_quad_tree<_Key,_Data>::search_region(key_type x1, key_type y1, key_type x2, key_type y2, data_array_type& result) const
{
    using namespace std;
    const node* p = m_root.get();
    array_inserter _inserter(result);
    ::mdds::search_region_node(p, x1, y1, x2, y2, _inserter);
}

template<typename _Key, typename _Data>
typename point_quad_tree<_Key,_Data>::search_result
point_quad_tree<_Key,_Data>::search_region(key_type x1, key_type y1, key_type x2, key_type y2) const
{
    using namespace std;
    search_result result;
    const node* p = m_root.get();
    search_result_inserter _inserter(result);
    ::mdds::search_region_node(p, x1, y1, x2, y2, _inserter);
    return result;
}

template<typename _Key, typename _Data>
void point_quad_tree<_Key,_Data>::remove(key_type x, key_type y)
{
    using namespace std;
    node_ptr delete_node = find_node(x, y);
    if (!delete_node)
        return;

    cout << "found the node to be removed at " << delete_node->x << "," << delete_node->y << " (" << *delete_node->data << ")" << endl;

    // Check if this is a leaf node, in which case we can just delete it 
    // without further processing.
    if (delete_node->leaf())
    {
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
        case quad_unspecified:
        default:
            throw general_error("quadrant for the replacement node is unspecified.");
    }

    // Reinsert all child nodes from the replacement node into the node to be 
    // deleted.    
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
        case quad_unspecified:
        default:
            throw general_error("quadrant for the replacement node is unspecified.");
    }

    // Finally, replace the node to be removed with the replacement node.
    delete_node->x = repl_node->x;
    delete_node->y = repl_node->y;
    delete_node->data = repl_node->data;

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
        case quad_unspecified:
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

template<typename _Key, typename _Data>
void point_quad_tree<_Key,_Data>::dump_tree_svg(const ::std::string& fpath) const
{
    using namespace std;
    ofstream file(fpath.c_str());
    file << "<svg width=\"24cm\" height=\"24cm\" viewBox=\"-2 -2 202 202\" xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << endl;
    file << "<defs>"
         << "  <marker id=\"Triangle\""
         << "    viewBox=\"0 0 10 10\" refX=\"10\" refY=\"5\" "
         << "    markerUnits=\"strokeWidth\""
         << "    markerWidth=\"6\" markerHeight=\"4\""
         << "    orient=\"auto\">"
         << "    <path d=\"M 0 0 L 10 5 L 0 10 z\" />"
         << "  </marker>"
         << "</defs>" << endl;
    
    file << "<path d=\"M 0 0 L 0 " << m_yrange.second + 10 << "\" stroke=\"blue\" stroke-width=\"0.5\" marker-end=\"url(#Triangle)\"/>" << endl;
    file << "<path d=\"M 0 0 L " << m_xrange.second + 10 << " 0\" stroke=\"blue\" stroke-width=\"0.5\" marker-end=\"url(#Triangle)\"/>" << endl;
    dump_node_svg(m_root.get(), file);
    file << "</svg>" << endl;
}

template<typename _NodePtr>
void draw_svg_arrow(::std::ofstream& file, const _NodePtr start, const _NodePtr end)
{
    using namespace std;
    file << "<g stroke=\"red\" marker-end=\"url(#Triangle)\">" << endl;
    file << "<line x1=\"" << start->x << "\" y1=\"" << start->y << "\" x2=\"" 
        << end->x << "\" y2=\"" << end->y << "\" stroke-width=\"0.5\"/>" << endl;
    file << "</g>" << endl;
}

template<typename _Key, typename _Data>
typename point_quad_tree<_Key,_Data>::node_ptr
point_quad_tree<_Key,_Data>::find_node(key_type x, key_type y) const
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

template<typename _Key, typename _Data>
typename point_quad_tree<_Key,_Data>::node_ptr
point_quad_tree<_Key,_Data>::find_replacement_node(key_type x, key_type y, const node_ptr& delete_node) const
{
    using namespace std;

    // Try to get a replacement candidate in each quadrant.
    node_distance dx_node, dy_node, min_city_block_node;

    cout << "northeast" << endl;
    find_candidate_in_quad(
        x, y, dx_node, dy_node, min_city_block_node, delete_node, quad_northeast);

    cout << "northwest" << endl;
    find_candidate_in_quad(
        x, y, dx_node, dy_node, min_city_block_node, delete_node, quad_northwest);

    cout << "southwest" << endl;
    find_candidate_in_quad(
        x, y, dx_node, dy_node, min_city_block_node, delete_node, quad_southwest);

    cout << "southeast" << endl;
    find_candidate_in_quad(
        x, y, dx_node, dy_node, min_city_block_node, delete_node, quad_southeast);

    // Check Criterion 1.

    if (dx_node.node)
        cout << "node closest to x axis: " << *dx_node.node->data << " (dx=" << dx_node.dist << ")" << endl;

    if (dy_node.node)
        cout << "node closest to y axis: " << *dy_node.node->data << " (dy=" << dy_node.dist << ")" << endl;

    if (dx_node.node == dy_node.node && ((dx_node.quad == quad_northwest) || (dx_node.quad == quad_southeast)))
    {
        cout << "node that satisfies Criterion 1: " << *dx_node.node->data << endl;
        return dx_node.node;
    }
    else
        cout << "unable to find node that satisfies Criterion 1." << endl;

    // Move on to Criterion 2.

    if (min_city_block_node.node)
    {
        cout << "node that satisfies Criterion 2: " << *min_city_block_node.node->data << " (dist=" << min_city_block_node.dist << ")" << endl;
        return min_city_block_node.node;
    }

    return node_ptr();
}

template<typename _Key, typename _Data>
void point_quad_tree<_Key,_Data>::find_candidate_in_quad(
    key_type x, key_type y, 
    node_distance& dx_node, node_distance& dy_node, node_distance& min_city_block_node,
    const node_ptr& delete_node, node_quadrant_t quad) const
{
    using namespace std;

    node_ptr repl_node = delete_node->get_quadrant_node(quad);
    if (!repl_node)
    {
        // No candidate in this quadrant.
        cout << "  no candidate in this quadrant" << endl;
        return;
    }

    node_quadrant_t oppo_quad = opposite(quad);
    while (repl_node->has_quadrant_node(oppo_quad))
        repl_node = repl_node->get_quadrant_node(oppo_quad);

    cout << "  candidate: " << repl_node->x << "," << repl_node->y << " (" << *repl_node->data << ")" << endl;

    // Calculate its distance to each of the borders.
    key_type dx = repl_node->x > x ? repl_node->x - x : x - repl_node->x;
    key_type dy = repl_node->y > y ? repl_node->y - y : y - repl_node->y;
    cout << "  dx = " << dx << ", dy = " << dy << endl;

    if (!dx_node.node || dx_node.dist > dx)
        dx_node = node_distance(quad, dx, repl_node);
    if (!dy_node.node || dy_node.dist > dy)
        dy_node = node_distance(quad, dy, repl_node);

    if (!min_city_block_node.node || min_city_block_node.dist > (dx + dy))
        min_city_block_node = node_distance(quad_unspecified, dx+dy, repl_node);
}

template<typename _Key, typename _Data>
void point_quad_tree<_Key,_Data>::adjust_quad(
    const key_range_type& hatched_xrange, const key_range_type& hatched_yrange, 
    node_ptr quad_root, direction_t dir, reinsert_tree_array_type& insert_list)
{
    using namespace std;

    if (!quad_root)
        return;

    cout << "adjust_quad: checking " << *quad_root->data << " (" << quad_root->x << "," << quad_root->y << ")" << endl;

    if ((hatched_xrange.first <= quad_root->x && quad_root->x <= hatched_xrange.second) ||
        (hatched_yrange.first <= quad_root->y && quad_root->y <= hatched_yrange.second))
    {
        cout << "  " << *quad_root->data << " is in the hatched region" << endl;
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

template<typename _Key, typename _Data>
void point_quad_tree<_Key,_Data>::set_new_root(
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

template<typename _Key, typename _Data>
void point_quad_tree<_Key,_Data>::insert_node(node_ptr& dest, node_ptr& node)
{
    node_ptr cur_node = dest;
    while (true)
    {
        if (cur_node->x == node->x && cur_node->y == node->y)
        {
            // When inserting a node instance directly (likely as part of tree
            // re-insertion), we are not supposed to have another node at
            // identical position.
            throw general_error("node with identical position encountered.");
        }

        node_quadrant_t quad = cur_node->get_quadrant(node->x, node->y);
        switch (quad)
        {
            case quad_northeast:
                if (cur_node->northeast)
                    cur_node = cur_node->northeast;
                else
                {
                    cur_node->northeast = node;
                    node->parent = cur_node;
                    return;
                }
                break;
            case quad_northwest:
                if (cur_node->northwest)
                    cur_node = cur_node->northwest;
                else
                {
                    cur_node->northwest = node;
                    node->parent = cur_node;
                    return;
                }
                break;
            case quad_southeast:
                if (cur_node->southeast)
                    cur_node = cur_node->southeast;
                else
                {
                    cur_node->southeast = node;
                    node->parent = cur_node;
                    return;
                }
                break;
            case quad_southwest:
                if (cur_node->southwest)
                    cur_node = cur_node->southwest;
                else
                {
                    cur_node->southwest = node;
                    node->parent = cur_node;
                    return;
                }
                break;
            default:
                throw general_error("unknown quadrant");
        }
    }
    assert(!"This should never be reached.");
}

template<typename _Key, typename _Data>
void point_quad_tree<_Key,_Data>::reinsert_tree(node_ptr& dest, node_ptr& root)
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

template<typename _Key, typename _Data>
void point_quad_tree<_Key,_Data>::reinsert_tree(node_ptr& dest, node_quadrant_t quad, node_ptr& root)
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
        case quad_unspecified:
        default:
            throw general_error("reinsert_tree: quadrant unspecified");
    }
}

template<typename _Key, typename _Data>
void point_quad_tree<_Key,_Data>::clear_all_nodes()
{
    ::mdds::disconnect_all_nodes(m_root);
    m_root.reset();
}

template<typename _Key, typename _Data>
void point_quad_tree<_Key,_Data>::dump_node_svg(const node* p, ::std::ofstream& file) const
{
    using namespace std;

    if (!p)
        return;

    file << "<circle cx=\"" << p->x << "\" cy=\"" << p->y << "\" r=\"0.3\""
        << " fill=\"black\" stroke=\"black\"/>" << endl;
    file << "<text x=\"" << p->x + 1 << "\" y=\"" << p->y + 3 << "\" font-size=\"3\" fill=\"black\">"
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

}

#endif
