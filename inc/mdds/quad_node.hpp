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

#ifndef __MDDS_QUAD_NODE_HPP__
#define __MDDS_QUAD_NODE_HPP__

#include "mdds/global.hpp"

#include <boost/intrusive_ptr.hpp>

namespace mdds {

#ifdef DEBUG_NODE_BASE
size_t node_instance_count = 0;
#endif

/**
 *   NW | NE
 * -----|-----
 *   SW | SE
 */
enum node_quadrant_t
{
    quad_northeast,
    quad_northwest,
    quad_southeast,
    quad_southwest,
    quad_unspecified
};

/** 
 *  NW  |  N  | NE
 * -----|-----|----- 
 *   W  |  C  |  E
 * -----|-----|----- 
 *  SW  |  S  | SE
 */
enum search_region_space_t
{
    region_northwest,
    region_north,
    region_northeast,
    region_east,
    region_southeast,
    region_south,
    region_southwest,
    region_west,
    region_center
};

/** 
 *        N 
 *        |
 *        |
 * W -----+----- E
 *        |
 *        |
 *        S
 */ 
enum direction_t
{
    dir_north,
    dir_west,
    dir_south,
    dir_east
};

node_quadrant_t opposite(node_quadrant_t quad)
{
    switch (quad)
    {
        case quad_northeast:
            return quad_southwest;
        case quad_northwest:
            return quad_southeast;
        case quad_southeast:
            return quad_northwest;
        case quad_southwest:
            return quad_northeast;
        case quad_unspecified:
        default:
            ;
    }
    return quad_unspecified;
}

template<typename _NodePtr, typename _NodeType, typename _Key>
struct quad_node_base
{    
#ifdef DEBUG_NODE_BASE
    static size_t get_instance_count()
    {
        return node_instance_count;
    }
#endif
    typedef _Key        key_type;
    typedef _NodePtr    node_ptr;
    typedef _NodeType   node_type;

    size_t      refcount;

    node_ptr    parent;
    node_ptr    northeast;
    node_ptr    northwest;
    node_ptr    southeast;
    node_ptr    southwest;

    key_type    x;
    key_type    y;

    quad_node_base(key_type _x, key_type _y) :
        refcount(0),
        parent(NULL),
        northeast(NULL),
        northwest(NULL),
        southeast(NULL),
        southwest(NULL),
        x(_x), 
        y(_y)
    {
#ifdef DEBUG_NODE_BASE
        ++node_instance_count;
#endif
    }

    /** 
     * When copying node, only the stored values should be copied. 
     * Connections to the parent and the neighboring nodes must not be copied.
     */
    quad_node_base(const quad_node_base& r) :
        refcount(0),
        parent(NULL),
        northeast(NULL),
        northwest(NULL),
        southeast(NULL),
        southwest(NULL),
        x(r.x), 
        y(r.y)
    {
#ifdef DEBUG_NODE_BASE
        ++node_instance_count;
#endif
    }

    bool leaf() const
    {
        return !northeast && !northwest && !southeast && !southwest;
    }

    bool operator==(const quad_node_base& r) const
    {
        return x == r.x && y == r.y;
    }

    /** 
     * Like the copy constructor, only the stored values should be copied. 
     */
    quad_node_base& operator=(const quad_node_base& r)
    {
        if (this == &r)
            // assignment to self.
            return *this;

        x = r.x;
        y = r.y;
        return *this;
    }

    ~quad_node_base()
    {
#ifdef DEBUG_NODE_BASE
        --node_instance_count;
#endif
        static_cast<node_type*>(this)->dispose();
    }

    /**
     * Return the quadrant of specified point in reference to this node.
     * 
     * @return quadrant where the other node is located in reference to this 
     *         node.
     */
    node_quadrant_t get_quadrant(key_type other_x, key_type other_y) const
    {
        if (other_x < x)
            // west
            return other_y < y ? quad_northwest : quad_southwest;

        // east
        return other_y < y ? quad_northeast : quad_southeast;
    }

    node_ptr get_quadrant_node(node_quadrant_t quad) const
    {
        switch (quad)
        {
            case quad_northeast:
                return northeast;
            case quad_northwest:
                return northwest;
            case quad_southeast:
                return southeast;
            case quad_southwest:
                return southwest;
            default:
                throw general_error("unknown quadrant type");
        }
        return node_ptr();
    }
};

template<typename _NodePtr>
inline void intrusive_ptr_add_ref(_NodePtr p)
{
    ++p->refcount;
}

template<typename _NodePtr>
inline void intrusive_ptr_release(_NodePtr p)
{
    --p->refcount;
    if (!p->refcount)
        delete p;
}

template<typename _NodePtr>
void disconnect_node_from_parent(_NodePtr p)
{
    if (!p || !p->parent)
        // Nothing to do.
        return;

    _NodePtr& parent = p->parent;
    if (parent->northeast && parent->northeast == p)
    {
        parent->northeast.reset();
    }
    else if (parent->northwest && parent->northwest == p)
    {
        parent->northwest.reset();
    }
    else if (parent->southwest && parent->southwest == p)
    {
        parent->southwest.reset();
    }
    else if (parent->southeast && parent->southeast == p)
    {
        parent->southeast.reset();
    }
    else
        throw general_error("parent node doesn't lead to the deleted node.");
}

template<typename _NodePtr>
void disconnect_all_nodes(_NodePtr p)
{
    if (!p)
        return;

    if (p->northeast)
    {
        disconnect_all_nodes(p->northeast);
        p->northeast.reset();
    }

    if (p->northwest)
    {
        disconnect_all_nodes(p->northwest);
        p->northwest.reset();
    }

    if (p->southeast)
    {
        disconnect_all_nodes(p->southeast);
        p->southeast.reset();
    }

    if (p->southwest)
    {
        disconnect_all_nodes(p->southwest);
        p->southwest.reset();
    }

    p->parent.reset();
}

template<typename _NodeType, typename _Key>
search_region_space_t get_search_region_space(
    _NodeType* p, _Key x1, _Key y1, _Key x2, _Key y2)
{
    typedef _Key key_type;

    key_type x = p->x, y = p->y;
    if (x < x1)
    {
        // western region
        if (y < y1)
        {
            return region_northwest;
        }
        else if (y1 <= y && y <= y2)
        {
            return region_west;
        }

        assert(y2 < y);
        return region_southwest;
    }
    else if (x1 <= x && x <= x2)
    {
        // central region
        if (y < y1)
        {
            return region_north;
        }
        else if (y1 <= y && y <= y2)
        {
            return region_center;
        }

        assert(y2 < y);
        return region_south;
    }
    
    // eastern region
    assert(x2 < x);
    if (y < y1)
    {
        return region_northeast;
    }
    else if (y1 <= y && y <= y2)
    {
        return region_east;
    }
    
    assert(y2 < y);
    return region_southeast;
}

}

#endif
