/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2018 Kohei Yoshida
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

#include "mdds/global.hpp"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <memory>
#include <cassert>
#include <algorithm>
#include <functional>

namespace mdds {

namespace detail { namespace rtree {

template<typename _Key, typename _BBox>
_Key calc_linear_intersection(size_t dim, const _BBox& bb1, const _BBox& bb2)
{
    using key_type = _Key;

    key_type start1 = bb1.start.d[dim], end1 = bb1.end.d[dim];
    key_type start2 = bb2.start.d[dim], end2 = bb2.end.d[dim];

    // Ensure that start1 < start2.

    if (start1 > start2)
    {
        std::swap(start1, start2);
        std::swap(end1, end2);
    }

    assert(start1 < start2);

    if (end1 < start2)
    {
        // 1 : |------|
        // 2 :           |-------|

        // These two are not intersected at all. Bail out.
        return key_type();
    }

    if (end1 < end2)
    {
        // 1 : |---------|
        // 2 :      |----------|

        return end1 - start2;
    }

    // 1 : |--------------|
    // 2 :      |-----|

    return end2 - start2;
}

template<typename _Key, typename _BBox, size_t _Dim>
_Key calc_intersection(const _BBox& bb1, const _BBox& bb2)
{
    static_assert(_Dim > 0, "Dimension cannot be zero.");

    using key_type = _Key;

    key_type total_volume = calc_linear_intersection<_Key,_BBox>(0, bb1, bb2);
    if (!total_volume)
        return key_type();

    for (size_t dim = 1; dim < _Dim; ++dim)
    {
        key_type segment_len = calc_linear_intersection<_Key,_BBox>(dim, bb1, bb2);
        if (!segment_len)
            return key_type();

        total_volume *= segment_len;
    }

    return total_volume;
}

template<typename _Key, typename _BBox, size_t _Dim>
bool enlarge_box_to_fit(_BBox& parent, const _BBox& child)
{
    bool enlarged = false;

    for (size_t dim = 0; dim < _Dim; ++dim)
    {
        if (child.start.d[dim] < parent.start.d[dim])
        {
            parent.start.d[dim] = child.start.d[dim];
            enlarged = true;
        }

        if (parent.end.d[dim] < child.end.d[dim])
        {
            parent.end.d[dim] = child.end.d[dim];
            enlarged = true;
        }
    }

    return enlarged;
}

template<typename _Key, typename _BBox, size_t _Dim>
_Key calc_area(const _BBox& bb)
{
    static_assert(_Dim > 0, "Dimension cannot be zero.");
    using key_type = _Key;

    key_type area = bb.end.d[0] - bb.start.d[0];
    for (size_t dim = 1; dim < _Dim; ++dim)
        area *= bb.end.d[dim] - bb.start.d[dim];

    return area;
}

/**
 * The margin here is defined as the sum of the lengths of the edges of a
 * bounding box, per the original paper on R*-tree.  It's half-margin
 * because it only adds one of the two edges in each dimension.
 */
template<typename _Key, typename _BBox, size_t _Dim>
_Key calc_half_margin(const _BBox& bb)
{
    static_assert(_Dim > 0, "Dimension cannot be zero.");
    using key_type = _Key;

    key_type margin = bb.end.d[0] - bb.start.d[0];
    for (size_t dim = 1; dim < _Dim; ++dim)
        margin += bb.end.d[dim] - bb.start.d[dim];

    return margin;
}

/**
 * Area enlargement is calculated by calculating the area of the enlarged
 * box subtracted by the area of the original box prior to the enlargement.
 *
 * @param bb_host bounding box of the area receiving the new object.
 * @param bb_guest bounding of the new object being inserted.
 *
 * @return quantity of the area enlargement.
 */
template<typename _Key, typename _BBox, size_t _Dim>
_Key calc_area_enlargement(const _BBox& bb_host, const _BBox& bb_guest)
{
    static_assert(_Dim > 0, "Dimension cannot be zero.");
    using key_type = _Key;
    using bounding_box = _BBox;

    // Calculate the original area.
    key_type original_area = calc_area<_Key,_BBox,_Dim>(bb_host);

    // Enlarge the box for the new object if needed.
    bounding_box bb_host_enlarged = bb_host; // make a copy.
    bool enlarged = enlarge_box_to_fit<_Key,_BBox,_Dim>(bb_host_enlarged, bb_guest);
    if (!enlarged)
        // Area enlargement did not take place.
        return key_type();

    key_type enlarged_area = calc_area<_Key,_BBox,_Dim>(bb_host_enlarged);

    return enlarged_area - original_area;
}

template<typename _Key, typename _BBox, typename _Iter, size_t _Dim>
_BBox calc_bounding_box(_Iter it, _Iter it_end)
{
    _BBox bb = it->box;
    for (++it; it != it_end; ++it)
        detail::rtree::enlarge_box_to_fit<_Key,_BBox,_Dim>(bb, it->box);

    return bb;
}

template<typename _Key>
struct min_value_pos
{
    _Key value;
    size_t pos;

