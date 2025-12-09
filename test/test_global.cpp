/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.

#include <cstring>
#include <chrono>

cmd_options::cmd_options() : test_func(false), test_perf(false)
{}

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
        opt.test_func = true;

    return true;
}

double get_current_time()
{
    uint64_t usec_since_epoch =
        std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch())
            .count();

    return usec_since_epoch / 1000000.0;
}

stack_watch::stack_watch() : m_start_time(get_current_time())
{}

void stack_watch::reset()
{
    m_start_time = get_current_time();
}

double stack_watch::get_duration() const
{
    return get_current_time() - m_start_time;
}

stack_printer::stack_printer(const char* msg) : m_msg(msg)
{
    std::cout << m_msg << ": --begin" << std::endl;
    m_start_time = get_current_time();
}

stack_printer::stack_printer(std::string msg) : m_msg(std::move(msg))
{
    std::cout << m_msg << ": --begin" << std::endl;
    m_start_time = get_current_time();
}

stack_printer::~stack_printer()
{
    double end_time = get_current_time();
    std::cout << m_msg << ": --end (duration: " << (end_time - m_start_time) << " sec)" << std::endl;
}

void stack_printer::print_time(int line) const
{
    double end_time = get_current_time();
    std::cout << m_msg << ": --(" << line << ") (duration: " << (end_time - m_start_time) << " sec)" << std::endl;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
