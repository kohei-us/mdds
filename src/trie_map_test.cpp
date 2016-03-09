/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2015 Kohei Yoshida
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

#define MDDS_TRIE_MAP_DEBUG 1
//#define MDDS_TREI_MAP_DEBUG_DUMP_TRIE 1
//#define MDDS_TREI_MAP_DEBUG_DUMP_PACKED 1

#include "mdds/trie_map.hpp"
#include "mdds/global.hpp"
#include "test_global.hpp"

using namespace std;
using namespace mdds;

typedef packed_trie_map<trie::std_string_trait, int> packed_int_map_type;

bool verify_entries(
    const packed_int_map_type& db, const packed_int_map_type::entry* entries,
    size_t entry_size)
{
    auto items = db.prefix_search(nullptr, 0);
    for (size_t i = 0, n = items.size(); i < n; ++i)
        cout << items[i].first << ": " << items[i].second << endl;

    const packed_int_map_type::entry* p = entries;
    const packed_int_map_type::entry* p_end = p + entry_size;
    for (; p != p_end; ++p)
    {
        auto it = db.find(p->key, p->keylen);
        if (it == db.end() || it->second != p->value)
            return false;
    }

    return true;
}

void trie_packed_test1()
{
    stack_printer __stack_printer__("::trie_packed_test1");

    packed_int_map_type::entry entries[] =
    {
        { MDDS_ASCII("a"),  13 },
        { MDDS_ASCII("aa"), 10 },
        { MDDS_ASCII("ab"), 3 },
        { MDDS_ASCII("b"),  7 },
    };

    size_t entry_size = MDDS_N_ELEMENTS(entries);
    packed_int_map_type db(entries, entry_size, -1);
    assert(db.size() == 4);
    assert(verify_entries(db, entries, entry_size));

    // invalid keys
    assert(db.find(MDDS_ASCII("ac")) == db.end());
    assert(db.find(MDDS_ASCII("c")) == db.end());

    // Get all key-value pairs.
    auto prefix_list = db.prefix_search(nullptr, 0);
    assert(prefix_list.size() == 4);
    assert(prefix_list[0].first == "a");
    assert(prefix_list[1].first == "aa");
    assert(prefix_list[2].first == "ab");
    assert(prefix_list[3].first == "b");
    assert(prefix_list[0].second == 13);
    assert(prefix_list[1].second == 10);
    assert(prefix_list[2].second == 3);
    assert(prefix_list[3].second == 7);

    auto it = db.find(MDDS_ASCII("a"));
    assert(it->first == "a");
    assert(it->second == 13);
    ++it;
    assert(it->first == "aa");
    assert(it->second == 10);
    ++it;
    assert(it->first == "ab");
    assert(it->second == 3);
    ++it;
    assert(it->first == "b");
    assert(it->second == 7);
    ++it;
    assert(it == db.end());
}

void trie_packed_test2()
{
    stack_printer __stack_printer__("::trie_packed_test2");

    packed_int_map_type::entry entries[] =
    {
        { MDDS_ASCII("aaron"),     0 },
        { MDDS_ASCII("al"),        1 },
        { MDDS_ASCII("aldi"),      2 },
        { MDDS_ASCII("andy"),      3 },
        { MDDS_ASCII("bison"),     4 },
        { MDDS_ASCII("bruce"),     5 },
        { MDDS_ASCII("charlie"),   6 },
        { MDDS_ASCII("charlotte"), 7 },
        { MDDS_ASCII("david"),     8 },
        { MDDS_ASCII("dove"),      9 },
        { MDDS_ASCII("e"),        10 },
        { MDDS_ASCII("eva"),      11 },
    };

    size_t entry_size = MDDS_N_ELEMENTS(entries);
    packed_int_map_type db(entries, entry_size, -1);
    assert(db.size() == 12);
    assert(verify_entries(db, entries, entry_size));

    // invalid keys
    assert(db.find(MDDS_ASCII("aarons")) == db.end());
    assert(db.find(MDDS_ASCII("a")) == db.end());
    assert(db.find(MDDS_ASCII("biso")) == db.end());
    assert(db.find(MDDS_ASCII("dAvid")) == db.end());
}

