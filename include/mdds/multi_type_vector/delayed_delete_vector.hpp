/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <algorithm>
#include <type_traits>

namespace mdds { namespace mtv {

/**
 * Vector that delays deleting from the front of the vector, which avoids
 * O(n^2) memory move operations when code needs to delete items from one
 * element block and add to another element block.
 */
template<typename T, typename Allocator = std::allocator<T>>
class delayed_delete_vector
{
    using store_type = std::vector<T, Allocator>;
    store_type m_vec;
    std::size_t m_front_offset = 0; // number of elements removed from front of array
public:
    typedef typename store_type::value_type value_type;
    typedef typename store_type::size_type size_type;
    typedef typename store_type::difference_type difference_type;
    typedef typename store_type::reference reference;
    typedef typename store_type::const_reference const_reference;
    typedef typename store_type::pointer pointer;
    typedef typename store_type::const_pointer const_pointer;
    typedef typename store_type::iterator iterator;
    typedef typename store_type::reverse_iterator reverse_iterator;
    typedef typename store_type::const_iterator const_iterator;
    typedef typename store_type::const_reverse_iterator const_reverse_iterator;

    delayed_delete_vector() noexcept(std::is_nothrow_default_constructible_v<store_type>) : m_vec()
    {}

    delayed_delete_vector(size_t n, const T& val) : m_vec(n, val)
    {}

    delayed_delete_vector(size_t n) : m_vec(n)
    {}

    template<typename InputIt>
    delayed_delete_vector(InputIt first, InputIt last) : m_vec(first, last)
    {}

    iterator begin() noexcept
    {
        return m_vec.begin() + m_front_offset;
    }

    iterator end() noexcept
    {
        return m_vec.end();
    }

    const_iterator begin() const noexcept
    {
        return m_vec.begin() + m_front_offset;
    }

    const_iterator end() const noexcept
    {
        return m_vec.end();
    }

    reverse_iterator rbegin() noexcept
    {
        return m_vec.rbegin();
    }

    const_reverse_iterator rbegin() const noexcept
    {
        return m_vec.rbegin();
    }

    reverse_iterator rend() noexcept
    {
        return m_vec.rend() - m_front_offset;
    }

    const_reverse_iterator rend() const noexcept
    {
        return m_vec.rend() - m_front_offset;
    }

    reference operator[](size_type pos)
    {
        return m_vec[pos + m_front_offset];
    }

    const_reference operator[](size_type pos) const
    {
        return m_vec[pos + m_front_offset];
    }

    reference at(size_type pos)
    {
        return m_vec.at(pos + m_front_offset);
    }

    const_reference at(size_type pos) const
    {
        return m_vec.at(pos + m_front_offset);
    }

    void push_back(const T& value)
    {
        m_vec.push_back(value);
    }

    void push_back(T&& value)
    {
        m_vec.push_back(std::move(value));
    }

    void swap(delayed_delete_vector& other) noexcept(std::is_nothrow_swappable_v<store_type>)
    {
        m_vec.swap(other.m_vec);
    }

    template<typename... Args>
    void emplace_back(Args&&... args)
    {
        m_vec.emplace_back(std::forward<Args>(args)...);
    }

    iterator insert(iterator pos, const T& value)
    {
        return m_vec.insert(pos, value);
    }

    iterator insert(const_iterator pos, T&& value)
    {
        return m_vec.insert(pos, std::move(value));
    }

    template<typename InputIt>
    void insert(iterator pos, InputIt first, InputIt last)
    {
        m_vec.insert(pos, first, last);
    }

    void resize(size_type count)
    {
        clear_removed();
        m_vec.resize(count);
    }

    iterator erase(iterator pos)
    {
        if (pos == m_vec.begin() + m_front_offset)
        {
            ++m_front_offset;
            return m_vec.begin() + m_front_offset;
        }
        else
            return m_vec.erase(pos);
    }

    iterator erase(iterator first, iterator last)
    {
        return m_vec.erase(first, last);
    }

    size_type capacity() const noexcept
    {
        return m_vec.capacity();
    }

    void shrink_to_fit()
    {
        clear_removed();
        m_vec.shrink_to_fit();
    }

    void reserve(size_type new_cap)
    {
        clear_removed();
        m_vec.reserve(new_cap);
    }

    size_type size() const
    {
        return m_vec.size() - m_front_offset;
    }

    template<typename InputIt>
    void assign(InputIt first, InputIt last)
    {
        clear_removed();
        m_vec.assign(first, last);
    }

    T* data()
    {
        return m_vec.data() + m_front_offset;
    }

    const T* data() const
    {
        return m_vec.data() + m_front_offset;
    }

private:
    void clear_removed()
    {
        m_vec.erase(m_vec.begin(), m_vec.begin() + m_front_offset);
        m_front_offset = 0;
    }
};

template<typename T>
bool operator==(const delayed_delete_vector<T>& lhs, const delayed_delete_vector<T>& rhs)
{
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

namespace detail {

template<>
struct is_std_vector_bool_store<delayed_delete_vector<bool>>
{
    using type = std::true_type;
};

} // namespace detail

}} // namespace mdds::mtv

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
