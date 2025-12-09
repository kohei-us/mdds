/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#ifndef INCLUDED_MDDS_MULTI_TYPE_VECTOR_DIR_ITERATOR_NODE_HPP
#define INCLUDED_MDDS_MULTI_TYPE_VECTOR_DIR_ITERATOR_NODE_HPP

namespace mdds { namespace detail { namespace mtv {

/**
 * Node that represents the content of each iterator.  The private data part
 * is an implementation detail that should never be accessed externally.
 * What the end position stores in its private data is totally &
 * intentionally undefined.
 */
template<typename ParentT, typename SizeT>
struct iterator_value_node
{
    using parent_type = ParentT;
    using size_type = SizeT;

    mdds::mtv::element_t type;
    size_type position;
    size_type size;
    mdds::mtv::base_element_block* data;

    iterator_value_node(const parent_type* parent, size_type block_index)
        : type(mdds::mtv::element_type_empty), position(0), size(0), data(nullptr), __private_data(parent, block_index)
    {}

    void swap(iterator_value_node& other)
    {
        std::swap(type, other.type);
        std::swap(position, other.position);
        std::swap(size, other.size);
        std::swap(data, other.data);

        __private_data.swap(other.__private_data);
    }

    struct private_data
    {
        const parent_type* parent;
        size_type block_index;

        private_data() : parent(nullptr), block_index(0)
        {}
        private_data(const parent_type* _parent, size_type _block_index) : parent(_parent), block_index(_block_index)
        {}

        void swap(private_data& other)
        {
            std::swap(parent, other.parent);
            std::swap(block_index, other.block_index);
        }
    };
    private_data __private_data;

    bool operator==(const iterator_value_node& other) const
    {
        return type == other.type && position == other.position && size == other.size && data == other.data &&
               __private_data.parent == other.__private_data.parent &&
               __private_data.block_index == other.__private_data.block_index;
    }

    bool operator!=(const iterator_value_node& other) const
    {
        return !operator==(other);
    }
};

template<typename ParentT, typename SizeT>
struct private_data_no_update
{
    using node_type = iterator_value_node<ParentT, SizeT>;

    static void inc(node_type&)
    {}
    static void dec(node_type&)
    {}
};

template<typename ParentT, typename SizeT>
struct private_data_forward_update
{
    using node_type = iterator_value_node<ParentT, SizeT>;

    static void inc(node_type& nd)
    {
        ++nd.__private_data.block_index;
    }

    static void dec(node_type& nd)
    {
        --nd.__private_data.block_index;
    }
};

}}} // namespace mdds::detail::mtv

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
