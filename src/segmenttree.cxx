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

#include "segmenttree.hxx"
#include "global.hxx"

#include <list>
#include <string>
#include <functional>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <iterator>
#include <limits>
#include <hash_map>

using namespace std;
using ::boost::shared_ptr;
using ::__gnu_cxx::hash_map;

namespace mdds {

namespace {

struct segment_base
{
    segment_tree::value_type low;
    segment_tree::value_type high;

    segment_base() :
        low(numeric_limits<segment_tree::value_type>::min()),
        high(numeric_limits<segment_tree::value_type>::max())
    {
    }

    segment_base(segment_tree::value_type _low, segment_tree::value_type _high) :
        low(_low), high(_high)
    {
    }

    /** 
     * Check if this segment contains the other segment passed to this method.
     *
     * @param other the other segment
     */
    bool contains(const segment_base& other) const
    {
        return (low <= other.low) && (other.high <= high);
    }

    /** 
     * Check if this segment contains a given point.  Note that a segment is 
     * defined as low <= x < high, not as low <= x <= high. 
     *  
     * @param point point to be queried 
     */
    bool contains(segment_tree::value_type point) const
    {
        return (low <= point) && (point < high);
    }
};

struct data_segment : public segment_base
{
    const segment_tree::segment_data_type* data;

    explicit data_segment(segment_tree::value_type _low, segment_tree::value_type _high,
                         const segment_tree::segment_data_type* p) :
        segment_base(_low, _high), data(p) {}
};

}

class segment_tree_impl
{
public:
    struct segment_data_hash
    {
        size_t operator() (const segment_tree::segment_data_type* p) const
        {
            return p->hash();
        }
    };
    typedef hash_map<const segment_tree::segment_data_type*, size_t, segment_data_hash> data_index_map;

    /** 
     * data that each node stores, both leaf and non-leaf.
     */
    struct node_data
    {
        /** segment represented by this node */
        segment_base segment;
        /** 
         * list of segment IDs that can be used to retrieve full segment data 
         * from mSegmentList.
         */            
        list<size_t> segmentIds;

        node_data();
    };
    typedef shared_ptr<node_data> data_ptr;

    struct node;
    typedef ::boost::shared_ptr<node> node_ptr;

    struct node
    {
        node_ptr    parent; /// parent node
        node_ptr    left;   /// left child node or previous sibling if it's a leaf node.
        node_ptr    right;  /// right child node or next sibling if it's aleaf node.
        data_ptr    data;
        bool        isLeaf;

        node(bool _isLeaf);
    };

    segment_tree_impl();
    ~segment_tree_impl();

    void build();
    void insert_segment(segment_tree::value_type low, segment_tree::value_type high, 
                       const segment_tree::segment_data_type* data);
    void search(segment_tree::value_type point, list<const segment_tree::segment_data_type*>& segments);

    void insert_segment_into_tree(const data_segment& segment);

private:
    void build_tree();
    void build_tree_non_leaf(const list<node_ptr>& nodeList);
    node_ptr make_parent_node(const node_ptr& node1, const node_ptr& node2) const;

    void walk_tree_to_insert(const node_ptr& pNode, const data_segment& segment);

    void dump_tree_layer(const list<node_ptr>& nodeList, unsigned int level = 0) const;
    void dump_tree() const;

private:
    node_ptr mp_root_node;
    node_ptr mp_left_node;

    /** 
     * A list of stored segments used during initialization.  This member 
     * gets destroyed once the tree is built.
     */
    vector<data_segment>    m_segment_list;

