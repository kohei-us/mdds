/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2024 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_main.hpp"

#include <sstream>

#define _TEST_FUNC_SCOPE MDDS_TEST_FUNC_SCOPE_NS("trie_test_pack_value_type")

namespace trie_test_pack_value_type {

namespace {

struct uint8_traits : mdds::trie::default_traits
{
    using pack_value_type = std::uint8_t;
};

struct uint16_traits : mdds::trie::default_traits
{
    using pack_value_type = std::uint16_t;
};

struct uint32_traits : mdds::trie::default_traits
{
    using pack_value_type = std::uint32_t;
};

struct uint64_traits : mdds::trie::default_traits
{
    using pack_value_type = std::uint64_t;
};

using uint8_map_type = mdds::trie_map<std::string, int, uint8_traits>;
using uint16_map_type = mdds::trie_map<std::string, int, uint16_traits>;
using uint32_map_type = mdds::trie_map<std::string, int, uint32_traits>;
using uint64_map_type = mdds::trie_map<std::string, int, uint64_traits>;

} // anonymous namespace

void test_uint8_max_value()
{
    _TEST_FUNC_SCOPE;

    uint8_map_type db;

    auto max_value = std::numeric_limits<std::uint8_t>::max();

    for (std::uint8_t i = 0; i < max_value; ++i)
        db.insert(std::to_string(i), i);

    TEST_ASSERT(db.size() == max_value);

    try
    {
        // packed variant receives copied values
        auto packed = uint8_map_type::packed_type(db);
        TEST_ASSERT(!"size_error was expected to be thrown");
    }
    catch (const mdds::size_error&)
    {
        // expected
    }

    try
    {
        // packed variant receives moved values
        auto packed = db.pack();
        TEST_ASSERT(!"size_error was expected to be thrown");
    }
    catch (const mdds::size_error&)
    {
        // expected
    }

    std::vector<std::pair<std::string, int>> entries_pool;

    for (std::uint8_t i = 0; i < max_value; ++i)
        entries_pool.emplace_back(std::to_string(i), i);

    std::sort(entries_pool.begin(), entries_pool.end(), [](const auto& l, const auto& r) { return l.first < r.first; });

    std::vector<uint8_map_type::packed_type::entry> entries;
    for (const auto& [key, value] : entries_pool)
        entries.emplace_back(key.data(), key.size(), value);

    TEST_ASSERT(entries.size() == max_value);

    try
    {
        uint8_map_type::packed_type packed(entries.data(), entries.size());
        TEST_ASSERT(!"size_error was expected to be thrown");
    }
    catch (const mdds::size_error&)
    {
        // expected
    }
}

template<typename MapT>
std::size_t calc_state_size()
{
    using pack_value_type = typename MapT::traits_type::pack_value_type;
    std::cout << "pack value size: " << sizeof(pack_value_type) << std::endl;

    MapT db;
    auto max_value = std::numeric_limits<std::uint8_t>::max();

    for (std::uint8_t i = 1; i < max_value; ++i)
        db.insert(std::to_string(i), i);

    std::cout << "value count: " << db.size() << std::endl;

    auto packed = db.pack();
    std::ostringstream os;
    packed.save_state(os);

    auto bin = os.str();
    std::cout << "state size: " << bin.size() << std::endl;
    return bin.size();
}

void test_state_size()
{
    _TEST_FUNC_SCOPE;

    auto i8_size = calc_state_size<uint8_map_type>();
    auto i16_size = calc_state_size<uint16_map_type>();
    auto i32_size = calc_state_size<uint32_map_type>();
    auto i64_size = calc_state_size<uint64_map_type>();
    TEST_ASSERT(i8_size < i16_size);
    TEST_ASSERT(i16_size < i32_size);
    TEST_ASSERT(i32_size < i64_size);
}

void run()
{
    test_uint8_max_value();
    test_state_size();
}

} // namespace trie_test_pack_value_type

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
