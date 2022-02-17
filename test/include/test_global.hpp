/*************************************************************************
 *
 * Copyright (c) 2010-2018 Kohei Yoshida
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

#ifndef INCLUDED_TEST_GLOBAL_HPP
#define INCLUDED_TEST_GLOBAL_HPP

#ifdef NDEBUG
// release build
#undef NDEBUG
#include <cassert>
#define NDEBUG
#else
// debug build
#include <cassert>
#endif

#include <stdio.h>
#include <string>
#ifdef _WIN32
#include <windows.h>
#undef max
#undef min
#endif

#include <cstdint>
#include <iostream>

struct cmd_options
{
    bool test_func;
    bool test_perf;

    cmd_options();
};

bool parse_cmd_options(int argc, char** argv, cmd_options& opt);

double get_current_time();

class stack_watch
{
public:
    stack_watch();

    void reset();
    double get_duration() const;

private:
    double m_start_time;
};

class stack_printer
{
public:
    explicit stack_printer(const char* msg);

    ~stack_printer();

    void print_time(int line) const;

private:
    std::string m_msg;
    double m_start_time;
};

using std::cerr;
using std::cout;
using std::endl;

#endif
