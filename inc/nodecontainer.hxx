/*************************************************************************
 *
 * Copyright (c) 2008-2009 Kohei Yoshida
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

#ifndef _NODECONTAINER_HXX_
#define _NODECONTAINER_HXX_

#include <map>
#include <vector>
#include "global.hxx"

namespace mdds {

/** 
 * This class stores nodes with 2-dimensional coordinates sorted first by
 * their x values then by their y values.
 */
class NodeContainer2D
{
public:
    struct Node
    {
        IntX x;
        IntY y;
    };

public:
    NodeContainer2D();
    ~NodeContainer2D();

    void add(const Node& node);
    void getAllNodes(::std::vector<Node>& nodes) const;
    void clear();

private:
    typedef ::std::vector<Node>         NodeList;
    typedef ::std::map<IntY, NodeList>  YNodeMap;
    typedef ::std::map<IntX, YNodeMap>  XYNodeMap;

    XYNodeMap mNodeMap;
};

}


#endif
