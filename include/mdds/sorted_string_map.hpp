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

#include <cstdlib>
#include <string_view>

namespace mdds {

/**
 * Single key-value entry.  Caller must provide at compile time a static array
 * of these entries.
 *
 * @param key memory address of the first character of the char buffer that
 *            stores the key.
 * @param key_length length of the char buffer.
 * @param value value associated with the key.
 */
template<typename ValueT, typename SizeT>
struct chars_map_entry
{
    const char* key;
    SizeT key_length;
    ValueT value;
};

template<typename ValueT, typename SizeT>
struct string_view_map_entry
{
    std::string_view key;
    ValueT value;
};

/**
 * sorted_string_map provides an efficient way to map string keys to
 * arbitrary values, provided that the keys are known at compile time and
 * are sorted in ascending order.
 */
template<typename ValueT, template<typename, typename> class EntryT = chars_map_entry>
class sorted_string_map
{
public:
    using value_type = ValueT;
    using size_type = std::size_t;
    using entry = EntryT<ValueT, size_type>;

    /**
     * Constructor.
     *
     * @param entries pointer to the array of key-value entries.
     * @param entry_size size of the key-value entry array.
     * @param null_value null value to return when the find method fails to
     *                   find a matching entry.
     */
    sorted_string_map(const entry* entries, size_type entry_size, value_type null_value);

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
    value_type find(const char* input, size_type len) const;

    /**
     * Find a value associated with a specified string key.
     *
     * @param input string key to match.
     *
     * @return value associated with the key, or the null value in case the
     *         key is not found.
     */
    value_type find(std::string_view input) const;

    /**
     * Return the number of entries in the map.  Since the number of entries
     * is statically defined at compile time, this method always returns the
     * same value.
     *
     * @return the number of entries in the map.
     */
    size_type size() const;

private:
    const entry* m_entries;
    value_type m_null_value;
    size_type m_entry_size;
    const entry* m_entry_end;
};

} // namespace mdds

#include "sorted_string_map_def.inl"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
