/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2015-2018 Kohei Yoshida
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
#include "test_global.hpp" // This must be the first header to be included.
//#define MDDS_TRIE_MAP_DEBUG_DUMP_TRIE 1
//#define MDDS_TRIE_MAP_DEBUG_DUMP_PACKED 1

#include "mdds/trie_map.hpp"
#include "mdds/global.hpp"

#include <iterator>
#include <fstream>
#include <vector>
#include <list>

using namespace std;
using namespace mdds;

using packed_int_map_type = packed_trie_map<trie::std_string_trait, int>;
using packed_str_map_type = packed_trie_map<trie::std_string_trait, std::string>;

bool verify_entries(
    const packed_int_map_type& db, const packed_int_map_type::entry* entries,
    size_t entry_size)
{
    auto results = db.prefix_search(nullptr, 0);
    for (auto it = results.begin(), ite = results.end(); it != ite; ++it)
        cout << it->first << ": " << it->second << endl;

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

template<typename T>
bool check_equal(const T& left, const T& right)
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
    stack_printer __stack_printer__("::trie_packed_test1");

    packed_int_map_type::entry entries[] =
    {
        { MDDS_ASCII("a"),  13 },
        { MDDS_ASCII("aa"), 10 },
        { MDDS_ASCII("ab"), 3 },
        { MDDS_ASCII("b"),  7 },
    };

    size_t entry_size = MDDS_N_ELEMENTS(entries);
    packed_int_map_type db(entries, entry_size);
    assert(db.size() == 4);
    assert(verify_entries(db, entries, entry_size));

    // invalid keys
    assert(db.find(MDDS_ASCII("ac")) == db.end());
    assert(db.find(MDDS_ASCII("c")) == db.end());

    {
        // Get all key-value pairs.
        auto results = db.prefix_search(nullptr, 0);
        size_t n = std::distance(results.begin(), results.end());
        assert(n == 4);
        auto it = results.begin();
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
        assert(it == results.end());
    }

    {
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
    packed_int_map_type db(entries, entry_size);
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
    packed_int_map_type db(entries, entry_size);
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
    packed_int_map_type db(entries, entry_size);
    assert(db.size() == 6);
    assert(verify_entries(db, entries, entry_size));

    // Try invalid keys.
    assert(db.find("foo", 3) == db.end());
    assert(db.find("andy133", 7) == db.end());

    // Test prefix search on 'andy'.
    auto results = db.prefix_search(MDDS_ASCII("andy"));
    size_t n = std::distance(results.begin(), results.end());
    assert(n == 3);
    auto it = results.begin();
    assert(it->first == "andy");
    ++it;
    assert(it->first == "andy1");
    ++it;
    assert(it->first == "andy13");
    ++it;
    assert(it == results.end());

    results = db.prefix_search(MDDS_ASCII("andy's toy"));
    n = std::distance(results.begin(), results.end());
    assert(n == 0);

    results = db.prefix_search(MDDS_ASCII("e"));
    n = std::distance(results.begin(), results.end());
    assert(n == 0);

    results = db.prefix_search(MDDS_ASCII("b"));
    n = std::distance(results.begin(), results.end());
    assert(n == 1);
    it = results.begin();
    assert(it->first == "bruce");
    assert(it->second == name_bruce);
    ++it;
    assert(it == results.end());
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

    packed_value_map_type db(entries->data(), entries->size());

    // Delete the original entry store.
    entries.reset();

    auto results = db.prefix_search(nullptr, 0);
    std::for_each(results.begin(), results.end(),
        [](const packed_value_map_type::const_iterator::value_type& v)
        {
            cout << v.first << ": " << v.second.value << endl;
        }
    );

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
    packed_custom_str_map_type db(entries, n_entries);
    for (size_t i = 0; i < n_entries; ++i)
    {
        auto it = db.find(entries[i].key, entries[i].keylen);
        cout << it->second << endl;
        assert(it->second == entries[i].value);
    }

    // Find all keys that start with 'M'.
    auto results = db.prefix_search(key_max, 1);
    size_t n = std::distance(results.begin(), results.end());
    assert(n == 2);
    auto it = results.begin();
    assert(it->first.data == it->second);
    assert(it->second == "Max");
    ++it;
    assert(it->first.data == it->second);
    assert(it->second == "Ming");
    ++it;
    assert(it == results.end());
}

void trie_packed_test_iterator_empty()
{
    stack_printer __stack_printer__("::trie_packed_test_iterator_empty");
    packed_int_map_type db(nullptr, 0);

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

    trie_map_type db;

    db.insert(MDDS_ASCII("a"), 1);
    packed_type packed = db.pack();
    assert(db.size() == packed.size());
    packed_type::const_iterator it = packed.begin();
    packed_type::const_iterator ite = packed.end();
    assert(it != ite);
    assert(it->first == "a");
    assert(it->second == 1);

    db.insert(MDDS_ASCII("ab"), 2);
    packed = db.pack(); // this invalidates the end position.
    assert(db.size() == packed.size());

    it = packed.begin();
    ite = packed.end();
    assert(it != ite);
    assert(it->first == "a");
    assert(it->second == 1);

    ++it;
    bool check_true = check_equal(*it++, kv("ab", 2));
    assert(check_true);
    assert(it == ite);

    db.insert(MDDS_ASCII("aba"), 3);
    db.insert(MDDS_ASCII("abb"), 4);
    db.insert(MDDS_ASCII("abc"), 5);
    db.insert(MDDS_ASCII("bc"),  6);
    db.insert(MDDS_ASCII("bcd"), 7);

    packed = db.pack();
    assert(db.size() == packed.size());

    it = packed.begin();
    ite = packed.end();

    assert(*it == kv("a", 1));
    assert(check_equal(*(++it), kv("ab", 2)));
    assert(check_equal(*(++it), kv("aba", 3)));
    assert(check_equal(*(++it), kv("abb", 4)));
    assert(check_equal(*(++it), kv("abc", 5)));
    assert(check_equal(*(++it), kv("bc",  6)));
    assert(check_equal(*(++it), kv("bcd", 7)));
    assert(it->first == "bcd");
    assert(it->second == 7);
    ++it;
    assert(it == ite);

    --it;
    assert(it != ite);
    assert(check_equal(*it, kv("bcd", 7)));
    --it;
    assert(check_equal(*it, kv("bc", 6)));
    --it;
    assert(check_equal(*it, kv("abc", 5)));
    --it;
    assert(check_equal(*it, kv("abb", 4)));
    --it;
    assert(check_equal(*it, kv("aba", 3)));
    --it;
    assert(check_equal(*it, kv("ab", 2)));
    assert(check_equal(*(--it), kv("a", 1)));
    assert(it == packed.begin());

    assert(check_equal(*(++it), kv("ab",  2)));
    assert(check_equal(*(++it), kv("aba", 3)));
    --it;
    assert(check_equal(*it, kv("ab", 2)));
    --it;
    assert(check_equal(*it, kv("a",  1)));
    ++it;
    assert(check_equal(*it, kv("ab", 2)));
    ++it;
    assert(check_equal(*it, kv("aba", 3)));

    // Post-decrement operator.
    assert(check_equal(*it--, kv("aba", 3)));
    assert(check_equal(*it, kv("ab", 2)));
}

void trie_packed_test_prefix_search1()
{
    stack_printer __stack_printer__("::trie_packed_test_prefix_search1");

    using trie_map_type = trie_map<trie::std_string_trait, int>;
    using packed_type = trie_map_type::packed_type;

    trie_map_type db;
    db.insert(MDDS_ASCII("andy"),   1);
    db.insert(MDDS_ASCII("andy1"),  2);
    db.insert(MDDS_ASCII("andy12"), 3);

    {
        auto results = db.prefix_search(MDDS_ASCII("andy"));
        auto it = results.begin();
        assert(it != results.end());
        assert(it->first == "andy");
        ++it;
        assert(it->first == "andy1");
        ++it;
        assert(it->first == "andy12");
        ++it;
        assert(it == results.end());

        size_t n = std::distance(results.begin(), results.end());
        assert(n == 3);
    }

    packed_type packed = db.pack();
    {
        auto results = packed.prefix_search(MDDS_ASCII("andy"));
        auto it = results.begin();
        assert(it != results.end());
        assert(it->first == "andy");
        ++it;
        assert(it->first == "andy1");
        ++it;
        assert(it->first == "andy12");
        ++it;
        assert(it == results.end());

        size_t n = std::distance(results.begin(), results.end());
        assert(n == 3);
    }
}

void trie_packed_test_key_as_input()
{
    stack_printer __stack_printer__("::trie_packed_test_key_as_input");

    typedef trie_map<trie::std_string_trait, int> trie_map_type;
    trie_map_type db;

    db.insert(std::string("string as key"), 1);
    db.insert("literal as key", 2);
    auto packed = db.pack();

    auto it = packed.find("literal as key");
    assert(it != packed.end());
    assert(it->first == "literal as key");
    assert(it->second == 2);

    auto results = packed.prefix_search("str");
    auto rit = results.begin();
    assert(rit != results.end());
    assert(rit->first == "string as key");
    assert(rit->second == 1);
    ++rit;
    assert(rit == results.end());
}

void trie_packed_test_copying()
{
    stack_printer __stack_printer__("::trie_packed_test_copying");
    using map_type = packed_trie_map<trie::std_string_trait, int>;

    map_type::entry entries[] =
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

    auto verify_content = [&entries](const map_type& db)
    {
        auto it = db.begin();
        const map_type::entry* p_entries = entries;
        const map_type::entry* p_entries_end = p_entries + db.size();
        size_t n = std::distance(p_entries, p_entries_end);
        assert(db.size() == n);
        assert(!db.empty());

        for (; p_entries != p_entries_end; ++p_entries, ++it)
        {
            std::string key_expected(p_entries->key, p_entries->keylen);
            assert(key_expected == it->first);
            assert(p_entries->value == it->second);
        }
    };

    auto db = mdds::make_unique<map_type>(entries, MDDS_N_ELEMENTS(entries));
    auto db_copied(*db);
    assert(*db == db_copied);
    assert(db->size() == db_copied.size());
    db.reset();

    auto it = db_copied.find("charlie");
    assert(it != db_copied.end());
    assert(it->first == "charlie");
    assert(it->second == 6);

    verify_content(db_copied);

    auto db_moved(std::move(db_copied));
    assert(db_copied.empty());
    assert(!db_moved.empty());
    assert(db_moved.size() == MDDS_N_ELEMENTS(entries));

    it = db_copied.find("bison");
    assert(it == db_copied.end());
    it = db_moved.find("bison");
    assert(it != db_moved.end());
    assert(it->first == "bison");
    assert(it->second == 4);

    verify_content(db_moved);

    map_type db_copy_assigned;
    assert(db_copy_assigned.empty());
    db_copy_assigned = db_moved;
    assert(db_copy_assigned == db_moved);

    verify_content(db_moved);
    verify_content(db_copy_assigned);

    map_type db_move_assigned;
    assert(db_move_assigned.empty());
    db_move_assigned = std::move(db_moved);
    assert(db_move_assigned != db_moved);

    verify_content(db_move_assigned);
    assert(db_moved.empty());
}

void trie_packed_test_non_equal()
{
    stack_printer __stack_printer__("::trie_packed_test_non_equal");

    using map_type = packed_trie_map<trie::std_string_trait, int>;

    map_type::entry entries1[] =
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

    map_type::entry entries2[] =
    {
        { MDDS_ASCII("aaron"),     0 },
        { MDDS_ASCII("al"),        1 },
        { MDDS_ASCII("aldi"),      2 },
        { MDDS_ASCII("andy"),      3 },
        { MDDS_ASCII("bison"),     4 },
        { MDDS_ASCII("bruce"),     2 }, // different value
        { MDDS_ASCII("charlie"),   6 },
        { MDDS_ASCII("charlotte"), 7 },
        { MDDS_ASCII("david"),     8 },
        { MDDS_ASCII("dove"),      9 },
        { MDDS_ASCII("e"),        10 },
        { MDDS_ASCII("eva"),      11 },
    };

    // fewer entries
    map_type::entry entries3[] =
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
    };

    map_type db1(entries1, MDDS_N_ELEMENTS(entries1));
    map_type db2(entries2, MDDS_N_ELEMENTS(entries2));
    map_type db3(entries3, MDDS_N_ELEMENTS(entries3));
    assert(db1 != db2);
    assert(db1 != db3);
    assert(db2 != db3);

    map_type db4(entries1, MDDS_N_ELEMENTS(entries1));
    map_type db5(entries2, MDDS_N_ELEMENTS(entries2));
    map_type db6(entries3, MDDS_N_ELEMENTS(entries3));

    assert(db1 == db4);
    assert(db2 == db5);
    assert(db3 == db6);
}

