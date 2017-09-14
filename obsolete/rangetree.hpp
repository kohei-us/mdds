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

#ifndef RANGETREE_HXX
#define RANGETREE_HXX

#include "global.hxx"
#include "nodecontainer.hxx"
#include <string>
#include <list>
#include <map>
#include <memory>
#include <vector>

namespace mdds {

/** 
 * This class implements range tree and priority search tree.   This data 
 * structure and the search algorithm are modified from the following 
 * literature: 
 *  
 * Section 1.3 - Priority Search Trees (p.19) in Foundations of 
 * Multidimensional Metric Data Structures by Hanan Samet, 2006, Morgan 
 * Kaufmann Publishers. 
 */
class RangeTree
{
public:
    RangeTree();
    ~RangeTree();

    void insertPoint(const char* name, IntX x, IntY y);

    /** 
     * Build up a priority search tree from the current set of points.
     */
    void build();

    /** 
     * Dumps the current snapshot of the internal tree.
     */
    void dump() const;

    /** 
     * Find all points that fall between specified x value range, inclusive of 
     * the boundary values. 
     * 
     * @param xlow lower X boundary value
     * @param xhigh higher X boundary value
     */
    void search(IntX xlow, IntX xhigh) const;

    /** 
     * Find all points that fall between specified x value ranges, and 
     * satisfies specified lower bound on y values.
     * 
     * @param xlow
     * @param xhigh
     * @param ylow
     */
    void search(IntX xlow, IntX xhigh, IntY ylow);

    void printSearchResult() const;

private:
    struct Node;
    struct Data;
    struct PrioSearchData;

    typedef ::std::shared_ptr<Node> NodePtr;
    typedef ::std::shared_ptr<Data> DataPtr;
    typedef ::std::multimap<int, NodePtr>  HeapMap;

    struct Data
    {
        ::std::string name;
        IntX x;
        IntY y;
        NodePtr maxHeap;
        IntX xlow;
        IntX xhigh;
        
        Data(const char* _name, IntX _x, IntY _y, const NodePtr& _maxHeap);
        Data(const Data& data);

        PrioSearchData* getPrioSearchData();
    };

    /** 
     * Data container used when building a priority search tree.  It keeps a 
     * sorted list of y-values that are used to set an appropriate y-value for
     * the non-leaf nodes.
     */
    struct PrioSearchData : public Data
    {
        /** a sorted list of y values and their associated leaf nodes, the
         *  latter of which will be used as maximum heap nodes later. */
        HeapMap heaps;

        PrioSearchData(const char* _name, IntX _x, IntY _y, const NodePtr& _maxHeap);
    };

    /** 
     * A leaf node points to its adjacent nodes using 'left' and 'right' node
     * pointers, while a non-leaf node uses them to point to its left and 
     * right child nodes, respectively.  It's important to understand this 
     * distinction when tracing the code! 
     */
    struct Node
    {
        DataPtr data; 
        NodePtr parent;
        NodePtr left;   // left child node (non-leaf) or previous data node (leaf)
        NodePtr right;  // right child node (non-leaf) or next data node (leaf)
        NodePtr next;   // to chain together multiple nodes at the same node position.
        bool    isLeaf;

        Node(const char* _name, IntX _x, IntY _y, bool _isLeaf=true);
    };

    enum RangeOverlap
    {
        RANGE_CONTAINING,
        RANGE_CONTAINED,
        RANGE_OVERLAP_LEFT,
        RANGE_OVERLAP_RIGHT,
        RANGE_NONE
    };

private:
    void addHitNode(const NodePtr& node);

    void output(NodePtr node, const char* prefix = "", const char* suffix = "", bool printMaxHeap = false) const;

    /** 
     * Heap max is the node within the subtree of an arbitrary node that has 
     * y value greater than or equal to the y values of any other node in the
     * same subtree.
     * 
     * @param subRootNode
     * 
     * @return NodePtr
     */
    NodePtr heapMax(NodePtr node) const;

    void searchY(NodePtr node, IntY ylow);

    bool isOutOfRangeY(NodePtr node, IntY ylow) const;

    void dumpTree(const ::std::list<NodePtr>& nodeList, int level = 0) const;

    NodePtr makeParentNode(const NodePtr& node1, const NodePtr& node2) const;

    /** 
     * Sweep the leaf nodes and build the next level.
     */
    void buildTree();

    void combineMaps(HeapMap& rSet1, const HeapMap& rSet2) const;

    /** 
     * Sweep the passed node list and build the next level.  Only called from 
     * buildTree().  Don't call this from anywhere else. 
     * 
     * @param nodeList list of nodes to sweep
     */
    void buildTreeNonLeaf(const ::std::list<NodePtr>& nodeList);

    void buildPrioSearchTree();

    void walkNode(NodePtr node);

    bool isValueInParents(const NodePtr& node, const NodePtr& heap) const;

    RangeOverlap checkRangeOverlap(const NodePtr& node, IntX xlow, IntX xhigh) const;

    NodePtr findCommonAncestor(const NodePtr& root, IntX xlow, IntX xhigh, IntY ylow);

private:
    NodePtr mpLeftLeafNode;
    NodePtr mpRootNode;

    /** a list of nodes founds during search. */
    NodeContainer2D mHitNodes;
//  ::std::vector<NodePtr> mHitNodes;
};

}

#endif
