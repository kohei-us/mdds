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

#include "mdds/packed_trie_map.hpp"
#include "mdds/global.hpp"
#include "test_global.hpp"

using namespace std;

typedef mdds::draft::packed_trie_map<
    mdds::draft::trie::std_string_trait, int> int_map_type;

bool verify_entries(
    const int_map_type& db, const int_map_type::entry* entries, size_t entry_size)
{
    auto items = db.prefix_search(nullptr, 0);
    for (size_t i = 0, n = items.size(); i < n; ++i)
        cout << items[i].first << ": " << items[i].second << endl;

    const int_map_type::entry* p = entries;
    const int_map_type::entry* p_end = p + entry_size;
    for (; p != p_end; ++p)
    {
        int res = db.find(p->key, p->keylen);
        if (res != p->value)
            return false;
    }

    return true;
}

void trie_test1()
{
    stack_printer __stack_printer__("::trie_test1");

    int_map_type::entry entries[] =
    {
        { MDDS_ASCII("a"),  13 },
        { MDDS_ASCII("aa"), 10 },
        { MDDS_ASCII("ab"), 3 },
        { MDDS_ASCII("b"),  7 },
    };

    size_t entry_size = MDDS_N_ELEMENTS(entries);
    int_map_type db(entries, entry_size, -1);
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

void trie_test2()
{
    stack_printer __stack_printer__("::trie_test2");

    int_map_type::entry entries[] =
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
    int_map_type db(entries, entry_size, -1);
    assert(db.size() == 12);
    assert(verify_entries(db, entries, entry_size));

    // invalid keys
    assert(db.find(MDDS_ASCII("aarons")) == -1);
    assert(db.find(MDDS_ASCII("a")) == -1);
    assert(db.find(MDDS_ASCII("biso")) == -1);
    assert(db.find(MDDS_ASCII("dAvid")) == -1);
}

void trie_test3()
{
    stack_printer __stack_printer__("::trie_test3");

    int_map_type::entry entries[] =
    {
        { MDDS_ASCII("NULL"), 1 },
        { MDDS_ASCII("Null"), 2 },
        { MDDS_ASCII("null"), 3 },
        { MDDS_ASCII("~"),    4 },
    };

    size_t entry_size = MDDS_N_ELEMENTS(entries);
    int_map_type db(entries, entry_size, -1);
    assert(db.size() == 4);
    assert(verify_entries(db, entries, entry_size));

    // invalid keys
    assert(db.find(MDDS_ASCII("NUll")) == -1);
    assert(db.find(MDDS_ASCII("Oull")) == -1);
    assert(db.find(MDDS_ASCII("Mull")) == -1);
    assert(db.find(MDDS_ASCII("hell")) == -1);
}

void trie_test4()
{
    stack_printer __stack_printer__("::trie_test4");

    enum name_type {
        name_none = 0,
        name_andy,
        name_bruce,
        name_charlie,
        name_david
    };

    int_map_type::entry entries[] =
    {
        { MDDS_ASCII("andy"),    name_andy },
        { MDDS_ASCII("andy1"),   name_andy },
        { MDDS_ASCII("andy13"),  name_andy },
        { MDDS_ASCII("bruce"),   name_bruce },
        { MDDS_ASCII("charlie"), name_charlie },
        { MDDS_ASCII("david"),   name_david },
    };

    size_t entry_size = MDDS_N_ELEMENTS(entries);
    int_map_type db(entries, entry_size, name_none);
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

typedef mdds::draft::packed_trie_map<
    mdds::draft::trie::std_string_trait, value_wrapper> value_map_type;

void trie_test_value_life_cycle()
{
    stack_printer __stack_printer__("::trie_test_value_life_cycle");

    using entry = value_map_type::entry;

    // Entries must be sorted by the key!
    std::unique_ptr<vector<entry>> entries(new vector<entry>);
    entries->push_back(entry(MDDS_ASCII("fifteen"), value_wrapper(15)));
    entries->push_back(entry(MDDS_ASCII("ten"), value_wrapper(10)));
    entries->push_back(entry(MDDS_ASCII("twelve"), value_wrapper(12)));
    entries->push_back(entry(MDDS_ASCII("two"), value_wrapper(2)));

    value_map_type db(entries->data(), entries->size(), value_wrapper(-1));

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

int main(int argc, char** argv)
{
    trie_test1();
    trie_test2();
    trie_test3();
    trie_test4();
    trie_test_value_life_cycle();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