namespace trie_packed_test_save_and_load_state {

struct _custom_variable_value
{
    enum class v_type { unknown, fp32, int64 };

    v_type type;

    union { float fp32; int64_t int64; } value;

    _custom_variable_value() : type(v_type::unknown) {}

    _custom_variable_value(float v) : type(v_type::fp32) { value.fp32 = v; }

    _custom_variable_value(int v) : type(v_type::int64) { value.int64 = v; }

    _custom_variable_value(const _custom_variable_value& other) : type(other.type)
    {
        switch (type)
        {
            case v_type::fp32:
                value.fp32 = other.value.fp32;
                break;
            case v_type::int64:
                value.int64 = other.value.int64;
                break;
            default:
                ;
        }
    }

    bool operator== (const _custom_variable_value& other) const
    {
        if (type != other.type)
            return false;

        switch (type)
        {
            case v_type::fp32:
                return value.fp32 == other.value.fp32;
            case v_type::int64:
                return value.int64 == other.value.int64;
            default:
                ;
        }

        return true;
    }

    bool operator!= (const _custom_variable_value& other) const
    {
        return !operator== (other);
    }
};

struct _custom_variable_serializer
{
    union bin_value
    {
        char buffer[8];
        float fp32;
        int64_t int64;
    };

    static constexpr bool variable_size = true;

