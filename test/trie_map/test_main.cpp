/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2015 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_main.hpp"

#include <iterator>
#include <fstream>
#include <vector>

using namespace std;
using namespace mdds;

template<typename MapT>
bool verify_entries(const MapT& db, const typename MapT::entry* entries, std::size_t entry_size)
{
    auto results = db.prefix_search(nullptr, 0);
    for (auto it = results.begin(), ite = results.end(); it != ite; ++it)
        std::cout << it->first << ": " << it->second << std::endl;

    const auto* p = entries;
    const auto* p_end = p + entry_size;
    for (; p != p_end; ++p)
    {
        auto it = db.find(p->key, p->keylen);
        if (it == db.end() || it->second != p->value)
            return false;
    }

    return true;
}

template<typename T1, typename T2>
bool check_equal(const T1& left, const T2& right)
{
    if (left.first != right.first)
    {
        cout << "left: " << left.first << "; right: " << right.first << endl;
        return false;
    }

    if (left.second != right.second)
    {
        cout << "left: " << left.second << "; right: " << right.second << endl;
        return false;
    }

    return true;
}

void trie_packed_test1()
{
    MDDS_TEST_FUNC_SCOPE;

    using _map_type = mdds::packed_trie_map<std::string, int, local_debug_traits>;

    _map_type::entry entries[] = {
        {MDDS_ASCII("a"), 13},
        {MDDS_ASCII("aa"), 10},
        {MDDS_ASCII("ab"), 3},
        {MDDS_ASCII("b"), 7},
    };

    size_t entry_size = std::size(entries);
    _map_type db(entries, entry_size);
    TEST_ASSERT(db.size() == 4);
    TEST_ASSERT(verify_entries(db, entries, entry_size));

    // invalid keys
    TEST_ASSERT(db.find(MDDS_ASCII("ac")) == db.end());
    TEST_ASSERT(db.find(MDDS_ASCII("c")) == db.end());

    {
        // Get all key-value pairs.
        auto results = db.prefix_search(nullptr, 0);
        size_t n = std::distance(results.begin(), results.end());
        TEST_ASSERT(n == 4);
        auto it = results.begin();
        TEST_ASSERT(it->first == "a");
        TEST_ASSERT(it->second == 13);
        ++it;
        TEST_ASSERT(it->first == "aa");
        TEST_ASSERT(it->second == 10);
        ++it;
        TEST_ASSERT(it->first == "ab");
        TEST_ASSERT(it->second == 3);
        ++it;
        TEST_ASSERT(it->first == "b");
        TEST_ASSERT(it->second == 7);
        ++it;
        TEST_ASSERT(it == results.end());
    }

    {
        auto it = db.find(MDDS_ASCII("a"));
        TEST_ASSERT(it->first == "a");
        TEST_ASSERT(it->second == 13);
        ++it;
        TEST_ASSERT(it->first == "aa");
        TEST_ASSERT(it->second == 10);
        ++it;
        TEST_ASSERT(it->first == "ab");
        TEST_ASSERT(it->second == 3);
        ++it;
        TEST_ASSERT(it->first == "b");
        TEST_ASSERT(it->second == 7);
        ++it;
        TEST_ASSERT(it == db.end());
    }
}

void trie_packed_test2()
{
    MDDS_TEST_FUNC_SCOPE;

    using _map_type = mdds::packed_trie_map<std::string, int, local_debug_traits>;

    _map_type::entry entries[] = {
        {MDDS_ASCII("aaron"), 0}, {MDDS_ASCII("al"), 1},    {MDDS_ASCII("aldi"), 2},    {MDDS_ASCII("andy"), 3},
        {MDDS_ASCII("bison"), 4}, {MDDS_ASCII("bruce"), 5}, {MDDS_ASCII("charlie"), 6}, {MDDS_ASCII("charlotte"), 7},
        {MDDS_ASCII("david"), 8}, {MDDS_ASCII("dove"), 9},  {MDDS_ASCII("e"), 10},      {MDDS_ASCII("eva"), 11},
    };

    size_t entry_size = std::size(entries);
    _map_type db(entries, entry_size);
    TEST_ASSERT(db.size() == 12);
    TEST_ASSERT(verify_entries(db, entries, entry_size));

    // invalid keys
    TEST_ASSERT(db.find(MDDS_ASCII("aarons")) == db.end());
    TEST_ASSERT(db.find(MDDS_ASCII("a")) == db.end());
    TEST_ASSERT(db.find(MDDS_ASCII("biso")) == db.end());
    TEST_ASSERT(db.find(MDDS_ASCII("dAvid")) == db.end());
}

void trie_packed_test3()
{
    MDDS_TEST_FUNC_SCOPE;

    using _map_type = mdds::packed_trie_map<std::string, int, local_debug_traits>;

    _map_type::entry entries[] = {
        {MDDS_ASCII("NULL"), 1},
        {MDDS_ASCII("Null"), 2},
        {MDDS_ASCII("null"), 3},
        {MDDS_ASCII("~"), 4},
    };

    size_t entry_size = std::size(entries);
    _map_type db(entries, entry_size);
    TEST_ASSERT(db.size() == 4);
    TEST_ASSERT(verify_entries(db, entries, entry_size));

    // invalid keys
    TEST_ASSERT(db.find(MDDS_ASCII("NUll")) == db.end());
    TEST_ASSERT(db.find(MDDS_ASCII("Oull")) == db.end());
    TEST_ASSERT(db.find(MDDS_ASCII("Mull")) == db.end());
    TEST_ASSERT(db.find(MDDS_ASCII("hell")) == db.end());
}

