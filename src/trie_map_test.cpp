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
using namespace mdds::draft;

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
        int res = db.find(p->key, p->keylen);
        if (res != p->value)
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
    assert(db.find(MDDS_ASCII("ac")) == -1);
    assert(db.find(MDDS_ASCII("c")) == -1);

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
    assert(db.find(MDDS_ASCII("aarons")) == -1);
    assert(db.find(MDDS_ASCII("a")) == -1);
    assert(db.find(MDDS_ASCII("biso")) == -1);
    assert(db.find(MDDS_ASCII("dAvid")) == -1);
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
    assert(db.find(MDDS_ASCII("NUll")) == -1);
    assert(db.find(MDDS_ASCII("Oull")) == -1);
    assert(db.find(MDDS_ASCII("Mull")) == -1);
    assert(db.find(MDDS_ASCII("hell")) == -1);
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
    assert(db.find("foo", 3) == name_none);
    assert(db.find("andy133", 7) == name_none);

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

    value_wrapper r = db.find(MDDS_ASCII("twelve"));
    assert(r.value == 12);

    r = db.find(MDDS_ASCII("two"));
    assert(r.value == 2);

    r = db.find(MDDS_ASCII("foo"));
    assert(r.value == -1);
}

struct custom_string
{
    std::string data;

    custom_string() {}
    custom_string(const std::string& _data) : data(_data) {}
};

struct custom_string_trait
{
    typedef uint16_t char_type;
    typedef custom_string string_type;
    typedef std::vector<char_type> buffer_type;

    static buffer_type init_buffer(const char_type* str, size_t length)
    {
        buffer_type buf;
        const char_type* str_end = str + length;
        for (; str != str_end; ++str)
            buf.push_back(*str);

        return buf;
    }

    static void push_back(buffer_type& buffer, char_type c)
    {
        buffer.push_back(c);
    }

    static void pop_back(buffer_type& buffer)
    {
        buffer.pop_back();
    }

    static string_type to_string(const buffer_type& buf)
    {
        // Cast all uint16_t chars to regular chars.
        string_type s;

        std::for_each(buf.begin(), buf.end(),
            [&](char_type c)
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
        std::string v = db.find(entries[i].key, entries[i].keylen);
        cout << v << endl;
        assert(v == entries[i].value);
    }

    // Find all keys that start with 'M'.
    std::vector<packed_custom_str_map_type::key_value_type> vs = db.prefix_search(key_max, 1);
    assert(vs.size() == 2);
    assert(vs[0].first.data == vs[0].second);
    assert(vs[0].second == "Max");
    assert(vs[1].first.data == vs[1].second);
    assert(vs[1].second == "Ming");
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

    custom_string res = db.find(MDDS_ASCII("Barak"));
    assert(res.data == "Obama");
    res = db.find(MDDS_ASCII("Bob"));
    assert(res.data == "Marley");
    res = db.find(MDDS_ASCII("Hideki"));
    assert(res.data == "Matsui");

    // Non-existent key.
    res = db.find(MDDS_ASCII("Von"));
    assert(res.data == "-");
    res = db.find(MDDS_ASCII("Bar"));
    assert(res.data == "-");

    // Perform prefix search on "B", which should return both "Barak" and "Bob".
    // The results should be sorted.
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

    packed_trie_map_type packed(db);
    assert(packed.size() == db.size());

    // Erase an existing key.
    bool erased = db.erase(MDDS_ASCII("Hideki"));
    assert(erased);
    assert(db.size() == 2);

    res = db.find(MDDS_ASCII("Hideki"));
    assert(res.data == null_value.data);

    // Try to erase a key that doesn't exist.
    erased = db.erase(MDDS_ASCII("Foo"));
    assert(!erased);
    assert(db.size() == 2);

    // Clear the whole thing.
    db.clear();
    assert(db.size() == 0);
}

int main(int argc, char** argv)
{
    trie_packed_test1();
    trie_packed_test2();
    trie_packed_test3();
    trie_packed_test4();
    trie_packed_test_value_life_cycle();
    trie_packed_test_custom_string();

    trie_test1();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