void trie_packed_test3()
{
    stack_printer __stack_printer__("::trie_packed_test3");

    packed_int_map_type::entry entries[] =
    {
        { MDDS_ASCII("NULL"), 1 },
        { MDDS_ASCII("Null"), 2 },
        { MDDS_ASCII("null"), 3 },
        { MDDS_ASCII("~"),    4 },
    };

    size_t entry_size = MDDS_N_ELEMENTS(entries);
    packed_int_map_type db(entries, entry_size, -1);
    assert(db.size() == 4);
    assert(verify_entries(db, entries, entry_size));

    // invalid keys
    assert(db.find(MDDS_ASCII("NUll")) == db.end());
    assert(db.find(MDDS_ASCII("Oull")) == db.end());
    assert(db.find(MDDS_ASCII("Mull")) == db.end());
    assert(db.find(MDDS_ASCII("hell")) == db.end());
}

void trie_packed_test4()
{
    stack_printer __stack_printer__("::trie_packed_test4");

    enum name_type {
        name_none = 0,
        name_andy,
        name_bruce,
        name_charlie,
        name_david
    };

    packed_int_map_type::entry entries[] =
    {
        { MDDS_ASCII("andy"),    name_andy },
        { MDDS_ASCII("andy1"),   name_andy },
        { MDDS_ASCII("andy13"),  name_andy },
        { MDDS_ASCII("bruce"),   name_bruce },
        { MDDS_ASCII("charlie"), name_charlie },
        { MDDS_ASCII("david"),   name_david },
    };

    size_t entry_size = MDDS_N_ELEMENTS(entries);
    packed_int_map_type db(entries, entry_size, name_none);
    assert(db.size() == 6);
    assert(verify_entries(db, entries, entry_size));

    // Try invalid keys.
    assert(db.find("foo", 3) == db.end());
    assert(db.find("andy133", 7) == db.end());

    // Test prefix search on 'andy'.
    auto prefix_list = db.prefix_search(MDDS_ASCII("andy"));
    assert(prefix_list.size() == 3);
    assert(prefix_list[0].first == "andy");
    assert(prefix_list[1].first == "andy1");
    assert(prefix_list[2].first == "andy13");

    prefix_list = db.prefix_search(MDDS_ASCII("andy's toy"));
    assert(prefix_list.empty());

    prefix_list = db.prefix_search(MDDS_ASCII("e"));
    assert(prefix_list.empty());

    prefix_list = db.prefix_search(MDDS_ASCII("b"));
    assert(prefix_list.size() == 1);
    assert(prefix_list[0].first == "bruce");
    assert(prefix_list[0].second == name_bruce);
}

struct value_wrapper
{
    int value;

    value_wrapper() : value(0) {}
    value_wrapper(int _value) : value(_value) {}
};

std::ostream& operator<<(std::ostream& os, const value_wrapper& vw)
{
    os << vw.value;
    return os;
}

typedef packed_trie_map<trie::std_string_trait, value_wrapper> packed_value_map_type;

void trie_packed_test_value_life_cycle()
{
    stack_printer __stack_printer__("::trie_packed_test_value_life_cycle");

    using entry = packed_value_map_type::entry;

    // Entries must be sorted by the key!
    std::unique_ptr<vector<entry>> entries(new vector<entry>);
    entries->push_back(entry(MDDS_ASCII("fifteen"), value_wrapper(15)));
    entries->push_back(entry(MDDS_ASCII("ten"), value_wrapper(10)));
    entries->push_back(entry(MDDS_ASCII("twelve"), value_wrapper(12)));
    entries->push_back(entry(MDDS_ASCII("two"), value_wrapper(2)));

    packed_value_map_type db(entries->data(), entries->size(), value_wrapper(-1));

    // Delete the original entry store.
    entries.reset();

    auto items = db.prefix_search(nullptr, 0);
    for (size_t i = 0, n = items.size(); i < n; ++i)
        cout << items[i].first << ": " << items[i].second.value << endl;

    auto it = db.find(MDDS_ASCII("twelve"));
    assert(it->second.value == 12);

    it = db.find(MDDS_ASCII("two"));
    assert(it->second.value == 2);

    it = db.find(MDDS_ASCII("foo"));
    assert(it == db.end());
}