    /** 
     * Map a segment data pointer to its corresponding segment identifier that
     * can be used to retrieve full segment data from mSegmentList. 
     */
    data_index_map            m_segment_data_map;
    bool m_is_tree_built;
};

// ----------------------------------------------------------------------------

segment_tree_impl::node_data::node_data()
{
}

// ----------------------------------------------------------------------------

segment_tree_impl::node::node(bool _isLeaf) :
    parent(static_cast<node*>(NULL)),
    left  (static_cast<node*>(NULL)), 
    right (static_cast<node*>(NULL)),
    data  (new node_data),
    isLeaf(_isLeaf)
{
}

// ----------------------------------------------------------------------------

segment_tree_impl::segment_tree_impl() :
    mp_root_node(static_cast<node*>(NULL)),
    mp_left_node(static_cast<node*>(NULL)),
    m_is_tree_built(false)
{
}

segment_tree_impl::~segment_tree_impl()
{
}

namespace {

struct ProcessEachSegment : public ::std::unary_function<data_segment, void>
{
    ProcessEachSegment(segment_tree_impl& rParent) :
        mrParent(rParent)
    {
    }

    void operator() (const data_segment& segment)
    {
        mrParent.insert_segment_into_tree(segment);
    }

private:
    segment_tree_impl& mrParent;
};

struct CollectEndPoints : public ::std::unary_function<data_segment, void>
{
    CollectEndPoints() :
        mpEndPoints(new vector<segment_tree::value_type>),
        mbDuplicateRemoved(false)
    {
    }

    CollectEndPoints(const CollectEndPoints& r) :
        mpEndPoints(r.mpEndPoints),
        mbDuplicateRemoved(r.mbDuplicateRemoved)
    {
    }

    void operator() (const data_segment& segment)
    {
        mpEndPoints->push_back(segment.low);
        mpEndPoints->push_back(segment.high);
    }

    const vector<segment_tree::value_type>* getEndPointList()
    {
        if (!mbDuplicateRemoved)
        {
            // Sort all end points and remove duplicates.
            sort(mpEndPoints->begin(), mpEndPoints->end());
            mpEndPoints->erase(
                unique(mpEndPoints->begin(), mpEndPoints->end()), mpEndPoints->end());
            mbDuplicateRemoved = true;
        }
        return mpEndPoints.get();
    }

private:
    shared_ptr< vector<segment_tree::value_type> > mpEndPoints;
    bool mbDuplicateRemoved;
};

}

void segment_tree_impl::build()
{
    // Go through all inserted segments, collect their end points, remove 
    // duplicates and sort them.

    CollectEndPoints func;
    func = for_each(m_segment_list.begin(), m_segment_list.end(), func);
    const vector<segment_tree::value_type>* pList = func.getEndPointList();

    if (!pList)
        return;

    copy(pList->begin(), pList->end(), ostream_iterator<segment_tree::value_type>(cout, " "));
    cout << endl;

    // Build the leaf nodes based on the unique end point values.
    cout << "Build the leaf node chain" << endl;
    node_ptr pPrevNode, pThisNode;
    for (vector<segment_tree::value_type>::const_iterator itr = pList->begin(), itrEnd = pList->end();
          itr != itrEnd; ++itr)
    {
        cout << "node value: " << *itr << endl;
        pThisNode.reset(new node(true));
        if (!pPrevNode)
            mp_left_node = pThisNode;
        pThisNode->left = pPrevNode;
        if (pPrevNode)
        {    
            pPrevNode->right = pThisNode;
            pPrevNode->data->segment.high = *itr;
        }

        // Assign data to this node.
        node_data& data = *pThisNode->data;
        data.segment.low = *itr;

        pPrevNode = pThisNode;
    }

#if 0
    // Make sure the leaf node chain is constructed correctly.
    cout << "Make sure the left node chain is constructed correctly." << endl;
    pThisNode = mp_left_node;
    while (pThisNode)
    {
        cout << "node value: " << pThisNode->data->segment.low << " - " << pThisNode->data->segment.high << endl;
        pThisNode = pThisNode->right;
    }
#endif

    // Build the binary tree upwards from the leaf nodes.
    build_tree();

    for_each(m_segment_list.begin(), m_segment_list.end(), ProcessEachSegment(*this));

    // Double-check to make sure the tree has been built correctly.
    dump_tree();

    m_is_tree_built = true;
}

void segment_tree_impl::insert_segment(segment_tree::value_type low, segment_tree::value_type high, 
                                    const segment_tree::segment_data_type* data)
{
    if (m_is_tree_built)
        throw TreeLockedException();

    m_segment_list.push_back(data_segment(low, high, data));
    m_segment_data_map.insert(
        data_index_map::value_type(data, m_segment_list.size()-1));
}

static void walkTreeToSearch(const segment_tree_impl::node_ptr& pNode, segment_tree::value_type point, list<size_t>& segments)
{
    if (!pNode)
        return;

    const segment_tree_impl::node_data& data = *pNode->data;
    if (data.segment.contains(point))
    {
        // This node's segment contains a given point.  Get all segment names 
        // that the node is associated with.
        copy(data.segmentIds.begin(), data.segmentIds.end(), 
             back_inserter(segments));
    }

    if (pNode->isLeaf)
        return;

    if (pNode->left && pNode->left->data->segment.contains(point))
        walkTreeToSearch(pNode->left, point, segments);
    else if (pNode->right && pNode->right->data->segment.contains(point))
        walkTreeToSearch(pNode->right, point, segments);
}

namespace {

class SegmentIdToPointer : unary_function<size_t, void>
{
public:
    SegmentIdToPointer(vector<data_segment>* _segmentList) :
        mpSegmentList(_segmentList), mpPtrList(new list<const segment_tree::segment_data_type*>) {}

