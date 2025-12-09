/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2014 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp"
#define MDDS_SORTED_STRING_MAP_DEBUG 1
#include "mdds/sorted_string_map.hpp"
#include "mdds/global.hpp"

#include <cstring>
#include <vector>
#include <fstream>
#include <unordered_set>

#define KEY_FINDER_TYPE \
    do \
    { \
        std::cout << "key finder type: " << keyfinder_name<KeyFinderT>{}() << std::endl; \
    } while (false)

enum name_type
{
    name_none = 0,
    name_andy,
    name_bruce,
    name_charlie,
    name_david
};

namespace std {

template<>
struct hash<name_type>
{
    std::size_t operator()(const name_type& v) const
    {
        return static_cast<std::size_t>(v);
    }
};

} // namespace std

struct move_only_value
{
    int value = {};

    move_only_value()
    {}
    move_only_value(int _value) : value(_value)
    {}
    move_only_value(move_only_value&& r) : value(r.value)
    {
        r.value = {};
    }

    move_only_value(const move_only_value&) = delete;

    bool operator==(const move_only_value& r) const
    {
        return value == r.value;
    }
};

namespace std {

template<>
struct hash<move_only_value>
{
    std::size_t operator()(const move_only_value& v) const
    {
        return std::hash<decltype(v.value)>{}(v.value);
    }
};

} // namespace std

template<template<typename> class KeyFinderT>
struct keyfinder_name;

template<>
struct keyfinder_name<mdds::ssmap::linear_key_finder>
{
    std::string_view operator()() const
    {
        return "linear";
    }
};

template<>
struct keyfinder_name<mdds::ssmap::hash_key_finder>
{
    std::string_view operator()() const
    {
        return "hash";
    }
};

template<template<typename> class KeyFinderT>
void ssmap_test_basic()
{
    MDDS_TEST_FUNC_SCOPE;
    KEY_FINDER_TYPE;

    using map_type = mdds::sorted_string_map<name_type, KeyFinderT>;

    const typename map_type::entry_type entries[] = {
        {"andy", name_andy},   {"andy1", name_andy},      {"andy13", name_andy},
        {"bruce", name_bruce}, {"charlie", name_charlie}, {"david", name_david},
    };

    constexpr auto n_entries = std::size(entries);
    map_type names(entries, n_entries, name_none);
    for (size_t i = 0; i < n_entries; ++i)
    {
        std::cout << "* key = " << entries[i].key << std::endl;
        bool res = names.find(entries[i].key) == entries[i].value;
        TEST_ASSERT(res);
    }

    // Try invalid keys.
    TEST_ASSERT(names.find("foo", 3) == name_none);
    TEST_ASSERT(names.find("andy133", 7) == name_none);

    // reverse lookup
    TEST_ASSERT(names.find_key(name_bruce) == "bruce");
    TEST_ASSERT(names.find_key(name_charlie) == "charlie");
    TEST_ASSERT(names.find_key(name_david) == "david");

    // negative case
    TEST_ASSERT(names.find_key(name_none).empty());

    // 'name_andy' is associated with three keys
    const std::unordered_set<std::string_view> keys_andy = {"andy", "andy1", "andy13"};

    TEST_ASSERT(keys_andy.count(names.find_key(name_andy)) > 0);
}

template<template<typename> class KeyFinderT>
void ssmap_test_mixed_case_null()
{
    MDDS_TEST_FUNC_SCOPE;
    KEY_FINDER_TYPE;

    typedef mdds::sorted_string_map<int, KeyFinderT> map_type;

    const typename map_type::entry_type entries[] = {
        {"NULL", 1},
        {"Null", 2},
        {"null", 3},
        {"~", 4},
    };

    size_t entry_count = std::size(entries);
    map_type names(entries, entry_count, -1);
    for (size_t i = 0; i < entry_count; ++i)
    {
        std::cout << "* key = " << entries[i].key << std::endl;
        bool res = names.find(entries[i].key) == entries[i].value;
        TEST_ASSERT(res);
    }

    // Try invalid keys.
    TEST_ASSERT(names.find(MDDS_ASCII("NUll")) == -1);
    TEST_ASSERT(names.find(MDDS_ASCII("Oull")) == -1);
    TEST_ASSERT(names.find(MDDS_ASCII("Mull")) == -1);
    TEST_ASSERT(names.find(MDDS_ASCII("hell")) == -1);

    // reverse lookup
    TEST_ASSERT(names.find_key(1) == "NULL");
    TEST_ASSERT(names.find_key(2) == "Null");
    TEST_ASSERT(names.find_key(3) == "null");
    TEST_ASSERT(names.find_key(4) == "~");
}

