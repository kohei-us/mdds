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

namespace trie_test_node {

void run();

}

namespace trie_test_move_value {

void run();

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