    static void write(std::ostream& os, const _custom_variable_value& v)
    {
        bin_value bv;

        switch (v.type)
        {
            case _custom_variable_value::v_type::unknown:
            {
                char c = 0;
                os.write(&c, 1);
                break;
            }
            case _custom_variable_value::v_type::fp32:
            {
                char c = 1;
                os.write(&c, 1);
                bv.fp32 = v.value.fp32;
                os.write(bv.buffer, 4);
                break;
            }
            case _custom_variable_value::v_type::int64:
            {
                char c = 2;
                os.write(&c, 1);
                bv.int64 = v.value.int64;
                os.write(bv.buffer, 8);
                break;
            }
        }
    }

    static void read(std::istream& is, size_t n, _custom_variable_value& v)
    {
        assert(n > 0);
        char c;
        is.read(&c, 1);

        switch (c)
        {
            case 0:
                v.type = _custom_variable_value::v_type::unknown;
                break;
            case 1:
                v.type = _custom_variable_value::v_type::fp32;
                break;
            case 2:
                v.type = _custom_variable_value::v_type::int64;
                break;
            default:
                assert(!"invalid value type");
        }

        n -= 1;
        bin_value bv;

        switch (v.type)
        {
            case _custom_variable_value::v_type::fp32:
                assert(n == 4);
                is.read(bv.buffer, 4);
                v.value.fp32 = bv.fp32;
                break;
            case _custom_variable_value::v_type::int64:
                assert(n == 8);
                is.read(bv.buffer, 8);
                v.value.int64 = bv.int64;
                break;
            case _custom_variable_value::v_type::unknown:
                break;
            default:
                assert(!"invalid value type");
        }
    }
};

/**
 * mock value struct containing one value string that only stores "zero",
 * "one", "two" or "three".  We use a custom serializer to store the value
 * using only 1 byte each.
 */
struct _custom_fixed_value
{
    std::string value_string; // only stores "zero", "one", "two" or "three".

    _custom_fixed_value() {}

    _custom_fixed_value(const char* p) : value_string(p, std::strlen(p)) {}

    bool operator== (const _custom_fixed_value& other) const
    {
        return value_string == other.value_string;
    }

    bool operator!= (const _custom_fixed_value& other) const
    {
        return !operator==(other);
    }
};

struct _custom_fixed_serializer
{
    static constexpr bool variable_size = false;
    static constexpr size_t value_size = 1;

    static void write(std::ostream& os, const _custom_fixed_value& v)
    {
        char bv = -1;

        if (v.value_string == "zero")
            bv = 0;
        else if (v.value_string == "one")
            bv = 1;
        else if (v.value_string == "two")
            bv = 2;
        else if (v.value_string == "three")
            bv = 3;

        os.write(&bv, 1);
    }

