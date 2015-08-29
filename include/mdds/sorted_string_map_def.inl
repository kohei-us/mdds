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

namespace mdds {

template<typename _ValueT>
sorted_string_map<_ValueT>::sorted_string_map(const entry* entries, size_type entry_size, value_type null_value) :
    m_entries(entries),
    m_null_value(null_value),
    m_entry_size(entry_size),
    m_entry_end(m_entries+m_entry_size) {}

template<typename _ValueT>
typename sorted_string_map<_ValueT>::value_type
sorted_string_map<_ValueT>::find(const char* input, size_type len) const
{
    const entry* p = m_entries;
    size_type pos = 0;
    for (; p != m_entry_end; ++p)
    {
        const char* key = p->key;
        size_type keylen = p->keylen;
        for (; pos < len && pos < keylen; ++pos)
        {
            if (input[pos] != key[pos])
                // Move to the next entry.
                break;
        }

        if (pos == len && len == keylen)
        {
            // Potential match found!  Parse the whole string to make sure
            // it's really a match.
            if (std::memcmp(input, key, len))
                // Not a match.
                return m_null_value;

            return p->value;
        }
    }
    return m_null_value;
}

template<typename _ValueT>
typename sorted_string_map<_ValueT>::size_type
sorted_string_map<_ValueT>::size() const
{
    return m_entry_size;
}

}