void trie_packed_test4()
{
    MDDS_TEST_FUNC_SCOPE;

    enum name_type
    {
        name_none = 0,
        name_andy,
        name_bruce,
        name_charlie,
        name_david
    };

    using _map_type = mdds::packed_trie_map<std::string, name_type, local_debug_traits>;

    _map_type::entry entries[] = {
        {MDDS_ASCII("andy"), name_andy},   {MDDS_ASCII("andy1"), name_andy},      {MDDS_ASCII("andy13"), name_andy},
        {MDDS_ASCII("bruce"), name_bruce}, {MDDS_ASCII("charlie"), name_charlie}, {MDDS_ASCII("david"), name_david},
    };

    size_t entry_size = std::size(entries);
    _map_type db(entries, entry_size);
    TEST_ASSERT(db.size() == 6);
    TEST_ASSERT(verify_entries(db, entries, entry_size));

    // Try invalid keys.
    TEST_ASSERT(db.find("foo", 3) == db.end());
    TEST_ASSERT(db.find("andy133", 7) == db.end());

    // Test prefix search on 'andy'.
    auto results = db.prefix_search(MDDS_ASCII("andy"));
    size_t n = std::distance(results.begin(), results.end());
    TEST_ASSERT(n == 3);
    auto it = results.begin();
    TEST_ASSERT(it->first == "andy");
    ++it;
    TEST_ASSERT(it->first == "andy1");
    ++it;
    TEST_ASSERT(it->first == "andy13");
    ++it;
    TEST_ASSERT(it == results.end());

    results = db.prefix_search(MDDS_ASCII("andy's toy"));
    n = std::distance(results.begin(), results.end());
    TEST_ASSERT(n == 0);

    results = db.prefix_search(MDDS_ASCII("e"));
    n = std::distance(results.begin(), results.end());
    TEST_ASSERT(n == 0);

    results = db.prefix_search(MDDS_ASCII("b"));
    n = std::distance(results.begin(), results.end());
    TEST_ASSERT(n == 1);
    it = results.begin();
    TEST_ASSERT(it->first == "bruce");
    TEST_ASSERT(it->second == name_bruce);
    ++it;
    TEST_ASSERT(it == results.end());
}

struct value_wrapper
{
    int value;

    value_wrapper() : value(0)
    {}
    value_wrapper(int _value) : value(_value)
    {}
};

std::ostream& operator<<(std::ostream& os, const value_wrapper& vw)
{
    os << vw.value;
    return os;
}

typedef packed_trie_map<std::string, value_wrapper> packed_value_map_type;

void trie_packed_test_value_life_cycle()
{
    MDDS_TEST_FUNC_SCOPE;

    using entry = packed_value_map_type::entry;

    // Entries must be sorted by the key!
    std::unique_ptr<vector<entry>> entries(new vector<entry>);
    entries->push_back(entry(MDDS_ASCII("fifteen"), value_wrapper(15)));
    entries->push_back(entry(MDDS_ASCII("ten"), value_wrapper(10)));
    entries->push_back(entry(MDDS_ASCII("twelve"), value_wrapper(12)));
    entries->push_back(entry(MDDS_ASCII("two"), value_wrapper(2)));

    packed_value_map_type db(entries->data(), entries->size());

    // Delete the original entry store.
    entries.reset();

    auto results = db.prefix_search(nullptr, 0);
    std::for_each(results.begin(), results.end(), [](const packed_value_map_type::const_iterator::value_type& v) {
        cout << v.first << ": " << v.second.value << endl;
    });

    auto it = db.find(MDDS_ASCII("twelve"));
    TEST_ASSERT(it->second.value == 12);

    it = db.find(MDDS_ASCII("two"));
    TEST_ASSERT(it->second.value == 2);

    it = db.find(MDDS_ASCII("foo"));
    TEST_ASSERT(it == db.end());
}

struct custom_string
{
    std::string data;

    custom_string()
    {}
    custom_string(const std::string& _data) : data(_data)
    {}
};

class custom_string_16
{
    using buffer_type = std::vector<std::uint16_t>;
    buffer_type m_buffer;

public:
    using value_type = buffer_type::value_type;
    using size_type = buffer_type::size_type;

    custom_string_16() = default;

    custom_string_16(const value_type* p, size_type n) : m_buffer(p, p + n)
    {}

    const value_type* data() const
    {
        return m_buffer.data();
    }

    value_type* data()
    {
        return m_buffer.data();
    }

    void push_back(value_type c)
    {
        m_buffer.push_back(c);
    }

    void pop_back()
    {
        m_buffer.pop_back();
    }

    size_type size() const
    {
        return m_buffer.size();
    }
};

std::ostream& operator<<(std::ostream& os, const custom_string_16& v)
{
    os << "(custom-string: size=" << v.size() << ")";
    return os;
}

using packed_custom_str_map_type = packed_trie_map<custom_string_16, std::string>;

void trie_packed_test_custom_string()
{
    MDDS_TEST_FUNC_SCOPE;

    const uint16_t key_alex[] = {0x41, 0x6C, 0x65, 0x78};
    const uint16_t key_bob[] = {0x42, 0x6F, 0x62};
    const uint16_t key_max[] = {0x4D, 0x61, 0x78};
    const uint16_t key_ming[] = {0x4D, 0x69, 0x6E, 0x67};

    const packed_custom_str_map_type::entry entries[] = {
        {key_alex, 4, "Alex"},
        {key_bob, 3, "Bob"},
        {key_max, 3, "Max"},
        {key_ming, 4, "Ming"},
    };

    size_t n_entries = std::size(entries);
    packed_custom_str_map_type db(entries, n_entries);

    for (size_t i = 0; i < n_entries; ++i)
    {
        auto it = db.find(entries[i].key, entries[i].keylen);
        cout << it->second << endl;
        TEST_ASSERT(it->second == entries[i].value);
    }

    // Find all keys that start with 'M'.
    auto results = db.prefix_search(key_max, 1);
    size_t n = std::distance(results.begin(), results.end());
    TEST_ASSERT(n == 2);
    auto it = results.begin();
    TEST_ASSERT(it->second == "Max");
    ++it;
    TEST_ASSERT(it->second == "Ming");
    ++it;
    TEST_ASSERT(it == results.end());
}

void trie_packed_test_iterator_empty()
{
    MDDS_TEST_FUNC_SCOPE;

    packed_int_map_type db(nullptr, 0);

    // empty container
    packed_int_map_type::const_iterator it = db.begin();
    packed_int_map_type::const_iterator ite = db.end();

    TEST_ASSERT(it == ite);
}

