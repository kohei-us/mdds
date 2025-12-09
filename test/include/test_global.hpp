// SPDX-FileCopyrightText: 2010 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#ifdef NDEBUG
// release build
#undef NDEBUG
#include <cassert>
#define NDEBUG
#else
// debug build
#include <cassert>
#endif

#include <cstdio>
#include <string>
#include <cstdint>
#include <iostream>
#include <sstream>

#define TEST_ASSERT(condition) \
    do \
    { \
        if (!(condition)) \
        { \
            std::cerr << "Assertion failed: " << #condition << ", file " << __FILE__ << ", line " << __LINE__ \
                      << std::endl; \
            std::abort(); \
        } \
    } while (false)

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
    stack_printer(std::string msg);

    ~stack_printer();

    void print_time(int line) const;

private:
    std::string m_msg;
    double m_start_time;
};

using std::cerr;
using std::cout;
using std::endl;

#define MDDS_TEST_FUNC_SCOPE stack_printer __sp__(__func__)

#define MDDS_TEST_FUNC_SCOPE_MSG(stream) \
    std::ostringstream __sp_os__; \
    __sp_os__ << __func__ << ' ' << stream; \
    stack_printer __sp__(__sp_os__.str())

#define MDDS_TEST_FUNC_SCOPE_NS(stream) \
    std::ostringstream __sp_os__; \
    __sp_os__ << stream << "::" << __func__; \
    stack_printer __sp__(__sp_os__.str())
