// SPDX-FileCopyrightText: 2019 David Tardon
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <mdds/sorted_string_map.hpp>

#include <span>

void stop();

using mdds::sorted_string_map;

void test_sorted_string_map()
{
    sorted_string_map<int> empty_ssmap(std::span<const sorted_string_map<int>::entry_type>{}, 0);

    sorted_string_map<int>::entry_type entries[] = {
        {"aaaa", 1},
        {"bbb", 2},
        {"cc", 3},
        {"d", 4},
    };
    sorted_string_map<int> ssmap_int(entries, 0);

    stop();
}

// vim: set shiftwidth=4 softtabstop=4 expandtab:
