/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2015 Kohei Yoshida
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

#include <cassert>
#include <algorithm>
#include <memory>
#include <sstream>
#include <ostream>
#include <iostream>
#include <iomanip>
#include <type_traits>
#include <cstring>

#ifdef MDDS_TRIE_MAP_DEBUG
using std::cout;
using std::endl;
#endif

#define HAS_STATIC_CONSTEXPR_MEMBER(MEMBER) \
    template<typename T> \
    class has_##MEMBER \
    { \
        using y_type = char; \
        using n_type = long; \
\
        template<typename U> \
        static y_type test(decltype(U::MEMBER)); \
        template<typename U> \
        static n_type test(...); \
\
    public: \
        static constexpr bool value = sizeof(test<T>(0)) == sizeof(y_type); \
    };

namespace mdds {

namespace trie { namespace detail {

HAS_STATIC_CONSTEXPR_MEMBER(dump_packed_construction_state)
HAS_STATIC_CONSTEXPR_MEMBER(dump_packed_buffer)

template<typename TrieT, typename NodeT, typename = void>
struct dump_packed_trie_node_tree
{
    void operator()(std::ostream&, const NodeT&)
    {}
};

template<typename TrieT, typename NodeT>
struct dump_packed_trie_node_tree<
    TrieT, NodeT, typename std::enable_if_t<has_dump_packed_construction_state<typename TrieT::traits_type>::value>>
{
    using trie_node = NodeT;

    void operator()(std::ostream& os, const NodeT& root)
    {
        if constexpr (!TrieT::traits_type::dump_packed_construction_state)
            return;

        typename TrieT::key_type buffer;
        traverse_node(os, buffer, root);
    }

private:
    void traverse_node(std::ostream& os, typename TrieT::key_type& buffer, const NodeT& node)
    {
        if (node.value)
        {
            // This node has value.
            os << "trie-node: key='" << buffer << "'; value=" << *node.value << std::endl;
        }

        for (const trie_node* p : node.children)
        {
            const trie_node& this_node = *p;
            buffer.push_back(this_node.key);
            traverse_node(os, buffer, this_node);
            buffer.pop_back();
        }
    }
};

template<typename PackedT, typename HandlerT>
struct traverse_packed_buffer
{
    using packed_type = PackedT;

    void operator()(const packed_type& packed, HandlerT hdl) const
    {
        size_t n = packed.size();
        size_t i = 0;
        hdl.root_offset(i, packed[i]);
        ++i;

        while (i < n)
        {
            hdl.node_value(i, packed[i]);
            ++i;

            hdl.node_index_size(i, packed[i]);
            size_t index_size = packed[i];
            ++i;
            index_size /= 2;

            for (size_t j = 0; j < index_size; ++j)
            {
                hdl.node_child_key(i, packed[i]);
                ++i;
                hdl.node_child_offset(i, packed[i]);
                ++i;
            }
        }
    }
};

template<typename TrieT, typename PackedT, typename = void>
struct dump_packed_buffer
{
    using packed_type = PackedT;

    void operator()(std::ostream&, const packed_type&)
    {}
};

template<typename TrieT, typename PackedT>
struct dump_packed_buffer<TrieT, PackedT, std::enable_if_t<has_dump_packed_buffer<typename TrieT::traits_type>::value>>
{
    using packed_type = PackedT;
    using key_type = typename TrieT::key_type;
    using key_unit_type = typename key_type::value_type;
    using value_type = typename TrieT::value_type;

    void operator()(std::ostream& os, const packed_type& packed)
    {
        os << "packed size: " << packed.size() << std::endl;

        struct _handler
        {
            std::ostream& m_os;
            std::size_t m_this_node_offset = 0;

            _handler(std::ostream& _os) : m_os(_os)
            {}

            /** first element in the buffer. */
            void root_offset(size_t i, const uintptr_t& v) const
            {
                m_os << std::setw(4) << i << ": root node offset: " << v << std::endl;
            }

            /** first element in each node section. */
            void node_value(size_t i, const uintptr_t& v)
            {
                m_this_node_offset = i;
                const value_type* value = reinterpret_cast<const value_type*>(v);
                m_os << std::setw(4) << i << ": node value pointer: " << value << std::endl;
            }

            /**
             * second element in each node section that stores the size of
             * the child data sub-section.
             */
            void node_index_size(size_t i, const uintptr_t& v) const
            {
                m_os << std::setw(4) << i << ": index size: " << size_t(v) << std::endl;
            }

            /** element that stores the key value for child node. */
            void node_child_key(size_t i, const uintptr_t& v) const
            {
                key_unit_type key = v;
                m_os << std::setw(4) << i << ": key: " << key << std::endl;
            }

            /** element that stores the relative offset of the child node. */
            void node_child_offset(size_t i, const uintptr_t& v) const
            {
                size_t offset = v;
                m_os << std::setw(4) << i << ": offset: " << offset << " (abs: " << (m_this_node_offset - offset) << ")"
                     << std::endl;
            }

        } handler(os);

        traverse_packed_buffer<packed_type, _handler>{}(packed, handler);
    }
};

template<typename PackedT>
void verify_packed_position(const PackedT& packed, const typename PackedT::value_type* pos)
{
    const auto* end_pos = packed.data() + packed.size();

    if (packed.data() <= pos && pos < end_pos)
        return;

    std::ostringstream os;
    os << "stored position is outside the valid data region (pos=" << pos << "; packed-begin=" << packed.data()
       << "; packed-end=" << end_pos;
    throw integrity_error(os.str());
}

template<typename ValueStoreT>
void verify_packed_value(const ValueStoreT& value_store, const typename ValueStoreT::value_type* pv)
{
    auto it = std::find_if(value_store.begin(), value_store.end(), [pv](const auto& v) { return &v == pv; });

    if (it == value_store.end())
    {
        std::ostringstream os;
        os << "no value found in the value store with a memory address of " << pv;
        throw integrity_error(os.str());
    }
}

template<typename KeyUnitT, typename SizeT>
const uintptr_t* find_child_pos(const uintptr_t* first_child, SizeT n_child_nodes, KeyUnitT c)
{
    using size_type = SizeT;
    using key_unit_type = KeyUnitT;

    for (size_type low = 0, high = n_child_nodes - 1; low <= high;)
    {
        size_type i = (low + high) / 2; // take the mid position

        // each slot contains the key for the child and offset to its index position
        const uintptr_t* child_pos = first_child + i * 2;
        const key_unit_type node_key = *child_pos;

        if (c == node_key)
        {
            // Match found!
            return child_pos;
        }

        if (low == high)
            // No more child node key to test. Bail out.
            break;

        if (high - low == 1)
        {
            // Only two more child keys left.
            if (i == low)
                low = high;
            else
            {
                assert(i == high);
                high = low;
            }
        }
        else if (c < node_key)
            // Move on to the lower sub-group.
            high = i;
        else
            // Move on to the higher sub-group.
            low = i;
    }

    // no match found
    return nullptr;
}

template<typename KeyUnitT, typename SizeT>
const uintptr_t* find_prefix_node(
    const uintptr_t* p, const KeyUnitT* prefix, const KeyUnitT* prefix_end,
    std::function<void(const uintptr_t*, const uintptr_t*, const uintptr_t*)> node_func)
{
    using size_type = SizeT;

    if (prefix == prefix_end)
    {
        // target node found!
        size_t index_size = *(p + 1);
        const uintptr_t* child_pos = p + 2;
        const uintptr_t* child_end = child_pos + index_size;
        node_func(p, child_pos, child_end);
        return p;
    }

    const uintptr_t* p0 = p; // store the head offset position of this node.

    // Find the child node with a matching key character.

    ++p;
    size_type index_size = *p;
    size_type n = index_size / 2; // number of child nodes
    ++p;

    if (!n)
    {
        // This is a leaf node - no more child nodes to test
        node_func(nullptr, nullptr, nullptr);
        return nullptr;
    }

    const uintptr_t* child_end = p + index_size;

    if (const auto* child_pos = find_child_pos(p, n, *prefix); child_pos)
    {
        // Match found!
        size_type offset = *(child_pos + 1);
        node_func(p0, child_pos, child_end);
        const uintptr_t* p_child = p0 - offset;
        ++prefix;
        return find_prefix_node<KeyUnitT, SizeT>(p_child, prefix, prefix_end, std::move(node_func));
    }

    // no matching node found - search failed
    node_func(nullptr, nullptr, nullptr);
    return nullptr;
}

inline const char* value_type_size_name(bool variable_size)
{
    return variable_size ? "varaible size" : "fixed size";
}

union bin_value
{
    char buffer[8];
    uint8_t ui8;
    uint16_t ui16;
    uint32_t ui32;
    uint64_t ui64;
};

using value_addrs_type = std::map<const void*, size_t>;

template<typename FuncT, typename ValueT>
struct write_variable_size_values_to_ostream
{
    value_addrs_type operator()(std::ostream& os, const std::deque<ValueT>& value_store) const
    {
        bin_value bv;

        value_addrs_type value_addrs;

        size_t pos = 0;
        for (const ValueT& v : value_store)
        {
            auto sp_size = os.tellp(); // position to come back to to write the size.
            bv.ui32 = 0;
            os.write(bv.buffer, 4); // write 0 as a placeholder.

            auto sp_start = os.tellp();
            FuncT::write(os, v);
            auto sp_end = os.tellp();

            bv.ui32 = sp_end - sp_start; // bytes written

            // go back and write the actual bytes written.
            os.seekp(sp_size);
            os.write(bv.buffer, 4);
            os.seekp(sp_end);

            value_addrs.insert({&v, pos++});
        }

        return value_addrs;
    }
};

template<typename FuncT, typename ValueT>
struct write_fixed_size_values_to_ostream
{
    value_addrs_type operator()(std::ostream& os, const std::deque<ValueT>& value_store) const
    {
        bin_value bv;
        value_addrs_type value_addrs;

        // Write the size of constant-size values.
        bv.ui32 = FuncT::value_size;
        os.write(bv.buffer, 4);

        size_t pos = 0;
        for (const ValueT& v : value_store)
        {
            auto sp_start = os.tellp();
            FuncT::write(os, v);
            auto sp_end = os.tellp();

            size_t bytes_written = sp_end - sp_start;
            if (bytes_written != FuncT::value_size)
            {
                std::ostringstream msg;
                msg << "bytes written (" << bytes_written << ") does not equal the value size (" << FuncT::value_size
                    << ")";
                throw size_error(msg.str());
            }

            value_addrs.insert({&v, pos++});
        }

        return value_addrs;
    }
};

template<typename FuncT, typename ValueT, typename SizeTrait>
struct write_values_to_ostream;

template<typename FuncT, typename ValueT>
struct write_values_to_ostream<FuncT, ValueT, std::true_type> : write_variable_size_values_to_ostream<FuncT, ValueT>
{
};

template<typename FuncT, typename ValueT>
struct write_values_to_ostream<FuncT, ValueT, std::false_type> : write_fixed_size_values_to_ostream<FuncT, ValueT>
{
};

template<typename FuncT, typename ValueT>
struct read_fixed_size_values_from_istream
{
    using value_store_type = std::deque<ValueT>;