    void assign(_Key new_value, size_t new_pos)
    {
        value = new_value;
        pos = new_pos;
    }

    void assign_if_less(_Key new_value, size_t new_pos)
    {
        if (new_value < value)
            assign(new_value, new_pos);
    }
};

}}

template<typename _Key, typename _Value, typename _Trait>
rtree<_Key,_Value,_Trait>::point::point()
{
    // Initialize the point values with the key type's default value.
    key_type* p = d;
    key_type* p_end = p + trait_type::dimensions;

    for (; p != p_end; ++p)
        *p = key_type{};
}

template<typename _Key, typename _Value, typename _Trait>
rtree<_Key,_Value,_Trait>::point::point(std::initializer_list<key_type> vs)
{
    // Initialize the point values with the key type's default value.
    key_type* dst = d;
    key_type* dst_end = dst + trait_type::dimensions;

    for (const key_type& v : vs)
    {
        if (dst == dst_end)
            throw std::range_error("number of elements exceeds the dimension size.");

        *dst = v;
        ++dst;
    }
}

template<typename _Key, typename _Value, typename _Trait>
std::string
rtree<_Key,_Value,_Trait>::point::to_string() const
{
    std::ostringstream os;

    os << "(";
    for (size_t i = 0; i < trait_type::dimensions; ++i)
    {
        if (i > 0)
            os << ", ";
        os << d[i];
    }
    os << ")";

    return os.str();
}

template<typename _Key, typename _Value, typename _Trait>
bool rtree<_Key,_Value,_Trait>::point::operator== (const point& other) const
{
    const key_type* left = d;
    const key_type* right = other.d;
    const key_type* left_end = left + trait_type::dimensions;

    for (; left != left_end; ++left, ++right)
    {
        if (*left != *right)
            return false;
    }

    return true;
}

template<typename _Key, typename _Value, typename _Trait>
bool rtree<_Key,_Value,_Trait>::point::operator!= (const point& other) const
{
    return !operator== (other);
}

template<typename _Key, typename _Value, typename _Trait>
rtree<_Key,_Value,_Trait>::bounding_box::bounding_box() {}

template<typename _Key, typename _Value, typename _Trait>
rtree<_Key,_Value,_Trait>::bounding_box::bounding_box(const point& start, const point& end) :
    start(start), end(end) {}

template<typename _Key, typename _Value, typename _Trait>
std::string
rtree<_Key,_Value,_Trait>::bounding_box::to_string() const
{
    std::ostringstream os;
    os << start.to_string() << " - " << end.to_string();
    return os.str();
}

template<typename _Key, typename _Value, typename _Trait>
bool rtree<_Key,_Value,_Trait>::bounding_box::operator== (const bounding_box& other) const
{
    return start == other.start && end == other.end;
}

template<typename _Key, typename _Value, typename _Trait>
bool rtree<_Key,_Value,_Trait>::bounding_box::operator!= (const bounding_box& other) const
{
    return !operator== (other);
}

template<typename _Key, typename _Value, typename _Trait>
bool rtree<_Key,_Value,_Trait>::bounding_box::contains(const point& pt) const
{
    for (size_t dim = 0; dim < trait_type::dimensions; ++dim)
    {
        if (pt.d[dim] < start.d[dim] || end.d[dim] < pt.d[dim])
            return false;
    }

    return true;
}

template<typename _Key, typename _Value, typename _Trait>
bool rtree<_Key,_Value,_Trait>::bounding_box::contains(const bounding_box& bb) const
{
    for (size_t dim = 0; dim < trait_type::dimensions; ++dim)
    {
        if (bb.start.d[dim] < start.d[dim] || end.d[dim] < bb.end.d[dim])
            return false;
    }

    return true;
}

template<typename _Key, typename _Value, typename _Trait>
bool rtree<_Key,_Value,_Trait>::bounding_box::contains_at_boundary(const bounding_box& bb) const
{
    for (size_t dim = 0; dim < trait_type::dimensions; ++dim)
    {
        if (start.d[dim] == bb.start.d[dim] || bb.end.d[dim] == end.d[dim])
            return true;
    }

    return false;
}

template<typename _Key, typename _Value, typename _Trait>
rtree<_Key,_Value,_Trait>::node_store::node_store() :
    type(node_type::unspecified), parent(nullptr), node_ptr(nullptr), count(0) {}

template<typename _Key, typename _Value, typename _Trait>
rtree<_Key,_Value,_Trait>::node_store::node_store(node_store&& r) :
    type(r.type),
    box(r.box),
    parent(r.parent),
    node_ptr(r.node_ptr),
    count(r.count)
{
    r.type = node_type::unspecified;
    r.parent = nullptr;
    r.node_ptr = nullptr;
    r.count = 0;
}

template<typename _Key, typename _Value, typename _Trait>
rtree<_Key,_Value,_Trait>::node_store::node_store(node_type type, const bounding_box& box, node* node_ptr) :
    type(type), box(box), parent(nullptr), node_ptr(node_ptr), count(0) {}

template<typename _Key, typename _Value, typename _Trait>
rtree<_Key,_Value,_Trait>::node_store::~node_store()
{
    if (node_ptr)
    {
        switch (type)
        {
            case node_type::directory_leaf:
            case node_type::directory_nonleaf:
                delete static_cast<directory_node*>(node_ptr);
                break;
            case node_type::value:
                delete static_cast<value_node*>(node_ptr);
                break;
            case node_type::unspecified:
            default:
                assert(!"node::~node: unknown node type!");
        }
    }
}

template<typename _Key, typename _Value, typename _Trait>
typename rtree<_Key,_Value,_Trait>::node_store
rtree<_Key,_Value,_Trait>::node_store::create_leaf_directory_node()
{
    node_store ret(node_type::directory_leaf, bounding_box(), new directory_node);
    return ret;
}

template<typename _Key, typename _Value, typename _Trait>
typename rtree<_Key,_Value,_Trait>::node_store
rtree<_Key,_Value,_Trait>::node_store::create_nonleaf_directory_node()
{
    node_store ret(node_type::directory_nonleaf, bounding_box(), new directory_node);
    return ret;
}

template<typename _Key, typename _Value, typename _Trait>
typename rtree<_Key,_Value,_Trait>::node_store
rtree<_Key,_Value,_Trait>::node_store::create_value_node(const bounding_box& box, value_type v)
{
    node_store ret(node_type::value, box, new value_node(std::move(v)));
    return ret;
}

template<typename _Key, typename _Value, typename _Trait>
typename rtree<_Key,_Value,_Trait>::node_store&
rtree<_Key,_Value,_Trait>::node_store::operator= (node_store&& other)
{
    node_store tmp(std::move(other));
    swap(tmp);
    return *this;
}

template<typename _Key, typename _Value, typename _Trait>
bool rtree<_Key,_Value,_Trait>::node_store::pack()
{
    if (!is_directory())
        return false;

    const directory_node* dir = static_cast<const directory_node*>(node_ptr);
    const dir_store_type& children = dir->children;
    if (children.empty())
    {
        // This node has no children.  Reset the bounding box to empty.
        bounding_box new_box;
        bool changed = new_box != box;
        box = new_box;
        return changed;
    }

    bounding_box new_box = dir->calc_extent();
    bool changed = new_box != box;
    box = new_box; // update the bounding box.
    return changed;
}

template<typename _Key, typename _Value, typename _Trait>
bool rtree<_Key,_Value,_Trait>::node_store::is_directory() const
{
    switch (type)
    {
        case node_type::directory_leaf:
        case node_type::directory_nonleaf:
            return true;
        default:
            ;
    }

    return false;
}

template<typename _Key, typename _Value, typename _Trait>
bool rtree<_Key,_Value,_Trait>::node_store::is_root() const
{
    return parent == nullptr;
}

template<typename _Key, typename _Value, typename _Trait>
bool rtree<_Key,_Value,_Trait>::node_store::exceeds_capacity() const
{
    if (type != node_type::directory_leaf)
        return false;

    return count > trait_type::max_node_size;
}

template<typename _Key, typename _Value, typename _Trait>
void rtree<_Key,_Value,_Trait>::node_store::swap(node_store& other)
{
    std::swap(type, other.type);
    std::swap(box, other.box);
    std::swap(parent, other.parent);
    std::swap(node_ptr, other.node_ptr);
    std::swap(count, other.count);
}

template<typename _Key, typename _Value, typename _Trait>
void rtree<_Key,_Value,_Trait>::node_store::reset_parent_of_children()
{
    if (!is_directory())
        return;

    directory_node* dir = static_cast<directory_node*>(node_ptr);
    for (node_store& ns : dir->children)
        ns.parent = this;
}

template<typename _Key, typename _Value, typename _Trait>
rtree<_Key,_Value,_Trait>::node::node() {}

template<typename _Key, typename _Value, typename _Trait>
rtree<_Key,_Value,_Trait>::node::~node() {}

template<typename _Key, typename _Value, typename _Trait>
rtree<_Key,_Value,_Trait>::value_node::value_node(value_type value) :
    value(std::move(value)) {}

template<typename _Key, typename _Value, typename _Trait>
rtree<_Key,_Value,_Trait>::value_node::~value_node() {}

template<typename _Key, typename _Value, typename _Trait>
rtree<_Key,_Value,_Trait>::directory_node::directory_node() {}

template<typename _Key, typename _Value, typename _Trait>
rtree<_Key,_Value,_Trait>::directory_node::~directory_node() {}

template<typename _Key, typename _Value, typename _Trait>
void rtree<_Key,_Value,_Trait>::directory_node::insert(node_store&& ns)
{
    children.push_back(std::move(ns));
}

template<typename _Key, typename _Value, typename _Trait>
typename rtree<_Key,_Value,_Trait>::bounding_box
rtree<_Key,_Value,_Trait>::directory_node::calc_extent() const
{
    auto it = children.cbegin(), ite = children.cend();

    bounding_box box =
        detail::rtree::calc_bounding_box<_Key,bounding_box,decltype(it),trait_type::dimensions>(it, ite);

    return box;
}

template<typename _Key, typename _Value, typename _Trait>
void rtree<_Key,_Value,_Trait>::const_search_results::add_node_store(const node_store* ns)
{
    m_store.push_back(ns);
}

template<typename _Key, typename _Value, typename _Trait>
typename rtree<_Key,_Value,_Trait>::const_iterator
rtree<_Key,_Value,_Trait>::const_search_results::cbegin() const
{
    return const_iterator(m_store.cbegin());
}

template<typename _Key, typename _Value, typename _Trait>
typename rtree<_Key,_Value,_Trait>::const_iterator
rtree<_Key,_Value,_Trait>::const_search_results::cend() const
{
    return const_iterator(m_store.cend());
}

template<typename _Key, typename _Value, typename _Trait>
typename rtree<_Key,_Value,_Trait>::const_iterator
rtree<_Key,_Value,_Trait>::const_search_results::begin() const
{
    return const_iterator(m_store.cbegin());
}

template<typename _Key, typename _Value, typename _Trait>
typename rtree<_Key,_Value,_Trait>::const_iterator
rtree<_Key,_Value,_Trait>::const_search_results::end() const
{
    return const_iterator(m_store.cend());
}

template<typename _Key, typename _Value, typename _Trait>
rtree<_Key,_Value,_Trait>::const_iterator::const_iterator(
    typename store_type::const_iterator pos) : m_pos(pos) {}

template<typename _Key, typename _Value, typename _Trait>
void rtree<_Key,_Value,_Trait>::const_iterator::update_current_node()
{
    const node_store* p = *m_pos;
    assert(p->type == node_type::value);
    m_cur_node.box = p->box;
    m_cur_node.value = static_cast<const value_node*>(p->node_ptr)->value;
}

template<typename _Key, typename _Value, typename _Trait>
bool rtree<_Key,_Value,_Trait>::const_iterator::operator== (const const_iterator& other) const
{
    return m_pos == other.m_pos;
}

template<typename _Key, typename _Value, typename _Trait>
bool rtree<_Key,_Value,_Trait>::const_iterator::operator!= (const const_iterator& other) const
{
    return !operator== (other);
}

template<typename _Key, typename _Value, typename _Trait>
typename rtree<_Key,_Value,_Trait>::const_iterator&
rtree<_Key,_Value,_Trait>::const_iterator::operator++ ()
{
    ++m_pos;
    return *this;
}

template<typename _Key, typename _Value, typename _Trait>
typename rtree<_Key,_Value,_Trait>::const_iterator
rtree<_Key,_Value,_Trait>::const_iterator::operator++ (int)
{
    const_iterator ret(m_pos);
    ++m_pos;
    return ret;
}

template<typename _Key, typename _Value, typename _Trait>
typename rtree<_Key,_Value,_Trait>::const_iterator&
rtree<_Key,_Value,_Trait>::const_iterator::operator-- ()
{
    --m_pos;
    return *this;
}

template<typename _Key, typename _Value, typename _Trait>
typename rtree<_Key,_Value,_Trait>::const_iterator
rtree<_Key,_Value,_Trait>::const_iterator::operator-- (int)
{
    const_iterator ret(m_pos);
    --m_pos;
    return ret;
}

template<typename _Key, typename _Value, typename _Trait>
const typename rtree<_Key,_Value,_Trait>::const_iterator::value_type&
rtree<_Key,_Value,_Trait>::const_iterator::operator*()
{
    update_current_node();
    return m_cur_node;
}

template<typename _Key, typename _Value, typename _Trait>
const typename rtree<_Key,_Value,_Trait>::const_iterator::value_type*
rtree<_Key,_Value,_Trait>::const_iterator::operator->()
{
    update_current_node();
    return &m_cur_node;
}

template<typename _Key, typename _Value, typename _Trait>
rtree<_Key,_Value,_Trait>::rtree() : m_root(node_store::create_leaf_directory_node())
{
    static_assert(trait_type::min_node_size <= trait_type::max_node_size / 2,
        "Minimum node size must be less than half of the maximum node size.");
}

template<typename _Key, typename _Value, typename _Trait>
rtree<_Key,_Value,_Trait>::~rtree()
{
}

template<typename _Key, typename _Value, typename _Trait>
void rtree<_Key,_Value,_Trait>::insert(const point& start, const point& end, value_type value)
{
    std::cout << __FILE__ << "#" << __LINE__ << " (rtree:insert): start=" << start.to_string() << "; end=" << end.to_string() << std::endl;
    bounding_box bb(start, end);
    node_store* ns = find_node_for_insertion(bb);
    assert(ns);
    assert(ns->type == node_type::directory_leaf);
    directory_node* dir = static_cast<directory_node*>(ns->node_ptr);

    // Insert the new value to this node.
    node_store new_ns = node_store::create_value_node(bb, std::move(value));
    new_ns.parent = ns;
    dir->insert(std::move(new_ns));
    ++ns->count;

    if (ns->exceeds_capacity())
    {
        split_node(ns);
        return;
    }

    if (ns->count == 1)
        ns->box = bb;
    else
        detail::rtree::enlarge_box_to_fit<key_type,bounding_box,trait_type::dimensions>(ns->box, bb);

    std::cout << __FILE__ << "#" << __LINE__ << " (rtree:insert): ns count = " << ns->count << std::endl;
    std::cout << __FILE__ << "#" << __LINE__ << " (rtree:insert): ns box = " << ns->box.to_string() << std::endl;

    bb = ns->box; // grab the parent bounding box.

    // Propagate the bounding box update up the tree all the way to the root.
    for (ns = ns->parent; ns; ns = ns->parent)
    {
        std::cout << __FILE__ << "#" << __LINE__ << " (rtree:insert): ns count = " << ns->count << std::endl;
        std::cout << __FILE__ << "#" << __LINE__ << " (rtree:insert): ns box = " << ns->box.to_string() << std::endl;

        assert(ns->count > 0);
        detail::rtree::enlarge_box_to_fit<key_type,bounding_box,trait_type::dimensions>(ns->box, bb);
    }
}

template<typename _Key, typename _Value, typename _Trait>
typename rtree<_Key,_Value,_Trait>::const_search_results
rtree<_Key,_Value,_Trait>::search(const point& pt) const
{
    const_search_results ret;
    search_descend(pt, m_root, ret);
    return ret;
}

template<typename _Key, typename _Value, typename _Trait>
void rtree<_Key,_Value,_Trait>::erase(const_iterator pos)
{
    const node_store* ns = *pos.m_pos;
    assert(ns->type == node_type::value);
    assert(ns->parent);

    bounding_box bb_erased = ns->box;

    // Move up to the parent and find its stored location.
    node_store* parent = ns->parent;
    assert(parent->type == node_type::directory_leaf);
    directory_node* dir = static_cast<directory_node*>(parent->node_ptr);

    dir_store_type& children = dir->children;

    auto it = std::find_if(children.begin(), children.end(),
        [ns](const node_store& this_ns) -> bool
        {
            return &this_ns == ns;
        }
    );

    // Remove its entry from the parent node.
    assert(it != children.end());
    children.erase(it);
    --parent->count;
    assert(parent->count == children.size());

    if (!parent->is_root() && children.size() < trait_type::min_node_size)
        throw std::runtime_error("TODO: reduce tree and perform re-insertion.");

    shrink_tree_upward(parent, bb_erased);
}

template<typename _Key, typename _Value, typename _Trait>
const typename rtree<_Key,_Value,_Trait>::bounding_box&
rtree<_Key,_Value,_Trait>::get_root_extent() const
{
    return m_root.box;
}

template<typename _Key, typename _Value, typename _Trait>
bool rtree<_Key,_Value,_Trait>::empty() const
{
    return !m_root.count;
}

template<typename _Key, typename _Value, typename _Trait>
template<typename _Func>
void rtree<_Key,_Value,_Trait>::walk(_Func func) const
{
    std::function<void(const node_store*)> func_descend = [&](const node_store* ns)
    {
        node_properties np;
        np.type = ns->type;
        np.box = ns->box;
        func(np);

        switch (ns->type)
        {
            case node_type::directory_leaf:
            case node_type::directory_nonleaf:
            {
                const directory_node* dir =
                    static_cast<const directory_node*>(ns->node_ptr);

                for (const node_store& ns_child : dir->children)
                    func_descend(&ns_child);

                break;
            }
            case node_type::value:
                // Do nothing.
                break;
            default:
                assert(!"The tree should not contain node of this type!");
        }
    };

    func_descend(&m_root);
}

template<typename _Key, typename _Value, typename _Trait>
void rtree<_Key,_Value,_Trait>::check_integrity() const
{
    switch (m_root.type)
    {
        case node_type::directory_leaf:
        case node_type::directory_nonleaf:
            // Good.
            break;
        default:
            throw integrity_error("The root node must be a directory node.");
    }

    if (m_root.parent)
        throw integrity_error("The root node should not have a non-null parent.");

    std::vector<const node_store*> ns_stack;

    auto to_string = [](node_type nt) -> const char*
    {
        switch (nt)
        {
            case node_type::unspecified:
                return "unspecified";
            case node_type::directory_leaf:
                return "directory-leaf";
            case node_type::directory_nonleaf:
                return "directory-nonleaf";
            case node_type::value:
                return "value";
        }

        return "???";
    };

    std::function<void(const node_store*, int)> func_descend = [&ns_stack,&func_descend,to_string](const node_store* ns, int level)
    {
        std::string indent;
        for (int i = 0; i < level; ++i)
            indent += "  ";

        std::cout << indent << "node: " << ns << "; type: " << to_string(ns->type) << "; extent: " << ns->box.to_string() << std::endl;

        const node_store* parent = nullptr;
        bounding_box parent_bb;
        if (!ns_stack.empty())
        {
            parent = ns_stack.back();
            parent_bb = parent->box;
        }

        if (parent)
        {
            if (ns->parent != parent)
            {
                std::ostringstream os;
                os << "The parent node pointer does not point to the real parent. (expected: " << parent << "; actual: " << ns->parent << ")";
                throw integrity_error(os.str());
            }

            if (!parent_bb.contains(ns->box))
            {
                std::ostringstream os;
                os << "The extent of the child " << ns->box.to_string() << " is not within the extent of the parent " << parent_bb.to_string() << ".";
                throw integrity_error(os.str());
            }
        }

        ns_stack.push_back(ns);

        switch (ns->type)
        {
            case node_type::directory_leaf:
            case node_type::directory_nonleaf:
            {
                const directory_node* dir =
                    static_cast<const directory_node*>(ns->node_ptr);

                if (ns->count != dir->children.size())
                {
                    std::ostringstream os;
                    os << "Incorrect count of child nodes detected. (expected: " << dir->children.size() << "; actual: " << ns->count << ")";
                    throw integrity_error(os.str());
                }

                // Check to make sure the bounding box of the current node is
                // tightly packed.
                bounding_box bb_expected = dir->calc_extent();

                if (bb_expected != ns->box)
                {
                    std::ostringstream os;
                    os << "The extent of the node " << ns->box.to_string() << " does not equal truly tight extent " << bb_expected.to_string();
                    throw integrity_error(os.str());
                }

                for (const node_store& ns_child : dir->children)
                    func_descend(&ns_child, level+1);

                break;
            }
            case node_type::value:
                // Do nothing.
                break;
            default:
                throw integrity_error("Unexpected node type!");
        }

        ns_stack.pop_back();
    };

    func_descend(&m_root, 0);
}

template<typename _Key, typename _Value, typename _Trait>
void rtree<_Key,_Value,_Trait>::split_node(node_store* ns)
{
    assert(ns->type == node_type::directory_leaf);
    assert(ns->count == trait_type::max_node_size+1);

    directory_node* dir = static_cast<directory_node*>(ns->node_ptr);
    dir_store_type& children = dir->children;

    constexpr size_t dist_max = trait_type::max_node_size - trait_type::min_node_size * 2 + 2;

    // Store the sum of margins for each dimension axis.
    detail::rtree::min_value_pos<key_type> min_margin_dim;

    for (size_t dim = 0; dim < trait_type::dimensions; ++dim)
    {
        // Sort the entries by the lower then by the upper value of their bounding boxes.

        std::sort(children.begin(), children.end(),
            [dim](const node_store& a, const node_store& b) -> bool
            {
                if (a.box.start.d[dim] != b.box.start.d[dim])
                    return a.box.start.d[dim] < b.box.start.d[dim];

                return a.box.end.d[dim] < b.box.end.d[dim];
            }
        );

        key_type sum_of_margins = key_type(); // it's actually the sum of half margins.

        for (size_t dist = 1; dist <= dist_max; ++dist)
        {
            // The first group contains m-1+dist entries, while the second
            // group contains the rest.

            auto it = children.begin();
            auto it_end = it;
            std::advance(it_end, trait_type::min_node_size - 1 + dist);

            bounding_box bb1 = detail::rtree::calc_bounding_box<_Key,bounding_box,decltype(it),trait_type::dimensions>(it, it_end);
            it = it_end;
            it_end = children.end();
            assert(it != it_end);
            bounding_box bb2 = detail::rtree::calc_bounding_box<_Key,bounding_box,decltype(it),trait_type::dimensions>(it, it_end);

            // Compute the half margins of the first and second groups.
            key_type margin1 = detail::rtree::calc_half_margin<_Key,bounding_box,trait_type::dimensions>(bb1);
            key_type margin2 = detail::rtree::calc_half_margin<_Key,bounding_box,trait_type::dimensions>(bb2);
            key_type margins = margin1 + margin2;

            std::cout << __FILE__ << "#" << __LINE__ << " (rtree:split_node): dist = " << dist << "; margin1 = " << margin1 << "; margin2 = " << margin2 << std::endl;

            sum_of_margins += margins;
        }

        std::cout << __FILE__ << "#" << __LINE__ << " (rtree:split_node): dim = " << dim << "; sum margins = " << sum_of_margins << std::endl;

        if (dim > 0)
            min_margin_dim.assign_if_less(sum_of_margins, dim);
        else
            min_margin_dim.assign(sum_of_margins, dim);
    }

    // Pick the dimension axis with the lowest sum of margins.
    size_t min_dim = min_margin_dim.pos;
    std::cout << __FILE__ << "#" << __LINE__ << " (rtree:split_node): dim picked = " << min_dim << std::endl;

    // Along the chosen dimension axis, pick the distribution with the minimum
    // overlap value.
    detail::rtree::min_value_pos<key_type> min_overlap_dist;

    for (size_t dist = 1; dist <= dist_max; ++dist)
    {
        // The first group contains m-1+dist entries, while the second
        // group contains the rest.
        distribution dist_data(dist, children);
        bounding_box bb1 = detail::rtree::calc_bounding_box<_Key,bounding_box,decltype(dist_data.g1.begin),trait_type::dimensions>(dist_data.g1.begin, dist_data.g1.end);
        bounding_box bb2 = detail::rtree::calc_bounding_box<_Key,bounding_box,decltype(dist_data.g2.begin),trait_type::dimensions>(dist_data.g2.begin, dist_data.g2.end);

        key_type overlap = detail::rtree::calc_intersection<_Key,bounding_box,trait_type::dimensions>(bb1, bb2);
        std::cout << __FILE__ << "#" << __LINE__ << " (rtree:split_node): dist = " << dist << "; overlap = " << overlap << std::endl;
        if (dist == 1)
            min_overlap_dist.assign(overlap, dist);
        else
            min_overlap_dist.assign_if_less(overlap, dist);
    }

    std::cout << __FILE__ << "#" << __LINE__ << " (rtree:split_node): dist picked = " << min_overlap_dist.pos << std::endl;
    distribution dist_picked(min_overlap_dist.pos, children);

    // Move the child nodes in group 2 into a brand-new sibling node.
    node_store node_g2 = node_store::create_leaf_directory_node();
    directory_node* dir_sibling = static_cast<directory_node*>(node_g2.node_ptr);

    for (auto it = dist_picked.g2.begin; it != dist_picked.g2.end; ++it)
        dir_sibling->children.push_back(std::move(*it));
    node_g2.count = dir_sibling->children.size();
    node_g2.pack();

    // Remove the nodes in group 2 from the original node by shrinking the node store.
    ns->count = dist_picked.g1.size;
    dir->children.resize(dist_picked.g1.size);
    ns->pack(); // Re-calculate the bounding box.

    if (ns->is_root())
    {
        // Create a new root node and make it the parent of the original root
        // and the new sibling nodes.
        assert(ns == &m_root);
        node_store node_g1 = node_store::create_nonleaf_directory_node();
        m_root.swap(node_g1);
        node_g1.parent = &m_root;
        node_g2.parent = &m_root;
        directory_node* dir_root = static_cast<directory_node*>(m_root.node_ptr);
        dir_root->children.emplace_back(std::move(node_g1));
        dir_root->children.emplace_back(std::move(node_g2));
        m_root.count = 2;
        m_root.pack();

        for (node_store& ns_child : dir_root->children)
            ns_child.reset_parent_of_children();
    }
    else
    {
        // Place the new siblig under the same parent.
        assert(ns->parent);
        node_g2.parent = ns->parent;
        node_store& ns_parent = *ns->parent;
        assert(ns_parent.type == node_type::directory_nonleaf);
        directory_node* dir_parent = static_cast<directory_node*>(ns_parent.node_ptr);
        dir_parent->children.emplace_back(std::move(node_g2));
        ++ns_parent.count;
        ns_parent.pack();

        // Update the parent pointer of the children _after_ the group 2 node
        // has been inserted into the buffer, as the pointer value of the node
        // changes after the insertion.
        dir_parent->children.back().reset_parent_of_children();
    }
}

template<typename _Key, typename _Value, typename _Trait>
typename rtree<_Key,_Value,_Trait>::node_store*
rtree<_Key,_Value,_Trait>::find_node_for_insertion(const bounding_box& bb)
{
    node_store* dst = &m_root;

    for (size_t i = 0; i < trait_type::max_tree_depth; ++i)
    {
        if (dst->type == node_type::directory_leaf)
            return dst;

        assert(dst->type == node_type::directory_nonleaf);

        directory_node* dir = static_cast<directory_node*>(dst->node_ptr);
        dir_store_type& children = dir->children;

        // If this non-leaf directory contains at least one leaf directory,
        // pick the entry with minimum overlap increase.  If all of its child
        // nodes are non-leaf directories, then pick the entry with minimum
        // area enlargement.

        auto it = std::find_if(children.cbegin(), children.cend(),
            [](const node_store& ns) -> bool
            {
                return ns.type == node_type::directory_leaf;
            }
        );

        bool has_leaf_dir = it != children.cend();

        if (has_leaf_dir)
        {
            // Compare the amounts of overlap increase.

            key_type min_overlap = key_type();
            key_type min_area_enlargement = key_type();
            key_type min_area = key_type();

            dst = nullptr;

            for (node_store& ns : children)
            {
                directory_node* dir = static_cast<directory_node*>(ns.node_ptr);
                key_type overlap = calc_overlap_cost(bb, *dir);
                key_type area_enlargement = detail::rtree::calc_area_enlargement<_Key,bounding_box,trait_type::dimensions>(ns.box, bb);
                key_type area = detail::rtree::calc_area<_Key,bounding_box,trait_type::dimensions>(ns.box);

                bool pick_this = false;

                if (!dst)
                    pick_this = true;
                else if (overlap < min_overlap)
                    // Pick the entry with the smaller overlap cost increase.
                    pick_this = true;
                else if (area_enlargement < min_area_enlargement)
                    // Pick the entry with the smaller area enlargment.
                    pick_this = true;
                else if (area < min_area)
                    // Resolve ties by picking the one with on the smaller area
                    // rectangle.
                    pick_this = true;

                if (pick_this)
                {
                    min_overlap = overlap;
                    min_area_enlargement = area_enlargement;
                    min_area = area;
                    dst = &ns;
                }
            }

            continue;
        }

        // Compare the costs of area enlargements.
        key_type min_cost = key_type();
        key_type min_area = key_type();
        dst = nullptr;

        for (node_store& ns : children)
        {
            key_type cost = detail::rtree::calc_area_enlargement<_Key,bounding_box,trait_type::dimensions>(ns.box, bb);
            key_type area = detail::rtree::calc_area<_Key,bounding_box,trait_type::dimensions>(ns.box);

            bool pick_this = false;

            if (!dst)
                pick_this = true;
            else if (cost < min_cost)
                // Pick the entry with the smaller area enlargment.
                pick_this = true;
            else if (area < min_area)
                // Resolve ties by picking the one with on the smaller area
                // rectangle.
                pick_this = true;

            if (pick_this)
            {
                min_cost = cost;
                min_area = area;
                dst = &ns;
            }
        }
    }

    throw std::runtime_error("Maximum tree depth has been reached.");
}

template<typename _Key, typename _Value, typename _Trait>
typename rtree<_Key,_Value,_Trait>::key_type
rtree<_Key,_Value,_Trait>::calc_overlap_cost(
    const bounding_box& bb, const directory_node& dir) const
{
    key_type overlap_cost = key_type();

    for (const node_store& ns : dir.children)
        overlap_cost += detail::rtree::calc_intersection<_Key,bounding_box,trait_type::dimensions>(ns.box, bb);

    return overlap_cost;
}

template<typename _Key, typename _Value, typename _Trait>
void rtree<_Key,_Value,_Trait>::search_descend(
    const point& pt, const node_store& ns, const_search_results& results) const
{
    if (!ns.box.contains(pt))
        return;

    switch (ns.type)
    {
        case node_type::directory_nonleaf:
        case node_type::directory_leaf:
        {
            const directory_node* node = static_cast<const directory_node*>(ns.node_ptr);
            for (const node_store& child : node->children)
                search_descend(pt, child, results);
            break;
        }
        case node_type::value:
        {
            results.add_node_store(&ns);
            break;
        }
        case node_type::unspecified:
            throw std::runtime_error("unspecified node type.");
    }
}

template<typename _Key, typename _Value, typename _Trait>
void rtree<_Key,_Value,_Trait>::shrink_tree_upward(node_store* ns, const bounding_box& bb_affected)
{
    if (!ns)
        return;

    // Check if the affected bounding box is at a corner.
    if (!ns->box.contains_at_boundary(bb_affected))
        return;

    bounding_box original_bb = ns->box; // Store the original bounding box before the packing.
    bool updated = ns->pack();

    if (!updated)
        // The extent hasn't changed. There is no point going upward.
        return;

    shrink_tree_upward(ns->parent, original_bb);
}

} // namespace mdds

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

