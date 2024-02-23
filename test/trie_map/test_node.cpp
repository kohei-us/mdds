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
        assert(!node.valid());
        assert(!node.has_child());
        assert(!node.has_value());
        assert(!node.child('c').valid());
    }

    {
        map_type::packed_type::const_node_type node; // default constructor
        assert(!node.valid());
        assert(!node.has_child());
        assert(!node.has_value());
        assert(!node.child('c').valid());
    }

    auto verify = [](std::string_view name, const auto& trie) {
        std::cout << "trie type: " << name << std::endl;

        auto node = trie.root_node();
        assert(node.valid());
        assert(!node.has_child());
        assert(!node.has_value());
        assert(!node.child('d').valid());
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
        assert(node.valid());
        assert(node.has_child());
        assert(!node.has_value());

        node = node.child('t');
        assert(node.valid());
        assert(node.has_child());
        assert(!node.has_value());

        node = node.child('e');
        assert(node.valid());
        assert(node.has_child());
        assert(!node.has_value());

        node = node.child('s');
        assert(node.valid());
        assert(node.has_child());
        assert(!node.has_value());

        node = node.child('t');
        assert(node.valid());
        assert(!node.has_child());
        assert(node.has_value());
        assert(node.value() == 42);

        // no more child nodes
        assert(!node.child('f').valid());
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
        assert(node.valid());
        assert(!node.has_value());
        assert(node.has_child());

        for (char c = 'a'; c <= 'z'; ++c)
        {
            auto child = node.child(c);
            assert(child.valid());
            assert(child.has_value());
            assert(child.value() == int(c - 'a'));
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
    assert(it != errors.end());
    assert(it->second == true);

    auto node = errors.root_node();
    assert(node.valid());
    node = node.child('R');
    assert(node.valid());
    node = node.child('E');
    assert(node.valid());
    node = node.child('F');
    assert(node.valid());
    node = node.child('!');
    assert(node.valid());
    assert(node.has_value());
    assert(node.value() == true);
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
