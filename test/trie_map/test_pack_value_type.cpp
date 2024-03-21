/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2024 Kohei Yoshida
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

#include "test_main.hpp"

#include <sstream>

#define _TEST_FUNC_SCOPE MDDS_TEST_FUNC_SCOPE_NS("trie_test_pack_value_type")

namespace trie_test_pack_value_type {

namespace {

struct uint8_traits : mdds::trie::default_traits
{
    using pack_value_type = std::uint8_t;
};

using uint8_map_type = mdds::trie_map<std::string, int, uint8_traits>;

} // namespace

void test_uint8_max_value()
{
    _TEST_FUNC_SCOPE;

    uint8_map_type db;

    auto max_value = std::numeric_limits<std::uint8_t>::max();

    for (std::uint8_t i = 0; i < max_value; ++i)
        db.insert(std::to_string(i), i);

    assert(db.size() == max_value);

    try
    {
        // packed variant receives copied values
        auto packed = uint8_map_type::packed_type(db);
        assert(!"size_error was expected to be thrown");
    }
    catch (const mdds::size_error&)
    {
        // expected
    }

    try
    {
        // packed variant receives moved values
        auto packed = db.pack();
        assert(!"size_error was expected to be thrown");
    }
    catch (const mdds::size_error&)
    {
        // expected
    }
}

void run()
{
    test_uint8_max_value();
}

} // namespace trie_test_pack_value_type

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