    static void read(std::istream& is, size_t n, _custom_fixed_value& v)
    {
        assert(n == 1);
        char bv = -1;
        is.read(&bv, 1);

        switch (bv)
        {
            case 0:
                v.value_string = "zero";
                break;
            case 1:
                v.value_string = "one";
                break;
            case 2:
                v.value_string = "two";
                break;
            case 3:
                v.value_string = "three";
                break;
            default:
                v.value_string = "???";
        }
    }
};

void test1()
{
    stack_printer __stack_printer__("trie_packed_test_save_and_load_state::test1");
    packed_int_map_type empty_db;

    std::string saved_state;

    {
        std::ostringstream state;
        empty_db.save_state(state);
        saved_state = state.str();
    }

    packed_int_map_type restored;

    {
        std::istringstream state(saved_state);
        restored.load_state(state);
    }

    assert(restored == empty_db);
}

void test2()
{
    stack_printer __stack_printer__("trie_packed_test_save_and_load_state::test2");
    packed_int_map_type::entry entries[] =
    {
        { MDDS_ASCII("bruce"),     5 },
        { MDDS_ASCII("charlie"),   6 },
        { MDDS_ASCII("charlotte"), 7 },
        { MDDS_ASCII("david"),     8 },
        { MDDS_ASCII("dove"),      9 },
    };

    packed_int_map_type db(entries, MDDS_N_ELEMENTS(entries));

    std::string saved_state;

    {
        std::ostringstream state;
        db.save_state(state);
        saved_state = state.str();
    }

    packed_int_map_type restored;
    assert(restored != db);

    {
        std::istringstream state(saved_state);
        restored.load_state(state);
    }

    assert(restored == db);
}

void test3()
{
    stack_printer __stack_printer__("trie_packed_test_save_and_load_state::test3");
    std::vector<packed_str_map_type::entry> entries =
    {
        { MDDS_ASCII("Abby"),       "ABBY"       },
        { MDDS_ASCII("Ashley"),     "ASHLEY"     },
        { MDDS_ASCII("Candelaria"), "CANDELARIA" },
        { MDDS_ASCII("Carita"),     "CARITA"     },
        { MDDS_ASCII("Christal"),   "CHRISTAL"   },
        { MDDS_ASCII("Cory"),       "CORY"       },
        { MDDS_ASCII("Estrella"),   "ESTRELLA"   },
        { MDDS_ASCII("Etha"),       "ETHA"       },
        { MDDS_ASCII("Harley"),     "HARLEY"     },
        { MDDS_ASCII("Irish"),      "IRISH"      },
        { MDDS_ASCII("Kiara"),      "KIARA"      },
        { MDDS_ASCII("Korey"),      "KOREY"      },
        { MDDS_ASCII("Laurene"),    "LAURENE"    },
        { MDDS_ASCII("Michiko"),    "MICHIKO"    },
        { MDDS_ASCII("Miriam"),     "MIRIAM"     },
        { MDDS_ASCII("Mitzi"),      "MITZI"      },
        { MDDS_ASCII("Seth"),       "SETH"       },
        { MDDS_ASCII("Sindy"),      "SINDY"      },
        { MDDS_ASCII("Tawanna"),    "TAWANNA"    },
        { MDDS_ASCII("Tyra"),       "TYRA"       },
    };

    packed_str_map_type db(entries.data(), entries.size());

    // Run some search.
    auto results = db.prefix_search("Mi");
    auto it = results.begin();
    assert(it != results.end());
    assert(it->first == "Michiko");
    assert(it->second == "MICHIKO");
    ++it;
    assert(it != results.end());
    assert(it->first == "Miriam");
    assert(it->second == "MIRIAM");
    ++it;
    assert(it != results.end());
    assert(it->first == "Mitzi");
    assert(it->second == "MITZI");
    ++it;
    assert(it == results.end());

    std::string saved_state;

    {
        std::ostringstream state;
        db.save_state(state);
        saved_state = state.str();
    }

    packed_str_map_type restored;

    {
        std::istringstream state(saved_state);
        restored.load_state(state);
    }

    assert(db == restored);
}

void test4()
{
    stack_printer __stack_printer__("trie_packed_test_save_and_load_state::test4");
    using map_type = packed_trie_map<trie::std_string_trait, std::vector<int64_t>>;

    std::vector<map_type::entry> entries =
    {
        { MDDS_ASCII("Abby"),     { 65, 98, 98, 121 }                      },
        { MDDS_ASCII("Ashley"),   { 65, 115, 104, 108, 101, 121 }          },
        { MDDS_ASCII("Christal"), { 67, 104, 114, 105, 115, 116, 97, 108 } },
        { MDDS_ASCII("Cory"),     { 67, 111, 114, 121 }                    },
        { MDDS_ASCII("Harley"),   { 72, 97, 114, 108, 101, 121 }           },
        { MDDS_ASCII("Kiara"),    { 75, 105, 97, 114, 97 }                 },
        { MDDS_ASCII("Mitzi"),    { 77, 105, 116, 122, 105 }               },
    };

    map_type db(entries.data(), entries.size());
    assert(db.size() == entries.size());

    std::string saved_state;
    {
        std::ostringstream state;
        db.save_state(state);
        saved_state = state.str();
    }

    map_type restored;

    {
        std::istringstream state(saved_state);
        restored.load_state(state);
    }

    assert(db == restored);
}

void test5()
{
    stack_printer __stack_printer__("trie_packed_test_save_and_load_state::test5");
    using map_type = packed_trie_map<trie::std_string_trait, float>;

    std::vector<map_type::entry> entries =
    {
        { MDDS_ASCII("Abby"),     1.0f },
        { MDDS_ASCII("Ashley"),   1.1f },
        { MDDS_ASCII("Christal"), 1.2f },
        { MDDS_ASCII("Cory"),     1.3f },
        { MDDS_ASCII("Harley"),   1.4f },
        { MDDS_ASCII("Kiara"),    1.5f },
        { MDDS_ASCII("Mitzi"),    1.6f },
    };

    map_type db(entries.data(), entries.size());
    assert(db.size() == entries.size());

    std::string saved_state;
    {
        std::ostringstream state;
        db.save_state(state);
        saved_state = state.str();
    }

    map_type restored;

    {
        std::istringstream state(saved_state);
        restored.load_state(state);
    }

    assert(db == restored);
}

template<typename SeqT>
void test6()
{
    stack_printer __stack_printer__("trie_packed_test_save_and_load_state::test6");
    using map_type = packed_trie_map<trie::std_string_trait, SeqT>;

    std::vector<typename map_type::entry> entries =
    {
        { MDDS_ASCII("Abby"),     { 65.0, 98.1, 98.2, 121.3 }              },
        { MDDS_ASCII("Ashley"),   { 65.0, 11.5, 1.04, 1.08, .101, .12586 } },
        { MDDS_ASCII("Christal"), { 67.0, -10.4, -114.236 }                },
        { MDDS_ASCII("Cory"),     { 67.0, 122.111 }                        },
        { MDDS_ASCII("Harley"),   { 72.0, 97.12, -1.114 }                  },
        { MDDS_ASCII("Kiara"),    { 75.0, 1.05, 9.7, 1.14, -97.5 }         },
        { MDDS_ASCII("Mitzi"),    { 77.0, 10.5, 11.6, 1.22, 10.5 }         },
    };

    map_type db(entries.data(), entries.size());
    assert(db.size() == entries.size());

    std::string saved_state;
    {
        std::ostringstream state;
        db.save_state(state);
        saved_state = state.str();
    }

    map_type restored;

    {
        std::istringstream state(saved_state);
        restored.load_state(state);
    }

    assert(db == restored);
}

void test7()
{
    stack_printer __stack_printer__("trie_packed_test_save_and_load_state::test7");
    using map_type = packed_trie_map<trie::std_string_trait, _custom_variable_value>;

    std::vector<map_type::entry> entries =
    {
        { MDDS_ASCII("Alan"),      1.2f },
        { MDDS_ASCII("Cory"),      -125 },
        { MDDS_ASCII("Eleni"),     966 },
        { MDDS_ASCII("Evia"),      -0.987f },
        { MDDS_ASCII("Nathaniel"), 0 },
        { MDDS_ASCII("Rebbecca"),  1.234f },
        { MDDS_ASCII("Rodrick"),   34253536 },
        { MDDS_ASCII("Stuart"),    12 },
        { MDDS_ASCII("Verline"),   56 },
    };

    map_type db(entries.data(), entries.size());
    assert(db.size() == entries.size());

    std::string saved_state;
    {
        std::ostringstream state;
        db.save_state<_custom_variable_serializer>(state);
        saved_state = state.str();
    }

    map_type restored;

    {
        std::istringstream state(saved_state);
        restored.load_state<_custom_variable_serializer>(state);
    }

    assert(db == restored);
}

void test8()
{
    stack_printer __stack_printer__("trie_packed_test_save_and_load_state::test8");
    using map_type = packed_trie_map<trie::std_string_trait, _custom_fixed_value>;

    std::vector<map_type::entry> entries =
    {
        { MDDS_ASCII("Bernardine"), "zero"  },
        { MDDS_ASCII("Donny"),      "two"   },
        { MDDS_ASCII("Julia"),      "one"   },
        { MDDS_ASCII("Lindsy"),     "three" },
        { MDDS_ASCII("Martine"),    "three" },
        { MDDS_ASCII("Shana"),      "two"   },
        { MDDS_ASCII("Sonia"),      "zero"  },
        { MDDS_ASCII("Tracie"),     "one"   },
        { MDDS_ASCII("Vanita"),     "two"   },
        { MDDS_ASCII("Yung"),       "zero"  },
    };

    map_type db(entries.data(), entries.size());
    assert(db.size() == entries.size());

    std::string saved_state;
    {
        std::ostringstream state;
        db.save_state<_custom_fixed_serializer>(state);
        saved_state = state.str();
    }

    map_type restored;

    {
        std::istringstream state(saved_state);
        restored.load_state<_custom_fixed_serializer>(state);
    }

    assert(db == restored);

    // Run some query to make sure it is still functional.
    auto it = restored.find("Tracie");
    assert(it->first == "Tracie");
    assert(it->second.value_string == "one");
}

void run()
{
    test1();
    test2();
    test3();
    test4();
    test5();
    test6<std::vector<double>>();
    test6<std::deque<double>>();
    test6<std::list<double>>();
    test7();
    test8();
}

}