struct custom_string
{
    std::string data;

    custom_string() {}
    custom_string(const std::string& _data) : data(_data) {}
};

struct custom_string_trait
{
    typedef uint16_t key_unit_type;
    typedef custom_string key_type;
    typedef std::vector<key_unit_type> key_buffer_type;

    static key_buffer_type to_key_buffer(const key_unit_type* str, size_t length)
    {
        key_buffer_type buf;
        const key_unit_type* str_end = str + length;
        for (; str != str_end; ++str)
            buf.push_back(*str);

        return buf;
    }

    static void push_back(key_buffer_type& buffer, key_unit_type c)
    {
        buffer.push_back(c);
    }

    static void pop_back(key_buffer_type& buffer)
    {
        buffer.pop_back();
    }

    static key_type to_key(const key_buffer_type& buf)
    {
        // Cast all uint16_t chars to regular chars.
        key_type s;

        std::for_each(buf.begin(), buf.end(),
            [&](key_unit_type c)
            {
                s.data.push_back(static_cast<char>(c));
            }
        );
        return s;
    }
};

typedef packed_trie_map<custom_string_trait, std::string> packed_custom_str_map_type;

void trie_packed_test_custom_string()
{
    stack_printer __stack_printer__("::trie_packed_test_custom_string");

    const uint16_t key_alex[] = { 0x41, 0x6C, 0x65, 0x78 };
    const uint16_t key_bob[]  = { 0x42, 0x6F, 0x62 };
    const uint16_t key_max[]  = { 0x4D, 0x61, 0x78 };
    const uint16_t key_ming[] = { 0x4D, 0x69, 0x6E, 0x67 };

    const packed_custom_str_map_type::entry entries[] = {
        { key_alex, 4, "Alex" },
        { key_bob,  3, "Bob"  },
        { key_max,  3, "Max"  },
        { key_ming, 4, "Ming" },
    };

    size_t n_entries = MDDS_N_ELEMENTS(entries);
    packed_custom_str_map_type db(entries, n_entries, "-");
    for (size_t i = 0; i < n_entries; ++i)
    {
        auto it = db.find(entries[i].key, entries[i].keylen);
        cout << it->second << endl;
        assert(it->second == entries[i].value);
    }

    // Find all keys that start with 'M'.
    std::vector<packed_custom_str_map_type::key_value_type> vs = db.prefix_search(key_max, 1);
    assert(vs.size() == 2);
    assert(vs[0].first.data == vs[0].second);
    assert(vs[0].second == "Max");
    assert(vs[1].first.data == vs[1].second);
    assert(vs[1].second == "Ming");
}

void trie_packed_test_iterator_empty()
{
    stack_printer __stack_printer__("::trie_packed_test_iterator_empty");
    packed_int_map_type db(-1);

    // empty container
    packed_int_map_type::const_iterator it = db.begin();
    packed_int_map_type::const_iterator ite = db.end();

    assert(it == ite);
}

