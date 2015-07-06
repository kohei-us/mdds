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

#include "rangetree.hxx"
#include "global.hxx"
#include <stdio.h>
#include <string>
#include <vector>
#include <cassert>
#include <cmath>

#define SHRINK_DATA_OBJECT_SIZE 1
#define DEBUG_SEARCH 1

using namespace std;
using namespace boost;

namespace mdds {

RangeTree::Data::Data(const char* _name, IntX _x, IntY _y, const NodePtr& _maxHeap) :
    name(_name), x(_x), y(_y), maxHeap(_maxHeap)
{
}

RangeTree::Data::Data(const Data& data) :
    name(data.name), x(data.x), y(data.y), maxHeap(data.maxHeap),
    xlow(data.xlow), xhigh(data.xhigh)
{
}

RangeTree::PrioSearchData* RangeTree::Data::getPrioSearchData()
{
    return static_cast<RangeTree::PrioSearchData*>(this);
}

// ---------------------------------------------------------------------------

RangeTree::PrioSearchData::PrioSearchData(const char* _name, IntX _x, IntY _y, const NodePtr& _maxHeap) :
    RangeTree::Data(_name, _x, _y, _maxHeap)
{
}

// ---------------------------------------------------------------------------

RangeTree::Node::Node(const char* _name, IntX _x, IntY _y, bool _isLeaf) :
    data(new PrioSearchData(_name, _x, _y, NodePtr())),
    left(static_cast<Node*>(nullptr)),
    right(static_cast<Node*>(nullptr)),
    next(static_cast<Node*>(nullptr)),
    isLeaf(_isLeaf)
{
}

// ---------------------------------------------------------------------------

RangeTree::RangeTree() :
    mpLeftLeafNode(static_cast<Node*>(nullptr))
{
}

RangeTree::~RangeTree()
{
}

void RangeTree::insertPoint(const char* name, IntX x, IntY y)
{
    NodePtr newNode(new Node(name, x, y));
    PrioSearchData* psdata = newNode->data->getPrioSearchData();
    psdata->xlow = psdata->xhigh = x;
    psdata->heaps.insert(HeapMap::value_type(psdata->y, newNode));

    if (empty(mpLeftLeafNode))
    {
        // No previous points.  This must be the first node.
        mpLeftLeafNode = newNode;
        return;
    }

    NodePtr pCurNode = mpLeftLeafNode;
    while (true)
    {
        if (x == pCurNode->data->x && y == pCurNode->data->y)
        {
            // Both x and y values are equal.  Make the new node the front node 
            // and push the previous node(s) to the back.

            if (!pCurNode->left.get())
                mpLeftLeafNode = newNode;

            newNode->left  = pCurNode->left;
            newNode->right = pCurNode->right;
            pCurNode->left.reset(static_cast<Node*>(nullptr));
            pCurNode->right.reset(static_cast<Node*>(nullptr));

            if (newNode->left.get())
                newNode->left->right = newNode;
            if (newNode->right.get())
                newNode->right->left = newNode;

            newNode->next = pCurNode;
            return;
        }

        if (x < pCurNode->data->x || (x == pCurNode->data->x && y <= pCurNode->data->y))
        {
            // The x value is greater.  Insert it at the current position and 
            // move the existing node(s) to the right.

            if (!pCurNode->left.get())
                mpLeftLeafNode = newNode;

            if (pCurNode->left.get())
                pCurNode->left->right = newNode;

            newNode->left = pCurNode->left;
            pCurNode->left = newNode;
            newNode->right = pCurNode;

            return;
        }

        if (empty(pCurNode->right))
            // there is no more node to the right.  Break out of the loop.
            break;

        pCurNode = pCurNode->right;
    } 

    // We're at the end of the node chain.  Append the new node to the end.
    pCurNode->right = newNode;
    newNode->left = pCurNode;
}

void RangeTree::build()
{
    if (empty(mpLeftLeafNode))
        // No node.
        return;

    // I only do this just to count the nodes.  Maybe I can do this at node
    // insertion time so that I can skip this code altogether ?

    NodePtr pCurNode = mpLeftLeafNode;
    size_t nodeCount = 0;
    while (true)
    {
        ++nodeCount;
        NodePtr pChain = pCurNode->next;
        while (!empty(pChain))
            pChain = pChain->next;

        if (empty(pCurNode->right))
            break;

        pCurNode = pCurNode->right;
    }

    if (nodeCount == 0)
        return;
    else if (nodeCount == 1)
    {
        mpRootNode = mpLeftLeafNode;
        return;
    }

    // Start to build up the tree.
    buildTree();
}

void RangeTree::dump() const
{
    list<NodePtr> nodeList;
    nodeList.push_back(mpRootNode);
    dumpTree(nodeList);

    fprintf(stdout, "**************************************************\n");
}

void RangeTree::search(IntX xlow, IntX xhigh) const
{
    fprintf(stdout, "RangeTree::search: %d - %d\n", xlow, xhigh);

    if (empty(mpRootNode))
        return;

    NodePtr pNode = mpRootNode;
    while (!pNode->isLeaf)
    {
        if (xlow <= pNode->data->x)
            pNode = pNode->left;
        else
            // Note that the right node can be empty.
            pNode = empty(pNode->right) ? pNode->left : pNode->right;
    }

    if (!empty(pNode) && pNode->data->x < xlow)
        pNode = pNode->right;

    while (!empty(pNode) && pNode->data->x <= xhigh)
    {
        fprintf(stdout, "RangeTree::search: '%s' (%d, %d)\n", pNode->data->name.c_str(), 
                pNode->data->x, pNode->data->y);fflush(stdout);
        pNode = pNode->right;
    }
}

void RangeTree::search(IntX xlow, IntX xhigh, IntY ylow)
{
    fprintf(stdout, "RangeTree::search: ----------------------------------------------------------------------\n");fflush(stdout);
    fprintf(stdout, "RangeTree::search: priority search x = %d to %d;  y = %d to inf\n",
            xlow, xhigh, ylow);fflush(stdout);

    mHitNodes.clear();

    // TODO:
    // 1) When the desired range contains the root range, run searchY on the
    // entire tree.

    NodePtr node = findCommonAncestor(mpRootNode, xlow, xhigh, ylow);
    if (empty(node))
        return;

//  output(node, "common ancestor is", "", true);

    // node is a nonleaf node and must process its subtrees.
    NodePtr nodeCommon = node;
    node = node->left;
    // Process the left subtree.
//  fprintf(stdout, "RangeTree::search: process left subtree...\n");fflush(stdout);
    while (true)
    {
        NodePtr heapNode = heapMax(node);
        if (isOutOfRangeY(heapNode, ylow))
            break;

        if (xlow <= heapNode->data->x)
        {
//          output(heapNode, "", "(left)");
            addHitNode(heapNode);
        }

        if (node->isLeaf)
            break;

        RangeOverlap rightStatus = checkRangeOverlap(node->right, xlow, xhigh);

        if (rightStatus == RANGE_CONTAINED)
        {
//          fprintf(stdout, "RangeTree::search: go left\n");fflush(stdout);
            // right child tree falls entirely within the desired x-range,
            // while the left tree is not.
            searchY(node->right, ylow);
            node = node->left;
        }
        else
        {
//          fprintf(stdout, "RangeTree::search: go right\n");fflush(stdout);
            node = node->right;
        }
    }

    node = nodeCommon->right;

    // Process the right subtree.
//  fprintf(stdout, "RangeTree::search: process right subtree...\n");fflush(stdout);
    while (true)
    {
        if (empty(node))
            break;

        NodePtr heapNode = heapMax(node);
        if (isOutOfRangeY(heapNode, ylow))
            return;

        if (heapNode->data->x <= xhigh)
        {
//          output(heapNode, "", "(right)");
            addHitNode(heapNode);
        }

        if (node->isLeaf)
            return;

//      fprintf(stdout, "RangeTree::search: mid range = %g\n", node->data->midRange);fflush(stdout);

        RangeOverlap leftStatus = checkRangeOverlap(node->left, xlow, xhigh);

        if (leftStatus == RANGE_CONTAINED)
        {
//          fprintf(stdout, "RangeTree::search: go right\n");fflush(stdout);
            // left child tree falls entirely within the desired x-range,
            // while the right tree is not.
            searchY(node->left, ylow);
            node = node->right;
        }
        else
        {
//          fprintf(stdout, "RangeTree::search: go left\n");fflush(stdout);
            node = node->left;
        }
    }
}

void RangeTree::printSearchResult() const
{
    vector<NodeContainer2D::Node> hitNodes;
    mHitNodes.getAllNodes(hitNodes);
    fprintf(stdout, "RangeTree::search: hit node size = %d\n", hitNodes.size());fflush(stdout);
    for (vector<NodeContainer2D::Node>::const_iterator itr = hitNodes.begin(), itrEnd = hitNodes.end();
         itr != itrEnd; ++itr)
    {
        fprintf(stdout, "  node: (%d, %d)\n", itr->x, itr->y);
    }
}

void RangeTree::addHitNode(const NodePtr& node)
{
    NodeContainer2D::Node node2;
    node2.x = node->data->x;
    node2.y = node->data->y;
    mHitNodes.add(node2);
//  mHitNodes.push_back(node);
}

void RangeTree::output(NodePtr node, const char* prefix, const char* suffix, bool printMaxHeap) const
{
    const DataPtr& data = node->data;
    const NodePtr& maxHeap = node->data->maxHeap;
    fprintf(stdout, "%s '%s' - (%d, %d)", prefix, data->name.c_str(), data->x, data->y);
    if (!empty(maxHeap) && printMaxHeap)
    {
        const DataPtr& heapData = maxHeap->data;
        fprintf(stdout, " [max heap '%s' (%d, %d)]", heapData->name.c_str(),
                heapData->x, heapData->y);
    }
    fprintf(stdout, " %s\n", suffix);
}

RangeTree::NodePtr RangeTree::heapMax(NodePtr node) const
{
    if (empty(node))
        return node;

    return node->data->maxHeap;
}

void RangeTree::searchY(NodePtr node, IntY ylow)
{
    if (empty(node))
        return;

    NodePtr heapNode = heapMax(node);
    if (isOutOfRangeY(heapNode, ylow))
        return;

//  output(heapNode, "", "(y-search)");
    addHitNode(heapNode);

    searchY(node->left,  ylow);
    searchY(node->right, ylow);
}

bool RangeTree::isOutOfRangeY(NodePtr node, IntY ylow) const
{
    if (empty(node))
        return true;

    return node->data->y < ylow;
}

void RangeTree::dumpTree(const list<NodePtr>& nodeList, int level) const
{
    size_t nodeCount = nodeList.size();
    if (nodeCount == 0)
        return;

    fprintf(stdout, "-------------------- level %d\n", level);
    fprintf(stdout, "RangeTree::dumpTree: node count = %d\n", nodeCount);fflush(stdout);
    list<NodePtr> newNodeList;
    list<NodePtr>::const_iterator itr = nodeList.begin(), itrEnd = nodeList.end();
    for (; itr != itrEnd; ++itr)
    {
        NodePtr pNode = *itr;
        fprintf(stdout, "  '%s' - (%d, %d) - leaf (%d) - ", 
                pNode->data->name.c_str(), pNode->data->x, pNode->data->y, 
                pNode->isLeaf);

        if (!empty(pNode->data->maxHeap))
        {
            const DataPtr& heapData = pNode->data->maxHeap->data;
            fprintf(stdout, "max heap ['%s' - (%d, %d)]", 
                    heapData->name.c_str(), heapData->x, heapData->y);
        }

#if not SHRINK_DATA_OBJECT_SIZE
        // y-value list for priority search tree.
        PrioSearchData* psData = pNode->data->getPrioSearchData();
        HeapMap::const_iterator itrPS = psData->heaps.begin(), itrPSEnd = psData->heaps.end();
        for (; itrPS != itrPSEnd; ++itrPS)
            fprintf(stdout, " %d", itrPS->first);
#endif

        // x ranges
        fprintf(stdout, " (%d - %d)", pNode->data->xlow, pNode->data->xhigh);
        fprintf(stdout, "\n");

        if (!pNode->isLeaf && !empty(pNode->left))
            newNodeList.push_back(pNode->left);
        if (!pNode->isLeaf && !empty(pNode->right))
            newNodeList.push_back(pNode->right);
    }
    dumpTree(newNodeList, level+1);
}

RangeTree::NodePtr RangeTree::makeParentNode(const NodePtr& node1, const NodePtr& node2) const
{
    NodePtr parentNode;
    if (empty(node2))
    {
        parentNode.reset(new Node("", node1->data->x + 1, 0, false));
        node1->parent = parentNode;
        parentNode->left = node1;

        // for priority search tree
        PrioSearchData* parentPS = parentNode->data->getPrioSearchData();
        parentPS->xlow = node1->data->getPrioSearchData()->xlow;
        parentPS->xhigh = node1->data->getPrioSearchData()->xhigh;
    }
    else
    {
        parentNode.reset(new Node("", (node1->data->x + node2->data->x)/2, 0, false));
    
        node1->parent = parentNode;
        node2->parent = parentNode;
        parentNode->left  = node1;
        parentNode->right = node2;
    
        // for priority search tree
        PrioSearchData* parentPS = parentNode->data->getPrioSearchData();
        parentPS->xlow  = node1->data->getPrioSearchData()->xlow;
        parentPS->xhigh = node2->data->getPrioSearchData()->xhigh;    
    }
    return parentNode;
}

void RangeTree::buildTree()
{
    NodePtr pNode1 = mpLeftLeafNode;
    if (empty(mpLeftLeafNode))
        // The tree is empty.
        return;

    // Sweep the leaf nodes.

    list<NodePtr> nodeList;
    while (true)
    {
        NodePtr pNode2 = pNode1->right;
        if (!empty(pNode2))
        {
            NodePtr parentNode = makeParentNode(pNode1, pNode2);
            nodeList.push_back(parentNode);
            PrioSearchData* parentPS = parentNode->data->getPrioSearchData();
            parentPS->heaps.insert(HeapMap::value_type(pNode1->data->y, pNode1));
            parentPS->heaps.insert(HeapMap::value_type(pNode2->data->y, pNode2));
        }
        else
        {
            NodePtr parentNode = makeParentNode(pNode1, NodePtr());
            nodeList.push_back(parentNode);
            PrioSearchData* parentPS = parentNode->data->getPrioSearchData();
            parentPS->heaps.insert(HeapMap::value_type(pNode1->data->y, pNode1));
        }
        
        if (!pNode2.get() || !pNode2->right.get())
            // no more nodes.  Break out of the loop.
            break;

        pNode1 = pNode2->right;
    }

    buildTreeNonLeaf(nodeList);
    buildPrioSearchTree();
    dump();
}

void RangeTree::combineMaps(HeapMap& rSet1, const HeapMap& rSet2) const
{
    HeapMap::const_iterator itr = rSet2.begin(), itrEnd = rSet2.end();
    for (; itr != itrEnd; ++itr)
        rSet1.insert(HeapMap::value_type(itr->first, itr->second));
}

void RangeTree::buildTreeNonLeaf(const list<NodePtr>& nodeList)
{
    size_t nodeCount = nodeList.size();
    if (nodeCount == 1)
    {
        mpRootNode = nodeList.front();
        return;
    }
    else if (nodeCount == 0)
        return;

    list<NodePtr> newNodeList;
    vector<NodePtr> nodePair;
    nodePair.reserve(2);
    list<NodePtr>::const_iterator itr = nodeList.begin(), itrEnd = nodeList.end();
    for (; itr != itrEnd; ++itr)
    {
        nodePair.push_back(*itr);
        if (nodePair.size() == 2)
        {
            NodePtr parentNode = makeParentNode(nodePair[0], nodePair[1]);
            PrioSearchData* parentPS = parentNode->data->getPrioSearchData();
            combineMaps(parentPS->heaps, nodePair[0]->data->getPrioSearchData()->heaps);
            combineMaps(parentPS->heaps, nodePair[1]->data->getPrioSearchData()->heaps);

            nodePair.clear();
            newNodeList.push_back(parentNode);
        }
    }

    if (!nodePair.empty())
    {
        NodePtr parentNode = makeParentNode(nodePair[0], NodePtr());
        PrioSearchData* parentPS = parentNode->data->getPrioSearchData();
        combineMaps(parentPS->heaps, nodePair[0]->data->getPrioSearchData()->heaps);

        nodePair.clear();
        newNodeList.push_back(parentNode);
    }

    // Move up one level, and do the same procedure until the root node is reached.
    buildTreeNonLeaf(newNodeList);
}

void RangeTree::buildPrioSearchTree()
{
    // At this point, the nodes in the tree have accumulated the y values from
    // bottom up.  Run down from the top root node while picking the maximum y
    // value for each node.  Note that once a number is picked for a node, 
    // that same number cannnot be used again for any of its child nodes.

    walkNode(mpRootNode);
}

void RangeTree::walkNode(NodePtr node)
{
    // y-value list for priority search tree.
    HeapMap& heaps = node->data->getPrioSearchData()->heaps;

    HeapMap::reverse_iterator itr = heaps.rbegin(), itrEnd = heaps.rend();
    for (; itr != itrEnd; ++itr)
    {
        // Check if the number has already been taken by the parent nodes.
        if (isValueInParents(node->parent, itr->second))
            continue;

        // the number has not been taken yet.  Let's take it.
        node->data->maxHeap = itr->second;
        break;
    }

#if SHRINK_DATA_OBJECT_SIZE
    // Shrink the object from PrioSearchData to Data.
    node->data.reset(new Data(*node->data.get()));
#endif

    if (node->isLeaf)
        return;

    if (!empty(node->left))
        walkNode(node->left);

    if (!empty(node->right))
        walkNode(node->right);
}

bool RangeTree::isValueInParents(const NodePtr& node, const NodePtr& heap) const
{
    NodePtr parent = node;
    while (!empty(parent))
    {
        if (empty(parent->data->maxHeap))
        {
            parent = parent->parent;
            continue;
        }
        if (parent->data->maxHeap.get() == heap.get())
            return true;
        parent = parent->parent;
    }
    return false;
}

RangeTree::RangeOverlap RangeTree::checkRangeOverlap(const NodePtr& node, IntX xlow, IntX xhigh) const
{
//  fprintf(stdout, "RangeTree::checkRangeOverlap: node range (%d - %d) - [%d - %d]\n",
//          node->data->xlow, node->data->xhigh, xlow, xhigh);

    if (empty(node))
    {
//      fprintf(stdout, "RangeTree::checkRangeOverlap: NONE (empty)\n");fflush(stdout);
        return RANGE_NONE;
    }

    if (node->data->xhigh < xlow || xhigh < node->data->xlow)
    {
//      fprintf(stdout, "RangeTree::checkRangeOverlap: NONE\n");fflush(stdout);
        return RANGE_NONE;
    }

    if (xlow <= node->data->xlow && node->data->xhigh <= xhigh)
    {
//      fprintf(stdout, "RangeTree::checkRangeOverlap: CONTAINED\n");fflush(stdout);
        //     xlow |--------| xhigh
        // node range |----|
        return RANGE_CONTAINED;
    }

    if (node->data->xlow <= xlow && xhigh <= node->data->xhigh)
    {
//      fprintf(stdout, "RangeTree::checkRangeOverlap: CONTAINED\n");fflush(stdout);
        //          xlow |---| xhigh
        // node range |--------|
        return RANGE_CONTAINING;
    }

    if (node->data->xlow <= xhigh)
    {
//      fprintf(stdout, "RangeTree::checkRangeOverlap: RIGHT\n");fflush(stdout);
        //     xlow |-----| xhigh
        // node range |--------|
        return RANGE_OVERLAP_RIGHT;
    }

    if (xlow <= node->data->xhigh)
    {
//      fprintf(stdout, "RangeTree::checkRangeOverlap: LEFT\n");fflush(stdout);
        //           xlow |-----| xhigh
        // node range |-----|
        return RANGE_OVERLAP_LEFT;
    }

//  fprintf(stdout, "RangeTree::checkRangeOverlap: NONE (last)\n");fflush(stdout);
    return RANGE_NONE;
}

RangeTree::NodePtr RangeTree::findCommonAncestor(const NodePtr& root, IntX xlow, IntX xhigh, IntY ylow)
{
    NodePtr node = root;
    if (empty(node))
        return NodePtr();

    if (checkRangeOverlap(node, xlow, xhigh) == RANGE_NONE)
        // The root range does not overlap the desired range at all.  Bail out.
        return NodePtr();

    // In case the desired range is not entirely contained within root's
    // x-range, adjust it to fit within root's range.
    if (xlow < node->data->xlow)
        xlow = node->data->xlow;

    if (node->data->xhigh < xhigh)
        xhigh = node->data->xhigh;

    while (true)
    {
        NodePtr heapNode = heapMax(node);
        if (isOutOfRangeY(heapNode, ylow))
            // y value is out of range.  Halt the search.
            return NodePtr();

        if (xlow <= heapNode->data->x && heapNode->data->x <= xhigh)
        {
            // a node found!
//          output(heapNode, "", "(common)");
            addHitNode(heapNode);
        }

        if (node->isLeaf)
            return NodePtr();

        // At this point, it is guaranteed that the node range contains the
        // desired range.  Check the range of the left and right nodes.  If
        // one range contains the desired range while the other one does not
        // overlap it, then proceed the descent.

        if (checkRangeOverlap(node->left, xlow, xhigh) == RANGE_NONE)
            node = node->right;
        else if (checkRangeOverlap(node->right, xlow, xhigh) == RANGE_NONE)
            node = node->left;
        else
            return node;
    }

    return NodePtr();
}

}
