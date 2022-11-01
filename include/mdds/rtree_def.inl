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
#include <cmath>

namespace mdds {

namespace detail { namespace rtree {

template<typename T>
using remove_cvref_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

inline const char* to_string(node_type nt)
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
}

inline size_t calc_optimal_segment_size_for_pack(size_t init_size, size_t min_size, size_t max_size, size_t value_count)
{
    // Keep increasing the size until the remainder becomes at least the min size.
    size_t final_size = init_size;
    for (; final_size < max_size; ++final_size)
    {
        size_t mod = value_count % final_size;
        if (!mod || mod >= min_size)
            return final_size;
    }

    // If increasing the size doesn't work, then decrease it.
    final_size = init_size;
    for (--final_size; min_size < final_size; --final_size)
    {
        size_t mod = value_count % final_size;
        if (!mod || mod >= min_size)
            return final_size;
    }

    // Nothing has worked.  Use the original value.
    return init_size;
}

template<typename Extent>
auto calc_linear_intersection(size_t dim, const Extent& bb1, const Extent& bb2)
    -> remove_cvref_t<decltype(bb1.start.d[0])>
{
    using key_type = remove_cvref_t<decltype(bb1.start.d[0])>;

    key_type start1 = bb1.start.d[dim], end1 = bb1.end.d[dim];
    key_type start2 = bb2.start.d[dim], end2 = bb2.end.d[dim];

    // Ensure that start1 < start2.

    if (start1 > start2)
    {
        std::swap(start1, start2);
        std::swap(end1, end2);
    }

    assert(start1 <= start2);

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

template<typename Extent>
bool intersects(const Extent& bb1, const Extent& bb2)
{
    constexpr size_t dim_size = sizeof(bb1.start.d) / sizeof(bb1.start.d[0]);
    using key_type = remove_cvref_t<decltype(bb1.start.d[0])>;

    for (size_t dim = 0; dim < dim_size; ++dim)
    {
        key_type start1 = bb1.start.d[dim], end1 = bb1.end.d[dim];
        key_type start2 = bb2.start.d[dim], end2 = bb2.end.d[dim];

        // Ensure that start1 <= start2.

        if (start1 > start2)
        {
            std::swap(start1, start2);
            std::swap(end1, end2);
        }

        assert(start1 <= start2);

        if (end1 < start2)
        {
            // 1 : |------|
            // 2 :           |-------|

            // These two are not intersected at all. Bail out.
            return false;
        }
    }

    return true;
}

template<typename Extent>
auto calc_intersection(const Extent& bb1, const Extent& bb2) -> remove_cvref_t<decltype(bb1.start.d[0])>
{
    constexpr size_t dim_size = sizeof(bb1.start.d) / sizeof(bb1.start.d[0]);
    static_assert(dim_size > 0, "Dimension cannot be zero.");
    using key_type = remove_cvref_t<decltype(bb1.start.d[0])>;

    key_type total_volume = calc_linear_intersection<Extent>(0, bb1, bb2);
    if (!total_volume)
        return key_type();

    for (size_t dim = 1; dim < dim_size; ++dim)
    {
        key_type segment_len = calc_linear_intersection<Extent>(dim, bb1, bb2);
        if (!segment_len)
            return key_type();

        total_volume *= segment_len;
    }

    return total_volume;
}

template<typename Extent>
bool enlarge_extent_to_fit(Extent& parent, const Extent& child)
{
    constexpr size_t dim_size = sizeof(parent.start.d) / sizeof(parent.start.d[0]);
    static_assert(dim_size > 0, "Dimension cannot be zero.");
    bool enlarged = false;

    for (size_t dim = 0; dim < dim_size; ++dim)
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

template<typename Extent>
auto calc_area(const Extent& bb) -> remove_cvref_t<decltype(bb.start.d[0])>
{
    constexpr size_t dim_size = sizeof(bb.start.d) / sizeof(bb.start.d[0]);
    static_assert(dim_size > 0, "Dimension cannot be zero.");
    using key_type = remove_cvref_t<decltype(bb.start.d[0])>;

    key_type area = bb.end.d[0] - bb.start.d[0];
    for (size_t dim = 1; dim < dim_size; ++dim)
        area *= bb.end.d[dim] - bb.start.d[dim];

    return area;
}

template<typename Pt>
auto calc_square_distance(const Pt& pt1, const Pt& pt2) -> remove_cvref_t<decltype(pt1.d[0])>
{
    constexpr size_t dim_size = sizeof(pt1.d) / sizeof(pt1.d[0]);
    static_assert(dim_size > 0, "Dimension cannot be zero.");
    using key_type = remove_cvref_t<decltype(pt1.d[0])>;

    key_type dist = key_type();
    for (size_t dim = 0; dim < dim_size; ++dim)
    {
        key_type v1 = pt1.d[dim], v2 = pt2.d[dim];

        if (v1 > v2)
            std::swap(v1, v2); // ensure that v1 <= v2.

        assert(v1 <= v2);

        key_type diff = v2 - v1;
        dist += diff * diff;
    }

    return dist;
}

/**
 * The margin here is defined as the sum of the lengths of the edges of a
 * bounding box, per the original paper on R*-tree.  It's half-margin
 * because it only adds one of the two edges in each dimension.
 */
template<typename Extent>
auto calc_half_margin(const Extent& bb) -> remove_cvref_t<decltype(bb.start.d[0])>
{
    constexpr size_t dim_size = sizeof(bb.start.d) / sizeof(bb.start.d[0]);
    static_assert(dim_size > 0, "Dimension cannot be zero.");
    using key_type = remove_cvref_t<decltype(bb.start.d[0])>;

    key_type margin = bb.end.d[0] - bb.start.d[0];
    for (size_t dim = 1; dim < dim_size; ++dim)
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
template<typename Extent>
auto calc_area_enlargement(const Extent& bb_host, const Extent& bb_guest)
    -> remove_cvref_t<decltype(bb_host.start.d[0])>
{
    constexpr size_t dim_size = sizeof(bb_host.start.d) / sizeof(bb_host.start.d[0]);
    static_assert(dim_size > 0, "Dimension cannot be zero.");
    using key_type = remove_cvref_t<decltype(bb_host.start.d[0])>;
    using extent = Extent;

    // Calculate the original area.
    key_type original_area = calc_area<Extent>(bb_host);

    // Enlarge the box for the new object if needed.
    extent bb_host_enlarged = bb_host; // make a copy.
    bool enlarged = enlarge_extent_to_fit<Extent>(bb_host_enlarged, bb_guest);
    if (!enlarged)
        // Area enlargement did not take place.
        return key_type();

    key_type enlarged_area = calc_area<Extent>(bb_host_enlarged);

    return enlarged_area - original_area;
}

template<typename Iter>
auto calc_extent(Iter it, Iter it_end) -> decltype(it->extent)
{
    auto bb = it->extent;
    for (++it; it != it_end; ++it)
        enlarge_extent_to_fit(bb, it->extent);

    return bb;
}

template<typename Extent>
auto get_center_point(const Extent& extent) -> decltype(extent.start)
{
    constexpr size_t dim_size = sizeof(extent.start.d) / sizeof(extent.start.d[0]);
    static_assert(dim_size > 0, "Dimension cannot be zero.");
    using point_type = decltype(extent.start);
    using key_type = decltype(extent.start.d[0]);

    point_type ret;

    static const key_type two = 2;

    for (size_t dim = 0; dim < dim_size; ++dim)
        ret.d[dim] = (extent.end.d[dim] + extent.start.d[dim]) / two;

    return ret;
}

template<typename KeyT>
struct min_value_pos
{
    KeyT value = KeyT();
    size_t pos = 0;
    size_t count = 0;

    void assign(KeyT new_value, size_t new_pos)
    {
        if (count)
        {
            // Assign only if it's less than the current value.
            if (new_value < value)
            {
                value = new_value;
                pos = new_pos;
            }
        }
        else
        {
            // The very first value. Just take it.
            value = new_value;
            pos = new_pos;
        }

        ++count;
    }
};

template<typename KeyT>
struct reinsertion_bucket
{
    using key_type = KeyT;

    key_type distance;
    size_t src_pos;
};

template<typename _NodePtrT>
struct ptr_to_string
{
    using node_ptr_type = _NodePtrT;
    using node_ptr_map_type = std::unordered_map<node_ptr_type, std::string>;

    node_ptr_map_type node_ptr_map;

    std::string operator()(node_ptr_type np) const
    {
        auto it = node_ptr_map.find(np);
        return (it == node_ptr_map.end()) ? "(*, *)" : it->second;
    }

    ptr_to_string()
    {
        static_assert(std::is_pointer<node_ptr_type>::value, "Node pointer type must be a real pointer type.");
    }

    ptr_to_string(const ptr_to_string&) = delete;
    ptr_to_string(ptr_to_string&& other) : node_ptr_map(std::move(other.node_ptr_map))
    {}
};

}} // namespace detail::rtree

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::point_type::point_type()
{
    // Initialize the point values with the key type's default value.
    key_type* p = d;
    key_type* p_end = p + traits_type::dimensions;

    for (; p != p_end; ++p)
        *p = key_type{};
}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::point_type::point_type(std::initializer_list<key_type> vs)
{
    // Initialize the point values with the key type's default value.
    key_type* dst = d;
    key_type* dst_end = dst + traits_type::dimensions;

    for (const key_type& v : vs)
    {
        if (dst == dst_end)
            throw std::range_error("number of elements exceeds the dimension size.");

        *dst = v;
        ++dst;
    }
}

template<typename KeyT, typename ValueT, typename Traits>
std::string rtree<KeyT, ValueT, Traits>::point_type::to_string() const
{
    std::ostringstream os;

    os << "(";
    for (size_t i = 0; i < traits_type::dimensions; ++i)
    {
        if (i > 0)
            os << ", ";
        os << d[i];
    }
    os << ")";

    return os.str();
}

template<typename KeyT, typename ValueT, typename Traits>
bool rtree<KeyT, ValueT, Traits>::point_type::operator==(const point_type& other) const
{
    const key_type* left = d;
    const key_type* right = other.d;
    const key_type* left_end = left + traits_type::dimensions;

    for (; left != left_end; ++left, ++right)
    {
        if (*left != *right)
            return false;
    }

    return true;
}

template<typename KeyT, typename ValueT, typename Traits>
bool rtree<KeyT, ValueT, Traits>::point_type::operator!=(const point_type& other) const
{
    return !operator==(other);
}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::extent_type::extent_type()
{}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::extent_type::extent_type(const point_type& _start, const point_type& _end)
    : start(_start), end(_end)
{}

template<typename KeyT, typename ValueT, typename Traits>
std::string rtree<KeyT, ValueT, Traits>::extent_type::to_string() const
{
    std::ostringstream os;
    os << start.to_string();

    if (!is_point())
        os << " - " << end.to_string();

    return os.str();
}

template<typename KeyT, typename ValueT, typename Traits>
bool rtree<KeyT, ValueT, Traits>::extent_type::is_point() const
{
    return start == end;
}

template<typename KeyT, typename ValueT, typename Traits>
bool rtree<KeyT, ValueT, Traits>::extent_type::operator==(const extent_type& other) const
{
    return start == other.start && end == other.end;
}

template<typename KeyT, typename ValueT, typename Traits>
bool rtree<KeyT, ValueT, Traits>::extent_type::operator!=(const extent_type& other) const
{
    return !operator==(other);
}

template<typename KeyT, typename ValueT, typename Traits>
bool rtree<KeyT, ValueT, Traits>::extent_type::contains(const point_type& pt) const
{
    for (size_t dim = 0; dim < traits_type::dimensions; ++dim)
    {
        if (pt.d[dim] < start.d[dim] || end.d[dim] < pt.d[dim])
            return false;
    }

    return true;
}

template<typename KeyT, typename ValueT, typename Traits>
bool rtree<KeyT, ValueT, Traits>::extent_type::contains(const extent_type& bb) const
{
    for (size_t dim = 0; dim < traits_type::dimensions; ++dim)
    {
        if (bb.start.d[dim] < start.d[dim] || end.d[dim] < bb.end.d[dim])
            return false;
    }

    return true;
}

template<typename KeyT, typename ValueT, typename Traits>
bool rtree<KeyT, ValueT, Traits>::extent_type::intersects(const extent_type& bb) const
{
    return detail::rtree::intersects(bb, *this);
}

template<typename KeyT, typename ValueT, typename Traits>
bool rtree<KeyT, ValueT, Traits>::extent_type::contains_at_boundary(const extent_type& bb) const
{
    for (size_t dim = 0; dim < traits_type::dimensions; ++dim)
    {
        if (start.d[dim] == bb.start.d[dim] || bb.end.d[dim] == end.d[dim])
            return true;
    }

    return false;
}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::node_store::node_store()
    : type(node_type::unspecified), parent(nullptr), node_ptr(nullptr), count(0), valid_pointer(true)
{}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::node_store::node_store(node_store&& r)
    : type(r.type), extent(r.extent), parent(r.parent), node_ptr(r.node_ptr), count(r.count),
      valid_pointer(r.valid_pointer)
{
    r.type = node_type::unspecified;
    r.extent = extent_type();
    r.parent = nullptr;
    r.node_ptr = nullptr;
    r.count = 0;
    r.valid_pointer = true;
}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::node_store::node_store(node_type _type, const extent_type& _extent, node* _node_ptr)
    : type(_type), extent(_extent), parent(nullptr), node_ptr(_node_ptr), count(0), valid_pointer(true)
{}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::node_store::~node_store()
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

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::node_store rtree<KeyT, ValueT, Traits>::node_store::clone() const
{
    auto func_copy_dir = [this](node_store& cloned, const directory_node* src) {
        directory_node* dir = cloned.get_directory_node();
        assert(dir);
        for (const node_store& ns : src->children)
            dir->children.push_back(ns.clone());

        cloned.count = count;
        cloned.extent = extent;
    };

    switch (type)
    {
        case node_type::directory_leaf:
        {
            const directory_node* src = static_cast<const directory_node*>(node_ptr);
            node_store cloned = create_leaf_directory_node();
            func_copy_dir(cloned, src);
            return cloned;
        }
        case node_type::directory_nonleaf:
        {
            const directory_node* src = static_cast<const directory_node*>(node_ptr);
            node_store cloned = create_nonleaf_directory_node();
            func_copy_dir(cloned, src);
            return cloned;
        }
        case node_type::value:
        {
            const value_node* vn = static_cast<const value_node*>(node_ptr);
            return create_value_node(extent, vn->value);
        }
        case node_type::unspecified:
        default:
            assert(!"node::~node: unknown node type!");
    }
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::node_store rtree<KeyT, ValueT, Traits>::node_store::create_leaf_directory_node()
{
    node_store ret(node_type::directory_leaf, extent_type(), new directory_node);
    ret.valid_pointer = false;
    return ret;
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::node_store rtree<
    KeyT, ValueT, Traits>::node_store::create_nonleaf_directory_node()
{
    node_store ret(node_type::directory_nonleaf, extent_type(), new directory_node);
    ret.valid_pointer = false;
    return ret;
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::node_store rtree<KeyT, ValueT, Traits>::node_store::create_value_node(
    const extent_type& extent, value_type&& v)
{
    node_store ret(node_type::value, extent, new value_node(std::move(v)));
    return ret;
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::node_store rtree<KeyT, ValueT, Traits>::node_store::create_value_node(
    const extent_type& extent, const value_type& v)
{
    node_store ret(node_type::value, extent, new value_node(v));
    return ret;
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::node_store& rtree<KeyT, ValueT, Traits>::node_store::operator=(node_store&& other)
{
    node_store tmp(std::move(other));
    swap(tmp);
    return *this;
}

template<typename KeyT, typename ValueT, typename Traits>
bool rtree<KeyT, ValueT, Traits>::node_store::pack()
{
    const directory_node* dir = get_directory_node();
    if (!dir)
        return false;

    const dir_store_type& children = dir->children;
    if (children.empty())
    {
        // This node has no children.  Reset the bounding box to empty.
        extent_type new_box;
        bool changed = new_box != extent;
        extent = new_box;
        return changed;
    }

    extent_type new_box = dir->calc_extent();
    bool changed = new_box != extent;
    extent = new_box; // update the bounding box.
    return changed;
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::node_store::pack_upward()
{
    bool propagate = true;
    for (node_store* p = parent; propagate && p; p = p->parent)
        propagate = p->pack();
}

template<typename KeyT, typename ValueT, typename Traits>
bool rtree<KeyT, ValueT, Traits>::node_store::is_directory() const
{
    switch (type)
    {
        case node_type::directory_leaf:
        case node_type::directory_nonleaf:
            return true;
        default:;
    }

    return false;
}

template<typename KeyT, typename ValueT, typename Traits>
bool rtree<KeyT, ValueT, Traits>::node_store::is_root() const
{
    return parent == nullptr;
}

template<typename KeyT, typename ValueT, typename Traits>
bool rtree<KeyT, ValueT, Traits>::node_store::exceeds_capacity() const
{
    if (type != node_type::directory_leaf)
        return false;

    return count > traits_type::max_node_size;
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::node_store::swap(node_store& other)
{
    std::swap(type, other.type);
    std::swap(extent, other.extent);
    std::swap(parent, other.parent);
    std::swap(node_ptr, other.node_ptr);
    std::swap(count, other.count);
    std::swap(valid_pointer, other.valid_pointer);
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::node_store::reset_parent_pointers_of_children()
{
    if (valid_pointer)
        return;

    directory_node* dir = get_directory_node();
    if (!dir)
        return;

    for (node_store& ns : dir->children)
    {
        ns.parent = this;
        ns.reset_parent_pointers_of_children();
    }

    valid_pointer = true;
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::node_store::reset_parent_pointers()
{
    valid_pointer = false;
    reset_parent_pointers_of_children();
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::directory_node* rtree<KeyT, ValueT, Traits>::node_store::get_directory_node()
{
    if (!is_directory())
        return nullptr;

    return static_cast<directory_node*>(node_ptr);
}

template<typename KeyT, typename ValueT, typename Traits>
const typename rtree<KeyT, ValueT, Traits>::directory_node* rtree<
    KeyT, ValueT, Traits>::node_store::get_directory_node() const
{
    if (!is_directory())
        return nullptr;

    return static_cast<const directory_node*>(node_ptr);
}

template<typename KeyT, typename ValueT, typename Traits>
bool rtree<KeyT, ValueT, Traits>::node_store::erase_child(const node_store* p)
{
    if (!is_directory())
        return false;

    directory_node* dir = static_cast<directory_node*>(node_ptr);
    bool erased = dir->erase(p);
    if (erased)
        --count;

    assert(count == dir->children.size());
    return erased;
}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::node::node()
{}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::node::~node()
{}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::value_node::value_node(value_type&& _value) : value(std::move(_value))
{}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::value_node::value_node(const value_type& _value) : value(_value)
{}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::value_node::~value_node()
{}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::directory_node::directory_node()
{}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::directory_node::~directory_node()
{}

template<typename KeyT, typename ValueT, typename Traits>
bool rtree<KeyT, ValueT, Traits>::directory_node::erase(const node_store* ns)
{
    auto it = std::find_if(
        children.begin(), children.end(), [ns](const node_store& this_ns) -> bool { return &this_ns == ns; });

    if (it == children.end())
        return false;

    // NB: std::deque::erase invalidates all elements when the erased element
    // is somwhere in the middle. But if the erased element is either the
    // first or the last element, only the erased element becomes invalidated.

    std::size_t pos = std::distance(children.begin(), it);
    bool all_valid = pos == 0 || pos == children.size() - 1;

    it = children.erase(it);

    if (!all_valid)
    {
        for (node_store& this_ns : children)
            this_ns.valid_pointer = false;
    }

    return true;
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::node_store* rtree<
    KeyT, ValueT, Traits>::directory_node::get_child_with_minimal_overlap(const extent_type& bb)
{
    key_type min_overlap = key_type();
    key_type min_area_enlargement = key_type();
    key_type min_area = key_type();

    node_store* dst = nullptr;

    for (node_store& ns : children)
    {
        directory_node* dir = static_cast<directory_node*>(ns.node_ptr);
        key_type overlap = dir->calc_overlap_cost(bb);
        key_type area_enlargement = detail::rtree::calc_area_enlargement<extent_type>(ns.extent, bb);
        key_type area = detail::rtree::calc_area<extent_type>(ns.extent);

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

    return dst;
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::node_store* rtree<
    KeyT, ValueT, Traits>::directory_node::get_child_with_minimal_area_enlargement(const extent_type& bb)
{
    // Compare the costs of area enlargements.
    key_type min_cost = key_type();
    key_type min_area = key_type();

    node_store* dst = nullptr;

    for (node_store& ns : children)
    {
        key_type cost = detail::rtree::calc_area_enlargement(ns.extent, bb);
        key_type area = detail::rtree::calc_area(ns.extent);

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

    return dst;
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::extent_type rtree<KeyT, ValueT, Traits>::directory_node::calc_extent() const
{
    auto it = children.cbegin(), ite = children.cend();

    extent_type box;
    if (it != ite)
        box = detail::rtree::calc_extent(it, ite);

    return box;
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::key_type rtree<KeyT, ValueT, Traits>::directory_node::calc_overlap_cost(
    const extent_type& bb) const
{
    key_type overlap_cost = key_type();

    for (const node_store& ns : children)
        overlap_cost += detail::rtree::calc_intersection<extent_type>(ns.extent, bb);

    return overlap_cost;
}

template<typename KeyT, typename ValueT, typename Traits>
bool rtree<KeyT, ValueT, Traits>::directory_node::has_leaf_directory() const
{
    for (const auto& ns : children)
    {
        if (ns.type == node_type::directory_leaf)
            return true;
    }

    return false;
}

template<typename KeyT, typename ValueT, typename Traits>
template<typename NS>
void rtree<KeyT, ValueT, Traits>::search_results_base<NS>::add_node_store(node_store_type* ns, size_t depth)
{
    m_store.emplace_back(ns, depth);
}

template<typename KeyT, typename ValueT, typename Traits>
template<typename NS>
rtree<KeyT, ValueT, Traits>::search_results_base<NS>::entry::entry(node_store_type* _ns, size_t _depth)
    : ns(_ns), depth(_depth)
{}

template<typename KeyT, typename ValueT, typename Traits>
template<typename _SelfIter, typename _StoreIter, typename _ValueT>
rtree<KeyT, ValueT, Traits>::iterator_base<_SelfIter, _StoreIter, _ValueT>::iterator_base(store_iterator_type pos)
    : m_pos(std::move(pos))
{}

template<typename KeyT, typename ValueT, typename Traits>
template<typename _SelfIter, typename _StoreIter, typename _ValueT>
bool rtree<KeyT, ValueT, Traits>::iterator_base<_SelfIter, _StoreIter, _ValueT>::operator==(
    const self_iterator_type& other) const
{
    return m_pos == other.m_pos;
}

template<typename KeyT, typename ValueT, typename Traits>
template<typename _SelfIter, typename _StoreIter, typename _ValueT>
bool rtree<KeyT, ValueT, Traits>::iterator_base<_SelfIter, _StoreIter, _ValueT>::operator!=(
    const self_iterator_type& other) const
{
    return !operator==(other);
}

template<typename KeyT, typename ValueT, typename Traits>
template<typename _SelfIter, typename _StoreIter, typename _ValueT>
typename rtree<KeyT, ValueT, Traits>::template iterator_base<_SelfIter, _StoreIter, _ValueT>::self_iterator_type& rtree<
    KeyT, ValueT, Traits>::iterator_base<_SelfIter, _StoreIter, _ValueT>::operator++()
{
    ++m_pos;
    return static_cast<self_iterator_type&>(*this);
}

template<typename KeyT, typename ValueT, typename Traits>
template<typename _SelfIter, typename _StoreIter, typename _ValueT>
typename rtree<KeyT, ValueT, Traits>::template iterator_base<_SelfIter, _StoreIter, _ValueT>::self_iterator_type rtree<
    KeyT, ValueT, Traits>::iterator_base<_SelfIter, _StoreIter, _ValueT>::operator++(int)
{
    self_iterator_type ret(m_pos);
    ++m_pos;
    return ret;
}

template<typename KeyT, typename ValueT, typename Traits>
template<typename _SelfIter, typename _StoreIter, typename _ValueT>
typename rtree<KeyT, ValueT, Traits>::template iterator_base<_SelfIter, _StoreIter, _ValueT>::self_iterator_type& rtree<
    KeyT, ValueT, Traits>::iterator_base<_SelfIter, _StoreIter, _ValueT>::operator--()
{
    --m_pos;
    return static_cast<self_iterator_type&>(*this);
}

template<typename KeyT, typename ValueT, typename Traits>
template<typename _SelfIter, typename _StoreIter, typename _ValueT>
typename rtree<KeyT, ValueT, Traits>::template iterator_base<_SelfIter, _StoreIter, _ValueT>::self_iterator_type rtree<
    KeyT, ValueT, Traits>::iterator_base<_SelfIter, _StoreIter, _ValueT>::operator--(int)
{
    self_iterator_type ret(m_pos);
    --m_pos;
    return ret;
}

template<typename KeyT, typename ValueT, typename Traits>
template<typename _SelfIter, typename _StoreIter, typename _ValueT>
const typename rtree<KeyT, ValueT, Traits>::extent_type& rtree<KeyT, ValueT, Traits>::iterator_base<
    _SelfIter, _StoreIter, _ValueT>::extent() const
{
    return m_pos->ns->extent;
}

template<typename KeyT, typename ValueT, typename Traits>
template<typename _SelfIter, typename _StoreIter, typename _ValueT>
size_t rtree<KeyT, ValueT, Traits>::iterator_base<_SelfIter, _StoreIter, _ValueT>::depth() const
{
    return m_pos->depth;
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::const_iterator rtree<KeyT, ValueT, Traits>::const_search_results::cbegin() const
{
    return const_iterator(m_store.cbegin());
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::const_iterator rtree<KeyT, ValueT, Traits>::const_search_results::cend() const
{
    return const_iterator(m_store.cend());
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::const_iterator rtree<KeyT, ValueT, Traits>::const_search_results::begin() const
{
    return const_iterator(m_store.cbegin());
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::const_iterator rtree<KeyT, ValueT, Traits>::const_search_results::end() const
{
    return const_iterator(m_store.cend());
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::iterator rtree<KeyT, ValueT, Traits>::search_results::begin()
{
    return iterator(m_store.begin());
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::iterator rtree<KeyT, ValueT, Traits>::search_results::end()
{
    return iterator(m_store.end());
}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::const_iterator::const_iterator(store_iterator_type pos) : base_type(std::move(pos))
{}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::iterator::iterator(store_iterator_type pos) : base_type(std::move(pos))
{}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::bulk_loader::bulk_loader()
{}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::bulk_loader::insert(const extent_type& extent, value_type&& value)
{
    insert_impl(extent, std::move(value));
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::bulk_loader::insert(const extent_type& extent, const value_type& value)
{
    insert_impl(extent, value);
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::bulk_loader::insert(const point_type& position, value_type&& value)
{
    insert_impl(extent_type({position, position}), std::move(value));
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::bulk_loader::insert(const point_type& position, const value_type& value)
{
    insert_impl(extent_type({position, position}), value);
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::bulk_loader::insert_impl(const extent_type& extent, value_type&& value)
{
    node_store ns_value = node_store::create_value_node(extent, std::move(value));
    m_store.emplace_back(std::move(ns_value));
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::bulk_loader::insert_impl(const extent_type& extent, const value_type& value)
{
    node_store ns_value = node_store::create_value_node(extent, value);
    m_store.emplace_back(std::move(ns_value));
}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits> rtree<KeyT, ValueT, Traits>::bulk_loader::pack()
{
    size_t depth = 0;
    for (; m_store.size() > traits_type::max_node_size; ++depth)
        pack_level(m_store, depth);

    // By this point, the number of directory nodes should have been reduced
    // below the max node size. Create a root directory and store them there.

    assert(m_store.size() <= traits_type::max_node_size);

    node_store root = node_store::create_leaf_directory_node();
    if (depth > 0)
        root.type = node_type::directory_nonleaf;

    directory_node* dir = root.get_directory_node();
    assert(dir);
    dir->children.swap(m_store);

    root.count = dir->children.size();
    root.pack();

    rtree tree(std::move(root));
    return tree;
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::bulk_loader::pack_level(dir_store_type& store, size_t depth)
{
    assert(!store.empty());

    float n_total_node = std::ceil(store.size() / float(traits_type::max_node_size));
    float n_splits_per_dim = std::ceil(std::pow(n_total_node, 1.0f / float(traits_type::dimensions)));

    // The first dimension will start with one segment.
    std::vector<dir_store_segment> segments;
    segments.emplace_back(store.begin(), store.end(), store.size());

    for (size_t dim = 0; dim < traits_type::dimensions; ++dim)
    {
        if (segments[0].size <= traits_type::max_node_size)
            break;

        std::vector<dir_store_segment> next_segments;

        for (dir_store_segment& seg : segments)
        {
            assert(seg.size == size_t(std::distance(seg.begin, seg.end)));

            if (seg.size <= traits_type::max_node_size)
            {
                next_segments.push_back(seg);
                continue;
            }

            // Sort by the current dimension key.
            std::sort(seg.begin, seg.end, [dim](const node_store& left, const node_store& right) -> bool {
                // Compare the middle points.
                float left_key = (left.extent.end.d[dim] + left.extent.start.d[dim]) / 2.0f;
                float right_key = (right.extent.end.d[dim] + right.extent.start.d[dim]) / 2.0f;

                return left_key < right_key;
            });

            // Size of each segment in this dimension splits.
            size_t segment_size = detail::rtree::calc_optimal_segment_size_for_pack(
                std::ceil(seg.size / n_splits_per_dim), traits_type::min_node_size, traits_type::max_node_size,
                seg.size);

            size_t n_cur_segment = 0;
            auto begin = seg.begin;
            for (auto it = begin; it != seg.end; ++it, ++n_cur_segment)
            {
                if (n_cur_segment == segment_size)
                {
                    // Push a new segment.
                    next_segments.emplace_back(begin, it, n_cur_segment);
                    begin = it;
                    n_cur_segment = 0;
                }
            }

            if (begin != seg.end)
            {
                size_t n = std::distance(begin, seg.end);
                next_segments.emplace_back(begin, seg.end, n);
            }
        }

#ifdef MDDS_RTREE_DEBUG
        size_t test_total = 0;
        for (const auto& seg : next_segments)
            test_total += seg.size;

        if (test_total != store.size())
            throw std::logic_error("The total combined segment sizes must equal the size of the inserted values!");
#endif
        segments.swap(next_segments);
    }

#ifdef MDDS_RTREE_DEBUG
    // Check the final segment.
    size_t test_total = 0;
    for (const auto& seg : segments)
        test_total += seg.size;

    if (test_total != store.size())
        throw std::logic_error("The total combined segment sizes must equal the size of the inserted values!");
#endif

    assert(!segments.empty());

    // Create a set of directory nodes from the current segments.
    dir_store_type next_store;
    for (dir_store_segment& seg : segments)
    {
        node_store ns = node_store::create_leaf_directory_node();
        if (depth > 0)
            ns.type = node_type::directory_nonleaf;

        directory_node* dir = ns.get_directory_node();
        assert(dir); // this better not be null since we know it's a directory node.

        for (auto it = seg.begin; it != seg.end; ++it)
            dir->children.push_back(std::move(*it));

        ns.count = dir->children.size();
        ns.pack();
        next_store.push_back(std::move(ns));
    }

    store.swap(next_store);
}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::rtree() : m_root(node_store::create_leaf_directory_node())
{
    static_assert(
        traits_type::min_node_size <= traits_type::max_node_size / 2,
        "Minimum node size must be less than half of the maximum node size.");

    static_assert(
        traits_type::reinsertion_size <= (traits_type::max_node_size - traits_type::min_node_size + 1),
        "Reinsertion size is too large.");
}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::rtree(rtree&& other) : m_root(std::move(other.m_root))
{
    // The root node must be a valid directory at all times.
    other.m_root = node_store::create_leaf_directory_node();

    // Since the moved root has its memory location changed, we need to update
    // the parent pointers in its child nodes.
    m_root.reset_parent_pointers();
}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::rtree(const rtree& other) : m_root(other.m_root.clone())
{
    m_root.reset_parent_pointers();
}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::rtree(node_store&& root) : m_root(std::move(root))
{
    m_root.reset_parent_pointers();
}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>::~rtree()
{}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>& rtree<KeyT, ValueT, Traits>::operator=(const rtree& other)
{
    rtree tmp(other);
    tmp.swap(*this);
    return *this;
}

template<typename KeyT, typename ValueT, typename Traits>
rtree<KeyT, ValueT, Traits>& rtree<KeyT, ValueT, Traits>::operator=(rtree&& other)
{
    rtree tmp(std::move(other));
    tmp.swap(*this);
    return *this;
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::insert(const extent_type& extent, value_type&& value)
{
    insert_impl(extent.start, extent.end, std::move(value));
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::insert(const extent_type& extent, const value_type& value)
{
    insert_impl(extent.start, extent.end, value);
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::insert(const point_type& position, value_type&& value)
{
    insert_impl(position, position, std::move(value));
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::insert(const point_type& position, const value_type& value)
{
    insert_impl(position, position, value);
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::insert_impl(const point_type& start, const point_type& end, value_type&& value)
{
    extent_type bb(start, end);
    node_store new_ns = node_store::create_value_node(bb, std::move(value));

    std::unordered_set<size_t> reinserted_depths;
    insert(std::move(new_ns), &reinserted_depths);
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::insert_impl(const point_type& start, const point_type& end, const value_type& value)
{
    extent_type bb(start, end);
    node_store new_ns = node_store::create_value_node(bb, value);

    std::unordered_set<size_t> reinserted_depths;
    insert(std::move(new_ns), &reinserted_depths);
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::insert(node_store&& ns, std::unordered_set<size_t>* reinserted_depths)
{
    extent_type ns_box = ns.extent;

    insertion_point insert_pt = find_leaf_directory_node_for_insertion(ns_box);
    node_store* dir_ns = insert_pt.ns;
    size_t depth = insert_pt.depth;

    assert(dir_ns);
    assert(dir_ns->type == node_type::directory_leaf);
    directory_node* dir = static_cast<directory_node*>(dir_ns->node_ptr);

    // Insert the new value to this node.
    ns.parent = insert_pt.ns;
    dir->children.push_back(std::move(ns));
    ++dir_ns->count;

    if (dir_ns->exceeds_capacity())
    {
        if (traits_type::enable_forced_reinsertion)
        {
            if (reinserted_depths && !reinserted_depths->count(depth))
            {
                // We perform forced re-insertion exactly once per depth level.
                reinserted_depths->insert(depth);
                perform_forced_reinsertion(dir_ns, *reinserted_depths);
            }
            else
                split_node(dir_ns);
        }
        else
            split_node(dir_ns);

        return;
    }

    if (dir_ns->count == 1)
        dir_ns->extent = ns_box;
    else
        detail::rtree::enlarge_extent_to_fit<extent_type>(dir_ns->extent, ns_box);

    extent_type bb = dir_ns->extent; // grab the parent bounding box.

    // Propagate the bounding box update up the tree all the way to the root.
    for (dir_ns = dir_ns->parent; dir_ns; dir_ns = dir_ns->parent)
    {
        assert(dir_ns->count > 0);
        detail::rtree::enlarge_extent_to_fit<extent_type>(dir_ns->extent, bb);
    }
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::insert_dir(node_store&& ns, size_t max_depth)
{
    assert(ns.is_directory());
    extent_type ns_box = ns.extent;
    node_store* dir_ns = find_nonleaf_directory_node_for_insertion(ns_box, max_depth);
    assert(dir_ns);
    assert(dir_ns->type == node_type::directory_nonleaf);
    directory_node* dir = static_cast<directory_node*>(dir_ns->node_ptr);

    // Insert the new directory to this node.
    ns.parent = dir_ns;
    ns.valid_pointer = false;
    dir->children.push_back(std::move(ns));
    ++dir_ns->count;
    dir->children.back().reset_parent_pointers_of_children();

    if (dir_ns->exceeds_capacity())
    {
        split_node(dir_ns);
        return;
    }

    if (dir_ns->count == 1)
        dir_ns->extent = ns_box;
    else
        detail::rtree::enlarge_extent_to_fit<extent_type>(dir_ns->extent, ns_box);

    extent_type bb = dir_ns->extent; // grab the parent bounding box.

    // Propagate the bounding box update up the tree all the way to the root.
    for (dir_ns = dir_ns->parent; dir_ns; dir_ns = dir_ns->parent)
    {
        assert(dir_ns->count > 0);
        detail::rtree::enlarge_extent_to_fit<extent_type>(dir_ns->extent, bb);
    }
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::const_search_results rtree<KeyT, ValueT, Traits>::search(
    const point_type& pt, search_type st) const
{
    search_condition_type dir_cond, value_cond;

    switch (st)
    {
        case search_type::overlap:
        {
            dir_cond = [&pt](const node_store& ns) -> bool { return ns.extent.contains(pt); };

            value_cond = dir_cond;
            break;
        }
        case search_type::match:
        {
            dir_cond = [&pt](const node_store& ns) -> bool { return ns.extent.contains(pt); };

            value_cond = [&pt](const node_store& ns) -> bool { return ns.extent.start == pt && ns.extent.end == pt; };

            break;
        }
        default:
            throw std::runtime_error("Unhandled search type.");
    }

    const_search_results ret;
    search_descend(0, dir_cond, value_cond, m_root, ret);
    return ret;
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::search_results rtree<KeyT, ValueT, Traits>::search(
    const point_type& pt, search_type st)
{
    search_condition_type dir_cond, value_cond;

    switch (st)
    {
        case search_type::overlap:
        {
            dir_cond = [&pt](const node_store& ns) -> bool { return ns.extent.contains(pt); };

            value_cond = dir_cond;
            break;
        }
        case search_type::match:
        {
            dir_cond = [&pt](const node_store& ns) -> bool { return ns.extent.contains(pt); };

            value_cond = [&pt](const node_store& ns) -> bool { return ns.extent.start == pt && ns.extent.end == pt; };

            break;
        }
        default:
            throw std::runtime_error("Unhandled search type.");
    }

    search_results ret;
    search_descend(0, dir_cond, value_cond, m_root, ret);
    return ret;
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::const_search_results rtree<KeyT, ValueT, Traits>::search(
    const extent_type& extent, search_type st) const
{
    search_condition_type dir_cond, value_cond;

    switch (st)
    {
        case search_type::overlap:
        {
            dir_cond = [&extent](const node_store& ns) -> bool { return ns.extent.intersects(extent); };

            value_cond = dir_cond;
            break;
        }
        case search_type::match:
        {
            dir_cond = [&extent](const node_store& ns) -> bool { return ns.extent.contains(extent); };

            value_cond = [&extent](const node_store& ns) -> bool { return ns.extent == extent; };

            break;
        }
        default:
            throw std::runtime_error("Unhandled search type.");
    }

    const_search_results ret;
    search_descend(0, dir_cond, value_cond, m_root, ret);
    return ret;
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::search_results rtree<KeyT, ValueT, Traits>::search(
    const extent_type& extent, search_type st)
{
    search_condition_type dir_cond, value_cond;

    switch (st)
    {
        case search_type::overlap:
        {
            dir_cond = [&extent](const node_store& ns) -> bool { return ns.extent.intersects(extent); };

            value_cond = dir_cond;
            break;
        }
        case search_type::match:
        {
            dir_cond = [&extent](const node_store& ns) -> bool { return ns.extent.contains(extent); };

            value_cond = [&extent](const node_store& ns) -> bool { return ns.extent == extent; };

            break;
        }
        default:
            throw std::runtime_error("Unhandled search type.");
    }

    search_results ret;
    search_descend(0, dir_cond, value_cond, m_root, ret);
    return ret;
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::erase(const const_iterator& pos)
{
    const node_store* ns = pos.m_pos->ns;
    size_t depth = pos.m_pos->depth;
    erase_impl(ns, depth);
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::erase(const iterator& pos)
{
    const node_store* ns = pos.m_pos->ns;
    size_t depth = pos.m_pos->depth;
    erase_impl(ns, depth);
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::erase_impl(const node_store* ns, size_t depth)
{
    assert(ns->type == node_type::value);
    assert(ns->parent);

    extent_type bb_erased = ns->extent;

    // Move up to the parent and find its stored location.
    node_store* dir_ns = ns->parent;
    --depth;
    assert(dir_ns->type == node_type::directory_leaf);
    bool erased = dir_ns->erase_child(ns);
    assert(erased);
    (void)erased; // to avoid getting a warning on "variable set but not used".

    if (dir_ns->is_root())
    {
        shrink_tree_upward(dir_ns, bb_erased);
        return;
    }

    if (dir_ns->count >= traits_type::min_node_size)
    {
        // The parent directory still contains enough nodes. No need to dissolve it.
        shrink_tree_upward(dir_ns, bb_erased);
        return;
    }

    // Dissolve the node the erased value node belongs to, and reinsert
    // all its siblings.

    assert(!dir_ns->is_root());
    assert(dir_ns->count < traits_type::min_node_size);

    dir_store_type orphan_value_nodes;
    directory_node* dir = static_cast<directory_node*>(dir_ns->node_ptr);
    dir->children.swap(orphan_value_nodes); // moves all the rest of the value node entries to the orphan store.

    // Move up one level, and remove this directory node from its parent directory node.
    node_store* child_ns = dir_ns;
    dir_ns = dir_ns->parent;
    --depth;
    erased = dir_ns->erase_child(child_ns);
    assert(erased);

    dir_ns->reset_parent_pointers();
    dir_ns->pack();

    orphan_node_entries_type orphan_dir_nodes;

    while (!dir_ns->is_root() && dir_ns->count < traits_type::min_node_size)
    {
        // This directory node is now underfilled. Move all its children out
        // for re-insertion and dissolve this node.
        dir = static_cast<directory_node*>(dir_ns->node_ptr);

        while (!dir->children.empty())
        {
            orphan_dir_nodes.emplace_back();
            orphan_dir_nodes.back().ns.swap(dir->children.back());
            orphan_dir_nodes.back().depth = depth + 1; // depth of the children.
            dir->children.pop_back();
        }

        // Find and remove this node from its parent store.
        node_store* dir_ns_child = dir_ns;
        dir_ns = dir_ns->parent;
        --depth;
        erased = dir_ns->erase_child(dir_ns_child);
        assert(erased);
        dir_ns->reset_parent_pointers_of_children();
        dir_ns->pack();
    }

    while (!orphan_dir_nodes.empty())
    {
        orphan_node_entry& entry = orphan_dir_nodes.back();
        insert_dir(std::move(entry.ns), entry.depth);
        orphan_dir_nodes.pop_back();
    }

    while (!orphan_value_nodes.empty())
    {
        insert(std::move(orphan_value_nodes.back()), nullptr);
        orphan_value_nodes.pop_back();
    }

    if (m_root.count == 1)
    {
        // If the root node only has one child, make that child the new root.
        // Be careful not to leak memory here...

        dir = static_cast<directory_node*>(m_root.node_ptr);
        assert(dir->children.size() == 1);
        node_store new_root(std::move(dir->children.back()));
        dir->children.clear();

        new_root.parent = nullptr;
        m_root.swap(new_root);
        m_root.reset_parent_pointers();
    }
}

template<typename KeyT, typename ValueT, typename Traits>
const typename rtree<KeyT, ValueT, Traits>::extent_type& rtree<KeyT, ValueT, Traits>::extent() const
{
    return m_root.extent;
}

template<typename KeyT, typename ValueT, typename Traits>
bool rtree<KeyT, ValueT, Traits>::empty() const
{
    return !m_root.count;
}

template<typename KeyT, typename ValueT, typename Traits>
size_t rtree<KeyT, ValueT, Traits>::size() const
{
    size_t n = 0;
    descend_with_func([&n](const node_properties& np) {
        if (np.type == node_type::value)
            ++n;
    });

    return n;
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::swap(rtree& other)
{
    m_root.swap(other.m_root);
    m_root.reset_parent_pointers();
    other.m_root.reset_parent_pointers();
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::clear()
{
    node_store new_root = node_store::create_leaf_directory_node();
    m_root.swap(new_root);
}

template<typename KeyT, typename ValueT, typename Traits>
template<typename FuncT>
void rtree<KeyT, ValueT, Traits>::walk(FuncT func) const
{
    descend_with_func(std::move(func));
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::check_integrity(const integrity_check_properties& props) const
{
    auto func_ptr_to_string = build_ptr_to_string_map();

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

    std::function<bool(const node_store*, int)> func_descend = [&ns_stack, &func_descend, &func_ptr_to_string,
                                                                &props](const node_store* ns, int level) -> bool {
        bool valid = true;

        std::string indent;
        for (int i = 0; i < level; ++i)
            indent += "    ";

        const node_store* parent = nullptr;
        extent_type parent_bb;
        if (!ns_stack.empty())
        {
            parent = ns_stack.back();
            parent_bb = parent->extent;
        }

        if (!props.throw_on_first_error)
        {
            std::cout << indent << "node: " << func_ptr_to_string(ns) << "; parent: " << func_ptr_to_string(ns->parent)
                      << "; type: " << to_string(ns->type) << "; extent: " << ns->extent.to_string() << std::endl;
        }

        if (parent)
        {
            if (ns->parent != parent)
            {
                std::ostringstream os;
                os << "The parent node pointer does not point to the real parent. (expected: " << parent
                   << "; stored in node: " << ns->parent << ")";
                if (props.throw_on_first_error)
                    throw integrity_error(os.str());
                std::cout << indent << "* " << os.str() << std::endl;
                valid = false;
            }

            if (!parent_bb.contains(ns->extent))
            {
                std::ostringstream os;
                os << "The extent of the child " << ns->extent.to_string() << " is not within the extent of the parent "
                   << parent_bb.to_string() << ".";
                if (props.throw_on_first_error)
                    throw integrity_error(os.str());
                std::cout << indent << "* " << os.str() << std::endl;
                valid = false;
            }

            switch (ns->type)
            {
                case node_type::directory_leaf:
                {
                    if (parent->type != node_type::directory_nonleaf)
                    {
                        std::ostringstream os;
                        os << "Parent of a leaf directory node must be non-leaf.";
                        if (props.throw_on_first_error)
                            throw integrity_error(os.str());
                        std::cout << indent << "* " << os.str() << std::endl;
                        valid = false;
                    }
                    break;
                }
                case node_type::directory_nonleaf:
                {
                    if (parent->type != node_type::directory_nonleaf)
                    {
                        std::ostringstream os;
                        os << "Parent of a non-leaf directory node must also be non-leaf.";
                        if (props.throw_on_first_error)
                            throw integrity_error(os.str());
                        std::cout << indent << "* " << os.str() << std::endl;
                        valid = false;
                    }
                    break;
                }
                case node_type::value:
                {
                    if (parent->type != node_type::directory_leaf)
                    {
                        std::ostringstream os;
                        os << "Parent of a value node must be a leaf directory node.";
                        if (props.throw_on_first_error)
                            throw integrity_error(os.str());
                        std::cout << indent << "* " << os.str() << std::endl;
                        valid = false;
                    }
                    break;
                }
                default:
                    throw integrity_error("Unexpected node type!");
            }
        }

        ns_stack.push_back(ns);

        switch (ns->type)
        {
            case node_type::directory_leaf:
            case node_type::directory_nonleaf:
            {
                const directory_node* dir = static_cast<const directory_node*>(ns->node_ptr);

                if (ns->count != dir->children.size())
                {
                    std::ostringstream os;
                    os << "Incorrect count of child nodes detected. (expected: " << dir->children.size()
                       << "; actual: " << ns->count << ")";

                    if (props.throw_on_first_error)
                        throw integrity_error(os.str());

                    std::cout << indent << "* " << os.str() << std::endl;
                    valid = false;
                }

                bool node_underfill_allowed = false;
                if (ns->is_root() && ns->type == node_type::directory_leaf)
                    // If the root directory is a leaf, it's allowed to be underfilled.
                    node_underfill_allowed = true;

                if (!node_underfill_allowed && ns->count < traits_type::min_node_size)
                {
                    std::ostringstream os;
                    os << "The number of child nodes (" << ns->count << ") is less than the minimum allowed number of "
                       << traits_type::min_node_size;

                    if (props.error_on_min_node_size && props.throw_on_first_error)
                        throw integrity_error(os.str());

                    std::cout << indent << "* " << os.str() << std::endl;

                    if (props.error_on_min_node_size)
                        valid = false;
                }

                if (traits_type::max_node_size < ns->count)
                {
                    std::ostringstream os;
                    os << "The number of child nodes (" << ns->count << ") exceeds the maximum allowed number of "
                       << traits_type::max_node_size;

                    if (props.throw_on_first_error)
                        throw integrity_error(os.str());

                    std::cout << indent << "* " << os.str() << std::endl;
                    valid = false;
                }

                // Check to make sure the bounding box of the current node is
                // tightly packed.
                extent_type bb_expected = dir->calc_extent();

                if (bb_expected != ns->extent)
                {
                    std::ostringstream os;
                    os << "The extent of the node " << ns->extent.to_string() << " does not equal truly tight extent "
                       << bb_expected.to_string();

                    if (props.throw_on_first_error)
                        throw integrity_error(os.str());

                    std::cout << indent << "* " << os.str() << std::endl;
                    valid = false;
                }

                for (const node_store& ns_child : dir->children)
                {
                    bool valid_subtree = func_descend(&ns_child, level + 1);
                    if (!valid_subtree)
                        valid = false;
                }

                break;
            }
            case node_type::value:
                // Do nothing.
                break;
            default:
                throw integrity_error("Unexpected node type!");
        }

        ns_stack.pop_back();

        return valid;
    };

    bool valid = func_descend(&m_root, 0);

    if (!valid)
        throw integrity_error("Tree contains one or more errors.");
}

template<typename KeyT, typename ValueT, typename Traits>
std::string rtree<KeyT, ValueT, Traits>::export_tree(export_tree_type mode) const
{
    switch (mode)
    {
        case export_tree_type::formatted_node_properties:
            return export_tree_formatted();
        case export_tree_type::extent_as_obj:
            return export_tree_extent_as_obj();
        case export_tree_type::extent_as_svg:
            return export_tree_extent_as_svg();
        default:
            throw std::runtime_error("unhandled export tree type.");
    }
}

template<typename KeyT, typename ValueT, typename Traits>
detail::rtree::ptr_to_string<const typename rtree<KeyT, ValueT, Traits>::node_store*> rtree<
    KeyT, ValueT, Traits>::build_ptr_to_string_map() const
{
    detail::rtree::ptr_to_string<const node_store*> func;

    std::function<void(const node_store*, int, int)> func_build_node_ptr =
        [&func_build_node_ptr, &func](const node_store* ns, int level, int pos) {
            std::ostringstream os;
            os << "(" << level << ", " << pos << ")";
            func.node_ptr_map.insert(std::make_pair(ns, os.str()));

            switch (ns->type)
            {
                case node_type::directory_leaf:
                case node_type::directory_nonleaf:
                {
                    const directory_node* dir = static_cast<const directory_node*>(ns->node_ptr);

                    int child_pos = 0;
                    for (const node_store& ns_child : dir->children)
                        func_build_node_ptr(&ns_child, level + 1, child_pos++);

                    break;
                }
                case node_type::value:
                    // Do nothing.
                    break;
                default:
                    throw integrity_error("Unexpected node type!");
            }
        };

    func_build_node_ptr(&m_root, 0, 0);

    return func;
}

template<typename KeyT, typename ValueT, typename Traits>
std::string rtree<KeyT, ValueT, Traits>::export_tree_formatted() const
{
    auto func_ptr_to_string = build_ptr_to_string_map();

    std::ostringstream os;

    std::function<void(const node_store*, int)> func_descend = [&func_descend, &os,
                                                                &func_ptr_to_string](const node_store* ns, int level) {
        std::string indent;
        for (int i = 0; i < level; ++i)
            indent += "    ";

        os << indent << "node: " << func_ptr_to_string(ns) << "; parent: " << func_ptr_to_string(ns->parent)
           << "; type: " << to_string(ns->type) << "; extent: " << ns->extent.to_string() << std::endl;

        switch (ns->type)
        {
            case node_type::directory_leaf:
            case node_type::directory_nonleaf:
            {
                const directory_node* dir = static_cast<const directory_node*>(ns->node_ptr);

                for (const node_store& ns_child : dir->children)
                    func_descend(&ns_child, level + 1);

                break;
            }
            case node_type::value:
                // Do nothing.
                break;
            default:
                throw integrity_error("Unexpected node type!");
        }
    };

    func_descend(&m_root, 0);

    return os.str();
}

template<typename KeyT, typename ValueT, typename Traits>
std::string rtree<KeyT, ValueT, Traits>::export_tree_extent_as_obj() const
{
    if (traits_type::dimensions != 2u)
        throw size_error("Only 2-dimensional trees are supported.");

    float unit_height =
        ((m_root.extent.end.d[0] - m_root.extent.start.d[0]) + (m_root.extent.end.d[1] - m_root.extent.start.d[1])) /
        5.0f;

    // Calculate the width to use for point data.
    float pt_width = std::min<float>(
        m_root.extent.end.d[0] - m_root.extent.start.d[0], m_root.extent.end.d[1] - m_root.extent.start.d[1]);
    pt_width /= 400.0f;
    pt_width = std::min<float>(pt_width, 1.0f);

    std::ostringstream os;
    size_t counter = 0;

    std::function<void(const node_store*, int)> func_descend = [&](const node_store* ns, int level) {
        size_t offset = counter * 4;
        point_type s = ns->extent.start;
        point_type e = ns->extent.end;
        if (s == e)
        {
            s.d[0] -= pt_width;
            s.d[1] -= pt_width;
            e.d[0] += pt_width;
            e.d[1] += pt_width;
        }

        os << "o extent " << counter << " (level " << level << ") " << s.to_string() << " - " << e.to_string()
           << std::endl;
        os << "v " << s.d[0] << ' ' << (level * unit_height) << ' ' << s.d[1] << std::endl;
        os << "v " << s.d[0] << ' ' << (level * unit_height) << ' ' << e.d[1] << std::endl;
        os << "v " << e.d[0] << ' ' << (level * unit_height) << ' ' << e.d[1] << std::endl;
        os << "v " << e.d[0] << ' ' << (level * unit_height) << ' ' << s.d[1] << std::endl;
        os << "f " << (offset + 1) << ' ' << (offset + 2) << ' ' << (offset + 3) << ' ' << (offset + 4) << std::endl;

        ++counter;

        switch (ns->type)
        {
            case node_type::directory_leaf:
            case node_type::directory_nonleaf:
            {
                const directory_node* dir = static_cast<const directory_node*>(ns->node_ptr);

                for (const node_store& ns_child : dir->children)
                    func_descend(&ns_child, level + 1);

                break;
            }
            case node_type::value:
                // Do nothing.
                break;
            default:
                throw integrity_error("Unexpected node type!");
        }
    };

    func_descend(&m_root, 0);

    return os.str();
}

template<typename KeyT, typename ValueT, typename Traits>
std::string rtree<KeyT, ValueT, Traits>::export_tree_extent_as_svg() const
{
    if (traits_type::dimensions != 2u)
        throw size_error("Only 2-dimensional trees are supported.");

    constexpr float min_avg_root_length = 800.0f;
    constexpr float max_avg_root_length = 1000.0f;
    float root_w = m_root.extent.end.d[0] - m_root.extent.start.d[0];
    float root_h = m_root.extent.end.d[1] - m_root.extent.start.d[1];

    // Adjust zooming for optimal output size. We don't want it to be too
    // large or too small.
    float zoom_ratio = 1.0;
    float root_avg = (root_w + root_h) / 2.0f;
    if (root_avg >= max_avg_root_length)
        zoom_ratio = max_avg_root_length / root_avg;
    if (root_avg <= min_avg_root_length)
        zoom_ratio = min_avg_root_length / root_avg;

    root_w *= zoom_ratio;
    root_h *= zoom_ratio;
    float root_x = m_root.extent.start.d[0] * zoom_ratio;
    float root_y = m_root.extent.start.d[1] * zoom_ratio;
    float r = root_avg / 100.0f * zoom_ratio;
    float stroke_w = r / 10.0f; // stroke width

    const std::string indent = "    ";

    // Uniform attributes to use for all drawing objects.

    std::string attrs_dir;
    {
        std::ostringstream os;
        os << " stroke=\"#999999\" stroke-width=\"" << stroke_w << "\" fill=\"green\" fill-opacity=\"0.05\"";
        attrs_dir = os.str();
    }

    std::string attrs_value;
    {
        std::ostringstream os;
        os << " stroke=\"brown\" stroke-width=\"" << stroke_w << "\" fill=\"brown\" fill-opacity=\"0.2\"";
        attrs_value = os.str();
    }

    std::ostringstream os;
    os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    os << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";

    std::function<void(const node_store*, int)> func_descend = [&](const node_store* ns, int level) {
        const extent_type& ext = ns->extent;

        float w = ext.end.d[0] - ext.start.d[0];
        float h = ext.end.d[1] - ext.start.d[1];
        float x = ext.start.d[0];
        float y = ext.start.d[1];
        w *= zoom_ratio;
        h *= zoom_ratio;
        x *= zoom_ratio;
        y *= zoom_ratio;
        x -= root_x;
        y -= root_y;

        if (level > 0)
        {
            const char* attrs = (ns->type == node_type::value) ? attrs_value.data() : attrs_dir.data();

            if (ext.is_point())
            {
                os << indent << "<circle cx=\"" << x << "\" cy=\"" << y << "\" r=\"" << r << "\"" << attrs << "/>\n";
            }
            else
            {
                os << indent << "<rect x=\"" << x << "\" y=\"" << y << "\" width=\"" << w << "\" height=\"" << h << "\""
                   << attrs << "/>\n";
            }
        }

        switch (ns->type)
        {
            case node_type::directory_leaf:
            case node_type::directory_nonleaf:
            {
                const directory_node* dir = static_cast<const directory_node*>(ns->node_ptr);

                for (const node_store& ns_child : dir->children)
                    func_descend(&ns_child, level + 1);

                break;
            }
            case node_type::value:
                // Do nothing.
                break;
            default:
                throw integrity_error("Unexpected node type!");
        }
    };

    os << "<svg version=\"1.2\" width=\"" << root_w << "\" height=\"" << root_h << "\">\n";
    func_descend(&m_root, 0);
    os << "</svg>";

    return os.str();
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::split_node(node_store* ns)
{
    directory_node* dir = ns->get_directory_node();

    assert(dir);
    assert(ns->count == traits_type::max_node_size + 1);

    dir_store_type& children = dir->children;

    sort_dir_store_by_split_dimension(children);

    size_t dist = pick_optimal_distribution(children);
    distribution dist_picked(dist, children);

    // Move the child nodes in group 2 into a brand-new sibling node.
    node_store node_g2 = node_store::create_leaf_directory_node();
    node_g2.type = ns->type;
    directory_node* dir_sibling = static_cast<directory_node*>(node_g2.node_ptr);

    for (auto it = dist_picked.g2.begin; it != dist_picked.g2.end; ++it)
    {
        assert(!it->valid_pointer);
        dir_sibling->children.push_back(std::move(*it));
    }

    node_g2.count = dir_sibling->children.size();
    node_g2.pack();

    // Remove the nodes in group 2 from the original node by shrinking the node store.
    ns->count = dist_picked.g1.size;
    assert(dist_picked.g1.size < dir->children.size());
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
            ns_child.reset_parent_pointers_of_children();
    }
    else
    {
        // Place the new sibling (node_g2) under the same parent as ns.
        assert(ns->parent);
        node_g2.parent = ns->parent;
        node_store* ns_parent = ns->parent;
        assert(ns_parent->type == node_type::directory_nonleaf);
        directory_node* dir_parent = static_cast<directory_node*>(ns_parent->node_ptr);
        dir_parent->children.emplace_back(std::move(node_g2));
        ++ns_parent->count;
        bool parent_size_changed = ns_parent->pack();

        // Update the parent pointer of the children _after_ the group 2 node
        // has been inserted into the buffer, as the pointer value of the node
        // changes after the insertion.
        ns->reset_parent_pointers();
        dir_parent->children.back().reset_parent_pointers_of_children();

        if (ns_parent->count > traits_type::max_node_size)
            // The parent node is overfilled.  Split it and keep working upward.
            split_node(ns_parent);
        else if (parent_size_changed)
            // The extent of the parent node has changed. Propagate the change upward.
            ns_parent->pack_upward();
    }
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::perform_forced_reinsertion(
    node_store* ns, std::unordered_set<size_t>& reinserted_depth)
{
    assert(ns->count == traits_type::max_node_size + 1);

    // Compute the distance between the centers of the value extents and the
    // center of the extent of the parent directory.

    point_type center_of_dir = detail::rtree::get_center_point(ns->extent);

    directory_node* dir = ns->get_directory_node();
    assert(dir);

    using buckets_type = std::vector<detail::rtree::reinsertion_bucket<key_type>>;
    buckets_type buckets;
    buckets.reserve(ns->count);

    size_t pos = 0;
    for (const node_store& ns_child : dir->children)
    {
        buckets.emplace_back();
        buckets.back().src_pos = pos++;

        point_type center_of_child = detail::rtree::get_center_point(ns_child.extent);
        buckets.back().distance = detail::rtree::calc_square_distance(center_of_dir, center_of_child);
    }

    // Sort the value entries in decreasing order of their distances.

    std::sort(
        buckets.begin(), buckets.end(),
        [](const typename buckets_type::value_type& left, const typename buckets_type::value_type& right) -> bool {
            return left.distance < right.distance;
        });

    assert(traits_type::reinsertion_size < buckets.size());

    // Remove the first set of entries from the parent directory.
    std::deque<node_store> nodes_to_reinsert(traits_type::reinsertion_size);

    for (size_t i = 0; i < traits_type::reinsertion_size; ++i)
    {
        size_t this_pos = buckets[i].src_pos;
        dir->children[this_pos].swap(nodes_to_reinsert[i]);
    }

    // Erase the swapped out nodes from the directory.
    auto it = std::remove_if(dir->children.begin(), dir->children.end(), [](const node_store& this_ns) -> bool {
        return this_ns.type == node_type::unspecified;
    });

    dir->children.erase(it, dir->children.end());
    ns->count -= nodes_to_reinsert.size();
    assert(ns->count == dir->children.size());

    // No need to invalidate pointers since they are all value nodes.

    if (ns->pack())
        ns->pack_upward();

    // Re-insert the values from the closest to farthest.

    while (!nodes_to_reinsert.empty())
    {
        node_store ns_to_reinsert(std::move(nodes_to_reinsert.front()));
        nodes_to_reinsert.pop_front();

        insert(std::move(ns_to_reinsert), &reinserted_depth);
    }
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::sort_dir_store_by_split_dimension(dir_store_type& children)
{
    // Store the sum of margins for each dimension axis.
    detail::rtree::min_value_pos<key_type> min_margin_dim;

    for (size_t dim = 0; dim < traits_type::dimensions; ++dim)
    {
        // Sort the entries by the lower then by the upper value of their
        // bounding boxes.  This invalidates the pointers of the child nodes.
        sort_dir_store_by_dimension(dim, children);

        key_type sum_of_margins = key_type(); // it's actually the sum of half margins.

        for (size_t dist = 1; dist <= max_dist_size; ++dist)
        {
            // The first group contains m-1+dist entries, while the second
            // group contains the rest.

            auto it = children.begin();
            auto it_end = it;
            std::advance(it_end, traits_type::min_node_size - 1 + dist);

            extent_type bb1 = detail::rtree::calc_extent(it, it_end);
            it = it_end;
            it_end = children.end();
            assert(it != it_end);
            extent_type bb2 = detail::rtree::calc_extent(it, it_end);

            // Compute the half margins of the first and second groups.
            key_type margin1 = detail::rtree::calc_half_margin<extent_type>(bb1);
            key_type margin2 = detail::rtree::calc_half_margin<extent_type>(bb2);
            key_type margins = margin1 + margin2;

            sum_of_margins += margins;
        }

        min_margin_dim.assign(sum_of_margins, dim);
    }

    // Pick the dimension axis with the lowest sum of margins.
    size_t min_dim = min_margin_dim.pos;

    sort_dir_store_by_dimension(min_dim, children);
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::sort_dir_store_by_dimension(size_t dim, dir_store_type& children)
{
    std::sort(children.begin(), children.end(), [dim](const node_store& a, const node_store& b) -> bool {
        if (a.extent.start.d[dim] != b.extent.start.d[dim])
            return a.extent.start.d[dim] < b.extent.start.d[dim];

        return a.extent.end.d[dim] < b.extent.end.d[dim];
    });

    for (node_store& ns : children)
        ns.valid_pointer = false;
}

template<typename KeyT, typename ValueT, typename Traits>
size_t rtree<KeyT, ValueT, Traits>::pick_optimal_distribution(dir_store_type& children) const
{
    // Along the chosen dimension axis, pick the distribution with the minimum
    // overlap value.
    detail::rtree::min_value_pos<key_type> min_overlap_dist;

    for (size_t dist = 1; dist <= max_dist_size; ++dist)
    {
        // The first group contains m-1+dist entries, while the second
        // group contains the rest.
        distribution dist_data(dist, children);
        extent_type bb1 = detail::rtree::calc_extent(dist_data.g1.begin, dist_data.g1.end);
        extent_type bb2 = detail::rtree::calc_extent(dist_data.g2.begin, dist_data.g2.end);

        key_type overlap = detail::rtree::calc_intersection<extent_type>(bb1, bb2);
        min_overlap_dist.assign(overlap, dist);
    }

    return min_overlap_dist.pos;
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::insertion_point rtree<
    KeyT, ValueT, Traits>::find_leaf_directory_node_for_insertion(const extent_type& bb)
{
    insertion_point ret;
    ret.ns = &m_root;

    for (size_t i = 0; i <= traits_type::max_tree_depth; ++i)
    {
        if (ret.ns->type == node_type::directory_leaf)
        {
            ret.depth = i;
            return ret;
        }

        assert(ret.ns->type == node_type::directory_nonleaf);

        directory_node* dir = static_cast<directory_node*>(ret.ns->node_ptr);

        // If this non-leaf directory contains at least one leaf directory,
        // pick the entry with minimum overlap increase.  If all of its child
        // nodes are non-leaf directories, then pick the entry with minimum
        // area enlargement.

        if (dir->has_leaf_directory())
            ret.ns = dir->get_child_with_minimal_overlap(bb);
        else
            ret.ns = dir->get_child_with_minimal_area_enlargement(bb);
    }

    throw std::runtime_error("Maximum tree depth has been reached.");
}

template<typename KeyT, typename ValueT, typename Traits>
typename rtree<KeyT, ValueT, Traits>::node_store* rtree<
    KeyT, ValueT, Traits>::find_nonleaf_directory_node_for_insertion(const extent_type& bb, size_t max_depth)
{
    node_store* dst = &m_root;

    for (size_t i = 0; i <= traits_type::max_tree_depth; ++i)
    {
        assert(dst->is_directory());

        if (!dst->count)
            // This node has no children.
            return dst;

        assert(dst->type == node_type::directory_nonleaf);

        if (i == max_depth)
            return dst;

        directory_node* dir = static_cast<directory_node*>(dst->node_ptr);

        if (dir->has_leaf_directory())
            return dst;

        assert(dst->type == node_type::directory_nonleaf);
        dst = dir->get_child_with_minimal_area_enlargement(bb);
        assert(dst);
    }

    throw std::runtime_error("Maximum tree depth has been reached.");
}

template<typename KeyT, typename ValueT, typename Traits>
template<typename FuncT>
void rtree<KeyT, ValueT, Traits>::descend_with_func(FuncT func) const
{
    std::function<void(const node_store*)> func_descend = [&](const node_store* ns) {
        node_properties np;
        np.type = ns->type;
        np.extent = ns->extent;
        func(np);

        switch (ns->type)
        {
            case node_type::directory_leaf:
            case node_type::directory_nonleaf:
            {
                const directory_node* dir = static_cast<const directory_node*>(ns->node_ptr);

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

template<typename KeyT, typename ValueT, typename Traits>
template<typename _ResT>
void rtree<KeyT, ValueT, Traits>::search_descend(
    size_t depth, const search_condition_type& dir_cond, const search_condition_type& value_cond,
    typename _ResT::node_store_type& ns, _ResT& results) const
{
    switch (ns.type)
    {
        case node_type::directory_nonleaf:
        case node_type::directory_leaf:
        {
            if (!dir_cond(ns))
                return;

            auto* dir_node = ns.get_directory_node();
            for (auto& child : dir_node->children)
                search_descend(depth + 1, dir_cond, value_cond, child, results);
            break;
        }
        case node_type::value:
        {
            if (!value_cond(ns))
                return;

            results.add_node_store(&ns, depth);
            break;
        }
        case node_type::unspecified:
            throw std::runtime_error("unspecified node type.");
    }
}

template<typename KeyT, typename ValueT, typename Traits>
void rtree<KeyT, ValueT, Traits>::shrink_tree_upward(node_store* ns, const extent_type& bb_affected)
{
    if (!ns)
        return;

    // Check if the affected bounding box is at a corner.
    if (!ns->extent.contains_at_boundary(bb_affected))
        return;

    extent_type original_bb = ns->extent; // Store the original bounding box before the packing.
    bool updated = ns->pack();

    if (!updated)
        // The extent hasn't changed. There is no point going upward.
        return;

    shrink_tree_upward(ns->parent, original_bb);
}

} // namespace mdds

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