    value_store_type operator()(std::istream& is, uint32_t value_count) const
    {
        value_store_type value_store;
        bin_value bv;

        // read the size of the value.
        is.read(bv.buffer, 4);
        size_t size = bv.ui32;

        if (size != FuncT::value_size)
        {
            std::ostringstream os;
            os << "wrong size of fixed value type (expected: " << FuncT::value_size << "; actual: " << size << ")";
            throw std::invalid_argument(os.str());
        }

        for (uint32_t i = 0; i < value_count; ++i)
        {
            value_store.emplace_back();
            FuncT::read(is, size, value_store.back());
        }

        return value_store;
    }
};

template<typename FuncT, typename ValueT>
struct read_variable_size_values_from_istream
{
    using value_store_type = std::deque<ValueT>;

    value_store_type operator()(std::istream& is, uint32_t value_count) const
    {
        value_store_type value_store;
        bin_value bv;

        for (uint32_t i = 0; i < value_count; ++i)
        {
            is.read(bv.buffer, 4);
            size_t size = bv.ui32;

            ValueT v;
            FuncT::read(is, size, v);

            value_store.push_back(std::move(v));
        }

        return value_store;
    }
};

template<typename FuncT, typename ValueT, typename SizeTrait>
struct read_values_from_istream;

template<typename FuncT, typename ValueT>
struct read_values_from_istream<FuncT, ValueT, std::true_type> : read_variable_size_values_from_istream<FuncT, ValueT>
{
};

template<typename FuncT, typename ValueT>
struct read_values_from_istream<FuncT, ValueT, std::false_type> : read_fixed_size_values_from_istream<FuncT, ValueT>
{
};

}} // namespace trie::detail

namespace trie {

template<typename T>
void numeric_value_serializer<T>::write(std::ostream& os, const T& v)
{
    static_assert(std::is_arithmetic<T>::value, "not a numeric type.");

    constexpr size_t s = sizeof(T);
    const char* p = reinterpret_cast<const char*>(&v);
    os.write(p, s);
}

template<typename T>
void numeric_value_serializer<T>::read(std::istream& is, size_t n, T& v)
{
    static_assert(std::is_arithmetic<T>::value, "not a numeric type.");

    constexpr size_t s = sizeof(T);
    assert(s == n);

    union
    {
        char buffer[s];
        T v;

    } buf;

    char* p = buf.buffer;

    while (n)
    {
        is.read(p, s);
        auto size_read = is.gcount();
        n -= size_read;
        p += size_read;
    }

    v = buf.v;
}

template<typename T>
void numeric_sequence_value_serializer<T>::write(std::ostream& os, const T& v)
{
    static_assert(
        std::is_arithmetic<typename T::value_type>::value, "value type of this vector is not a numeric type.");

    for (const auto& elem : v)
        element_serializer::write(os, elem);
}

template<typename T>
void numeric_sequence_value_serializer<T>::read(std::istream& is, size_t n, T& v)
{
    using elem_type = typename T::value_type;
    static_assert(
        std::is_arithmetic<typename T::value_type>::value, "value type of this vector is not a numeric type.");

    constexpr size_t elem_size = element_serializer::value_size;
    assert(n % elem_size == 0);

    size_t elem_count = n / elem_size;

    for (size_t i = 0; i < elem_count; ++i)
    {
        elem_type elem;
        element_serializer::read(is, elem_size, elem);
        v.push_back(elem);
    }
}

template<>
inline void variable_value_serializer<std::string>::write(std::ostream& os, const std::string& v)
{
    os.write(v.data(), v.size());
}

template<>
inline void variable_value_serializer<std::string>::read(std::istream& is, size_t n, std::string& v)
{
    v.resize(n);
    char* p = const_cast<char*>(v.data());

    while (n)
    {
        is.read(p, n);
        auto size_read = is.gcount();
        n -= size_read;
        p += size_read;
    }
}

} // namespace trie

template<typename KeyT, typename ValueT, typename TraitsT>
trie_map<KeyT, ValueT, TraitsT>::trie_map::trie_node::trie_node() : value(value_type()), has_value(false)
{}

template<typename KeyT, typename ValueT, typename TraitsT>
trie_map<KeyT, ValueT, TraitsT>::trie_map::trie_node::trie_node(const trie_node& other)
    : children(other.children), value(other.value), has_value(other.has_value)
{}

template<typename KeyT, typename ValueT, typename TraitsT>
trie_map<KeyT, ValueT, TraitsT>::trie_map::trie_node::trie_node(trie_node&& other)
    : children(std::move(other.children)), value(std::move(other.value)), has_value(std::move(other.has_value))
{}

template<typename KeyT, typename ValueT, typename TraitsT>
void trie_map<KeyT, ValueT, TraitsT>::trie_map::trie_node::swap(trie_node& other)
{
    children.swap(other.children);
    std::swap(value, other.value);
    std::swap(has_value, other.has_value);
}

template<typename KeyT, typename ValueT, typename TraitsT>
trie_map<KeyT, ValueT, TraitsT>::const_node_type::const_node_type(const trie_node* ref_node) : m_ref_node(ref_node)
{}

template<typename KeyT, typename ValueT, typename TraitsT>
trie_map<KeyT, ValueT, TraitsT>::const_node_type::const_node_type()
{}

template<typename KeyT, typename ValueT, typename TraitsT>
trie_map<KeyT, ValueT, TraitsT>::const_node_type::const_node_type(const const_node_type& other)
    : m_ref_node(other.m_ref_node)
{}

template<typename KeyT, typename ValueT, typename TraitsT>
auto trie_map<KeyT, ValueT, TraitsT>::const_node_type::operator=(const const_node_type& other) -> const_node_type&
{
    m_ref_node = other.m_ref_node;
    return *this;
}

template<typename KeyT, typename ValueT, typename TraitsT>
bool trie_map<KeyT, ValueT, TraitsT>::const_node_type::valid() const
{
    return m_ref_node != nullptr;
}

template<typename KeyT, typename ValueT, typename TraitsT>
bool trie_map<KeyT, ValueT, TraitsT>::const_node_type::has_child() const
{
    if (!m_ref_node)
        return false;

    return !m_ref_node->children.empty();
}

template<typename KeyT, typename ValueT, typename TraitsT>
bool trie_map<KeyT, ValueT, TraitsT>::const_node_type::has_value() const
{
    if (!m_ref_node)
        return false;

    return m_ref_node->has_value;
}

template<typename KeyT, typename ValueT, typename TraitsT>
auto trie_map<KeyT, ValueT, TraitsT>::const_node_type::value() const -> const value_type&
{
    return m_ref_node->value;
}

template<typename KeyT, typename ValueT, typename TraitsT>
auto trie_map<KeyT, ValueT, TraitsT>::const_node_type::child(key_unit_type c) const -> const_node_type
{
    if (!m_ref_node)
        return const_node_type();

    auto it = m_ref_node->children.find(c);
    if (it == m_ref_node->children.end())
        return const_node_type();

    return const_node_type(&it->second);
}

template<typename KeyT, typename ValueT, typename TraitsT>
trie_map<KeyT, ValueT, TraitsT>::trie_map()
{}

template<typename KeyT, typename ValueT, typename TraitsT>
trie_map<KeyT, ValueT, TraitsT>::trie_map(const trie_map& other) : m_root(other.m_root)
{}

template<typename KeyT, typename ValueT, typename TraitsT>
trie_map<KeyT, ValueT, TraitsT>::trie_map(trie_map&& other) : m_root(std::move(other.m_root))
{}

template<typename KeyT, typename ValueT, typename TraitsT>
typename trie_map<KeyT, ValueT, TraitsT>::const_iterator trie_map<KeyT, ValueT, TraitsT>::begin() const
{
    if (m_root.children.empty())
        // empty container
        return end();

    // Push the root node.
    key_type buf;
    const_node_stack_type node_stack;
    node_stack.emplace_back(&m_root, m_root.children.begin());

    // Push root's first child node.
    auto it = node_stack.back().child_pos;
    const_iterator::push_child_node_to_stack(node_stack, buf, it);

    // In theory there should always be at least one value node along the
    // left-most branch.

    while (!node_stack.back().node->has_value)
    {
        auto this_it = node_stack.back().child_pos;
        const_iterator::push_child_node_to_stack(node_stack, buf, this_it);
    }

    return const_iterator(std::move(node_stack), std::move(buf), trie::detail::iterator_type::normal);
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename trie_map<KeyT, ValueT, TraitsT>::iterator trie_map<KeyT, ValueT, TraitsT>::begin()
{
    if (m_root.children.empty())
        // empty container
        return end();

    // Push the root node.
    key_type buf;
    node_stack_type node_stack;
    node_stack.emplace_back(&m_root, m_root.children.begin());

    // Push root's first child node.
    auto it = node_stack.back().child_pos;
    iterator::push_child_node_to_stack(node_stack, buf, it);

    // In theory there should always be at least one value node along the
    // left-most branch.

    while (!node_stack.back().node->has_value)
    {
        auto this_it = node_stack.back().child_pos;
        iterator::push_child_node_to_stack(node_stack, buf, this_it);
    }

    return iterator(std::move(node_stack), std::move(buf), trie::detail::iterator_type::normal);
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename trie_map<KeyT, ValueT, TraitsT>::const_iterator trie_map<KeyT, ValueT, TraitsT>::end() const
{
    const_node_stack_type node_stack;
    node_stack.emplace_back(&m_root, m_root.children.end());
    return const_iterator(std::move(node_stack), key_type(), trie::detail::iterator_type::end);
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename trie_map<KeyT, ValueT, TraitsT>::iterator trie_map<KeyT, ValueT, TraitsT>::end()
{
    node_stack_type node_stack;
    node_stack.emplace_back(&m_root, m_root.children.end());
    return iterator(std::move(node_stack), key_type(), trie::detail::iterator_type::end);
}

template<typename KeyT, typename ValueT, typename TraitsT>
trie_map<KeyT, ValueT, TraitsT>& trie_map<KeyT, ValueT, TraitsT>::operator=(trie_map other)
{
    trie_map tmp(std::move(other));
    tmp.swap(*this);
    return *this;
}

template<typename KeyT, typename ValueT, typename TraitsT>
void trie_map<KeyT, ValueT, TraitsT>::swap(trie_map& other)
{
    m_root.swap(other.m_root);
}

template<typename KeyT, typename ValueT, typename TraitsT>
auto trie_map<KeyT, ValueT, TraitsT>::root_node() const -> const_node_type
{
    return const_node_type(&m_root);
}

template<typename KeyT, typename ValueT, typename TraitsT>
void trie_map<KeyT, ValueT, TraitsT>::insert(const key_type& key, const value_type& value)
{
    const key_unit_type* p = key.data();
    size_t n = key.size();
    const key_unit_type* p_end = p + n;
    insert_into_tree(m_root, p, p_end, value);
}

template<typename KeyT, typename ValueT, typename TraitsT>
void trie_map<KeyT, ValueT, TraitsT>::insert(const key_unit_type* key, size_type len, const value_type& value)
{
    const key_unit_type* key_end = key + len;
    insert_into_tree(m_root, key, key_end, value);
}

template<typename KeyT, typename ValueT, typename TraitsT>
bool trie_map<KeyT, ValueT, TraitsT>::erase(const key_unit_type* key, size_type len)
{
    const key_unit_type* key_end = key + len;

    const_node_stack_type node_stack;
    find_prefix_node_with_stack(node_stack, m_root, key, key_end);

    if (node_stack.empty() || !node_stack.back().node->has_value)
        // Nothing is erased.
        return false;

    const trie_node* node = node_stack.back().node;
    trie_node* node_mod = const_cast<trie_node*>(node);
    node_mod->has_value = false;

    // If this is a leaf node, remove it, and keep removing its parents until
    // we reach a parent node that still has at least one child node.

    while (!node->has_value && node->children.empty() && node_stack.size() > 1)
    {
        node_stack.pop_back();
        auto& si = node_stack.back();

        const_cast<trie_node*>(si.node)->children.erase(si.child_pos);
        node = si.node;
    }

    return true;
}

template<typename KeyT, typename ValueT, typename TraitsT>
void trie_map<KeyT, ValueT, TraitsT>::insert_into_tree(
    trie_node& node, const key_unit_type* key, const key_unit_type* key_end, const value_type& value)
{
    if (key == key_end)
    {
        node.value = value;
        node.has_value = true;
        return;
    }

    key_unit_type c = *key;

    auto it = node.children.lower_bound(c);
    if (it == node.children.end() || node.children.key_comp()(c, it->first))
    {
        // Insert a new node.
        it = node.children.insert(it, typename trie_node::children_type::value_type(c, trie_node()));
    }

    ++key;
    insert_into_tree(it->second, key, key_end, value);
}

template<typename KeyT, typename ValueT, typename TraitsT>
const typename trie_map<KeyT, ValueT, TraitsT>::trie_node* trie_map<KeyT, ValueT, TraitsT>::find_prefix_node(
    const trie_node& node, const key_unit_type* prefix, const key_unit_type* prefix_end) const
{
    if (prefix == prefix_end)
        // Right node is found.
        return &node;

    auto it = node.children.find(*prefix);
    if (it == node.children.end())
        return nullptr;

    ++prefix;
    return find_prefix_node(it->second, prefix, prefix_end);
}

template<typename KeyT, typename ValueT, typename TraitsT>
template<bool IsConst>
void trie_map<KeyT, ValueT, TraitsT>::find_prefix_node_with_stack(
    std::vector<stack_item<IsConst>>& node_stack, mdds::detail::const_t<trie_node, IsConst>& node,
    const key_unit_type* prefix, const key_unit_type* prefix_end) const
{
    if (prefix == prefix_end)
    {
        // Right node is found.
        node_stack.emplace_back(&node, node.children.begin());
        return;
    }

    auto it = node.children.find(*prefix);
    if (it == node.children.end())
        return;

    node_stack.emplace_back(&node, it);

    ++prefix;
    find_prefix_node_with_stack(node_stack, it->second, prefix, prefix_end);
}

template<typename KeyT, typename ValueT, typename TraitsT>
template<bool IsConst>
typename trie_map<KeyT, ValueT, TraitsT>::key_type trie_map<KeyT, ValueT, TraitsT>::build_key_buffer_from_node_stack(
    const std::vector<stack_item<IsConst>>& node_stack) const
{
    // Build the key value from the stack.
    key_type buf;
    auto end = node_stack.end();
    --end; // Skip the node with value which doesn't store a key element.
    std::for_each(node_stack.begin(), end, [&](const stack_item<IsConst>& si) { buf.push_back(si.child_pos->first); });

    return buf;
}

template<typename KeyT, typename ValueT, typename TraitsT>
void trie_map<KeyT, ValueT, TraitsT>::count_values(size_type& n, const trie_node& node) const
{
    if (node.has_value)
        ++n;

    std::for_each(
        node.children.begin(), node.children.end(),
        [&](const typename trie_node::children_type::value_type& v) { count_values(n, v.second); });
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename trie_map<KeyT, ValueT, TraitsT>::const_iterator trie_map<KeyT, ValueT, TraitsT>::find(
    const key_type& key) const
{
    return find(key.data(), key.size());
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename trie_map<KeyT, ValueT, TraitsT>::const_iterator trie_map<KeyT, ValueT, TraitsT>::find(
    const key_unit_type* input, size_type len) const
{
    const key_unit_type* input_end = input + len;
    const_node_stack_type node_stack;
    find_prefix_node_with_stack(node_stack, m_root, input, input_end);
    if (node_stack.empty() || !node_stack.back().node->has_value)
        // Specified key doesn't exist.
        return end();

    key_type buf = build_key_buffer_from_node_stack(node_stack);

    return const_iterator(std::move(node_stack), std::move(buf), trie::detail::iterator_type::normal);
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename trie_map<KeyT, ValueT, TraitsT>::iterator trie_map<KeyT, ValueT, TraitsT>::find(const key_type& key)
{
    return find(key.data(), key.size());
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename trie_map<KeyT, ValueT, TraitsT>::iterator trie_map<KeyT, ValueT, TraitsT>::find(
    const key_unit_type* input, size_type len)
{
    const key_unit_type* input_end = input + len;
    node_stack_type node_stack;
    find_prefix_node_with_stack(node_stack, m_root, input, input_end);
    if (node_stack.empty() || !node_stack.back().node->has_value)
        // Specified key doesn't exist.
        return end();

    key_type buf = build_key_buffer_from_node_stack(node_stack);

    return iterator(std::move(node_stack), std::move(buf), trie::detail::iterator_type::normal);
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename trie_map<KeyT, ValueT, TraitsT>::search_results trie_map<KeyT, ValueT, TraitsT>::prefix_search(
    const key_type& key) const
{
    return prefix_search(key.data(), key.size());
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename trie_map<KeyT, ValueT, TraitsT>::search_results trie_map<KeyT, ValueT, TraitsT>::prefix_search(
    const key_unit_type* prefix, size_type len) const
{
    const key_unit_type* prefix_end = prefix + len;
    std::vector<key_value_type> matches;

    const trie_node* node = find_prefix_node(m_root, prefix, prefix_end);
    return search_results(node, key_type(prefix, len));
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename trie_map<KeyT, ValueT, TraitsT>::size_type trie_map<KeyT, ValueT, TraitsT>::size() const
{
    size_type n = 0;
    count_values(n, m_root);
    return n;
}

template<typename KeyT, typename ValueT, typename TraitsT>
bool trie_map<KeyT, ValueT, TraitsT>::empty() const noexcept
{
    return m_root.children.empty() && !m_root.has_value;
}

template<typename KeyT, typename ValueT, typename TraitsT>
void trie_map<KeyT, ValueT, TraitsT>::clear()
{
    m_root.children.clear();
    m_root.has_value = false;
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename trie_map<KeyT, ValueT, TraitsT>::packed_type trie_map<KeyT, ValueT, TraitsT>::pack() const
{
    return packed_type(*this);
}

template<typename KeyT, typename ValueT, typename TraitsT>
void packed_trie_map<KeyT, ValueT, TraitsT>::traverse_range(
    trie_node& root, node_pool_type& node_pool, const typename packed_trie_map<KeyT, ValueT, TraitsT>::entry* start,
    const typename packed_trie_map<KeyT, ValueT, TraitsT>::entry* end, size_type pos)
{
    const entry* p = start;
    const entry* range_start = start;
    const entry* range_end = nullptr;
    key_unit_type range_char = 0;
    size_type range_count = 0;

    for (; p != end; ++p)
    {
        if (pos > p->keylen)
            continue;

        if (pos == p->keylen)
        {
            root.value = &p->value;
            continue;
        }

        ++range_count;
        key_unit_type c = p->key[pos];

        if (!range_char)
            range_char = c;
        else if (range_char != c)
        {
            // End of current character range.
            range_end = p;

            node_pool.emplace_back(range_char);
            root.children.push_back(&node_pool.back());
            traverse_range(*root.children.back(), node_pool, range_start, range_end, pos + 1);
            range_start = range_end;
            range_char = range_start->key[pos];
            range_end = nullptr;
            range_count = 1;
        }
    }

    if (range_count)
    {
        assert(range_char);
        node_pool.emplace_back(range_char);
        root.children.push_back(&node_pool.back());
        traverse_range(*root.children.back(), node_pool, range_start, end, pos + 1);
    }
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename packed_trie_map<KeyT, ValueT, TraitsT>::size_type packed_trie_map<KeyT, ValueT, TraitsT>::compact_node(
    const trie_node& node)
{
    std::vector<std::tuple<size_t, key_unit_type>> child_offsets;
    child_offsets.reserve(node.children.size());

    // Process child nodes first.
    std::for_each(node.children.begin(), node.children.end(), [&](const trie_node* p) {
        const trie_node& child_node = *p;
        size_type child_offset = compact_node(child_node);
        child_offsets.emplace_back(child_offset, child_node.key);
    });

    // Process this node.
    size_type offset = m_packed.size();
    if (node.value)
    {
        m_value_store.push_back(*node.value); // copy the value object.
        m_packed.push_back(uintptr_t(&m_value_store.back()));
    }
    else
        m_packed.push_back(uintptr_t(0));

    push_child_offsets(offset, child_offsets);
    return offset;
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename packed_trie_map<KeyT, ValueT, TraitsT>::size_type packed_trie_map<KeyT, ValueT, TraitsT>::compact_node(
    const typename trie_map<KeyT, ValueT, TraitsT>::trie_node& node)
{
    using node_type = typename trie_map<KeyT, ValueT, TraitsT>::trie_node;

    std::vector<std::tuple<size_t, key_unit_type>> child_offsets;
    child_offsets.reserve(node.children.size());

    // Process child nodes first.
    std::for_each(
        node.children.begin(), node.children.end(), [&](const typename node_type::children_type::value_type& v) {
            key_unit_type key = v.first;
            const node_type& child_node = v.second;
            size_type child_offset = compact_node(child_node);
            child_offsets.emplace_back(child_offset, key);
        });

    // Process this node.
    size_type offset = m_packed.size();
    if (node.has_value)
    {
        m_value_store.push_back(node.value); // copy the value object.
        m_packed.push_back(uintptr_t(&m_value_store.back()));
    }
    else
        m_packed.push_back(uintptr_t(0));

    push_child_offsets(offset, child_offsets);
    return offset;
}

template<typename KeyT, typename ValueT, typename TraitsT>
void packed_trie_map<KeyT, ValueT, TraitsT>::push_child_offsets(
    size_type offset, const child_offsets_type& child_offsets)
{
    m_packed.push_back(uintptr_t(child_offsets.size() * 2));

    std::for_each(child_offsets.begin(), child_offsets.end(), [&](const std::tuple<size_t, key_unit_type>& v) {
        key_unit_type key = std::get<1>(v);
        size_t child_offset = std::get<0>(v);
        m_packed.push_back(key);
        m_packed.push_back(offset - child_offset);
    });
}

template<typename KeyT, typename ValueT, typename TraitsT>
void packed_trie_map<KeyT, ValueT, TraitsT>::compact(const trie_node& root)
{
    packed_type init(size_t(1), uintptr_t(0));
    m_packed.swap(init);
    assert(m_packed.size() == 1);

    size_t root_offset = compact_node(root);
    m_packed[0] = root_offset;
}

template<typename KeyT, typename ValueT, typename TraitsT>
void packed_trie_map<KeyT, ValueT, TraitsT>::compact(const typename trie_map<KeyT, ValueT, TraitsT>::trie_node& root)
{
    packed_type init(size_t(1), uintptr_t(0));
    m_packed.swap(init);
    assert(m_packed.size() == 1);

    size_t root_offset = compact_node(root);
    m_packed[0] = root_offset;
}

template<typename KeyT, typename ValueT, typename TraitsT>
packed_trie_map<KeyT, ValueT, TraitsT>::const_node_type::const_node_type(
    const packed_type* packed, const value_store_type* value_store, const uintptr_t* p)
    : m_packed(packed), m_value_store(value_store), m_pos(p)
{}

template<typename KeyT, typename ValueT, typename TraitsT>
auto packed_trie_map<KeyT, ValueT, TraitsT>::const_node_type::operator=(const const_node_type& other)
    -> const_node_type&
{
    m_packed = other.m_packed;
    m_value_store = other.m_value_store;
    m_pos = other.m_pos;
    return *this;
}

template<typename KeyT, typename ValueT, typename TraitsT>
bool packed_trie_map<KeyT, ValueT, TraitsT>::const_node_type::valid() const
{
    return m_pos != nullptr;
}

template<typename KeyT, typename ValueT, typename TraitsT>
bool packed_trie_map<KeyT, ValueT, TraitsT>::const_node_type::has_child() const
{
    if (!m_pos)
        return false;

#ifdef MDDS_TRIE_MAP_DEBUG
    trie::detail::verify_packed_position(*m_packed, m_pos);
#endif

    size_type index_size = *(m_pos + 1);
    return index_size > 0u;
}

template<typename KeyT, typename ValueT, typename TraitsT>
bool packed_trie_map<KeyT, ValueT, TraitsT>::const_node_type::has_value() const
{
    if (!m_pos)
        return false;

#ifdef MDDS_TRIE_MAP_DEBUG
    trie::detail::verify_packed_position(*m_packed, m_pos);
#endif

    const auto* pv = reinterpret_cast<const value_type*>(*m_pos);
    return pv != nullptr;
}

template<typename KeyT, typename ValueT, typename TraitsT>
auto packed_trie_map<KeyT, ValueT, TraitsT>::const_node_type::value() const -> const value_type&
{
#ifdef MDDS_TRIE_MAP_DEBUG
    trie::detail::verify_packed_position(*m_packed, m_pos);
#endif

    const auto* pv = reinterpret_cast<const value_type*>(*m_pos);

#ifdef MDDS_TRIE_MAP_DEBUG
    trie::detail::verify_packed_value(*m_value_store, pv);
#endif

    return *pv;
}

template<typename KeyT, typename ValueT, typename TraitsT>
auto packed_trie_map<KeyT, ValueT, TraitsT>::const_node_type::child(key_unit_type c) const -> const_node_type
{
    if (!m_pos)
        return const_node_type();

#ifdef MDDS_TRIE_MAP_DEBUG
    trie::detail::verify_packed_position(*m_packed, m_pos);
#endif

    size_type index_size = *(m_pos + 1);
    if (!index_size)
        // no more child nodes
        return const_node_type();

    size_type n = index_size / 2;
    const uintptr_t* p = m_pos + 2;

    if (const uintptr_t* child_pos = trie::detail::find_child_pos(p, n, c); child_pos)
    {
        // Match found!
        size_type offset = *(child_pos + 1);
        const uintptr_t* p_child = m_pos - offset;
        return const_node_type(m_packed, m_value_store, p_child);
    }

    return const_node_type();
}

template<typename KeyT, typename ValueT, typename TraitsT>
packed_trie_map<KeyT, ValueT, TraitsT>::packed_trie_map() : m_packed(3, 0u)
{
    // root offset (0), root value (1), and root child count (2).
    m_packed[0] = 1;
}

template<typename KeyT, typename ValueT, typename TraitsT>
packed_trie_map<KeyT, ValueT, TraitsT>::packed_trie_map(const entry* entries, size_type entry_size)
{
    const entry* p = entries;
    const entry* p_end = p + entry_size;

#if defined(MDDS_TRIE_MAP_DEBUG)
    // Make sure the entries really are sorted.
    auto func_compare = [](const entry& left, const entry& right) -> bool {
        size_type n_key = std::min(left.keylen, right.keylen);
        int ret = std::memcmp(left.key, right.key, n_key);
        if (ret == 0)
            return left.keylen < right.keylen;

        return ret < 0;
    };

    if (!std::is_sorted(p, p_end, func_compare))
        throw integrity_error("the list of entries is not sorted.");

#endif

    // Populate the normal tree first.
    trie_node root(0);
    node_pool_type node_pool;
    traverse_range(root, node_pool, p, p_end, 0);

    // Compact the trie into a packed array.
    compact(root);

    trie::detail::dump_packed_trie_node_tree<packed_trie_map, trie_node>{}(std::cout, root);

#if defined(MDDS_TRIE_MAP_DEBUG) && defined(MDDS_TRIE_MAP_DEBUG_DUMP_PACKED)
    trie::detail::dump_packed_buffer<packed_trie_map, packed_type>{}(std::cout, m_packed);
#endif
}

template<typename KeyT, typename ValueT, typename TraitsT>
packed_trie_map<KeyT, ValueT, TraitsT>::packed_trie_map(const trie_map<key_type, value_type>& other)
{
    compact(other.m_root);
}

template<typename KeyT, typename ValueT, typename TraitsT>
packed_trie_map<KeyT, ValueT, TraitsT>::packed_trie_map(const packed_trie_map& other) : m_packed(other.m_packed)
{
    struct _handler
    {
        packed_trie_map& m_parent;

        void node(const uintptr_t* node_pos, key_unit_type /*c*/, size_t /*depth*/, size_t /*index_size*/)
        {
            uintptr_t value_ptr = *node_pos;

            if (value_ptr)
            {
                auto p = reinterpret_cast<const value_type*>(value_ptr);
                m_parent.m_value_store.push_back(*p); // copy the value object.
                const uintptr_t* head = m_parent.m_packed.data();
                size_t offset = std::distance(head, node_pos);
                m_parent.m_packed[offset] = uintptr_t(&m_parent.m_value_store.back());
            }
        }

        void move_up(const uintptr_t*, const uintptr_t*, const uintptr_t*)
        {}

        void next_child()
        {}

        void end()
        {}

        _handler(packed_trie_map& parent) : m_parent(parent)
        {}

    } handler(*this);

    traverse_tree(handler);
}

template<typename KeyT, typename ValueT, typename TraitsT>
packed_trie_map<KeyT, ValueT, TraitsT>::packed_trie_map(packed_trie_map&& other)
    : m_value_store(std::move(other.m_value_store)), m_packed(std::move(other.m_packed))
{
    // Even the empty structure needs to have the root offset and the empty root record.
    other.m_packed.resize(3, 0u); // root offset (0), root value (1), and root child count (2).
    other.m_packed[0] = 1;
}

template<typename KeyT, typename ValueT, typename TraitsT>
packed_trie_map<KeyT, ValueT, TraitsT>& packed_trie_map<KeyT, ValueT, TraitsT>::operator=(packed_trie_map other)
{
    packed_trie_map tmp(std::move(other));
    tmp.swap(*this);
    return *this;
}

template<typename KeyT, typename ValueT, typename TraitsT>
bool packed_trie_map<KeyT, ValueT, TraitsT>::operator==(const packed_trie_map& other) const
{
    if (m_value_store.size() != other.m_value_store.size())
        return false;

    // Since the two containers are of the same size, the iterator ranges should
    // be the same as well.
    auto left = cbegin(), right = other.cbegin();
    for (; left != cend(); ++left, ++right)
    {
        assert(right != other.cend());
        if (*left != *right)
            return false;
    }

    return true;
}

template<typename KeyT, typename ValueT, typename TraitsT>
bool packed_trie_map<KeyT, ValueT, TraitsT>::operator!=(const packed_trie_map& other) const
{
    return !operator==(other);
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename packed_trie_map<KeyT, ValueT, TraitsT>::const_iterator packed_trie_map<KeyT, ValueT, TraitsT>::begin() const
{
    return cbegin();
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename packed_trie_map<KeyT, ValueT, TraitsT>::const_iterator packed_trie_map<KeyT, ValueT, TraitsT>::end() const
{
    return cend();
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename packed_trie_map<KeyT, ValueT, TraitsT>::const_iterator packed_trie_map<KeyT, ValueT, TraitsT>::cbegin() const
{
    node_stack_type node_stack = get_root_stack();

    const stack_item* si = &node_stack.back();
    if (si->child_pos == si->child_end)
        // empty container.
        return const_iterator(std::move(node_stack), key_type());

    const uintptr_t* node_pos = si->node_pos;
    const uintptr_t* child_pos = si->child_pos;
    const uintptr_t* child_end = si->child_end;
    const uintptr_t* p = child_pos;

    // Follow the root node's left-most child.
    key_type buf;
    key_unit_type c = *p;
    buf.push_back(c);
    ++p;
    size_t offset = *p;

    node_pos -= offset; // jump to the child node.
    p = node_pos;
    ++p;
    size_t index_size = *p;
    ++p;
    child_pos = p;
    child_end = child_pos + index_size;

    // Push this child node onto the stack.
    node_stack.emplace_back(node_pos, child_pos, child_end);

    const value_type* pv = reinterpret_cast<const value_type*>(*node_pos);
    while (!pv)
    {
        // Keep following the left child node until we reach a node with value.
        const_iterator::push_child_node_to_stack(node_stack, buf, node_stack.back().child_pos);
        pv = reinterpret_cast<const value_type*>(*node_stack.back().node_pos);
    }

    return const_iterator(std::move(node_stack), std::move(buf), *pv);
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename packed_trie_map<KeyT, ValueT, TraitsT>::const_iterator packed_trie_map<KeyT, ValueT, TraitsT>::cend() const
{
    node_stack_type node_stack = get_root_stack();
    node_stack.back().child_pos = node_stack.back().child_end;
    return const_iterator(std::move(node_stack), key_type());
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename packed_trie_map<KeyT, ValueT, TraitsT>::node_stack_type packed_trie_map<
    KeyT, ValueT, TraitsT>::get_root_stack() const
{
    assert(!m_packed.empty());
    size_t root_offset = m_packed[0];
    assert(root_offset < m_packed.size());
    const uintptr_t* p = m_packed.data() + root_offset;
    const uintptr_t* node_pos = p;
    ++p;
    size_t index_size = *p;
    ++p;
    const uintptr_t* child_pos = p;
    const uintptr_t* child_end = child_pos + index_size;

    node_stack_type node_stack;
    node_stack.emplace_back(node_pos, child_pos, child_end);

    return node_stack;
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename packed_trie_map<KeyT, ValueT, TraitsT>::const_iterator packed_trie_map<KeyT, ValueT, TraitsT>::find(
    const key_type& key) const
{
    return find(key.data(), key.size());
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename packed_trie_map<KeyT, ValueT, TraitsT>::const_iterator packed_trie_map<KeyT, ValueT, TraitsT>::find(
    const key_unit_type* input, size_type len) const
{
    if (m_value_store.empty())
        return end();

    const key_unit_type* key_end = input + len;
    size_t root_offset = m_packed[0];
    assert(root_offset < m_packed.size());
    const uintptr_t* root = m_packed.data() + root_offset;

    node_stack_type node_stack;

    trie::detail::find_prefix_node<key_unit_type, size_type>(
        root, input, key_end,
        [&node_stack](const uintptr_t* node_pos, const uintptr_t* child_pos, const uintptr_t* child_end) {
            node_stack.emplace_back(node_pos, child_pos, child_end);
        });

    if (node_stack.empty() || !node_stack.back().node_pos)
        return end();

    const stack_item& si = node_stack.back();
    const value_type* pv = reinterpret_cast<const value_type*>(*si.node_pos);
    if (!pv)
        return end();

    // Build the key value from the stack.
    key_type buf;
    auto end = node_stack.end();
    --end; // Skip the node with value which doesn't store a key element.
    std::for_each(node_stack.begin(), end, [&](const stack_item& this_si) { buf.push_back(*this_si.child_pos); });

    return const_iterator(std::move(node_stack), std::move(buf), *pv);
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename packed_trie_map<KeyT, ValueT, TraitsT>::search_results packed_trie_map<KeyT, ValueT, TraitsT>::prefix_search(
    const key_type& key) const
{
    return prefix_search(key.data(), key.size());
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename packed_trie_map<KeyT, ValueT, TraitsT>::search_results packed_trie_map<KeyT, ValueT, TraitsT>::prefix_search(
    const key_unit_type* prefix, size_type len) const
{
    if (m_value_store.empty())
        return search_results(nullptr, key_type());

    const key_unit_type* prefix_end = prefix + len;

    size_t root_offset = m_packed[0];
    assert(root_offset < m_packed.size());
    const uintptr_t* root = m_packed.data() + root_offset;

    const uintptr_t* node = trie::detail::find_prefix_node<key_unit_type, size_type>(
        root, prefix, prefix_end, [](const uintptr_t*, const uintptr_t*, const uintptr_t*) {});

    return search_results(node, key_type(prefix, len));
}

template<typename KeyT, typename ValueT, typename TraitsT>
typename packed_trie_map<KeyT, ValueT, TraitsT>::size_type packed_trie_map<KeyT, ValueT, TraitsT>::size() const noexcept
{
    return m_value_store.size();
}

template<typename KeyT, typename ValueT, typename TraitsT>
bool packed_trie_map<KeyT, ValueT, TraitsT>::empty() const noexcept
{
    return m_value_store.empty();
}

template<typename KeyT, typename ValueT, typename TraitsT>
void packed_trie_map<KeyT, ValueT, TraitsT>::swap(packed_trie_map& other)
{
    m_value_store.swap(other.m_value_store);
    m_packed.swap(other.m_packed);
}

template<typename KeyT, typename ValueT, typename TraitsT>
auto packed_trie_map<KeyT, ValueT, TraitsT>::root_node() const -> const_node_type
{
    assert(m_packed.size() >= 3u);
    std::size_t root_offset = m_packed[0];
    return const_node_type(&m_packed, &m_value_store, m_packed.data() + root_offset);
}

template<typename KeyT, typename ValueT, typename TraitsT>
template<typename FuncT>
void packed_trie_map<KeyT, ValueT, TraitsT>::save_state(std::ostream& os) const
{
    trie::detail::bin_value bv;

    bv.ui16 = 0x0000; // write 2-byte flags
    bv.ui16 |= (0x0001 & FuncT::variable_size);
    os.write(bv.buffer, 2);

    // Write the number of values (4-bytes).
    bv.ui32 = m_value_store.size();
    os.write(bv.buffer, 4);

    using value_addrs_type = std::map<const void*, size_t>;
    value_addrs_type value_addrs;

    // Dump the stored values first.
    using value_size_type = std::bool_constant<FuncT::variable_size>;

    trie::detail::write_values_to_ostream<FuncT, value_type, value_size_type> func;
    value_addrs = func(os, m_value_store);

    // Write 0xFF to signify the end of the value section.
    bv.ui8 = 0xFF;
    os.write(bv.buffer, 1);

    // Write the size of uintptr_t.  It must be either 4 or 8.
    bv.ui8 = sizeof(uintptr_t);
    os.write(bv.buffer, 1);

    // Write the size of the packed blob.
    bv.ui64 = m_packed.size();
    os.write(bv.buffer, 8);

    struct _handler
    {
        size_t const m_elem_size = sizeof(uintptr_t);
        std::ostream& m_os;
        const value_addrs_type& m_value_addrs;
        const packed_trie_map& m_parent;

        inline void write(uintptr_t v) const
        {
            const char* p = reinterpret_cast<const char*>(&v);
            m_os.write(p, m_elem_size);
        }

        /** first element in the buffer. */
        void root_offset(size_t /*i*/, const uintptr_t& v) const
        {
            write(v);
        }

        /** first element in each node section. */
        void node_value(size_t /*i*/, const uintptr_t& v) const
        {
            const value_type* p = reinterpret_cast<const value_type*>(v);
            if (p)
            {
                // Replace the pointer of the value with its index into the value store.
                auto it = m_value_addrs.find(p);
                assert(it != m_value_addrs.cend());
                uintptr_t index = it->second;

                write(index);
            }
            else
            {
                // Use the numeric value with all bits set, to encode 0.
                // Because the index of the value is 0-based, we can't just
                // leave it as zero.
                uintptr_t max_bits = 0;
                max_bits = ~max_bits;
                write(max_bits);
            }
        }

        /**
         * second element in each node section that stores the size of
         * the child data sub-section.
         */
        void node_index_size(size_t /*i*/, const uintptr_t& v) const
        {
            write(v);
        }

        /** element that stores the key value for child node. */
        void node_child_key(size_t /*i*/, const uintptr_t& v) const
        {
            write(v);
        }

        /** element that stores the relative offset of the child node. */
        void node_child_offset(size_t /*i*/, const uintptr_t& v) const
        {
            write(v);
        }

        _handler(std::ostream& os, const value_addrs_type& value_addrs, const packed_trie_map& parent)
            : m_os(os), m_value_addrs(value_addrs), m_parent(parent)
        {}

    } handler(os, value_addrs, *this);

    trie::detail::traverse_packed_buffer<packed_type, _handler>{}(m_packed, handler);

    // Write 0xFF to signify the end of the packed blob.
    bv.ui8 = 0xFF;
    os.write(bv.buffer, 1);
}

template<typename KeyT, typename ValueT, typename TraitsT>
template<typename FuncT>
void packed_trie_map<KeyT, ValueT, TraitsT>::load_state(std::istream& is)
{
    trie::detail::bin_value bv;
    is.read(bv.buffer, 2);

    uint16_t flags = bv.ui16;
    bool variable_size = (flags & 0x0001) != 0;

    if (variable_size != FuncT::variable_size)
    {
        std::ostringstream os;
        os << "This stream is meant for a value type of " << trie::detail::value_type_size_name(variable_size)
           << ", but the actual value type is of " << trie::detail::value_type_size_name(FuncT::variable_size) << ".";
        throw std::invalid_argument(os.str());
    }

    // read the number of values
    is.read(bv.buffer, 4);
    uint32_t value_count = bv.ui32;

    using value_size_type = std::bool_constant<FuncT::variable_size>;
    trie::detail::read_values_from_istream<FuncT, value_type, value_size_type> func;
    m_value_store = func(is, value_count);

    // There should be a check byte of 0xFF.
    is.read(bv.buffer, 1);
    if (bv.ui8 != 0xFF)
        throw std::invalid_argument("failed to find the check byte at the end of the value section.");

    // Size of uintptr_t
    is.read(bv.buffer, 1);
    size_t ptr_size = bv.ui8;

    if (ptr_size != sizeof(uintptr_t))
        throw std::invalid_argument("size of uintptr_t is different.");

    // Size of the packed blob.
    is.read(bv.buffer, 8);

    size_t n = bv.ui64;
    packed_type packed;
    packed.reserve(n);

    for (size_t i = 0; i < n; ++i)
    {
        is.read(bv.buffer, sizeof(uintptr_t));
        const uintptr_t* p = reinterpret_cast<const uintptr_t*>(bv.buffer);
        packed.push_back(*p);
    }

    // the last byte must be 0xFF.
    is.read(bv.buffer, 1);
    if (bv.ui8 != 0xFF)
        throw std::invalid_argument("failed to find the check byte at the end of the packed blob section.");

    m_packed.swap(packed);

    struct _handler
    {
        uintptr_t m_max_value;
        packed_trie_map& m_parent;

        void root_offset(size_t /*i*/, const uintptr_t& /*v*/) const
        {}

        void node_value(size_t i, const uintptr_t& v) const
        {
            if (v == m_max_value)
                m_parent.m_packed[i] = 0;
            else
            {
                // Replace the value index with its memory address.
                const value_type& val = m_parent.m_value_store[v];
                const uintptr_t addr = reinterpret_cast<const uintptr_t>(&val);
                m_parent.m_packed[i] = addr;
            }
        }

        void node_index_size(size_t /*i*/, const uintptr_t& /*v*/) const
        {}
        void node_child_key(size_t /*i*/, const uintptr_t& /*v*/) const
        {}
        void node_child_offset(size_t /*i*/, const uintptr_t& /*v*/) const
        {}

        _handler(packed_trie_map& parent) : m_max_value(0), m_parent(parent)
        {
            m_max_value = ~m_max_value;
        }

    } handler(*this);

    trie::detail::traverse_packed_buffer<packed_type, _handler>{}(m_packed, handler);
}

template<typename KeyT, typename ValueT, typename TraitsT>
void packed_trie_map<KeyT, ValueT, TraitsT>::dump_structure() const
{
#ifdef MDDS_TRIE_MAP_DEBUG
    trie::detail::dump_packed_buffer<packed_trie_map, packed_type>{}(std::cout, m_packed);

    cout << "--" << endl;
    cout << "entry size: " << m_value_store.size() << endl;
    cout << "memory size: " << m_packed.size() << endl;

    const uintptr_t* head = m_packed.data();

    struct _handler
    {
        const uintptr_t* m_head;

        _handler(const uintptr_t* head) : m_head(head)
        {}

        void node(const uintptr_t* node_pos, key_unit_type c, size_t depth, size_t index_size)
        {
            auto value_ptr = reinterpret_cast<const value_type*>(*node_pos);
            size_t offset = std::distance(m_head, node_pos);

            cout << "  --" << endl;
            cout << "  offset: " << offset << endl;
            cout << "  key: " << c << endl;
            cout << "  depth: " << depth << endl;
            cout << "  child count: " << (index_size / 2) << endl;
            cout << "  value address: " << value_ptr;

            if (value_ptr)
            {
                cout << "; value: " << *value_ptr;
            }

            cout << endl;
        }

        void move_up(const uintptr_t* node_pos, const uintptr_t* child_pos, const uintptr_t* child_end)
        {
            size_t offset = std::distance(m_head, node_pos);
            size_t child_size = std::distance(child_pos, child_end) / 2;
            cout << "  --" << endl;
            cout << "  move up: (offset: " << offset << "; child count: " << child_size << ")" << endl;
        }

        void next_child()
        {
            cout << "  next child" << endl;
        }

        void end()
        {}

    } handler(head);

    traverse_tree(handler);
#endif
}

template<typename KeyT, typename ValueT, typename TraitsT>
template<typename _Handler>
void packed_trie_map<KeyT, ValueT, TraitsT>::traverse_tree(_Handler hdl) const
{
    node_stack_type node_stack = get_root_stack();
    stack_item* si = &node_stack.back();
    if (si->child_pos == si->child_end)
        // empty container
        return;

    const uintptr_t* node_pos = si->node_pos;
    const uintptr_t* child_pos = si->child_pos;
    const uintptr_t* child_end = si->child_end;
    const uintptr_t* p = child_pos;

    for (bool in_tree = true; in_tree;)
    {
        // Descend until the leaf node is reached by following the left-most child nodes.
        while (true)
        {
            auto key = *p;
            size_t depth = node_stack.size();
            ++p;
            size_t offset = *p;

            // jump down to the child node.
            node_pos -= offset;
            p = node_pos;
            ++p;
            size_t index_size = *p; // size of the buffer that stores child nodes' keys and offsets.

            hdl.node(node_pos, key, depth, index_size);

            ++p;
            child_pos = p;
            child_end = child_pos + index_size;

            // Push this child node onto the stack.
            node_stack.emplace_back(node_pos, child_pos, child_end);

            if (!index_size)
                // no child nodes i.e. leaf node.  Bail out of the loop.
                break;
        }

        // Ascend up the tree until a node with an unvisited child node is
        // found, then move sideways.
        while (true)
        {
            // move up.
            node_stack.pop_back();
            si = &node_stack.back();
            hdl.move_up(si->node_pos, si->child_pos, si->child_end);
            std::advance(si->child_pos, 2); // move to the next child node slot.
            if (si->child_pos != si->child_end)
            {
                // This is an unvisited child node. Bail out of the loop.
                hdl.next_child();
                node_pos = si->node_pos;
                p = si->child_pos;
                break;
            }

            if (node_stack.size() == 1)
            {
                // End of the tree has reached.
                in_tree = false;
                break;
            }
        }
    }

    hdl.end();
}

} // namespace mdds

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
