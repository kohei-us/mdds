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

enum node_quadrant_t
{
    quad_north_east,
    quad_north_west,
    quad_south_east,
    quad_south_west
};

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
            return other_y < y ? quad_north_west : quad_south_west;

        // east
        return other_y < y ? quad_north_east : quad_south_west;
    }

    node_ptr get_quadrant_node(key_type other_x, key_type other_y)
    {
        node_quadrant_t quad = get_quadrant(other_x, other_y);
        switch (quad)
        {
            case quad_north_east:
                return northeast;
            case quad_north_west:
                return northwest;
            case quad_south_east:
                return southeast;
            case quad_south_west:
                return southwest;
            default:
                throw general_error("unknown quadrant type");
        }
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
void disconnect_node(_NodePtr p)
{
    if (!p)
        return;

    p->parent.reset();
    p->northeast.reset();
    p->northwest.reset();
    p->southeast.reset();
    p->southwest.reset();
}

}

#endif