void trie_packed_test_iterator()
{
    MDDS_TEST_FUNC_SCOPE;

    using trie_map_type = trie_map<std::string, int>;
    using packed_type = trie_map_type::packed_type;
    using kv = std::pair<std::string, int>;

    trie_map_type db;

    db.insert(MDDS_ASCII("a"), 1);
    packed_type packed = db.pack();
    TEST_ASSERT(db.size() == packed.size());
    packed_type::const_iterator it = packed.begin();
    packed_type::const_iterator ite = packed.end();
    TEST_ASSERT(it != ite);
    TEST_ASSERT(it->first == "a");
    TEST_ASSERT(it->second == 1);

    db.insert(MDDS_ASCII("ab"), 2);
    packed = db.pack(); // this invalidates the end position.
    TEST_ASSERT(db.size() == packed.size());

    it = packed.begin();
    ite = packed.end();
    TEST_ASSERT(it != ite);
    TEST_ASSERT(it->first == "a");
    TEST_ASSERT(it->second == 1);

    ++it;
    bool check_true = check_equal(*it++, kv("ab", 2));
    TEST_ASSERT(check_true);
    TEST_ASSERT(it == ite);

    db.insert(MDDS_ASCII("aba"), 3);
    db.insert(MDDS_ASCII("abb"), 4);
    db.insert(MDDS_ASCII("abc"), 5);
    db.insert(MDDS_ASCII("bc"), 6);
    db.insert(MDDS_ASCII("bcd"), 7);

    packed = db.pack();
    TEST_ASSERT(db.size() == packed.size());

    it = packed.begin();
    ite = packed.end();

    TEST_ASSERT(*it == kv("a", 1));
    TEST_ASSERT(check_equal(*(++it), kv("ab", 2)));
    TEST_ASSERT(check_equal(*(++it), kv("aba", 3)));
    TEST_ASSERT(check_equal(*(++it), kv("abb", 4)));
    TEST_ASSERT(check_equal(*(++it), kv("abc", 5)));
    TEST_ASSERT(check_equal(*(++it), kv("bc", 6)));
    TEST_ASSERT(check_equal(*(++it), kv("bcd", 7)));
    TEST_ASSERT(it->first == "bcd");
    TEST_ASSERT(it->second == 7);
    ++it;
    TEST_ASSERT(it == ite);

    --it;
    TEST_ASSERT(it != ite);
    TEST_ASSERT(check_equal(*it, kv("bcd", 7)));
    --it;
    TEST_ASSERT(check_equal(*it, kv("bc", 6)));
    --it;
    TEST_ASSERT(check_equal(*it, kv("abc", 5)));
    --it;
    TEST_ASSERT(check_equal(*it, kv("abb", 4)));
    --it;
    TEST_ASSERT(check_equal(*it, kv("aba", 3)));
    --it;
    TEST_ASSERT(check_equal(*it, kv("ab", 2)));
    TEST_ASSERT(check_equal(*(--it), kv("a", 1)));
    TEST_ASSERT(it == packed.begin());

    TEST_ASSERT(check_equal(*(++it), kv("ab", 2)));
    TEST_ASSERT(check_equal(*(++it), kv("aba", 3)));
    --it;
    TEST_ASSERT(check_equal(*it, kv("ab", 2)));
    --it;
    TEST_ASSERT(check_equal(*it, kv("a", 1)));
    ++it;
    TEST_ASSERT(check_equal(*it, kv("ab", 2)));
    ++it;
    TEST_ASSERT(check_equal(*it, kv("aba", 3)));

    // Post-decrement operator.
    TEST_ASSERT(check_equal(*it--, kv("aba", 3)));
    TEST_ASSERT(check_equal(*it, kv("ab", 2)));
}

void trie_packed_test_prefix_search1()
{
    MDDS_TEST_FUNC_SCOPE;

    using trie_map_type = trie_map<std::string, int>;
    using packed_type = trie_map_type::packed_type;

    trie_map_type db;
    db.insert(MDDS_ASCII("andy"), 1);
    db.insert(MDDS_ASCII("andy1"), 2);
    db.insert(MDDS_ASCII("andy12"), 3);

    {
        auto results = db.prefix_search(MDDS_ASCII("andy"));
        auto it = results.begin();
        TEST_ASSERT(it != results.end());
        TEST_ASSERT(it->first == "andy");
        ++it;
        TEST_ASSERT(it->first == "andy1");
        ++it;
        TEST_ASSERT(it->first == "andy12");
        ++it;
        TEST_ASSERT(it == results.end());

        size_t n = std::distance(results.begin(), results.end());
        TEST_ASSERT(n == 3);
    }

    packed_type packed = db.pack();
    {
        auto results = packed.prefix_search(MDDS_ASCII("andy"));
        auto it = results.begin();
        TEST_ASSERT(it != results.end());
        TEST_ASSERT(it->first == "andy");
        ++it;
        TEST_ASSERT(it->first == "andy1");
        ++it;
        TEST_ASSERT(it->first == "andy12");
        ++it;
        TEST_ASSERT(it == results.end());

        size_t n = std::distance(results.begin(), results.end());
        TEST_ASSERT(n == 3);
    }
}

void trie_packed_test_key_as_input()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef trie_map<std::string, int> trie_map_type;
    trie_map_type db;

    db.insert(std::string("string as key"), 1);
    db.insert("literal as key", 2);
    auto packed = db.pack();

    auto it = packed.find("literal as key");
    TEST_ASSERT(it != packed.end());
    TEST_ASSERT(it->first == "literal as key");
    TEST_ASSERT(it->second == 2);

    auto results = packed.prefix_search("str");
    auto rit = results.begin();
    TEST_ASSERT(rit != results.end());
    TEST_ASSERT(rit->first == "string as key");
    TEST_ASSERT(rit->second == 1);
    ++rit;
    TEST_ASSERT(rit == results.end());
}