void trie_test1()
{
    stack_printer __stack_printer__("::trie_test1");

    typedef trie_map<trie::std_string_trait, custom_string> trie_map_type;
    typedef packed_trie_map<trie::std_string_trait, custom_string> packed_trie_map_type;

    trie_map_type db;
    const trie_map_type& dbc = db;

    assert(db.size() == 0);
    db.insert(MDDS_ASCII("Barak"), custom_string("Obama"));
    assert(db.size() == 1);
    db.insert(MDDS_ASCII("Bob"), custom_string("Marley"));
    assert(db.size() == 2);
    db.insert(MDDS_ASCII("Hideki"), custom_string("Matsui"));
    assert(db.size() == 3);

    auto it = dbc.find(MDDS_ASCII("Barak"));
    assert(it->first == "Barak");
    custom_string res = it->second;
    assert(res.data == "Obama");

    res = dbc.find(MDDS_ASCII("Bob"))->second;
    assert(res.data == "Marley");
    res = dbc.find(MDDS_ASCII("Hideki"))->second;
    assert(res.data == "Matsui");

    // Non-existent key.
    it = dbc.find(MDDS_ASCII("Von"));
    assert(it == dbc.end());
    it = dbc.find(MDDS_ASCII("Bar"));
    assert(it == dbc.end());

    // Perform prefix search on "B", which should return both "Barak" and "Bob".
    // The results should be sorted.
    {
        auto matches = dbc.prefix_search(MDDS_ASCII("B"));
        size_t n = std::distance(matches.begin(), matches.end());
        assert(n == 2);
        auto it2 = matches.begin();
        assert(it2->first == "Barak");
        assert(it2->second.data == "Obama");
        ++it2;
        assert(it2->first == "Bob");
        assert(it2->second.data == "Marley");

        matches = dbc.prefix_search(MDDS_ASCII("Hi"));
        n = std::distance(matches.begin(), matches.end());
        assert(n == 1);
        it2 = matches.begin();
        assert(it2->first == "Hideki");
        assert(it2->second.data == "Matsui");

        // Invalid prefix searches.
        matches = dbc.prefix_search(MDDS_ASCII("Bad"));
        assert(matches.begin() == matches.end());
        matches = dbc.prefix_search(MDDS_ASCII("Foo"));
        assert(matches.begin() == matches.end());
    }

    {
        // Create a packed version from it, and make sure it still generates the
        // same results.
        packed_trie_map_type packed(dbc);
        assert(packed.size() == dbc.size());

        {
            auto results = packed.prefix_search(MDDS_ASCII("B"));
            size_t n = std::distance(results.begin(), results.end());
            assert(n == 2);
            auto it2 = results.begin();
            assert(it2->first == "Barak");
            assert(it2->second.data == "Obama");
            ++it2;
            assert(it2->first == "Bob");
            assert(it2->second.data == "Marley");
            ++it2;
            assert(it2 == results.end());
        }

        {
            auto results = dbc.prefix_search(MDDS_ASCII("Hi"));
            size_t n = std::distance(results.begin(), results.end());
            assert(n == 1);
            auto it2 = results.begin();
            assert(it2->first == "Hideki");
            assert(it2->second.data == "Matsui");
        }

        // Invalid prefix searches.
        auto results = dbc.prefix_search(MDDS_ASCII("Bad"));
        assert(results.begin() == results.end());
        results = dbc.prefix_search(MDDS_ASCII("Foo"));
        assert(results.begin() == results.end());
    }

    {
        auto packed = dbc.pack();
        auto results = packed.prefix_search(MDDS_ASCII("B"));
        size_t n = std::distance(results.begin(), results.end());
        assert(n == 2);
        auto it2 = results.begin();
        assert(it2->first == "Barak");
        assert(it2->second.data == "Obama");
        ++it2;
        assert(it2->first == "Bob");
        assert(it2->second.data == "Marley");
    }

    // Erase an existing key.
    bool erased = db.erase(MDDS_ASCII("Hideki"));
    assert(erased);
    assert(db.size() == 2);

    it = dbc.find(MDDS_ASCII("Hideki"));
    assert(it == dbc.end());

    // Try to erase a key that doesn't exist.
    erased = db.erase(MDDS_ASCII("Foo"));
    assert(!erased);
    assert(db.size() == 2);

    // Clear the whole thing.
    db.clear();
    assert(db.size() == 0);
}