    SegmentIdToPointer(const SegmentIdToPointer& r) :
        mpSegmentList(r.mpSegmentList), mpPtrList(r.mpPtrList) {}

    void operator() (size_t i)
    {
        mpPtrList->push_back(mpSegmentList->at(i).data);
    }

    void getPointerList(list<const segment_tree::segment_data_type*>& ptrList)
    {
        mpPtrList->swap(ptrList);
    }
private:
    const vector<data_segment>* mpSegmentList;
    shared_ptr<list<const segment_tree::segment_data_type*> > mpPtrList;
};

}
void segment_tree_impl::search(segment_tree::value_type point, list<const segment_tree::segment_data_type*>& segments)
{
    list<size_t> segmentIds;
    walkTreeToSearch(mp_root_node, point, segmentIds);
    list<const segment_tree::segment_data_type*> segmentData;
    SegmentIdToPointer func(&m_segment_list);
    func = for_each(segmentIds.begin(), segmentIds.end(), func);
    func.getPointerList(segments);
}

void segment_tree_impl::walk_tree_to_insert(const segment_tree_impl::node_ptr& pNode, const data_segment& segment)
{
    if (!pNode)
        return;

    segment_tree_impl::node_data& data = *pNode->data;

    // Check if the segment of the current node is totally contained by this
    // segment.  If yes, register this segment name with this node.
    if (segment.contains(data.segment))
    {
        data_index_map::const_iterator itr = m_segment_data_map.find(segment.data);
        if (itr != m_segment_data_map.end())
        {
            size_t segId = itr->second;
            data.segmentIds.push_back(segId);
        }
        return;
    }

    if (pNode->isLeaf)
        return;

    walk_tree_to_insert(pNode->left,  segment);
    walk_tree_to_insert(pNode->right, segment);
}

void segment_tree_impl::insert_segment_into_tree(const data_segment& segment)
{
    if (!mp_root_node)
        // Root node is empty.  Nothing to do.
        return;

    walk_tree_to_insert(mp_root_node, segment);
}

void segment_tree_impl::build_tree()
{
    if (!mp_left_node)
        // The left leaf node is empty.  Nothing to build.
        return;

    node_ptr node1, node2;
    node1 = mp_left_node;

    list<node_ptr> nodeList;
    while (true)
    {
        node2 = node1->right;
        node_ptr parentNode = make_parent_node(node1, node2);
        nodeList.push_back(parentNode);
        
        if (!node2 || !node2->right)
            // no more nodes.  Break out of the loop.
            break;

        node1 = node2->right;
    }

    build_tree_non_leaf(nodeList);
}

void segment_tree_impl::build_tree_non_leaf(const list<node_ptr>& nodeList)
{
    size_t nodeCount = nodeList.size();
    if (nodeCount == 1)
    {
        mp_root_node = nodeList.front();
        return;
    }
    else if (nodeCount == 0)
        return;

    list<node_ptr> newNodeList;
    vector<node_ptr> nodePair;
    nodePair.reserve(2);
    list<node_ptr>::const_iterator itr = nodeList.begin(), itrEnd = nodeList.end();
    for (; itr != itrEnd; ++itr)
    {
        nodePair.push_back(*itr);
        if (nodePair.size() == 2)
        {
            node_ptr parentNode = make_parent_node(nodePair[0], nodePair[1]);
            nodePair.clear();
            newNodeList.push_back(parentNode);
        }
    }

    if (!nodePair.empty())
    {
        node_ptr parentNode = make_parent_node(nodePair[0], node_ptr());
        nodePair.clear();
        newNodeList.push_back(parentNode);
    }

    // Move up one level, and do the same procedure until the root node is reached.
    build_tree_non_leaf(newNodeList);
}

segment_tree_impl::node_ptr segment_tree_impl::make_parent_node(const node_ptr& node1, const node_ptr& node2) const
{
    node_ptr parentNode(new node(false));
    node1->parent = parentNode;
    parentNode->left = node1;
    if (node2)
    {
        node2->parent = parentNode;
        parentNode->right = node2;
    }

    // Parent node should carry the range of all of its child nodes.
    node_data& data = *parentNode->data;
    if (node1)
        data.segment.low  = node1->data->segment.low;
    if (node2)
        data.segment.high = node2->data->segment.high;

    return parentNode;
}

void segment_tree_impl::dump_tree_layer(const list<segment_tree_impl::node_ptr>& nodeList, unsigned int level) const
{
    typedef segment_tree_impl::node_ptr NodePtr;
    if (nodeList.empty())
        return;

    bool isLeaf = nodeList.front()->isLeaf;
    cout << "level " << level << " (" << (isLeaf?"leaf":"non-leaf") << ")" << endl;

    list<NodePtr> newList;
    list<NodePtr>::const_iterator itr = nodeList.begin(), itrEnd = nodeList.end();
    for (; itr != itrEnd; ++itr)
    {
        const NodePtr& p = *itr;
        if (!p)
        {
            cout << "(x) ";
            continue;
        }

        if (!p->data)
            continue;

        // Print out the node data.
        const segment_tree_impl::node_data& data = *p->data;
        cout << "(" << data.segment.low << "-" << data.segment.high;
        if (!data.segmentIds.empty())
        {
            list<size_t>::const_iterator itr = data.segmentIds.begin(), itrEnd = data.segmentIds.end();
            for (; itr != itrEnd; ++itr)
                cout << " " << m_segment_list[*itr].data->what();
        }
        cout << ") ";

        if (p->isLeaf)
            continue;

        if (p->left)
        {
            newList.push_back(p->left);
            if (p->right)
                newList.push_back(p->right);
        }
    }
    cout << endl;

    if (!newList.empty())
        dump_tree_layer(newList, level+1);
}

void segment_tree_impl::dump_tree() const
{
    if (!mp_root_node)
    {
        cout << "root node is empty" << endl;
        return;
    }

    list<node_ptr> nodeList;
    nodeList.push_back(mp_root_node);
    dump_tree_layer(nodeList);
}

// ============================================================================

segment_tree::segment_data_type::~segment_data_type() {}

segment_tree::segment_tree() :
    mpImpl(new segment_tree_impl)
{
}

segment_tree::~segment_tree()
{
}

void segment_tree::build()
{
    mpImpl->build();
}

void segment_tree::insert_segment(value_type low, value_type high, const segment_data_type* data)
{
    mpImpl->insert_segment(low, high, data);
}

void segment_tree::search(value_type point, list<const segment_data_type*>& segments)
{
    mpImpl->search(point, segments);
}

}
