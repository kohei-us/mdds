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

//!code-start
#include <mdds/multi_type_vector.hpp>
#include <iostream>
#include <vector>
#include <string>

using std::cout;
using std::endl;

using mtv_type = mdds::multi_type_vector<mdds::mtv::standard_element_blocks_traits>;

template<typename BlockT>
void print_block(const mtv_type::value_type& v)
{
    for (const auto& elem : BlockT::range(*v.data))
    {
        cout << " * " << elem << endl;
    }
}

int main() try
{
    mtv_type con(20); // Initialized with 20 empty elements.

    // Set values individually.
    con.set(0, 1.1);
    con.set(1, 1.2);
    con.set(2, 1.3);

    // Set a sequence of values in one step.
    std::vector<double> vals = { 10.1, 10.2, 10.3, 10.4, 10.5 };
    con.set(3, vals.begin(), vals.end());

    // Set string values.
    con.set<std::string>(10, "Andy");
    con.set<std::string>(11, "Bruce");
    con.set<std::string>(12, "Charlie");

    // Iterate through all blocks and print all elements.
    for (const auto& v : con)
    {
        switch (v.type)
        {
            case mdds::mtv::element_type_double:
            {
                cout << "numeric block of size " << v.size << endl;
                print_block<mdds::mtv::double_element_block>(v);
                break;
            }
            case mdds::mtv::element_type_string:
            {
                cout << "string block of size " << v.size << endl;
                print_block<mdds::mtv::string_element_block>(v);
                break;
            }
            case mdds::mtv::element_type_empty:
                cout << "empty block of size " << v.size << endl;
                cout << " - no data - " << endl;
            default:
                ;
        }
    }

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}
//!code-end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