void trie_packed_test_copying()
{
    MDDS_TEST_FUNC_SCOPE;

    using map_type = packed_trie_map<std::string, int>;

    map_type::entry entries[] = {
        {MDDS_ASCII("aaron"), 0}, {MDDS_ASCII("al"), 1},    {MDDS_ASCII("aldi"), 2},    {MDDS_ASCII("andy"), 3},
        {MDDS_ASCII("bison"), 4}, {MDDS_ASCII("bruce"), 5}, {MDDS_ASCII("charlie"), 6}, {MDDS_ASCII("charlotte"), 7},
        {MDDS_ASCII("david"), 8}, {MDDS_ASCII("dove"), 9},  {MDDS_ASCII("e"), 10},      {MDDS_ASCII("eva"), 11},
    };

    auto verify_content = [&entries](const map_type& db) {
        auto it = db.begin();
        const map_type::entry* p_entries = entries;
        const map_type::entry* p_entries_end = p_entries + db.size();
        size_t n = std::distance(p_entries, p_entries_end);
        TEST_ASSERT(db.size() == n);
        TEST_ASSERT(!db.empty());

        for (; p_entries != p_entries_end; ++p_entries, ++it)
        {
            std::string key_expected(p_entries->key, p_entries->keylen);
            TEST_ASSERT(key_expected == it->first);
            TEST_ASSERT(p_entries->value == it->second);
        }
    };

    auto db = std::make_unique<map_type>(entries, std::size(entries));
    auto db_copied(*db);
    TEST_ASSERT(*db == db_copied);
    TEST_ASSERT(db->size() == db_copied.size());
    db.reset();

    auto it = db_copied.find("charlie");
    TEST_ASSERT(it != db_copied.end());
    TEST_ASSERT(it->first == "charlie");
    TEST_ASSERT(it->second == 6);

    verify_content(db_copied);

    auto db_moved(std::move(db_copied));
    TEST_ASSERT(db_copied.empty());
    TEST_ASSERT(!db_moved.empty());
    TEST_ASSERT(db_moved.size() == std::size(entries));

    it = db_copied.find("bison");
    TEST_ASSERT(it == db_copied.end());
    it = db_moved.find("bison");
    TEST_ASSERT(it != db_moved.end());
    TEST_ASSERT(it->first == "bison");
    TEST_ASSERT(it->second == 4);

    verify_content(db_moved);

    map_type db_copy_assigned;
    TEST_ASSERT(db_copy_assigned.empty());
    db_copy_assigned = db_moved;
    TEST_ASSERT(db_copy_assigned == db_moved);

    verify_content(db_moved);
    verify_content(db_copy_assigned);

    map_type db_move_assigned;
    TEST_ASSERT(db_move_assigned.empty());
    db_move_assigned = std::move(db_moved);
    TEST_ASSERT(db_move_assigned != db_moved);

    verify_content(db_move_assigned);
    TEST_ASSERT(db_moved.empty());
}

void trie_packed_test_non_equal()
{
    MDDS_TEST_FUNC_SCOPE;

    using map_type = packed_trie_map<std::string, int>;

    map_type::entry entries1[] = {
        {MDDS_ASCII("aaron"), 0}, {MDDS_ASCII("al"), 1},    {MDDS_ASCII("aldi"), 2},    {MDDS_ASCII("andy"), 3},
        {MDDS_ASCII("bison"), 4}, {MDDS_ASCII("bruce"), 5}, {MDDS_ASCII("charlie"), 6}, {MDDS_ASCII("charlotte"), 7},
        {MDDS_ASCII("david"), 8}, {MDDS_ASCII("dove"), 9},  {MDDS_ASCII("e"), 10},      {MDDS_ASCII("eva"), 11},
    };

    map_type::entry entries2[] = {
        {MDDS_ASCII("aaron"), 0},   {MDDS_ASCII("al"), 1},        {MDDS_ASCII("aldi"), 2},
        {MDDS_ASCII("andy"), 3},    {MDDS_ASCII("bison"), 4},     {MDDS_ASCII("bruce"), 2}, // different value
        {MDDS_ASCII("charlie"), 6}, {MDDS_ASCII("charlotte"), 7}, {MDDS_ASCII("david"), 8},
        {MDDS_ASCII("dove"), 9},    {MDDS_ASCII("e"), 10},        {MDDS_ASCII("eva"), 11},
    };

    // fewer entries
    map_type::entry entries3[] = {
        {MDDS_ASCII("aaron"), 0}, {MDDS_ASCII("al"), 1},    {MDDS_ASCII("aldi"), 2},    {MDDS_ASCII("andy"), 3},
        {MDDS_ASCII("bison"), 4}, {MDDS_ASCII("bruce"), 5}, {MDDS_ASCII("charlie"), 6}, {MDDS_ASCII("charlotte"), 7},
        {MDDS_ASCII("david"), 8}, {MDDS_ASCII("dove"), 9},  {MDDS_ASCII("e"), 10},
    };

    map_type db1(entries1, std::size(entries1));
    map_type db2(entries2, std::size(entries2));
    map_type db3(entries3, std::size(entries3));
    TEST_ASSERT(db1 != db2);
    TEST_ASSERT(db1 != db3);
    TEST_ASSERT(db2 != db3);

    map_type db4(entries1, std::size(entries1));
    map_type db5(entries2, std::size(entries2));
    map_type db6(entries3, std::size(entries3));

    TEST_ASSERT(db1 == db4);
    TEST_ASSERT(db2 == db5);
    TEST_ASSERT(db3 == db6);
}

