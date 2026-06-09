/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2022 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "./global.hpp"

#include <cassert>
#include <algorithm>

namespace mdds {

namespace ssmap {
namespace detail {

template<typename ValueT>
struct compare
{
    using entry_type = map_entry<ValueT>;

    constexpr bool operator()(const entry_type& entry1, const entry_type& entry2) const
    {
        return entry1.key < entry2.key;
    }
};

} // namespace detail

template<typename ValueT>
constexpr linear_key_finder<ValueT>::linear_key_finder(
    const entry_type* entries, const entry_type* entries_end) noexcept
    : m_entries(entries), m_entries_end(entries_end)
{}

template<typename ValueT>
constexpr std::string_view linear_key_finder<ValueT>::operator()(const value_type& v) const
{
    auto it = std::find_if(m_entries, m_entries_end, [&v](const auto& e) { return e.value == v; });
    if (it == m_entries_end)
        return {};

    return it->key;
}

template<typename ValueT>
hash_key_finder<ValueT>::hash_key_finder(const entry_type* entries, const entry_type* entries_end) : m_entries(entries)
{
    size_type pos = 0u;
    for (const auto* e = entries; e != entries_end; ++e, ++pos)
        m_keys.insert_or_assign(e->value, pos);

    assert(pos == static_cast<decltype(pos)>(entries_end - entries));
}

template<typename ValueT>
std::string_view hash_key_finder<ValueT>::operator()(const value_type& v) const
{
    auto it = m_keys.find(v);
    if (it == m_keys.cend())
        return {};

    const auto* entry = m_entries + it->second;
    return entry->key;
}

} // namespace ssmap

template<typename ValueT, template<typename> class KeyFinderT>
constexpr sorted_string_map<ValueT, KeyFinderT>::sorted_string_map(
    std::span<const entry_type> entries, value_type null_value)
    : m_entries(entries.data()), m_null_value(std::move(null_value)), m_entry_size(entries.size()),
      m_entries_end(m_entries + m_entry_size), m_func_find_key(m_entries, m_entries_end)
{
#ifdef MDDS_SORTED_STRING_MAP_DEBUG
    if (!std::is_sorted(m_entries, m_entries_end, ssmap::detail::compare<value_type>{}))
        throw invalid_arg_error("mapped entries are not sorted");
#endif
}

template<typename ValueT, template<typename> class KeyFinderT>
constexpr sorted_string_map<ValueT, KeyFinderT>::sorted_string_map(
    const entry_type* entries, size_type entry_size, value_type null_value)
    : sorted_string_map(std::span<const entry_type>(entries, entry_size), std::move(null_value))
{}

template<typename ValueT, template<typename> class KeyFinderT>
constexpr const typename sorted_string_map<ValueT, KeyFinderT>::value_type& sorted_string_map<ValueT, KeyFinderT>::find(
    const char* input, size_type len) const
{
    return find(std::string_view(input, len));
}

template<typename ValueT, template<typename> class KeyFinderT>
constexpr const typename sorted_string_map<ValueT, KeyFinderT>::value_type& sorted_string_map<ValueT, KeyFinderT>::find(
    std::string_view input) const
{
    if (m_entry_size == 0)
        return m_null_value;

    const entry_type* val = std::lower_bound(
        m_entries, m_entries_end, entry_type{input, value_type{}}, ssmap::detail::compare<value_type>{});

    if (val == m_entries_end || val->key != input)
        return m_null_value;

    return val->value;
}

template<typename ValueT, template<typename> class KeyFinderT>
constexpr std::string_view sorted_string_map<ValueT, KeyFinderT>::find_key(const value_type& v) const
{
    return m_func_find_key(v);
}

template<typename ValueT, template<typename> class KeyFinderT>
constexpr typename sorted_string_map<ValueT, KeyFinderT>::size_type sorted_string_map<ValueT, KeyFinderT>::size() const
{
    return m_entry_size;
}

} // namespace mdds

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
