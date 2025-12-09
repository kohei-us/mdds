/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2024 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#define MDDS_TRIE_MAP_DEBUG 1
#include "test_global.hpp" // This must be the first header to be included.

#include <mdds/trie_map.hpp>
#include <mdds/global.hpp>

struct local_debug_traits : mdds::trie::default_traits
{
    static constexpr bool dump_packed_construction_state = true;
};

using packed_int_map_type = mdds::packed_trie_map<std::string, int>;
using packed_str_map_type = mdds::packed_trie_map<std::string, std::string>;

void trie_packed_test1();
void trie_packed_test2();
void trie_packed_test3();
void trie_packed_test4();
void trie_packed_test_value_life_cycle();
void trie_packed_test_custom_string();
void trie_packed_test_iterator_empty();
void trie_packed_test_iterator();
void trie_packed_test_prefix_search1();
void trie_packed_test_key_as_input();
void trie_packed_test_copying();
void trie_packed_test_non_equal();

namespace trie_packed_test_save_and_load_state {

void run();

}

void trie_test1();
void trie_test2();
void trie_test_iterator_empty();
void trie_test_iterator();
void trie_test_iterator_with_erase();
void trie_test_find_iterator();
void trie_test_prefix_search();
void trie_test_key_as_input();
void trie_test_copying();
void trie_test_value_update_from_iterator();
void trie_test_equality();

namespace trie_test_node {

void run();

}

namespace trie_test_move_value {

void run();

}

namespace trie_test_pack_value_type {

void run();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
