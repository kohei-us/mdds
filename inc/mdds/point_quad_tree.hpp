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

namespace mdds {

template<typename _Key, typename _Data>
class point_quad_tree
{
public:
    typedef _Key    key_type;
    typedef _Data   data_type;
    typedef size_t  size_type;

    struct node;
    typedef ::boost::intrusive_ptr<node> node_ptr;

    struct node : quad_node_base<node_ptr, node, key_type>
    {
        data_type* data;
        node(key_type _x, key_type _y, data_type* _data) :
            quad_node_base<node_ptr, node, key_type>(_x, _y),
            data(_data) {}

        void dispose()
        {
        }
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
    void search_region(key_type x1, key_type y1, key_type x2, key_type y2) const;

    void remove(data_type* data);

    void dump_tree_svg(const ::std::string& fpath) const;

private:
    search_region_space_t get_search_region_space(
        const node_ptr& node, key_type x1, key_type y1, key_type x2, key_type y2) const;

    void search_region_node(const node* p, key_type x1, key_type y1, key_type x2, key_type y2) const;

    void dump_node_svg(const node* p, ::std::ofstream& file) const;

private:
    node_ptr    m_root;

    ::std::pair<key_type, key_type> m_xrange;
    ::std::pair<key_type, key_type> m_yrange;
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
void point_quad_tree<_Key,_Data>::search_region(key_type x1, key_type y1, key_type x2, key_type y2) const
{
    using namespace std;
    cout << "search region: (" << x1 << "," << y1 << ") - (" << x2 << "," << y2 << ")" << endl;
    const node* p = m_root.get();
    search_region_node(p, x1, y1, x2, y2);
}

template<typename _Key, typename _Data>
void point_quad_tree<_Key,_Data>::search_region_node(const node* p, key_type x1, key_type y1, key_type x2, key_type y2) const
{
    using namespace std;

    if (!p)
        return;

    search_region_space_t region = ::mdds::get_search_region_space(p, x1, y1, x2, y2);
    
    switch (region)
    {
        case region_center:
            cout << *p->data << " (" << p->x << "," << p->y << ")" << endl;
            search_region_node(p->northeast.get(), x1, y1, x2, y2);
            search_region_node(p->northwest.get(), x1, y1, x2, y2);
            search_region_node(p->southeast.get(), x1, y1, x2, y2);
            search_region_node(p->southwest.get(), x1, y1, x2, y2);
            break;
        case region_east:
            search_region_node(p->northwest.get(), x1, y1, x2, y2);
            search_region_node(p->southwest.get(), x1, y1, x2, y2);
            break;
        case region_north:
            search_region_node(p->southeast.get(), x1, y1, x2, y2);
            search_region_node(p->southwest.get(), x1, y1, x2, y2);
            break;
        case region_northeast:
            search_region_node(p->southwest.get(), x1, y1, x2, y2);
            break;
        case region_northwest:
            search_region_node(p->southeast.get(), x1, y1, x2, y2);
            break;
        case region_south:
            search_region_node(p->northeast.get(), x1, y1, x2, y2);
            search_region_node(p->northwest.get(), x1, y1, x2, y2);
            break;
        case region_southeast:
            search_region_node(p->northwest.get(), x1, y1, x2, y2);
            break;
        case region_southwest:
            search_region_node(p->northeast.get(), x1, y1, x2, y2);
            break;
        case region_west:
            search_region_node(p->northeast.get(), x1, y1, x2, y2);
            search_region_node(p->southeast.get(), x1, y1, x2, y2);
            break;
        default:
            throw general_error("unknown search region");
    }
}

template<typename _Key, typename _Data>
void point_quad_tree<_Key,_Data>::remove(data_type* data)
{
}

template<typename _Key, typename _Data>
void point_quad_tree<_Key,_Data>::dump_tree_svg(const ::std::string& fpath) const
{
    using namespace std;
    ofstream file(fpath.c_str());
    file << "<svg width=\"14cm\" height=\"14cm\" viewBox=\"-2 -2 202 202\" xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" << endl;
    file << "<defs>"
         << "  <marker id=\"Triangle\""
         << "    viewBox=\"0 0 10 10\" refX=\"10\" refY=\"5\" "
         << "    markerUnits=\"strokeWidth\""
         << "    markerWidth=\"8\" markerHeight=\"6\""
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
void point_quad_tree<_Key,_Data>::dump_node_svg(const node* p, ::std::ofstream& file) const
{
    using namespace std;

    if (!p)
        return;

    file << "<circle cx=\"" << p->x << "\" cy=\"" << p->y << "\" r=\"0.5\""
        << " fill=\"black\" stroke=\"black\"/>" << endl;
    file << "<text x=\"" << p->x + 1 << "\" y=\"" << p->y + 6 << "\" font-size=\"5\" fill=\"black\">"
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