void trie_test2()
{
    stack_printer __stack_printer__("::trie_test2");
    using key_trait = trie::std_container_trait<std::vector<uint16_t>>;
    using map_type = trie_map<key_trait, int>;
    using key_type = map_type::key_type;

    auto print_key = [](const std::vector<uint16_t>& key, const char* msg)
    {
        cout << msg << ": ";
        std::copy(key.begin(), key.end(), std::ostream_iterator<uint16_t>(std::cout, " "));
        cout << endl;
    };

    map_type db;
    key_type key = { 2393, 99, 32589, 107, 0, 65535 };
    print_key(key, "original");
    int value = 1;
    db.insert(key, value);
    assert(db.size() == 1);
    {
        auto it = db.begin();
        assert(it != db.end());
        assert(it->first == key);

        print_key(it->first, "from trie_map");
    }

    auto packed = db.pack();
    assert(packed.size() == 1);

    {
        auto it = packed.begin();
        assert(it != packed.end());
        print_key(it->first, "from packed_trie_map");
        assert(it->first == key);
    }
}

void trie_test_iterator_empty()
{
    stack_printer __stack_printer__("::trie_test_iterator_empty");
    typedef trie_map<trie::std_string_trait, int> trie_map_type;
    trie_map_type db;
    const trie_map_type& dbc = db;

    // empty container
    trie_map_type::const_iterator it = dbc.begin();
    trie_map_type::const_iterator ite = dbc.end();

    assert(it == ite);
    assert(db.begin() == dbc.begin()); // non-const vs const iterators
    assert(dbc.end() == db.end()); // const vs non-const iterators
}

