/*************************************************************************
 *
 * Copyright (c) 2014-2015 Kohei Yoshida
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

#include <cstring>
#include <algorithm>

#include <cassert>

namespace mdds {

namespace detail {

// don't use it!
// Implementation detail!
template<typename _ValueT>
bool compare(
    const typename sorted_string_map<_ValueT>::entry& entry1, const typename sorted_string_map<_ValueT>::entry& entry2)
{
    if (entry1.keylen != entry2.keylen)
    {
        typename sorted_string_map<_ValueT>::size_type keylen = std::min(entry1.keylen, entry2.keylen);
        int ret = std::memcmp(entry1.key, entry2.key, keylen);
        if (ret == 0)
            return entry1.keylen < entry2.keylen;

        return ret < 0;
    }
    else
    {
        return std::memcmp(entry1.key, entry2.key, entry1.keylen) < 0;
    }
}

} // namespace detail

template<typename _ValueT>
sorted_string_map<_ValueT>::sorted_string_map(const entry* entries, size_type entry_size, value_type null_value)
    : m_entries(entries), m_null_value(null_value), m_entry_size(entry_size), m_entry_end(m_entries + m_entry_size)
{
#if defined(_GLIBCXX_DEBUG) || defined(MDDS_ASSERT_STRING_MAP)
    assert(std::is_sorted(m_entries, m_entry_end, detail::compare<_ValueT>));
#endif
}

template<typename _ValueT>
typename sorted_string_map<_ValueT>::value_type sorted_string_map<_ValueT>::find(const char* input, size_type len) const
{
    if (m_entry_size == 0)
        return m_null_value;

    entry ent;
    ent.key = input;
    ent.keylen = len;

    const entry* val = std::lower_bound(m_entries, m_entry_end, ent, detail::compare<_ValueT>);
    if (val == m_entry_end || val->keylen != len || std::memcmp(val->key, input, len))
        return m_null_value;

    return val->value;
}

template<typename _ValueT>
typename sorted_string_map<_ValueT>::size_type sorted_string_map<_ValueT>::size() const
{
    return m_entry_size;
}

} // namespace mdds