void trie_test1()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef trie_map<std::string, custom_string> trie_map_type;
    typedef packed_trie_map<std::string, custom_string> packed_trie_map_type;

    trie_map_type db;
    const trie_map_type& dbc = db;

    TEST_ASSERT(db.size() == 0);
    db.insert(MDDS_ASCII("Barak"), custom_string("Obama"));
    TEST_ASSERT(db.size() == 1);
    db.insert(MDDS_ASCII("Bob"), custom_string("Marley"));
    TEST_ASSERT(db.size() == 2);
    db.insert(MDDS_ASCII("Hideki"), custom_string("Matsui"));
    TEST_ASSERT(db.size() == 3);

    auto it = dbc.find(MDDS_ASCII("Barak"));
    TEST_ASSERT(it->first == "Barak");
    custom_string res = it->second;
    TEST_ASSERT(res.data == "Obama");

    res = dbc.find(MDDS_ASCII("Bob"))->second;
    TEST_ASSERT(res.data == "Marley");
    res = dbc.find(MDDS_ASCII("Hideki"))->second;
    TEST_ASSERT(res.data == "Matsui");

    // Non-existent key.
    it = dbc.find(MDDS_ASCII("Von"));
    TEST_ASSERT(it == dbc.end());
    it = dbc.find(MDDS_ASCII("Bar"));
    TEST_ASSERT(it == dbc.end());

    // Perform prefix search on "B", which should return both "Barak" and "Bob".
    // The results should be sorted.
    {
        auto matches = dbc.prefix_search(MDDS_ASCII("B"));
        size_t n = std::distance(matches.begin(), matches.end());
        TEST_ASSERT(n == 2);
        auto it2 = matches.begin();
        TEST_ASSERT(it2->first == "Barak");
        TEST_ASSERT(it2->second.data == "Obama");
        ++it2;
        TEST_ASSERT(it2->first == "Bob");
        TEST_ASSERT(it2->second.data == "Marley");

        matches = dbc.prefix_search(MDDS_ASCII("Hi"));
        n = std::distance(matches.begin(), matches.end());
        TEST_ASSERT(n == 1);
        it2 = matches.begin();
        TEST_ASSERT(it2->first == "Hideki");
        TEST_ASSERT(it2->second.data == "Matsui");

        // Invalid prefix searches.
        matches = dbc.prefix_search(MDDS_ASCII("Bad"));
        TEST_ASSERT(matches.begin() == matches.end());
        matches = dbc.prefix_search(MDDS_ASCII("Foo"));
        TEST_ASSERT(matches.begin() == matches.end());
    }

    {
        // Create a packed version from it, and make sure it still generates the
        // same results.
        packed_trie_map_type packed(dbc);
        TEST_ASSERT(packed.size() == dbc.size());

        {
            auto results = packed.prefix_search(MDDS_ASCII("B"));
            size_t n = std::distance(results.begin(), results.end());
            TEST_ASSERT(n == 2);
            auto it2 = results.begin();
            TEST_ASSERT(it2->first == "Barak");
            TEST_ASSERT(it2->second.data == "Obama");
            ++it2;
            TEST_ASSERT(it2->first == "Bob");
            TEST_ASSERT(it2->second.data == "Marley");
            ++it2;
            TEST_ASSERT(it2 == results.end());
        }

        {
            auto results = dbc.prefix_search(MDDS_ASCII("Hi"));
            size_t n = std::distance(results.begin(), results.end());
            TEST_ASSERT(n == 1);
            auto it2 = results.begin();
            TEST_ASSERT(it2->first == "Hideki");
            TEST_ASSERT(it2->second.data == "Matsui");
        }

        // Invalid prefix searches.
        auto results = dbc.prefix_search(MDDS_ASCII("Bad"));
        TEST_ASSERT(results.begin() == results.end());
        results = dbc.prefix_search(MDDS_ASCII("Foo"));
        TEST_ASSERT(results.begin() == results.end());
    }

    {
        trie_map_type copied(dbc);
        auto packed = copied.pack();
        auto results = packed.prefix_search(MDDS_ASCII("B"));
        size_t n = std::distance(results.begin(), results.end());
        TEST_ASSERT(n == 2);
        auto it2 = results.begin();
        TEST_ASSERT(it2->first == "Barak");
        TEST_ASSERT(it2->second.data == "Obama");
        ++it2;
        TEST_ASSERT(it2->first == "Bob");
        TEST_ASSERT(it2->second.data == "Marley");
    }

    // Erase an existing key.
    bool erased = db.erase(MDDS_ASCII("Hideki"));
    TEST_ASSERT(erased);
    TEST_ASSERT(db.size() == 2);

    it = dbc.find(MDDS_ASCII("Hideki"));
    TEST_ASSERT(it == dbc.end());

    // Try to erase a key that doesn't exist.
    erased = db.erase(MDDS_ASCII("Foo"));
    TEST_ASSERT(!erased);
    TEST_ASSERT(db.size() == 2);

    // Clear the whole thing.
    db.clear();
    TEST_ASSERT(db.size() == 0);
}

void trie_test2()
{
    MDDS_TEST_FUNC_SCOPE;

    using key_type = std::vector<uint16_t>;
    using map_type = trie_map<key_type, int>;

    auto print_key = [](const std::vector<uint16_t>& key, const char* msg) {
        cout << msg << ": ";
        std::copy(key.begin(), key.end(), std::ostream_iterator<uint16_t>(std::cout, " "));
        cout << endl;
    };

    map_type db;
    key_type key = {2393, 99, 32589, 107, 0, 65535};
    print_key(key, "original");
    int value = 1;
    db.insert(key, value);
    TEST_ASSERT(db.size() == 1);
    {
        auto it = db.begin();
        TEST_ASSERT(it != db.end());
        TEST_ASSERT(it->first == key);

        print_key(it->first, "from trie_map");
    }

    auto packed = db.pack();
    TEST_ASSERT(packed.size() == 1);

    {
        auto it = packed.begin();
        TEST_ASSERT(it != packed.end());
        print_key(it->first, "from packed_trie_map");
        TEST_ASSERT(it->first == key);
    }
}

void trie_test_iterator_empty()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef trie_map<std::string, int> trie_map_type;
    trie_map_type db;
    const trie_map_type& dbc = db;

    // empty container
    trie_map_type::const_iterator it = dbc.begin();
    trie_map_type::const_iterator ite = dbc.end();

    TEST_ASSERT(it == ite);
    TEST_ASSERT(db.begin() == dbc.begin()); // non-const vs const iterators
    TEST_ASSERT(dbc.end() == db.end()); // const vs non-const iterators
}