void trie_packed_test_iterator()
{
    stack_printer __stack_printer__("::trie_packed_test_iterator");

    using trie_map_type = trie_map<trie::std_string_trait, int>;
    using packed_type = trie_map_type::packed_type;
    using kv = packed_type::key_value_type;

    trie_map_type db(-1);

    db.insert(MDDS_ASCII("a"), 1);
    packed_type packed = db.pack();
    packed_type::const_iterator it = packed.begin();
    packed_type::const_iterator ite = packed.end();
    assert(it != ite);
    assert(it->first == "a");
    assert(it->second == 1);

    db.insert(MDDS_ASCII("ab"), 2);
    packed = db.pack(); // this invalidates the end position.

    it = packed.begin();
    ite = packed.end();
    assert(it != ite);
    assert(it->first == "a");
    assert(it->second == 1);

    ++it;
    assert(*it++ == kv("ab", 2));
    assert(it == ite);

    db.insert(MDDS_ASCII("aba"), 3);
    db.insert(MDDS_ASCII("abb"), 4);
    db.insert(MDDS_ASCII("abc"), 5);
    db.insert(MDDS_ASCII("bc"),  6);
    db.insert(MDDS_ASCII("bcd"), 7);

    packed = db.pack();
    it = packed.begin();
    ite = packed.end();

    assert(*it == kv("a", 1));
    assert(*(++it) == kv("ab", 2));
    assert(*(++it) == kv("aba", 3));
    assert(*(++it) == kv("abb", 4));
    assert(*(++it) == kv("abc", 5));
    assert(*(++it) == kv("bc",  6));
    assert(*(++it) == kv("bcd", 7));
    assert(it->first == "bcd");
    assert(it->second == 7);
    ++it;
    assert(it == ite);

    --it;
    assert(it != ite);
    assert(*it == kv("bcd", 7));
    --it;
    assert(*it == kv("bc", 6));
    --it;
    assert(*it == kv("abc", 5));
    --it;
    assert(*it == kv("abb", 4));
    --it;
    assert(*it == kv("aba", 3));
    --it;
    assert(*it == kv("ab", 2));
    assert(*(--it) == kv("a", 1));
    assert(it == packed.begin());

    assert(*(++it) == kv("ab",  2));
    assert(*(++it) == kv("aba", 3));
    --it;
    assert(*it == kv("ab", 2));
    --it;
    assert(*it == kv("a",  1));
    ++it;
    assert(*it == kv("ab", 2));
    ++it;
    assert(*it == kv("aba", 3));

    // Post-decrement operator.
    assert(*it-- == kv("aba", 3));
    assert(*it == kv("ab", 2));
}

void trie_test1()
{
    stack_printer __stack_printer__("::trie_test1");

    typedef trie_map<trie::std_string_trait, custom_string> trie_map_type;
    typedef packed_trie_map<trie::std_string_trait, custom_string> packed_trie_map_type;

    custom_string null_value("-");
    trie_map_type db(null_value);

    assert(db.size() == 0);
    db.insert(MDDS_ASCII("Barak"), custom_string("Obama"));
    assert(db.size() == 1);
    db.insert(MDDS_ASCII("Bob"), custom_string("Marley"));
    assert(db.size() == 2);
    db.insert(MDDS_ASCII("Hideki"), custom_string("Matsui"));
    assert(db.size() == 3);

    auto it = db.find(MDDS_ASCII("Barak"));
    assert(it->first == "Barak");
    custom_string res = it->second;
    assert(res.data == "Obama");

    res = db.find(MDDS_ASCII("Bob"))->second;
    assert(res.data == "Marley");
    res = db.find(MDDS_ASCII("Hideki"))->second;
    assert(res.data == "Matsui");

    // Non-existent key.
    it = db.find(MDDS_ASCII("Von"));
    assert(it == db.end());
    it = db.find(MDDS_ASCII("Bar"));
    assert(it == db.end());

    // Perform prefix search on "B", which should return both "Barak" and "Bob".
    // The results should be sorted.
    {
        auto matches = db.prefix_search(MDDS_ASCII("B"));
        assert(matches.size() == 2);
        assert(matches[0].first == "Barak");
        assert(matches[0].second.data == "Obama");
        assert(matches[1].first == "Bob");
        assert(matches[1].second.data == "Marley");

        matches = db.prefix_search(MDDS_ASCII("Hi"));
        assert(matches.size() == 1);
        assert(matches[0].first == "Hideki");
        assert(matches[0].second.data == "Matsui");

        // Invalid prefix searches.
        matches = db.prefix_search(MDDS_ASCII("Bad"));
        assert(matches.empty());
        matches = db.prefix_search(MDDS_ASCII("Foo"));
        assert(matches.empty());
    }

    {
        // Create a packed version from it, and make sure it still generates the
        // same results.
        packed_trie_map_type packed(db);
        assert(packed.size() == db.size());

        auto matches = packed.prefix_search(MDDS_ASCII("B"));
        assert(matches.size() == 2);
        assert(matches[0].first == "Barak");
        assert(matches[0].second.data == "Obama");
        assert(matches[1].first == "Bob");
        assert(matches[1].second.data == "Marley");

        matches = db.prefix_search(MDDS_ASCII("Hi"));
        assert(matches.size() == 1);
        assert(matches[0].first == "Hideki");
        assert(matches[0].second.data == "Matsui");

        // Invalid prefix searches.
        matches = db.prefix_search(MDDS_ASCII("Bad"));
        assert(matches.empty());
        matches = db.prefix_search(MDDS_ASCII("Foo"));
        assert(matches.empty());
    }

    {
        auto packed = db.pack();
        auto matches = packed.prefix_search(MDDS_ASCII("B"));
        assert(matches.size() == 2);
        assert(matches[0].first == "Barak");
        assert(matches[0].second.data == "Obama");
        assert(matches[1].first == "Bob");
        assert(matches[1].second.data == "Marley");
    }

    // Erase an existing key.
    bool erased = db.erase(MDDS_ASCII("Hideki"));
    assert(erased);
    assert(db.size() == 2);

    it = db.find(MDDS_ASCII("Hideki"));
    assert(it == db.end());

    // Try to erase a key that doesn't exist.
    erased = db.erase(MDDS_ASCII("Foo"));
    assert(!erased);
    assert(db.size() == 2);

    // Clear the whole thing.
    db.clear();
    assert(db.size() == 0);
}

