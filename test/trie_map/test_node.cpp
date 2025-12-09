/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2024 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_main.hpp"

#define _TEST_FUNC_SCOPE MDDS_TEST_FUNC_SCOPE_NS("trie_test_node")

namespace trie_test_node {

namespace {

using map_type = mdds::trie_map<std::string, int>;
using mdds::trie::dump_structure_type;

void test_empty()
{
    _TEST_FUNC_SCOPE;

    {
        map_type::const_node_type node; // default constructor
        TEST_ASSERT(!node.valid());
        TEST_ASSERT(!node.has_child());
        TEST_ASSERT(!node.has_value());
        TEST_ASSERT(!node.child('c').valid());
    }

    {
        map_type::packed_type::const_node_type node; // default constructor
        TEST_ASSERT(!node.valid());
        TEST_ASSERT(!node.has_child());
        TEST_ASSERT(!node.has_value());
        TEST_ASSERT(!node.child('c').valid());
    }

    auto verify = [](std::string_view name, const auto& trie) {
        std::cout << "trie type: " << name << std::endl;

        auto node = trie.root_node();
        TEST_ASSERT(node.valid());
        TEST_ASSERT(!node.has_child());
        TEST_ASSERT(!node.has_value());
        TEST_ASSERT(!node.child('d').valid());
    };

    map_type original;
    verify("original", original);

    auto packed = original.pack();
    verify("packed", packed);

    std::cout << packed.dump_structure(dump_structure_type::packed_buffer) << std::endl;
    std::cout << packed.dump_structure(dump_structure_type::trie_traversal) << std::endl;
}

void test_basic()
{
    _TEST_FUNC_SCOPE;

    auto verify = [](std::string_view name, const auto& trie) {
        std::cout << "trie type: " << name << std::endl;

        auto node = trie.root_node();
        TEST_ASSERT(node.valid());
        TEST_ASSERT(node.has_child());
        TEST_ASSERT(!node.has_value());

        node = node.child('t');
        TEST_ASSERT(node.valid());
        TEST_ASSERT(node.has_child());
        TEST_ASSERT(!node.has_value());

        node = node.child('e');
        TEST_ASSERT(node.valid());
        TEST_ASSERT(node.has_child());
        TEST_ASSERT(!node.has_value());

        node = node.child('s');
        TEST_ASSERT(node.valid());
        TEST_ASSERT(node.has_child());
        TEST_ASSERT(!node.has_value());

        node = node.child('t');
        TEST_ASSERT(node.valid());
        TEST_ASSERT(!node.has_child());
        TEST_ASSERT(node.has_value());
        TEST_ASSERT(node.value() == 42);

        // no more child nodes
        TEST_ASSERT(!node.child('f').valid());
    };

    map_type original;
    original.insert("test", 42);
    verify("original", original);

    auto packed = original.pack();
    verify("packed", packed);

    std::cout << packed.dump_structure(dump_structure_type::packed_buffer) << std::endl;
    std::cout << packed.dump_structure(dump_structure_type::trie_traversal) << std::endl;
}

void test_26_child_nodes()
{
    _TEST_FUNC_SCOPE;

    auto verify = [](std::string_view name, const auto& trie) {
        std::cout << "trie type: " << name << std::endl;

        auto node = trie.root_node();
        TEST_ASSERT(node.valid());
        TEST_ASSERT(!node.has_value());
        TEST_ASSERT(node.has_child());

        for (char c = 'a'; c <= 'z'; ++c)
        {
            auto child = node.child(c);
            TEST_ASSERT(child.valid());
            TEST_ASSERT(child.has_value());
            TEST_ASSERT(child.value() == int(c - 'a'));
            std::cout << "- char='" << c << "'; value=" << child.value() << std::endl;
        }
    };

    map_type original;

    for (char c = 'a'; c <= 'z'; ++c)
        original.insert(std::string{c}, int(c - 'a'));

    verify("original", original);
    auto packed = original.pack();
    verify("packed", packed);

    std::cout << packed.dump_structure(dump_structure_type::packed_buffer) << std::endl;
    std::cout << packed.dump_structure(dump_structure_type::trie_traversal) << std::endl;
}

void test_packed_nodes()
{
    _TEST_FUNC_SCOPE;

    using _map_type = mdds::packed_trie_map<std::string, bool>;

    // Keys must be sorted.
    const _map_type::entry entries[] = {
        {MDDS_ASCII("DIV/0!"), true}, {MDDS_ASCII("N/A"), true},  {MDDS_ASCII("NAME?"), true},
        {MDDS_ASCII("NULL!"), true},  {MDDS_ASCII("NUM!"), true}, {MDDS_ASCII("REF!"), true},
        {MDDS_ASCII("VALUE!"), true},
    };

    _map_type errors(entries, std::size(entries));

    auto it = errors.find("REF!");
    TEST_ASSERT(it != errors.end());
    TEST_ASSERT(it->second == true);

    auto node = errors.root_node();
    TEST_ASSERT(node.valid());
    node = node.child('R');
    TEST_ASSERT(node.valid());
    node = node.child('E');
    TEST_ASSERT(node.valid());
    node = node.child('F');
    TEST_ASSERT(node.valid());
    node = node.child('!');
    TEST_ASSERT(node.valid());
    TEST_ASSERT(node.has_value());
    TEST_ASSERT(node.value() == true);
}

} // anonymous namespace

void run()
{
    test_empty();
    test_basic();
    test_26_child_nodes();
    test_packed_nodes();
}

} // namespace trie_test_node

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