void trie_test_iterator()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef trie_map<std::string, int> trie_map_type;
    using kv = std::pair<std::string, int>;
    trie_map_type db;
    const trie_map_type& dbc = db;

    cout << "empty container" << endl;

    // empty container
    trie_map_type::const_iterator it = dbc.begin();
    trie_map_type::const_iterator ite = dbc.end();

    // The end iterator will never get invalidated since it only references
    // the root node which will never get modified as long as the parent
    // container is alive.

    TEST_ASSERT(it == ite);

    cout << "one element" << endl;

    db.insert(MDDS_ASCII("a"), 1);
    it = dbc.begin();
    TEST_ASSERT(it != ite);
    TEST_ASSERT(*it == kv("a", 1));
    ++it;
    TEST_ASSERT(it == ite);

    cout << "two elements" << endl;

    db.insert(MDDS_ASCII("ab"), 2);
    it = dbc.begin();
    TEST_ASSERT(it != ite);
    TEST_ASSERT(*it == kv("a", 1));
    ++it;
    TEST_ASSERT(it != ite);
    TEST_ASSERT(*it == kv("ab", 2));
    ++it;
    TEST_ASSERT(it == ite);

    cout << "more than two elements" << endl;

    db.insert(MDDS_ASCII("aba"), 3);
    db.insert(MDDS_ASCII("abb"), 4);
    db.insert(MDDS_ASCII("abc"), 5);
    db.insert(MDDS_ASCII("bc"), 6);
    db.insert(MDDS_ASCII("bcd"), 7);

    it = dbc.begin();
    TEST_ASSERT(*it == kv("a", 1));
    ++it;
    TEST_ASSERT(*it == kv("ab", 2));
    ++it;
    TEST_ASSERT(*it == kv("aba", 3));
    ++it;
    TEST_ASSERT(*it == kv("abb", 4));
    ++it;
    TEST_ASSERT(*it == kv("abc", 5));
    ++it;
    TEST_ASSERT(*it == kv("bc", 6));
    ++it;
    TEST_ASSERT(*it == kv("bcd", 7));
    TEST_ASSERT(it->first == "bcd");
    TEST_ASSERT(it->second == 7);
    ++it;
    TEST_ASSERT(it == ite);

    --it;
    TEST_ASSERT(it != ite);
    TEST_ASSERT(*it == kv("bcd", 7));
    --it;
    TEST_ASSERT(*it == kv("bc", 6));
    --it;
    TEST_ASSERT(*it == kv("abc", 5));
    --it;
    TEST_ASSERT(*it == kv("abb", 4));
    --it;
    TEST_ASSERT(*it == kv("aba", 3));
    --it;
    TEST_ASSERT(*it == kv("ab", 2));
    --it;
    TEST_ASSERT(*it == kv("a", 1));
    TEST_ASSERT(it == dbc.begin());
    ++it;
    TEST_ASSERT(*it == kv("ab", 2));
    ++it;
    TEST_ASSERT(*it == kv("aba", 3));
    --it;
    TEST_ASSERT(*it == kv("ab", 2));
    --it;
    TEST_ASSERT(*it == kv("a", 1));
    ++it;
    TEST_ASSERT(*it == kv("ab", 2));
    ++it;
    TEST_ASSERT(*it == kv("aba", 3));

    TEST_ASSERT(db.begin() != dbc.end()); // non-const vs const iterators
    TEST_ASSERT(dbc.begin() != db.end()); // const vs non-const iterators
}

void trie_test_iterator_with_erase()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef trie_map<std::string, int> trie_map_type;
    using kv = std::pair<std::string, int>;
    trie_map_type db;
    const trie_map_type& dbc = db;
    bool check_true = false;

    db.insert(MDDS_ASCII("Python"), 1);
    db.insert(MDDS_ASCII("C++"), 2);

    auto it = dbc.begin(), ite = dbc.end();
    check_true = (*it++ == kv("C++", 2));
    TEST_ASSERT(check_true);
    check_true = (*it++ == kv("Python", 1));
    TEST_ASSERT(check_true);
    TEST_ASSERT(it == ite);

    db.erase(MDDS_ASCII("C++"));
    it = dbc.begin();
    check_true = (*it++ == kv("Python", 1));
    TEST_ASSERT(check_true);
    TEST_ASSERT(it == ite);
    check_true = (*(--it) == kv("Python", 1));
    TEST_ASSERT(check_true);
    TEST_ASSERT(it == dbc.begin());

    db.clear();
    TEST_ASSERT(dbc.begin() == dbc.end());

    db.insert(MDDS_ASCII("A"), 1);
    db.insert(MDDS_ASCII("AB"), 2);
    db.insert(MDDS_ASCII("ABC"), 3);
    db.erase(MDDS_ASCII("AB"));

    it = dbc.begin();
    check_true = (*it++ == kv("A", 1));
    TEST_ASSERT(check_true);
    check_true = (*it++ == kv("ABC", 3));
    TEST_ASSERT(check_true);
    TEST_ASSERT(it == ite);

    check_true = (*(--it) == kv("ABC", 3));
    TEST_ASSERT(check_true);
    check_true = (*(--it) == kv("A", 1));
    TEST_ASSERT(check_true);
    TEST_ASSERT(it == dbc.begin());

    db.clear();
    db.insert(MDDS_ASCII("A"), 1);
    db.insert(MDDS_ASCII("AB"), 2);
    db.insert(MDDS_ASCII("ABC"), 3);
    db.erase(MDDS_ASCII("ABC"));

    it = dbc.begin();
    check_true = (*it++ == kv("A", 1));
    TEST_ASSERT(check_true);
    check_true = (*it++ == kv("AB", 2));
    TEST_ASSERT(check_true);
    TEST_ASSERT(it == ite);

    check_true = (*(--it) == kv("AB", 2));
    TEST_ASSERT(check_true);
    check_true = (*(--it) == kv("A", 1));
    TEST_ASSERT(check_true);
    TEST_ASSERT(it == dbc.begin());

    it = ite;
    --it;
    TEST_ASSERT(*it-- == kv("AB", 2)); // test post-decrement operator.
    TEST_ASSERT(*it == kv("A", 1));
}

