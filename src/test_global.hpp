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
typedef char            int8_t;
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

class StackPrinter
{
public:
    explicit StackPrinter(const char* msg) :
        msMsg(msg)
    {
        fprintf(stdout, "%s: --begin\n", msMsg.c_str());
        mfStartTime = getTime();
    }

    ~StackPrinter()
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --end (duration: %g sec)\n", msMsg.c_str(), (fEndTime-mfStartTime));
    }

    void printTime(int line) const
    {
        double fEndTime = getTime();
        fprintf(stdout, "%s: --(%d) (duration: %g sec)\n", msMsg.c_str(), line, (fEndTime-mfStartTime));
    }

private:
    double getTime() const
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

    ::std::string msMsg;
    double mfStartTime;
};

#endif