void trie_test_iterator()
{
    stack_printer __stack_printer__("::trie_test_iterator");
    typedef trie_map<trie::std_string_trait, int> trie_map_type;
    using kv = trie_map_type::key_value_type;
    trie_map_type db;
    const trie_map_type& dbc = db;

    cout << "empty container" << endl;

    // empty container
    trie_map_type::const_iterator it = dbc.begin();
    trie_map_type::const_iterator ite = dbc.end();

    // The end iterator will never get invalidated since it only references
    // the root node which will never get modified as long as the parent
    // container is alive.

    assert(it == ite);

    cout << "one element" << endl;

    db.insert(MDDS_ASCII("a"), 1);
    it = dbc.begin();
    assert(it != ite);
    assert(*it == kv("a", 1));
    ++it;
    assert(it == ite);

    cout << "two elements" << endl;

    db.insert(MDDS_ASCII("ab"), 2);
    it = dbc.begin();
    assert(it != ite);
    assert(*it == kv("a", 1));
    ++it;
    assert(it != ite);
    assert(*it == kv("ab", 2));
    ++it;
    assert(it == ite);

    cout << "more than two elements" << endl;

    db.insert(MDDS_ASCII("aba"), 3);
    db.insert(MDDS_ASCII("abb"), 4);
    db.insert(MDDS_ASCII("abc"), 5);
    db.insert(MDDS_ASCII("bc"),  6);
    db.insert(MDDS_ASCII("bcd"), 7);

    it = dbc.begin();
    assert(*it == kv("a", 1));
    ++it;
    assert(*it == kv("ab", 2));
    ++it;
    assert(*it == kv("aba", 3));
    ++it;
    assert(*it == kv("abb", 4));
    ++it;
    assert(*it == kv("abc", 5));
    ++it;
    assert(*it == kv("bc",  6));
    ++it;
    assert(*it == kv("bcd", 7));
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
    --it;
    assert(*it == kv("a", 1));
    assert(it == dbc.begin());
    ++it;
    assert(*it == kv("ab",  2));
    ++it;
    assert(*it == kv("aba", 3));
    --it;
    assert(*it == kv("ab", 2));
    --it;
    assert(*it == kv("a",  1));
    ++it;
    assert(*it == kv("ab", 2));
    ++it;
    assert(*it == kv("aba", 3));

    assert(db.begin() != dbc.end()); // non-const vs const iterators
    assert(dbc.begin() != db.end()); // const vs non-const iterators
}

void trie_test_iterator_with_erase()
{
    stack_printer __stack_printer__("::trie_test_iterator_with_erase");
    typedef trie_map<trie::std_string_trait, int> trie_map_type;
    using kv = trie_map_type::key_value_type;
    trie_map_type db;
    const trie_map_type& dbc = db;
    bool check_true = false;

    db.insert(MDDS_ASCII("Python"), 1);
    db.insert(MDDS_ASCII("C++"),    2);

    auto it = dbc.begin(), ite = dbc.end();
    check_true = (*it++ == kv("C++",    2));
    assert(check_true);
    check_true = (*it++ == kv("Python", 1));
    assert(check_true);
    assert(it == ite);

    db.erase(MDDS_ASCII("C++"));
    it = dbc.begin();
    check_true = (*it++ == kv("Python", 1));
    assert(check_true);
    assert(it == ite);
    check_true = (*(--it) == kv("Python", 1));
    assert(check_true);
    assert(it == dbc.begin());

    db.clear();
    assert(dbc.begin() == dbc.end());

    db.insert(MDDS_ASCII("A"),   1);
    db.insert(MDDS_ASCII("AB"),  2);
    db.insert(MDDS_ASCII("ABC"), 3);
    db.erase(MDDS_ASCII("AB"));

    it = dbc.begin();
    check_true = (*it++ == kv("A",  1));
    assert(check_true);
    check_true = (*it++ == kv("ABC", 3));
    assert(check_true);
    assert(it == ite);

    check_true = (*(--it) == kv("ABC", 3));
    assert(check_true);
    check_true = (*(--it) == kv("A", 1));
    assert(check_true);
    assert(it == dbc.begin());

    db.clear();
    db.insert(MDDS_ASCII("A"),   1);
    db.insert(MDDS_ASCII("AB"),  2);
    db.insert(MDDS_ASCII("ABC"), 3);
    db.erase(MDDS_ASCII("ABC"));

    it = dbc.begin();
    check_true = (*it++ == kv("A",  1));
    assert(check_true);
    check_true = (*it++ == kv("AB", 2));
    assert(check_true);
    assert(it == ite);

    check_true = (*(--it) == kv("AB", 2));
    assert(check_true);
    check_true = (*(--it) == kv("A",  1));
    assert(check_true);
    assert(it == dbc.begin());

    it = ite;
    --it;
    assert(*it-- == kv("AB", 2)); // test post-decrement operator.
    assert(*it == kv("A",  1));
}

void trie_test_find_iterator()
{
    stack_printer __stack_printer__("::trie_test_find_iterator");
    typedef trie_map<trie::std_string_trait, int> trie_map_type;
    trie_map_type db;
    const trie_map_type& dbc = db;

    db.insert(MDDS_ASCII("a"),  1);
    db.insert(MDDS_ASCII("aa"), 2);
    db.insert(MDDS_ASCII("ab"), 3);
    db.insert(MDDS_ASCII("b"),  4);
    {
        auto it = dbc.find(MDDS_ASCII("a"));
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
        assert(it == dbc.end());

        it = dbc.find(MDDS_ASCII("aa"));
        assert(it->first == "aa");
        assert(it->second == 2);
        ++it;
        assert(it->first == "ab");
        assert(it->second == 3);
        ++it;
        assert(it->first == "b");
        assert(it->second == 4);
        ++it;
        assert(it == dbc.end());

        it = dbc.find(MDDS_ASCII("ab"));
        assert(it->first == "ab");
        assert(it->second == 3);
        ++it;
        assert(it->first == "b");
        assert(it->second == 4);
        ++it;
        assert(it == dbc.end());

        it = dbc.find(MDDS_ASCII("b"));
        assert(it->first == "b");
        assert(it->second == 4);
        ++it;
        assert(it == dbc.end());
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

        it = packed.find(MDDS_ASCII("aa"));
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

        it = packed.find(MDDS_ASCII("ab"));
        assert(it->first == "ab");
        assert(it->second == 3);
        ++it;
        assert(it->first == "b");
        assert(it->second == 4);
        ++it;
        assert(it == packed.end());

        it = packed.find(MDDS_ASCII("b"));
        assert(it->first == "b");
        assert(it->second == 4);
        ++it;
        assert(it == packed.end());
    }
}

