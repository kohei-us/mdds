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

#include "./global.hpp"

#include <cstring>
#include <cassert>
#include <algorithm>

namespace mdds {

namespace ssmap {
namespace detail {

template<typename ValueT>
struct compare
{
    using entry_type = map_entry<ValueT>;

    bool operator()(const entry_type& entry1, const entry_type& entry2) const
    {
        if (entry1.key.size() == entry2.key.size())
            return std::memcmp(entry1.key.data(), entry2.key.data(), entry1.key.size()) < 0;

        std::size_t key_length = std::min(entry1.key.size(), entry2.key.size());
        int ret = std::memcmp(entry1.key.data(), entry2.key.data(), key_length);
        if (ret == 0)
            return entry1.key.size() < entry2.key.size();

        return ret < 0;
    }
};

} // namespace detail

template<typename ValueT>
linear_key_finder<ValueT>::linear_key_finder(const entry_type* entries, const entry_type* entries_end)
    : m_entries(entries), m_entries_end(entries_end)
{}

template<typename ValueT>
std::string_view linear_key_finder<ValueT>::operator()(const value_type& v) const
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
sorted_string_map<ValueT, KeyFinderT>::sorted_string_map(
    const entry_type* entries, size_type entry_size, value_type null_value)
    : m_entries(entries), m_null_value(std::move(null_value)), m_entry_size(entry_size),
      m_entries_end(m_entries + m_entry_size), m_func_find_key(m_entries, m_entries_end)
{
#ifdef MDDS_SORTED_STRING_MAP_DEBUG
    if (!std::is_sorted(m_entries, m_entries_end, ssmap::detail::compare<value_type>{}))
        throw invalid_arg_error("mapped entries are not sorted");
#endif
}

template<typename ValueT, template<typename> class KeyFinderT>
const typename sorted_string_map<ValueT, KeyFinderT>::value_type& sorted_string_map<ValueT, KeyFinderT>::find(
    const char* input, size_type len) const
{
    if (m_entry_size == 0)
        return m_null_value;

    const entry_type* val = std::lower_bound(
        m_entries, m_entries_end, entry_type{{input, len}, value_type{}}, ssmap::detail::compare<value_type>{});

    if (val == m_entries_end || val->key.size() != len || std::memcmp(val->key.data(), input, len))
        return m_null_value;

    return val->value;
}

template<typename ValueT, template<typename> class KeyFinderT>
const typename sorted_string_map<ValueT, KeyFinderT>::value_type& sorted_string_map<ValueT, KeyFinderT>::find(
    std::string_view input) const
{
    return find(input.data(), input.size());
}

template<typename ValueT, template<typename> class KeyFinderT>
std::string_view sorted_string_map<ValueT, KeyFinderT>::find_key(const value_type& v) const
{
    return m_func_find_key(v);
}

template<typename ValueT, template<typename> class KeyFinderT>
typename sorted_string_map<ValueT, KeyFinderT>::size_type sorted_string_map<ValueT, KeyFinderT>::size() const
{
    return m_entry_size;
}

} // namespace mdds

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