template<template<typename> class KeyFinderT>
void ssmap_test_find_string_view()
{
    MDDS_TEST_FUNC_SCOPE;
    KEY_FINDER_TYPE;

    constexpr int cv_unknown = -1;
    constexpr int cv_days = 0;
    constexpr int cv_hours = 1;
    constexpr int cv_minutes = 2;
    constexpr int cv_months = 3;
    constexpr int cv_quarters = 4;
    constexpr int cv_range = 5;
    constexpr int cv_seconds = 6;
    constexpr int cv_years = 7;

    using map_type = mdds::sorted_string_map<int, KeyFinderT>;

    constexpr typename map_type::entry_type entries[] = {
        {"days", cv_days},         {"hours", cv_hours}, {"minutes", cv_minutes}, {"months", cv_months},
        {"quarters", cv_quarters}, {"range", cv_range}, {"seconds", cv_seconds}, {"years", cv_years},
    };

    map_type mapping{entries, std::size(entries), cv_unknown};

    for (const auto& entry : entries)
    {
        auto v = mapping.find(entry.key);
        TEST_ASSERT(v == entry.value);
        auto k = mapping.find_key(entry.value);
        TEST_ASSERT(k == entry.key);
    }

    constexpr std::string_view unknown_keys[] = {
        "dayss", "Days", "ddays", "adfsd", "secoonds", "years ",
    };

    for (const auto& key : unknown_keys)
    {
        auto v = mapping.find(key);
        TEST_ASSERT(v == cv_unknown);
        auto k = mapping.find_key(cv_unknown);
        TEST_ASSERT(k.empty());
    }
}

template<template<typename> class KeyFinderT>
void ssmap_test_move_only_value_type()
{
    MDDS_TEST_FUNC_SCOPE;
    KEY_FINDER_TYPE;

    using map_type = mdds::sorted_string_map<move_only_value, KeyFinderT>;

    const typename map_type::entry_type entries[] = {
        {"0x01", {1}},
        {"0x02", {2}},
        {"0x03", {3}},
        {"0x04", {4}},
    };

    map_type mapping{entries, std::size(entries), {0}};

    for (const auto& e : entries)
    {
        const move_only_value& v = mapping.find(e.key);
        TEST_ASSERT(v.value == e.value.value);
        std::string_view k = mapping.find_key(e.value);
        TEST_ASSERT(k == e.key);
    }

    // test for null value
    const auto& v = mapping.find("0x05");
    TEST_ASSERT(v.value == 0);
    std::string_view k = mapping.find_key({5});
    TEST_ASSERT(k.empty()); // not found
}

void ssmap_test_perf()
{
    MDDS_TEST_FUNC_SCOPE;

    std::string content;
    {
        // load the entire file content in memory
        std::ifstream in("../../misc/sorted_string_data.dat");
        TEST_ASSERT(in);

        std::ostringstream os;
        os << in.rdbuf();
        content = os.str();
    }

    using map_type = mdds::sorted_string_map<int>;
    std::vector<map_type::entry_type> entries;

    {
        // populate entries from the data file
        int i = 0;
        const char* p0 = nullptr;
        const char* p = content.data();
        const char* p_end = p + content.size();
        for (; p != p_end; ++p)
        {
            if (!p0)
                p0 = p;

            if (*p == '\n')
            {
                std::size_t n = std::distance(p0, p);
                entries.push_back(map_type::entry_type{});
                entries.back().key = std::string_view{p0, n};
                entries.back().value = i++;
                p0 = nullptr;
            }
        }
    }

    std::cout << "entry count: " << entries.size() << std::endl;

    map_type names(entries.data(), entries.size(), -1);

    constexpr int repeat = 1000;
    {
        // worst case performance is when the key is not found
        stack_printer sp_find("perf-find-not-found");

        for (int i = 0; i < repeat; ++i)
        {
            auto v = names.find("test");
            TEST_ASSERT(v == -1);
        }
    }

    {
        stack_printer sp_find("perf-find-bottom");

        for (int i = 0; i < repeat; ++i)
        {
            auto v = names.find(entries.back().key);
            TEST_ASSERT(v == entries.back().value);
        }
    }

    {
        stack_printer sp_find("perf-find-middle");
        const std::size_t pos = entries.size() / 2;

        for (int i = 0; i < repeat; ++i)
        {
            auto v = names.find(entries[pos].key);
            TEST_ASSERT(v == entries[pos].value);
        }
    }

    {
        stack_printer sp_find("perf-find-top");

        for (int i = 0; i < repeat; ++i)
        {
            auto v = names.find(entries.front().key);
            TEST_ASSERT(v == entries.front().value);
        }
    }
}

int main(int argc, char** argv)
{
    cmd_options opt;
    if (!parse_cmd_options(argc, argv, opt))
        return EXIT_FAILURE;

    if (opt.test_func)
    {
        ssmap_test_basic<mdds::ssmap::linear_key_finder>();
        ssmap_test_basic<mdds::ssmap::hash_key_finder>();
        ssmap_test_mixed_case_null<mdds::ssmap::linear_key_finder>();
        ssmap_test_mixed_case_null<mdds::ssmap::hash_key_finder>();
        ssmap_test_find_string_view<mdds::ssmap::linear_key_finder>();
        ssmap_test_find_string_view<mdds::ssmap::hash_key_finder>();
        ssmap_test_move_only_value_type<mdds::ssmap::linear_key_finder>();
        ssmap_test_move_only_value_type<mdds::ssmap::hash_key_finder>();
    }

    if (opt.test_perf)
    {
        ssmap_test_perf();
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
