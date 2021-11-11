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

#include <mdds/segment_tree.hpp>
#include <string>
#include <iostream>

using std::cout;
using std::endl;

typedef ::mdds::segment_tree<long, std::string> db_type;

struct string_printer
{
    void operator() (const std::string& s) const
    {
        cout << "search hit: " << s << endl;
    }
};

int main() try
{
    db_type db;
    std::string A("A");
    std::string B("B");
    std::string C("C");

    // Insert data into the tree.
    db.insert(0,  10, A);
    db.insert(2,  20, B);
    db.insert(10, 15, C);

    // Don't forget to build it before calling search().
    db.build_tree();

    // Run search and get the result.
    db_type::search_results result = db.search(5);

    // Print the result.
    cout << "result size: " << result.size() << endl;
    std::for_each(result.begin(), result.end(), string_printer());
}
catch (...)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
