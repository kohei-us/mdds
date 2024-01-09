/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2022 Kohei Yoshida
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

#pragma once

#include "./cref_wrapper.hpp"

#include <string_view>
#include <unordered_map>

namespace mdds {

namespace ssmap {
namespace detail {

template<typename ValueT>
struct map_entry
{
    std::string_view key;
    ValueT value;
};

} // namespace detail

template<typename ValueT>
class linear_key_finder
{
    using value_type = ValueT;
    using size_type = typename std::string_view::size_type;
    using entry_type = detail::map_entry<ValueT>;

    const entry_type* m_entries;
    const entry_type* m_entries_end;

public:
    linear_key_finder(const entry_type* entries, const entry_type* entries_end);

    std::string_view operator()(const value_type& v) const;
};

template<typename ValueT>
class hash_key_finder
{
    using value_type = ValueT;
    using size_type = typename std::string_view::size_type;
    using entry_type = detail::map_entry<ValueT>;
    using keystore_type = std::unordered_map<
        mdds::detail::cref_wrapper<value_type>, size_type, typename mdds::detail::cref_wrapper<value_type>::hash>;

    const entry_type* m_entries;
    const entry_type* m_entries_end;
    keystore_type m_keys;

public:
    hash_key_finder(const entry_type* entries, const entry_type* entries_end);

    std::string_view operator()(const value_type& v) const;
};

} // namespace ssmap

/**
 * sorted_string_map is an immutable associative container that provides an
 * efficient way to map string keys to values of a user-specified type.
 * The keys must be known at compile time and must be sorted in ascending
 * order.
 *
 * Besides the minimal amount of memory required to store the size and memory
 * address of the caller-provided key-value entries and a few extra data,
 * it does not allocate any additional memory; it simply re-uses the
 * caller-provided key-value entries in all of its operations.
 *
 * @tparam ValueT Type of the values associated with the string keys.
 */
template<typename ValueT, template<typename> class FuncFindKeyT = ssmap::linear_key_finder>
class sorted_string_map
{
    using func_find_key_type = FuncFindKeyT<ValueT>;

public:
    using value_type = ValueT;
    using size_type = typename std::string_view::size_type;

    /**
     * Single key-value entry type.  Caller must provide at compile time a
     * static array of these entries.
     */
    using entry_type = ssmap::detail::map_entry<ValueT>;

    /**
     * Constructor.
     *
     * @param entries pointer to the array of key-value entries.
     * @param entry_size size of the key-value entry array.
     * @param null_value null value to return when the find method fails to
     *                   find a matching entry.
     */
    sorted_string_map(const entry_type* entries, size_type entry_size, value_type null_value);

    /**
     * Find a value associated with a specified string key.
     *
     * @param input pointer to a C-style string whose value represents the key
     *              to match.
     * @param len length of the matching string value.
     *
     * @return value associated with the key, or the null value in case the
     *         key is not found.
     */
    const value_type& find(const char* input, size_type len) const;

    /**
     * Find a value associated with a specified string key.
     *
     * @param input string key to match.
     *
     * @return value associated with the key, or the null value in case the
     *         key is not found.
     */
    const value_type& find(std::string_view input) const;

    /**
     * Find a key associated with a specified value.
     *
     * @param v Value to find the associated key of.
     *
     * @return Key associated with the value if found, or an empty string if one
     *         is not found.
     */
    std::string_view find_key(const value_type& v) const;

    /**
     * Return the number of entries in the map.  Since the number of entries
     * is statically defined at compile time, this method always returns the
     * same value.
     *
     * @return the number of entries in the map.
     */
    size_type size() const;

private:
    const entry_type* m_entries;
    const value_type m_null_value;
    const size_type m_entry_size;
    const entry_type* m_entries_end;

    func_find_key_type m_func_find_key;
};

} // namespace mdds

#include "sorted_string_map_def.inl"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
