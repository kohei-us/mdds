/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2020 Kohei Yoshida
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

#include <mdds/multi_type_vector.hpp>

#include <iostream>
#include <string>
#include <chrono>

namespace {

class stack_printer
{
public:
    explicit stack_printer(const char* msg) :
        m_msg(msg)
    {
        std::cout << m_msg << ": --begin" << std::endl;
        m_start_time = get_time();
    }

    ~stack_printer()
    {
        double end_time = get_time();
        std::cout << m_msg << ": --end (duration: " << (end_time-m_start_time) << " sec)" << std::endl;
    }

    void print_time(int line) const
    {
        double end_time = get_time();
        std::cout << m_msg << ": --(" << line << ") (duration: " << (end_time-m_start_time) << " sec)" << std::endl;
    }

private:
    double get_time() const
    {
        unsigned long usec_since_epoch =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();

        return usec_since_epoch / 1000000.0;
    }

    std::string m_msg;
    double m_start_time;
};

}

void run_no_position_hint()
{
    stack_printer __stack_printer__("::run_no_position_hint");

    //!code-start: no-pos-hint
    using mtv_type = mdds::multi_type_vector<mdds::mtv::standard_element_blocks_traits>;

    size_t size = 50000;

    // Initialize the container with one empty block of size 50000.
    mtv_type db(size);

    // Set non-empty value at every other logical position from top down.
    for (size_t i = 0; i < size; ++i)
    {
        if (i % 2)
            db.set<double>(i, 1.0);
    }
    //!code-end: no-pos-hint
}

void run_with_position_hint()
{
    stack_printer __stack_printer__("::run_with_position_hint");

    //!code-start: pos-hint
    using mtv_type = mdds::multi_type_vector<mdds::mtv::standard_element_blocks_traits>;

    size_t size = 50000;

    // Initialize the container with one empty block of size 50000.
    mtv_type db(size);
    mtv_type::iterator pos = db.begin();

    // Set non-empty value at every other logical position from top down.
    for (size_t i = 0; i < size; ++i)
    {
        if (i % 2)
            // Pass the position hint as the first argument, and receive a new
            // one returned from the method for the next call.
            pos = db.set<double>(pos, i, 1.0);
    }
    //!code-end: pos-hint
}

int main() try
{
    run_no_position_hint();
    run_with_position_hint();

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
