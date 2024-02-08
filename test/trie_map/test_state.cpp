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

#include <list>

#define _TEST_FUNC_SCOPE MDDS_TEST_FUNC_SCOPE_NS("trie_packed_test_save_and_load_state")

namespace trie_packed_test_save_and_load_state {

namespace {

struct _custom_variable_value
{
    enum class v_type
    {
        unknown,
        fp32,
        int64
    };

    v_type type;

    union
    {
        float fp32;
        int64_t int64;
    } value;

    _custom_variable_value() : type(v_type::unknown)
    {}

    _custom_variable_value(float v) : type(v_type::fp32)
    {
        value.fp32 = v;
    }

    _custom_variable_value(int v) : type(v_type::int64)
    {
        value.int64 = v;
    }

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
            default:;
        }
    }

    bool operator==(const _custom_variable_value& other) const
    {
        if (type != other.type)
            return false;

        switch (type)
        {
            case v_type::fp32:
                return value.fp32 == other.value.fp32;
            case v_type::int64:
                return value.int64 == other.value.int64;
            default:;
        }

        return true;
    }

    bool operator!=(const _custom_variable_value& other) const
    {
        return !operator==(other);
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

    _custom_fixed_value()
    {}

    _custom_fixed_value(const char* p) : value_string(p, std::strlen(p))
    {}

    bool operator==(const _custom_fixed_value& other) const
    {
        return value_string == other.value_string;
    }

    bool operator!=(const _custom_fixed_value& other) const
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

void test_1()
{
    _TEST_FUNC_SCOPE;

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

void test_2()
{
    _TEST_FUNC_SCOPE;

    packed_int_map_type::entry entries[] = {
        {MDDS_ASCII("bruce"), 5}, {MDDS_ASCII("charlie"), 6}, {MDDS_ASCII("charlotte"), 7},
        {MDDS_ASCII("david"), 8}, {MDDS_ASCII("dove"), 9},
    };

    packed_int_map_type db(entries, std::size(entries));

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

void test_3()
{
    _TEST_FUNC_SCOPE;

    std::vector<packed_str_map_type::entry> entries = {
        {MDDS_ASCII("Abby"), "ABBY"},
        {MDDS_ASCII("Ashley"), "ASHLEY"},
        {MDDS_ASCII("Candelaria"), "CANDELARIA"},
        {MDDS_ASCII("Carita"), "CARITA"},
        {MDDS_ASCII("Christal"), "CHRISTAL"},
        {MDDS_ASCII("Cory"), "CORY"},
        {MDDS_ASCII("Estrella"), "ESTRELLA"},
        {MDDS_ASCII("Etha"), "ETHA"},
        {MDDS_ASCII("Harley"), "HARLEY"},
        {MDDS_ASCII("Irish"), "IRISH"},
        {MDDS_ASCII("Kiara"), "KIARA"},
        {MDDS_ASCII("Korey"), "KOREY"},
        {MDDS_ASCII("Laurene"), "LAURENE"},
        {MDDS_ASCII("Michiko"), "MICHIKO"},
        {MDDS_ASCII("Miriam"), "MIRIAM"},
        {MDDS_ASCII("Mitzi"), "MITZI"},
        {MDDS_ASCII("Seth"), "SETH"},
        {MDDS_ASCII("Sindy"), "SINDY"},
        {MDDS_ASCII("Tawanna"), "TAWANNA"},
        {MDDS_ASCII("Tyra"), "TYRA"},
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

void test_4()
{
    _TEST_FUNC_SCOPE;

    using map_type = mdds::packed_trie_map<std::string, std::vector<int64_t>>;

    std::vector<map_type::entry> entries = {
        {MDDS_ASCII("Abby"), {65, 98, 98, 121}},
        {MDDS_ASCII("Ashley"), {65, 115, 104, 108, 101, 121}},
        {MDDS_ASCII("Christal"), {67, 104, 114, 105, 115, 116, 97, 108}},
        {MDDS_ASCII("Cory"), {67, 111, 114, 121}},
        {MDDS_ASCII("Harley"), {72, 97, 114, 108, 101, 121}},
        {MDDS_ASCII("Kiara"), {75, 105, 97, 114, 97}},
        {MDDS_ASCII("Mitzi"), {77, 105, 116, 122, 105}},
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

void test_5()
{
    _TEST_FUNC_SCOPE;

    using map_type = mdds::packed_trie_map<std::string, float>;

    std::vector<map_type::entry> entries = {
        {MDDS_ASCII("Abby"), 1.0f},  {MDDS_ASCII("Ashley"), 1.1f}, {MDDS_ASCII("Christal"), 1.2f},
        {MDDS_ASCII("Cory"), 1.3f},  {MDDS_ASCII("Harley"), 1.4f}, {MDDS_ASCII("Kiara"), 1.5f},
        {MDDS_ASCII("Mitzi"), 1.6f},
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
void test_6()
{
    _TEST_FUNC_SCOPE;

    using map_type = mdds::packed_trie_map<std::string, SeqT>;

    std::vector<typename map_type::entry> entries = {
        {MDDS_ASCII("Abby"), {65.0, 98.1, 98.2, 121.3}},
        {MDDS_ASCII("Ashley"), {65.0, 11.5, 1.04, 1.08, .101, .12586}},
        {MDDS_ASCII("Christal"), {67.0, -10.4, -114.236}},
        {MDDS_ASCII("Cory"), {67.0, 122.111}},
        {MDDS_ASCII("Harley"), {72.0, 97.12, -1.114}},
        {MDDS_ASCII("Kiara"), {75.0, 1.05, 9.7, 1.14, -97.5}},
        {MDDS_ASCII("Mitzi"), {77.0, 10.5, 11.6, 1.22, 10.5}},
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

void test_7()
{
    _TEST_FUNC_SCOPE;

    using map_type = mdds::packed_trie_map<std::string, _custom_variable_value>;

    std::vector<map_type::entry> entries = {
        {MDDS_ASCII("Alan"), 1.2f},        {MDDS_ASCII("Cory"), -125},   {MDDS_ASCII("Eleni"), 966},
        {MDDS_ASCII("Evia"), -0.987f},     {MDDS_ASCII("Nathaniel"), 0}, {MDDS_ASCII("Rebbecca"), 1.234f},
        {MDDS_ASCII("Rodrick"), 34253536}, {MDDS_ASCII("Stuart"), 12},   {MDDS_ASCII("Verline"), 56},
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

void test_8()
{
    _TEST_FUNC_SCOPE;

    using map_type = mdds::packed_trie_map<std::string, _custom_fixed_value>;

    std::vector<map_type::entry> entries = {
        {MDDS_ASCII("Bernardine"), "zero"}, {MDDS_ASCII("Donny"), "two"},     {MDDS_ASCII("Julia"), "one"},
        {MDDS_ASCII("Lindsy"), "three"},    {MDDS_ASCII("Martine"), "three"}, {MDDS_ASCII("Shana"), "two"},
        {MDDS_ASCII("Sonia"), "zero"},      {MDDS_ASCII("Tracie"), "one"},    {MDDS_ASCII("Vanita"), "two"},
        {MDDS_ASCII("Yung"), "zero"},
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

} // anonymous namespace

void run()
{
    test_1();
    test_2();
    test_3();
    test_4();
    test_5();
    test_6<std::vector<double>>();
    test_6<std::deque<double>>();
    test_6<std::list<double>>();
    test_7();
    test_8();
}

} // namespace trie_packed_test_save_and_load_state

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
