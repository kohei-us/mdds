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

#include <boost/intrusive_ptr.hpp>

namespace mdds {

#ifdef DEBUG_NODE_BASE
size_t node_instance_count = 0;
#endif

template<typename _NodePtr, typename _NodeType>
struct quad_node_base
{    
#ifdef DEBUG_NODE_BASE
    static size_t get_instance_count()
    {
        return node_instance_count;
    }
#endif
    size_t      refcount;

    _NodePtr    parent;
    _NodePtr    northeast;
    _NodePtr    northwest;
    _NodePtr    southeast;
    _NodePtr    southwest;
    bool        is_leaf;

    quad_node_base(bool _is_leaf) :
        refcount(0),
        is_leaf(_is_leaf)
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
        is_leaf(r.is_leaf)
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

        is_leaf = r.is_leaf;
        return *this;
    }

    ~quad_node_base()
    {
#ifdef DEBUG_NODE_BASE
        --node_instance_count;
#endif
        static_cast<_NodeType*>(this)->dispose();
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