void trie_test_find_iterator()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef trie_map<std::string, int> trie_map_type;
    trie_map_type db;
    const trie_map_type& dbc = db;

    db.insert(MDDS_ASCII("a"), 1);
    db.insert(MDDS_ASCII("aa"), 2);
    db.insert(MDDS_ASCII("ab"), 3);
    db.insert(MDDS_ASCII("b"), 4);
    {
        auto it = dbc.find(MDDS_ASCII("a"));
        TEST_ASSERT(it->first == "a");
        TEST_ASSERT(it->second == 1);
        ++it;
        TEST_ASSERT(it->first == "aa");
        TEST_ASSERT(it->second == 2);
        ++it;
        TEST_ASSERT(it->first == "ab");
        TEST_ASSERT(it->second == 3);
        ++it;
        TEST_ASSERT(it->first == "b");
        TEST_ASSERT(it->second == 4);
        ++it;
        TEST_ASSERT(it == dbc.end());

        it = dbc.find(MDDS_ASCII("aa"));
        TEST_ASSERT(it->first == "aa");
        TEST_ASSERT(it->second == 2);
        ++it;
        TEST_ASSERT(it->first == "ab");
        TEST_ASSERT(it->second == 3);
        ++it;
        TEST_ASSERT(it->first == "b");
        TEST_ASSERT(it->second == 4);
        ++it;
        TEST_ASSERT(it == dbc.end());

        it = dbc.find(MDDS_ASCII("ab"));
        TEST_ASSERT(it->first == "ab");
        TEST_ASSERT(it->second == 3);
        ++it;
        TEST_ASSERT(it->first == "b");
        TEST_ASSERT(it->second == 4);
        ++it;
        TEST_ASSERT(it == dbc.end());

        it = dbc.find(MDDS_ASCII("b"));
        TEST_ASSERT(it->first == "b");
        TEST_ASSERT(it->second == 4);
        ++it;
        TEST_ASSERT(it == dbc.end());
    }

    trie_map_type::packed_type packed = db.pack();
    {
        auto it = packed.find(MDDS_ASCII("a"));
        TEST_ASSERT(it->first == "a");
        TEST_ASSERT(it->second == 1);
        ++it;
        TEST_ASSERT(it->first == "aa");
        TEST_ASSERT(it->second == 2);
        ++it;
        TEST_ASSERT(it->first == "ab");
        TEST_ASSERT(it->second == 3);
        ++it;
        TEST_ASSERT(it->first == "b");
        TEST_ASSERT(it->second == 4);
        ++it;
        TEST_ASSERT(it == packed.end());

        it = packed.find(MDDS_ASCII("aa"));
        TEST_ASSERT(it->first == "aa");
        TEST_ASSERT(it->second == 2);
        ++it;
        TEST_ASSERT(it->first == "ab");
        TEST_ASSERT(it->second == 3);
        ++it;
        TEST_ASSERT(it->first == "b");
        TEST_ASSERT(it->second == 4);
        ++it;
        TEST_ASSERT(it == packed.end());

        it = packed.find(MDDS_ASCII("ab"));
        TEST_ASSERT(it->first == "ab");
        TEST_ASSERT(it->second == 3);
        ++it;
        TEST_ASSERT(it->first == "b");
        TEST_ASSERT(it->second == 4);
        ++it;
        TEST_ASSERT(it == packed.end());

        it = packed.find(MDDS_ASCII("b"));
        TEST_ASSERT(it->first == "b");
        TEST_ASSERT(it->second == 4);
        ++it;
        TEST_ASSERT(it == packed.end());
    }
}

void trie_test_prefix_search()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef trie_map<std::string, int> trie_map_type;
    trie_map_type db;
    const trie_map_type& dbc = db;

    db.insert(MDDS_ASCII("a"), 1);
    db.insert(MDDS_ASCII("aa"), 2);
    db.insert(MDDS_ASCII("ab"), 3);
    db.insert(MDDS_ASCII("b"), 4);

    cout << "Performing prefix search on 'a'..." << endl;

    trie_map_type::search_results results = dbc.prefix_search(MDDS_ASCII("a"));
    auto it = results.begin();
    auto ite = results.end();
    TEST_ASSERT(it != ite);
    TEST_ASSERT(it->first == "a");
    TEST_ASSERT(it->second == 1);
    ++it;
    TEST_ASSERT(it->first == "aa");
    TEST_ASSERT(it->second == 2);
    ++it;
    TEST_ASSERT(it->first == "ab");
    TEST_ASSERT(it->second == 3);
    ++it;
    TEST_ASSERT(it == ite);
    size_t n = std::distance(results.begin(), results.end());
    TEST_ASSERT(n == 3);

    cout << "Performing prefix search on 'b'..." << endl;

    results = dbc.prefix_search(MDDS_ASCII("b"));
    it = results.begin();
    ite = results.end();
    TEST_ASSERT(it != ite);
    TEST_ASSERT(it->first == "b");
    TEST_ASSERT(it->second == 4);
    ++it;
    TEST_ASSERT(it == ite);
    --it;
    TEST_ASSERT(it->first == "b");
    TEST_ASSERT(it->second == 4);
    n = std::distance(results.begin(), results.end());
    TEST_ASSERT(n == 1);

    // Only one element.
    db.clear();
    db.insert(MDDS_ASCII("dust"), 10);

    cout << "Performing prefix search on 'du'..." << endl;

    results = dbc.prefix_search(MDDS_ASCII("du"));
    it = results.begin();
    TEST_ASSERT(it->first == "dust");
    TEST_ASSERT(it->second == 10);
    bool check_true = (++it == results.end());
    TEST_ASSERT(check_true);
    --it;
    TEST_ASSERT(it->first == "dust");
    TEST_ASSERT(it->second == 10);
    n = std::distance(results.begin(), results.end());
    TEST_ASSERT(n == 1);
}

void trie_test_key_as_input()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef trie_map<std::string, int> trie_map_type;
    trie_map_type db;
    const trie_map_type& dbc = db;

    db.insert(std::string("string as key"), 1);
    db.insert("literal as key", 2);

    auto it = dbc.find("literal as key");
    TEST_ASSERT(it != dbc.end());
    TEST_ASSERT(it->first == "literal as key");
    TEST_ASSERT(it->second == 2);

    auto results = dbc.prefix_search("str");
    auto rit = results.begin();
    TEST_ASSERT(rit != results.end());
    TEST_ASSERT(rit->first == "string as key");
    TEST_ASSERT(rit->second == 1);
    ++rit;
    TEST_ASSERT(rit == results.end());
}