void trie_test_prefix_search()
{
    stack_printer __stack_printer__("::trie_test_prefix_search");

    typedef trie_map<trie::std_string_trait, int> trie_map_type;
    trie_map_type db;
    const trie_map_type& dbc = db;

    db.insert(MDDS_ASCII("a"),  1);
    db.insert(MDDS_ASCII("aa"), 2);
    db.insert(MDDS_ASCII("ab"), 3);
    db.insert(MDDS_ASCII("b"),  4);

    cout << "Performing prefix search on 'a'..." << endl;

    trie_map_type::search_results results = dbc.prefix_search(MDDS_ASCII("a"));
    auto it = results.begin();
    auto ite = results.end();
    assert(it != ite);
    assert(it->first == "a");
    assert(it->second == 1);
    ++it;
    assert(it->first == "aa");
    assert(it->second == 2);
    ++it;
    assert(it->first == "ab");
    assert(it->second == 3);
    ++it;
    assert(it == ite);
    size_t n = std::distance(results.begin(), results.end());
    assert(n == 3);

    cout << "Performing prefix search on 'b'..." << endl;

    results = dbc.prefix_search(MDDS_ASCII("b"));
    it = results.begin();
    ite = results.end();
    assert(it != ite);
    assert(it->first == "b");
    assert(it->second == 4);
    ++it;
    assert(it == ite);
    --it;
    assert(it->first == "b");
    assert(it->second == 4);
    n = std::distance(results.begin(), results.end());
    assert(n == 1);

    // Only one element.
    db.clear();
    db.insert(MDDS_ASCII("dust"), 10);

    cout << "Performing prefix search on 'du'..." << endl;

    results = dbc.prefix_search(MDDS_ASCII("du"));
    it = results.begin();
    assert(it->first == "dust");
    assert(it->second == 10);
    bool check_true = (++it == results.end());
    assert(check_true);
    --it;
    assert(it->first == "dust");
    assert(it->second == 10);
    n = std::distance(results.begin(), results.end());
    assert(n == 1);
}

void trie_test_key_as_input()
{
    stack_printer __stack_printer__("::trie_test_key_as_input");

    typedef trie_map<trie::std_string_trait, int> trie_map_type;
    trie_map_type db;
    const trie_map_type& dbc = db;

    db.insert(std::string("string as key"), 1);
    db.insert("literal as key", 2);

    auto it = dbc.find("literal as key");
    assert(it != dbc.end());
    assert(it->first == "literal as key");
    assert(it->second == 2);

    auto results = dbc.prefix_search("str");
    auto rit = results.begin();
    assert(rit != results.end());
    assert(rit->first == "string as key");
    assert(rit->second == 1);
    ++rit;
    assert(rit == results.end());
}

void trie_test_copying()
{
    stack_printer __stack_printer__("::trie_test_copying");

    typedef trie_map<trie::std_string_trait, int> trie_map_type;
    trie_map_type db;
    assert(db.empty());

    {
        auto db_copied(db);
        assert(db_copied.empty());
    }

    db.insert("twenty", 20);
    db.insert("twelve", 12);
    assert(db.size() == 2);

    {
        // copy constructor
        auto db_copied(db);
        const trie_map_type& dbc_copied = db_copied;
        assert(db_copied.size() == 2);

        auto it = dbc_copied.find("twenty");
        assert(it != dbc_copied.end());
        assert(it->first == "twenty");
        assert(it->second == 20);

        it = dbc_copied.find("twelve");
        assert(it != dbc_copied.end());
        assert(it->first == "twelve");
        assert(it->second == 12);
    }

    {
        // copy assignment
        trie_map_type db_copied;
        db_copied = db;
        const trie_map_type& dbc_copied = db_copied;
        assert(db_copied.size() == 2);

        auto it = dbc_copied.find("twenty");
        assert(it != dbc_copied.end());
        assert(it->first == "twenty");
        assert(it->second == 20);

        it = dbc_copied.find("twelve");
        assert(it != dbc_copied.end());
        assert(it->first == "twelve");
        assert(it->second == 12);
    }

    {
        // move constructor
        auto db_copied(db);
        auto db_moved(std::move(db_copied));
        const trie_map_type& dbc_moved = db_moved;
        assert(db_moved.size() == 2);
        assert(db_copied.empty());

        auto it = dbc_moved.find("twenty");
        assert(it != dbc_moved.end());
        assert(it->first == "twenty");
        assert(it->second == 20);

        it = dbc_moved.find("twelve");
        assert(it != dbc_moved.end());
        assert(it->first == "twelve");
        assert(it->second == 12);
    }

    {
        // move assignment
        auto db_copied(db);
        trie_map_type db_moved;
        db_moved = std::move(db_copied);
        const trie_map_type& dbc_moved = db_moved;
        assert(db_moved.size() == 2);
        assert(db_copied.empty());

        auto it = dbc_moved.find("twenty");
        assert(it != dbc_moved.end());
        assert(it->first == "twenty");
        assert(it->second == 20);

        it = dbc_moved.find("twelve");
        assert(it != dbc_moved.end());
        assert(it->first == "twelve");
        assert(it->second == 12);
    }
}

void trie_test_value_update_from_iterator()
{
    stack_printer __stack_printer__("::trie_test_value_update_from_iterator");

    typedef trie_map<trie::std_string_trait, int> trie_map_type;
    trie_map_type db;
    db.insert("one", 1);
    db.insert("two", 2);
    db.insert("three", 3);

    trie_map_type::iterator it = db.begin();
    assert(it->first == "one");
    assert(it->second == 1);
    it->second = 10; // update the value.
    it = db.begin();
    assert(it->first == "one");
    assert(it->second == 10);

    it = db.find("three");
    assert(it->first == "three");
    assert(it->second == 3);
    it->second = 345; // update the value again.
    it = db.find("three");
    assert(it->first == "three");
    assert(it->second == 345);
}

int main(int argc, char** argv)
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
