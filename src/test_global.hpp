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

#include <iostream>
#include <cstring>
#include <chrono>
#include <cstdint>

struct cmd_options
{
    bool test_func;
    bool test_perf;

    cmd_options() : test_func(false), test_perf(false) {}
};

bool parse_cmd_options(int argc, char** argv, cmd_options& opt)
{
    using namespace std;

    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            if (!strncmp(argv[i], "func", 4))
                opt.test_func = true;
            else if (!strncmp(argv[i], "perf", 4))
                opt.test_perf = true;
            else
            {
                cout << "unknown argument: " << argv[i] << endl;
                return false;
            }
        }
    }
    else
    {
        cout << "please specify test categories: [perf, func]" << endl;
        return false;
    }
    return true;
}

double get_current_time()
{
    uint64_t usec_since_epoch =
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

    return usec_since_epoch / 1000000.0;
}

class stack_watch
{
public:
    explicit stack_watch() : m_start_time(get_current_time()) {}
    void reset() { m_start_time = get_current_time(); }
    double get_duration() const { return get_current_time() - m_start_time; }

private:
    double m_start_time;
};

class stack_printer
{
public:
    explicit stack_printer(const char* msg) :
        m_msg(msg)
    {
        std::cout << m_msg << ": --begin" << std::endl;
        m_start_time = get_current_time();
    }

    ~stack_printer()
    {
        double end_time = get_current_time();
        std::cout << m_msg << ": --end (duration: " << (end_time-m_start_time) << " sec)" << std::endl;
    }

    void print_time(int line) const
    {
        double end_time = get_current_time();
        std::cout << m_msg << ": --(" << line << ") (duration: " << (end_time-m_start_time) << " sec)" << std::endl;
    }

private:
    std::string m_msg;
    double m_start_time;
};

#endif
