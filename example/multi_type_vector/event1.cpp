/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2020 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

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
