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

using std::cout;
using std::endl;

class event_hdl
{
public:
    void element_block_acquired(mdds::mtv::base_element_block* block)
    {
        (void)block;
        cout << "  * element block acquired" << endl;
    }

    void element_block_released(mdds::mtv::base_element_block* block)
    {
        (void)block;
        cout << "  * element block released" << endl;
    }
};

struct trait : mdds::mtv::standard_element_blocks_traits
{
    using event_func = event_hdl;
};

using mtv_type = mdds::multi_type_vector<trait>;

int main() try
{
    mtv_type db;  // starts with an empty container.

    cout << "inserting string 'foo'..." << endl;
    db.push_back(std::string("foo"));  // creates a new string element block.

    cout << "inserting string 'bah'..." << endl;
    db.push_back(std::string("bah"));  // appends to an existing string block.

    cout << "inserting int 100..." << endl;
    db.push_back(int(100)); // creates a new int element block.

    cout << "emptying the container..." << endl;
    db.clear(); // releases both the string and int element blocks.

    cout << "exiting program..." << endl;

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}
//!code-end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
