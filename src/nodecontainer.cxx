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

#include "nodecontainer.hxx"
#include <cassert>

using ::std::map;
using ::std::vector;

namespace mdds {

NodeContainer2D::NodeContainer2D()
{
}

NodeContainer2D::~NodeContainer2D()
{
}

void NodeContainer2D::add(const Node& node)
{
    // Check if the x value already exists.
    XYNodeMap::iterator posX = mNodeMap.find(node.x);
    {
        XYNodeMap::iterator itrEnd = mNodeMap.end();
        if (posX == itrEnd)
        {
            // This x value does not exist yet.  Insert a new map for it.
            if (!mNodeMap.insert(XYNodeMap::value_type(node.x, YNodeMap())).second)
                // insertion failed.
                return;
    
            posX = mNodeMap.find(node.x);
            assert(posX != itrEnd);
        }
    }

    // Now, check if a map for this y value already exists.  If not, create one.
    YNodeMap& ymap = posX->second;
    YNodeMap::iterator posY = ymap.find(node.y);
    {
        YNodeMap::iterator itrEnd = ymap.end();
        if (posY == itrEnd)
        {
            if (!ymap.insert(YNodeMap::value_type(node.y, NodeList())).second)
                // insertion failed.
                return;
    
            posY = ymap.find(node.y);
            assert(posY != itrEnd);
        }
    }

    NodeList& nodelist = posY->second;
    nodelist.push_back(node);
}

void NodeContainer2D::getAllNodes(vector<Node>& nodes) const
{
    XYNodeMap::const_iterator itrX = mNodeMap.begin(), itrXEnd = mNodeMap.end();
    for (; itrX != itrXEnd; ++itrX)
    {
        YNodeMap::const_iterator itrY = itrX->second.begin(), itrYEnd = itrX->second.end();
        for (; itrY != itrYEnd; ++itrY)
            copy(itrY->second.begin(), itrY->second.end(), back_inserter(nodes));
    }
}

void NodeContainer2D::clear()
{
    mNodeMap.clear();
}

}