void trie_test_copying()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef trie_map<std::string, int> trie_map_type;
    trie_map_type db;
    TEST_ASSERT(db.empty());

    {
        auto db_copied(db);
        TEST_ASSERT(db_copied.empty());
    }

    db.insert("twenty", 20);
    db.insert("twelve", 12);
    TEST_ASSERT(db.size() == 2);

    {
        // copy constructor
        auto db_copied(db);
        const trie_map_type& dbc_copied = db_copied;
        TEST_ASSERT(db_copied.size() == 2);

        auto it = dbc_copied.find("twenty");
        TEST_ASSERT(it != dbc_copied.end());
        TEST_ASSERT(it->first == "twenty");
        TEST_ASSERT(it->second == 20);

        it = dbc_copied.find("twelve");
        TEST_ASSERT(it != dbc_copied.end());
        TEST_ASSERT(it->first == "twelve");
        TEST_ASSERT(it->second == 12);
    }

    {
        // copy assignment
        trie_map_type db_copied;
        db_copied = db;
        const trie_map_type& dbc_copied = db_copied;
        TEST_ASSERT(db_copied.size() == 2);

        auto it = dbc_copied.find("twenty");
        TEST_ASSERT(it != dbc_copied.end());
        TEST_ASSERT(it->first == "twenty");
        TEST_ASSERT(it->second == 20);

        it = dbc_copied.find("twelve");
        TEST_ASSERT(it != dbc_copied.end());
        TEST_ASSERT(it->first == "twelve");
        TEST_ASSERT(it->second == 12);
    }

    {
        // move constructor
        auto db_copied(db);
        auto db_moved(std::move(db_copied));
        const trie_map_type& dbc_moved = db_moved;
        TEST_ASSERT(db_moved.size() == 2);
        TEST_ASSERT(db_copied.empty());

        auto it = dbc_moved.find("twenty");
        TEST_ASSERT(it != dbc_moved.end());
        TEST_ASSERT(it->first == "twenty");
        TEST_ASSERT(it->second == 20);

        it = dbc_moved.find("twelve");
        TEST_ASSERT(it != dbc_moved.end());
        TEST_ASSERT(it->first == "twelve");
        TEST_ASSERT(it->second == 12);
    }

    {
        // move assignment
        auto db_copied(db);
        trie_map_type db_moved;
        db_moved = std::move(db_copied);
        const trie_map_type& dbc_moved = db_moved;
        TEST_ASSERT(db_moved.size() == 2);
        TEST_ASSERT(db_copied.empty());

        auto it = dbc_moved.find("twenty");
        TEST_ASSERT(it != dbc_moved.end());
        TEST_ASSERT(it->first == "twenty");
        TEST_ASSERT(it->second == 20);

        it = dbc_moved.find("twelve");
        TEST_ASSERT(it != dbc_moved.end());
        TEST_ASSERT(it->first == "twelve");
        TEST_ASSERT(it->second == 12);
    }
}

void trie_test_value_update_from_iterator()
{
    MDDS_TEST_FUNC_SCOPE;

    typedef trie_map<std::string, int> trie_map_type;
    trie_map_type db;
    db.insert("one", 1);
    db.insert("two", 2);
    db.insert("three", 3);

    trie_map_type::iterator it = db.begin();
    TEST_ASSERT(it->first == "one");
    TEST_ASSERT(it->second == 1);
    it->second = 10; // update the value.
    it = db.begin();
    TEST_ASSERT(it->first == "one");
    TEST_ASSERT(it->second == 10);

    it = db.find("three");
    TEST_ASSERT(it->first == "three");
    TEST_ASSERT(it->second == 3);
    it->second = 345; // update the value again.
    it = db.find("three");
    TEST_ASSERT(it->first == "three");
    TEST_ASSERT(it->second == 345);
}

void trie_test_equality()
{
    MDDS_TEST_FUNC_SCOPE;

    using trie_map_type = trie_map<std::string, int>;

    trie_map_type db1, db2;
    TEST_ASSERT(db1 == db2);

    db1.insert("a", 1);
    TEST_ASSERT(db1 != db2);
    db2.insert("a", 1);
    TEST_ASSERT(db1 == db2);

    db1.insert("ab", 2);
    TEST_ASSERT(db1 != db2);
    db2.insert("ab", 2);
    TEST_ASSERT(db1 == db2);

    db1.insert("ac", 3); // different value
    TEST_ASSERT(db1 != db2);
    db2.insert("ac", 4); // different value
    TEST_ASSERT(db1 != db2);

    auto it = db1.find("ac");
    it->second = 4; // make the value equal
    TEST_ASSERT(db1 == db2);

    db1.clear();
    db2.clear();
    db1.insert("ab", 1);
    db2.insert("a", 1);
    TEST_ASSERT(db1 != db2);

    db1.clear();
    db2.clear();
    db1.insert("a", 1);
    db2.insert("b", 1);
    TEST_ASSERT(db1 != db2);
}

int main()
{
    try
    {
        trie_packed_test1();
        trie_packed_test2();
        trie_packed_test3();
        trie_packed_test4();
        trie_packed_test_value_life_cycle();
        trie_packed_test_custom_string();
        trie_packed_test_iterator_empty();
        trie_packed_test_iterator();
        trie_packed_test_prefix_search1();
        trie_packed_test_key_as_input();
        trie_packed_test_copying();
        trie_packed_test_non_equal();
        trie_packed_test_save_and_load_state::run();

        trie_test1();
        trie_test2();

        trie_test_iterator_empty();
        trie_test_iterator();
        trie_test_iterator_with_erase();
        trie_test_find_iterator();
        trie_test_prefix_search();
        trie_test_key_as_input();
        trie_test_copying();
        trie_test_value_update_from_iterator();
        trie_test_equality();

        trie_test_node::run();
        trie_test_move_value::run();
        trie_test_pack_value_type::run();
    }
    catch (const std::exception& e)
    {
        cout << "Test failed: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    cout << "Test finished successfully!" << endl;

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
