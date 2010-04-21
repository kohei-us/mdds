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

    struct node : quad_node_base<node_ptr, node>
    {
        data_type* data;
        node(bool _is_leaf) :
            quad_node_base<node_ptr, node>(_is_leaf),
            data(NULL) {}

        void dispose()
        {
        }
    };

    point_quad_tree();
    ~point_quad_tree();

private:
    node_ptr    m_root;
};

template<typename _Key, typename _Data>
point_quad_tree<_Key,_Data>::point_quad_tree() :
    m_root(NULL)
{
}

template<typename _Key, typename _Data>
point_quad_tree<_Key,_Data>::~point_quad_tree()
{
}

}

#endif