void trie_test_iterator_empty()
{
    stack_printer __stack_printer__("::trie_test_iterator_empty");
    typedef trie_map<trie::std_string_trait, int> trie_map_type;
    trie_map_type db(-1);

    // empty container
    trie_map_type::const_iterator it = db.begin();
    trie_map_type::const_iterator ite = db.end();

    assert(it == ite);
}

void trie_test_iterator()
{
    stack_printer __stack_printer__("::trie_test_iterator");
    typedef trie_map<trie::std_string_trait, int> trie_map_type;
    using kv = trie_map_type::key_value_type;
    trie_map_type db(-1);

    // empty container
    trie_map_type::const_iterator it = db.begin();
    trie_map_type::const_iterator ite = db.end();

    // The end iterator will never get invalidated since it only references
    // the root node which will never get modified as long as the parent
    // container is alive.

    assert(it == ite);

    db.insert(MDDS_ASCII("a"), 1);
    it = db.begin();
    assert(it != ite);
    assert(*it == kv("a", 1));
    ++it;
    assert(it == ite);

    db.insert(MDDS_ASCII("ab"), 2);
    it = db.begin();
    assert(it != ite);
    assert(*it == kv("a", 1));
    ++it;
    assert(it != ite);
    assert(*it == kv("ab", 2));
    ++it;
    assert(it == ite);

    db.insert(MDDS_ASCII("aba"), 3);
    db.insert(MDDS_ASCII("abb"), 4);
    db.insert(MDDS_ASCII("abc"), 5);
    db.insert(MDDS_ASCII("bc"),  6);
    db.insert(MDDS_ASCII("bcd"), 7);

    it = db.begin();
    assert(*it == kv("a", 1));
    assert(*(++it) == kv("ab", 2));
    assert(*(++it) == kv("aba", 3));
    assert(*(++it) == kv("abb", 4));
    assert(*(++it) == kv("abc", 5));
    assert(*(++it) == kv("bc",  6));
    assert(*(++it) == kv("bcd", 7));
    assert(it->first == "bcd");
    assert(it->second == 7);
    ++it;
    assert(it == ite);

    --it;
    assert(it != ite);
    assert(*it == kv("bcd", 7));
    --it;
    assert(*it == kv("bc", 6));
    --it;
    assert(*it == kv("abc", 5));
    --it;
    assert(*it == kv("abb", 4));
    --it;
    assert(*it == kv("aba", 3));
    --it;
    assert(*it == kv("ab", 2));
    assert(*(--it) == kv("a", 1));
    assert(it == db.begin());

    assert(*(++it) == kv("ab",  2));
    assert(*(++it) == kv("aba", 3));
    --it;
    assert(*it == kv("ab", 2));
    --it;
    assert(*it == kv("a",  1));
    ++it;
    assert(*it == kv("ab", 2));
    ++it;
    assert(*it == kv("aba", 3));
}

