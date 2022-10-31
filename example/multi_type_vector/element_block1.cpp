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
using mdds::mtv::double_element_block;
using mdds::mtv::string_element_block;

using mtv_type = mdds::multi_type_vector<mdds::mtv::standard_element_blocks_traits>;

int main() try
{
    mtv_type db;  // starts with an empty container.

    db.push_back(1.1);
    db.push_back(1.2);
    db.push_back(1.3);
    db.push_back(1.4);
    db.push_back(1.5);

    db.push_back(std::string("A"));
    db.push_back(std::string("B"));
    db.push_back(std::string("C"));
    db.push_back(std::string("D"));
    db.push_back(std::string("E"));

    // At this point, you have 2 blocks in the container.
    cout << "block size: " << db.block_size() << endl;
    cout << "--" << endl;

    // Get an iterator that points to the first block in the primary array.
    mtv_type::const_iterator it = db.begin();

    // Get a pointer to the raw array of the numeric element block using the
    // 'data' method.
    const double* p = double_element_block::data(*it->data);

    // Print the elements from this raw array pointer.
    for (const double* p_end = p + it->size; p != p_end; ++p)
        cout << *p << endl;

    cout << "--" << endl;

    ++it; // move to the next block, which is a string block.

    // Get a pointer to the raw array of the string element block.
    const std::string* pz = string_element_block::data(*it->data);

    // Print out the string elements.
    for (const std::string* pz_end = pz + it->size; pz != pz_end; ++pz)
        cout << *pz << endl;

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}
//!code-end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
