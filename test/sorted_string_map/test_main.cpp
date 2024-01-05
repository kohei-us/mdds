/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "test_global.hpp"
#define MDDS_SORTED_STRING_MAP_DEBUG 1
#include "mdds/sorted_string_map.hpp"
#include "mdds/global.hpp"

#include <cstring>
#include <vector>
#include <fstream>

enum name_type
{
    name_none = 0,
    name_andy,
    name_bruce,
    name_charlie,
    name_david
};

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

void ssmap_test_basic()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef mdds::sorted_string_map<name_type> map_type;

    map_type::entry entries[] = {
        {"andy", name_andy},   {"andy1", name_andy},      {"andy13", name_andy},
        {"bruce", name_bruce}, {"charlie", name_charlie}, {"david", name_david},
    };

    size_t entry_count = sizeof(entries) / sizeof(entries[0]);
    map_type names(entries, entry_count, name_none);
    for (size_t i = 0; i < entry_count; ++i)
    {
        std::cout << "* key = " << entries[i].key << std::endl;
        bool res = names.find(entries[i].key) == entries[i].value;
        assert(res);
    }

    // Try invalid keys.
    assert(names.find("foo", 3) == name_none);
    assert(names.find("andy133", 7) == name_none);
}

void ssmap_test_mixed_case_null()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef mdds::sorted_string_map<int> map_type;

    map_type::entry entries[] = {
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
        assert(res);
    }

    // Try invalid keys.
    assert(names.find(MDDS_ASCII("NUll")) == -1);
    assert(names.find(MDDS_ASCII("Oull")) == -1);
    assert(names.find(MDDS_ASCII("Mull")) == -1);
    assert(names.find(MDDS_ASCII("hell")) == -1);
}

void ssmap_test_find_string_view()
{
    MDDS_TEST_FUNC_SCOPE;

    constexpr int cv_unknown = -1;
    constexpr int cv_days = 0;
    constexpr int cv_hours = 1;
    constexpr int cv_minutes = 2;
    constexpr int cv_months = 3;
    constexpr int cv_quarters = 4;
    constexpr int cv_range = 5;
    constexpr int cv_seconds = 6;
    constexpr int cv_years = 7;

    using map_type = mdds::sorted_string_map<int>;

    constexpr map_type::entry entries[] = {
        {"days", cv_days},         {"hours", cv_hours}, {"minutes", cv_minutes}, {"months", cv_months},
        {"quarters", cv_quarters}, {"range", cv_range}, {"seconds", cv_seconds}, {"years", cv_years},
    };

    map_type mapping{entries, std::size(entries), cv_unknown};

    for (const auto& entry : entries)
    {
        auto v = mapping.find(entry.key);
        assert(v == entry.value);
        auto k = mapping.find_key(entry.value);
        assert(k == entry.key);
    }

    constexpr std::string_view unknown_keys[] = {
        "dayss", "Days", "ddays", "adfsd", "secoonds", "years ",
    };

    for (const auto& key : unknown_keys)
    {
        auto v = mapping.find(key);
        assert(v == cv_unknown);
        auto k = mapping.find_key(cv_unknown);
        assert(k.empty());
    }
}

void ssmap_test_move_only_value_type()
{
    MDDS_TEST_FUNC_SCOPE;

    using map_type = mdds::sorted_string_map<move_only_value>;

    const map_type::entry entries[] = {
        {"0x01", {1}},
        {"0x02", {2}},
        {"0x03", {3}},
        {"0x04", {4}},
    };

    map_type mapping{entries, std::size(entries), {0}};

    for (const auto& e : entries)
    {
        const move_only_value& v = mapping.find(e.key);
        assert(v.value == e.value.value);
        std::string_view k = mapping.find_key(e.value);
        assert(k == e.key);
    }

    // test for null value
    const auto& v = mapping.find("0x05");
    assert(v.value == 0);
    std::string_view k = mapping.find_key({5});
    assert(k.empty()); // not found
}

void ssmap_test_perf()
{
    MDDS_TEST_FUNC_SCOPE;

    std::string content;
    {
        // load the entire file content in memory
        std::ifstream in("../../misc/sorted_string_data.dat");
        assert(in);

        std::ostringstream os;
        os << in.rdbuf();
        content = os.str();
    }

    using map_type = mdds::sorted_string_map<int>;
    std::vector<map_type::entry> entries;

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
                entries.push_back(map_type::entry{});
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
            assert(v == -1);
        }
    }

    {
        stack_printer sp_find("perf-find-bottom");

        for (int i = 0; i < repeat; ++i)
        {
            auto v = names.find(entries.back().key);
            assert(v == entries.back().value);
        }
    }

    {
        stack_printer sp_find("perf-find-middle");
        const std::size_t pos = entries.size() / 2;

        for (int i = 0; i < repeat; ++i)
        {
            auto v = names.find(entries[pos].key);
            assert(v == entries[pos].value);
        }
    }

    {
        stack_printer sp_find("perf-find-top");

        for (int i = 0; i < repeat; ++i)
        {
            auto v = names.find(entries.front().key);
            assert(v == entries.front().value);
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
        ssmap_test_basic();
        ssmap_test_mixed_case_null();
        ssmap_test_find_string_view();
        ssmap_test_move_only_value_type();
    }

    if (opt.test_perf)
    {
        ssmap_test_perf();
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