void trie_test_iterator_with_erase()
{
    stack_printer __stack_printer__("::trie_test_iterator_with_erase");
    typedef trie_map<trie::std_string_trait, int> trie_map_type;
    using kv = trie_map_type::key_value_type;
    trie_map_type db(-1);

    db.insert(MDDS_ASCII("Python"), 1);
    db.insert(MDDS_ASCII("C++"),    2);

    auto it = db.begin(), ite = db.end();
    assert(*it++ == kv("C++",    2));
    assert(*it++ == kv("Python", 1));
    assert(it == ite);

    db.erase(MDDS_ASCII("C++"));
    it = db.begin();
    assert(*it++ == kv("Python", 1));
    assert(it == ite);
    assert(*(--it) == kv("Python", 1));
    assert(it == db.begin());

    db.clear();
    assert(db.begin() == db.end());

    db.insert(MDDS_ASCII("A"),   1);
    db.insert(MDDS_ASCII("AB"),  2);
    db.insert(MDDS_ASCII("ABC"), 3);
    db.erase(MDDS_ASCII("AB"));

    it = db.begin();
    assert(*it++ == kv("A",  1));
    assert(*it++ == kv("ABC", 3));
    assert(it == ite);

    assert(*(--it) == kv("ABC", 3));
    assert(*(--it) == kv("A", 1));
    assert(it == db.begin());

    db.clear();
    db.insert(MDDS_ASCII("A"),   1);
    db.insert(MDDS_ASCII("AB"),  2);
    db.insert(MDDS_ASCII("ABC"), 3);
    db.erase(MDDS_ASCII("ABC"));

    it = db.begin();
    assert(*it++ == kv("A",  1));
    assert(*it++ == kv("AB", 2));
    assert(it == ite);

    assert(*(--it) == kv("AB", 2));
    assert(*(--it) == kv("A",  1));
    assert(it == db.begin());

    it = ite;
    --it;
    assert(*it-- == kv("AB", 2)); // test post-decrement operator.
    assert(*it == kv("A",  1));
}

void trie_test_find_iterator()
{
    stack_printer __stack_printer__("::trie_test_find_iterator");
    typedef trie_map<trie::std_string_trait, int> trie_map_type;
    trie_map_type db(-1);
    db.insert(MDDS_ASCII("a"),  1);
    db.insert(MDDS_ASCII("aa"), 2);
    db.insert(MDDS_ASCII("ab"), 3);
    db.insert(MDDS_ASCII("b"),  4);
    {
        auto it = db.find(MDDS_ASCII("a"));
        assert(it->first == "a");
        assert(it->second == 1);
        ++it;
        assert(it->first == "aa");
        assert(it->second == 2);
        ++it;
        assert(it->first == "ab");
        assert(it->second == 3);
        ++it;
        assert(it->first == "b");
        assert(it->second == 4);
        ++it;
        assert(it == db.end());
    }

    trie_map_type::packed_type packed = db.pack();
    {
        auto it = packed.find(MDDS_ASCII("a"));
        assert(it->first == "a");
        assert(it->second == 1);
        ++it;
        assert(it->first == "aa");
        assert(it->second == 2);
        ++it;
        assert(it->first == "ab");
        assert(it->second == 3);
        ++it;
        assert(it->first == "b");
        assert(it->second == 4);
        ++it;
        assert(it == packed.end());
    }
}

int main(int argc, char** argv)
{
    trie_packed_test1();
    trie_packed_test2();
    trie_packed_test3();
    trie_packed_test4();
    trie_packed_test_value_life_cycle();
    trie_packed_test_custom_string();
    trie_packed_test_iterator_empty();
    trie_packed_test_iterator();

    trie_test1();

    trie_test_iterator_empty();
    trie_test_iterator();
    trie_test_iterator_with_erase();
    trie_test_find_iterator();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
