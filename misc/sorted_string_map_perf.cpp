/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2014 Kohei Yoshida
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

#include "../include/mdds/sorted_string_map.hpp"

#include <cstdlib>
#include <iostream>
#include <cstring>

#include <boost/unordered_map.hpp>

using namespace std;

#include <stdio.h>
#include <string>
#include <sys/time.h>

namespace {

class stack_printer
{
public:
    explicit stack_printer(const char* msg) :
        m_msg(msg)
    {
        fprintf(stdout, "%s: --begin\n", m_msg.c_str());
        m_start_time = getTime();
    }

    ~stack_printer()
    {
        double end_time = getTime();
        fprintf(stdout, "%s: --end (duration: %g sec)\n", m_msg.c_str(), (end_time - m_start_time));
    }

    void printTime(int line) const
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --(%d) (duration: %g sec)\n", m_msg.c_str(), line, (fEndTime - m_start_time));
    }

private:
    double getTime() const
    {
        timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec + tv.tv_usec / 1000000.0;
    }

    ::std::string m_msg;
    double m_start_time;
};

}

enum name_type {
    name_none = 0,
    name_andy,
    name_bruce,
    name_charlie,
    name_david
};

typedef mdds::sorted_string_map<name_type> map_type;

map_type::entry entries[] =
{
    { "andy", name_andy },
    { "andy1", name_andy },
    { "andy13", name_andy },
    { "bruce", name_bruce },
    { "charlie", name_charlie },
    { "david", name_david },
};

typedef boost::unordered_map<std::string, name_type> hashmap_type;

void init_hash_map(hashmap_type& hm)
{
    size_t n = sizeof(entries) / sizeof(entries[0]);
    const map_type::entry* p = entries;
    const map_type::entry* pend = p + n;
    for (; p != pend; ++p)
        hm.insert(hashmap_type::value_type(p->key, p->value));
}

void run(map_type& sm, const char* input)
{
    name_type type = sm.find(input, strlen(input));
}

void run_hash(hashmap_type& hm, const char* input)
{
    name_type type = name_none;
    hashmap_type::const_iterator it = hm.find(input);
    if (it != hm.end())
        type = it->second;
}

const char* tests[] = {
    "andy",
    "david",
    "charlie",
    "andy1",
    "bruce",
    "blah",
    "andy13"
};

int main()
{
    static const size_t repeat_count = 10000000;

    map_type sorted_map(entries, sizeof(entries)/sizeof(entries[0]), name_none);
    size_t n = sorted_map.size();
    cout << "entry count = " << n << endl;

    {
        stack_printer __stack_printer__("sorted entry");
        for (size_t rep = 0; rep < repeat_count; ++rep)
        {
            for (size_t i = 0; i < n; ++i)
                run(sorted_map, tests[i]);
        }
    }

    hashmap_type hm;
    init_hash_map(hm);
    {
        stack_printer __stack_printer__("hash map");
        for (size_t rep = 0; rep < repeat_count; ++rep)
        {
            for (size_t i = 0; i < n; ++i)
                run_hash(hm, tests[i]);
        }
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
