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
#include <type_traits>

#define _TEST_FUNC_SCOPE MDDS_TEST_FUNC_SCOPE_NS("trie_test_move_value")

namespace trie_test_move_value {

namespace {

struct move_value
{
    std::string value;

    move_value() = default;
    move_value(std::string v) : value(std::move(v))
    {}
    move_value(const move_value&) = delete;
    move_value(move_value&&) = default;

    move_value& operator=(const move_value&) = delete;
    move_value& operator=(move_value&&) = default;

    bool operator==(const move_value& other) const
    {
        return value == other.value;
    }
};

static_assert(!std::is_copy_constructible_v<move_value>);
static_assert(std::is_move_constructible_v<move_value>);

using map_type = mdds::trie_map<std::string, move_value>;

void test_basic()
{
    _TEST_FUNC_SCOPE;

    map_type store;
    store.insert("test", move_value("one"));
    store.insert("test", move_value("two")); // overwrite

    {
        auto results = store.prefix_search("te");
        auto it = results.begin();
        assert(it != results.end());
        assert(it->second.value == "two");
        assert(++it == results.end());
    }

    {
        auto it = store.find("test");
        assert(it != store.end());
        assert(it->second.value == "two");
        assert(++it == store.end());
    }

    auto packed = store.pack();

    {
        auto results = packed.prefix_search("te");
        auto it = results.begin();
        assert(it != results.end());
        assert(it->second.value == "two");
        assert(++it == results.end());
    }

    {
        auto it = packed.find("test");
        assert(it != packed.end());
        assert(it->second.value == "two");
        assert(++it == packed.end());
    }
}

void test_node_traversal()
{
    _TEST_FUNC_SCOPE;

    map_type store;
    store.insert("one", move_value("one"));
    store.insert("two", move_value("two"));

    {
        auto node = store.root_node();
        assert(node.valid());
        node = node.child('o');
        assert(node.valid());
        node = node.child('n');
        assert(node.valid());
        node = node.child('e');
        assert(node.valid());
        assert(node.has_value());
        assert(node.value().value == "one");

        node = store.root_node();
        assert(node.valid());
        node = node.child('t');
        assert(node.valid());
        node = node.child('w');
        assert(node.valid());
        node = node.child('o');
        assert(node.valid());
        assert(node.has_value());
        assert(node.value().value == "two");
    }

    auto packed = store.pack();

    {
        auto node = packed.root_node();
        assert(node.valid());
        node = node.child('o');
        assert(node.valid());
        node = node.child('n');
        assert(node.valid());
        node = node.child('e');
        assert(node.valid());
        assert(node.has_value());
        assert(node.value().value == "one");

        node = packed.root_node();
        assert(node.valid());
        node = node.child('t');
        assert(node.valid());
        node = node.child('w');
        assert(node.valid());
        node = node.child('o');
        assert(node.valid());
        assert(node.has_value());
        assert(node.value().value == "two");
    }
}

void test_equality()
{
    _TEST_FUNC_SCOPE;

    map_type store1, store2;
    store1.insert("one", move_value("one"));
    store1.insert("two", move_value("two"));
    store2.insert("one", move_value("one"));
    store2.insert("two", move_value("two"));

    // TODO: implement operator== for trie_map
    //  assert(store1 == store2);

    auto packed1 = store1.pack();
    auto packed2 = store2.pack();

    assert(packed1 == packed2);
}

void test_non_equality()
{
    _TEST_FUNC_SCOPE;

    map_type store1, store2;
    store1.insert("one", move_value("one"));
    store1.insert("two", move_value("two"));
    store2.insert("one", move_value("1"));
    store2.insert("two", move_value("2"));

    // TODO: implement operator!= for trie_map
    //  assert(store1 != store2);

    auto packed1 = store1.pack();
    auto packed2 = store2.pack();

    assert(packed1 != packed2);
}

void test_move_construction()
{
    _TEST_FUNC_SCOPE;

    map_type store;
    store.insert("one", move_value("one"));
    store.insert("two", move_value("two"));

    map_type moved(std::move(store));

    {
        auto it = moved.find("one");
        assert(it != moved.end());
        assert(it->second.value == "one");
        it = moved.find("two");
        assert(it != moved.end());
        assert(it->second.value == "two");
    }

    auto packed = moved.pack();
    decltype(packed) packed_moved(std::move(packed));
    {
        auto it = packed_moved.find("one");
        assert(it != packed_moved.end());
        assert(it->second.value == "one");
        it = packed_moved.find("two");
        assert(it != packed_moved.end());
        assert(it->second.value == "two");
    }
}

} // anonymous namespace

void run()
{
    test_basic();
    test_node_traversal();
    test_equality();
    test_non_equality();
    test_move_construction();
}

} // namespace trie_test_move_value

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
