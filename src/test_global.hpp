/*************************************************************************
 *
 * Copyright (c) 2010, 2011 Kohei Yoshida
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

#ifndef __TEST_GLOBAL_HPP__
#define __TEST_GLOBAL_HPP__

#include <stdio.h>
#include <string>
#ifdef _WIN32
#include <windows.h>
#undef max
#undef min
#else
#include <sys/time.h>
#endif

#include <iostream>
#include <cstring>

#ifdef _WIN32
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;
#if _MSC_VER < 1600
typedef char            int8_t;
#endif
typedef short           int16_t;
typedef int             int32_t;
#endif

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
#ifdef _WIN32
    FILETIME ft;
    __int64 *time64 = reinterpret_cast<__int64 *>(&ft);
    GetSystemTimeAsFileTime(&ft);
    return *time64 / 10000000.0;
#else
    timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
#endif
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
        fprintf(stdout, "%s: --begin\n", m_msg.c_str());
        m_start_time = get_current_time();
    }

    ~stack_printer()
    {
        double end_time = get_current_time();
        fprintf(stdout, "%s: --end (duration: %g sec)\n", m_msg.c_str(), (end_time-m_start_time));
    }

    void print_time(int line) const
    {
        double end_time = get_current_time();
        fprintf(stdout, "%s: --(%d) (duration: %g sec)\n", m_msg.c_str(), line, (end_time-m_start_time));
    }

private:
    std::string m_msg;
    double m_start_time;
};

#endif
