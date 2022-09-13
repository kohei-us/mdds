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
#include <algorithm>

namespace mdds {

namespace detail {

template<typename ValueT, typename SizeT, template<typename, typename> class EntryT>
struct entry_funcs;

template<typename ValueT, typename SizeT>
struct entry_funcs<ValueT, SizeT, chars_map_entry>
{
    using entry = chars_map_entry<ValueT, SizeT>;
    using size_type = SizeT;

    static bool compare(const entry& entry1, const entry& entry2)
    {
        if (entry1.key_length != entry2.key_length)
        {
            std::size_t key_length = std::min(entry1.key_length, entry2.key_length);
            int ret = std::memcmp(entry1.key, entry2.key, key_length);
            if (ret == 0)
                return entry1.key_length < entry2.key_length;

            return ret < 0;
        }
        else
        {
            return std::memcmp(entry1.key, entry2.key, entry1.key_length) < 0;
        }
    }

    static entry to_entry(const char* input, std::size_t len)
    {
        return entry{input, len, ValueT{}};
    }

    static const char* get_key_ptr(const entry& e)
    {
        return e.key;
    }

    static size_type get_key_size(const entry& e)
    {
        return e.key_length;
    }
};

template<typename ValueT, typename SizeT>
struct entry_funcs<ValueT, SizeT, string_view_map_entry>
{
    using entry = string_view_map_entry<ValueT, SizeT>;
    using size_type = SizeT;

    static bool compare(const entry& entry1, const entry& entry2)
    {
        if (entry1.key.size() != entry2.key.size())
        {
            std::size_t key_length = std::min(entry1.key.size(), entry2.key.size());
            int ret = std::memcmp(entry1.key.data(), entry2.key.data(), key_length);
            if (ret == 0)
                return entry1.key.size() < entry2.key.size();

            return ret < 0;
        }
        else
        {
            return std::memcmp(entry1.key.data(), entry2.key.data(), entry1.key.size()) < 0;
        }
    }

    static entry to_entry(const char* input, std::size_t len)
    {
        return entry{{input, len}, ValueT{}};
    }

    static const char* get_key_ptr(const entry& e)
    {
        return e.key.data();
    }

    static size_type get_key_size(const entry& e)
    {
        return e.key.size();
    }
};

} // namespace detail

template<typename ValueT, template<typename, typename> class EntryT>
sorted_string_map<ValueT, EntryT>::sorted_string_map(const entry* entries, size_type entry_size, value_type null_value)
    : m_entries(entries), m_null_value(null_value), m_entry_size(entry_size), m_entry_end(m_entries + m_entry_size)
{
#ifdef MDDS_SORTED_STRING_MAP_DEBUG
    using entry_funcs = detail::entry_funcs<value_type, size_type, EntryT>;

    if (!std::is_sorted(m_entries, m_entry_end, entry_funcs::compare))
        throw invalid_arg_error("mapped entries are not sorted");
#endif
}

template<typename ValueT, template<typename, typename> class EntryT>
typename sorted_string_map<ValueT, EntryT>::value_type sorted_string_map<ValueT, EntryT>::find(
    const char* input, size_type len) const
{
    if (m_entry_size == 0)
        return m_null_value;

    using entry_funcs = detail::entry_funcs<value_type, size_type, EntryT>;
    entry ent = entry_funcs::to_entry(input, len);

    const entry* val = std::lower_bound(m_entries, m_entry_end, ent, entry_funcs::compare);

    if (val == m_entry_end || entry_funcs::get_key_size(*val) != len ||
        std::memcmp(entry_funcs::get_key_ptr(*val), input, len))
        return m_null_value;

    return val->value;
}

template<typename ValueT, template<typename, typename> class EntryT>
typename sorted_string_map<ValueT, EntryT>::value_type sorted_string_map<ValueT, EntryT>::find(
    std::string_view input) const
{
    return find(input.data(), input.size());
}

template<typename ValueT, template<typename, typename> class EntryT>
typename sorted_string_map<ValueT, EntryT>::size_type sorted_string_map<ValueT, EntryT>::size() const
{
    return m_entry_size;
}

} // namespace mdds

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
