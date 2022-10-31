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
#include <mdds/multi_type_vector/collection.hpp>

#include <vector>
#include <iostream>

void example1()
{
    //!code-start: declare
    using mtv_type = mdds::multi_type_vector<mdds::mtv::standard_element_blocks_traits>;
    using collection_type = mdds::mtv::collection<mtv_type>;

    std::vector<mtv_type> columns(5);
    //!code-end: declare

    //!code-start: header-row
    // Populate the header row.
    const char* headers[] = { "ID", "Make", "Model", "Year", "Color" };
    size_t i = 0;
    for (const char* v : headers)
        columns[i++].push_back<std::string>(v);
    //!code-end: header-row

    //!code-start: column-1
    // Fill column 1.
    int c1_values[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };

    for (int v : c1_values)
        columns[0].push_back(v);
    //!code-end: column-1

    //!code-start: column-2
    // Fill column 2.
    const char* c2_values[] =
    {
        "Nissan", "Mercedes-Benz", "Nissan", "Suzuki", "Saab", "Subaru", "GMC", "Mercedes-Benz", "Toyota", "Nissan",
        "Mazda", "Dodge", "Ford", "Bentley", "GMC", "Audi", "GMC", "Mercury", "Pontiac", "BMW",
    };

    for (const char* v : c2_values)
        columns[1].push_back<std::string>(v);
    //!code-end: column-2

    //!code-start: column-3
    // Fill column 3.
    const char* c3_values[] =
    {
        "Frontier", "W201", "Frontier", "Equator", "9-5", "Tribeca", "Yukon XL 2500", "E-Class", "Camry Hybrid", "Frontier",
        "MX-5", "Ram Van 1500", "Edge", "Azure", "Sonoma Club Coupe", "S4", "3500 Club Coupe", "Villager", "Sunbird", "3 Series",
    };

    for (const char* v : c3_values)
        columns[2].push_back<std::string>(v);
    //!code-end: column-3

    //!code-start: column-4
    // Fill column 4.  Replace -1 with "unknown".
    int32_t c4_values[] =
    {
        1998, 1986, 2009, -1, -1, 2008, 2009, 2008, 2010, 2001,
        2008, 2000, -1, 2009, 1998, 2013, 1994, 2000, 1990, 1993,
    };

    for (int32_t v : c4_values)
    {
        if (v < 0)
            // Insert a string value "unknown".
            columns[3].push_back<std::string>("unknown");
        else
            columns[3].push_back(v);
    }
    //!code-end: column-4

    //!code-start: column-5
    // Fill column 5
    const char* c5_values[] =
    {
        "Turquoise", "Fuscia", "Teal", "Fuscia", "Green", "Khaki", "Pink", "Goldenrod", "Turquoise", "Yellow",
        "Orange", "Goldenrod", "Fuscia", "Goldenrod", "Mauv", "Crimson", "Turquoise", "Teal", "Indigo", "LKhaki",
    };

    for (const char* v : c5_values)
        columns[4].push_back<std::string>(v);
    //!code-end: column-5

    //!code-start: wrap
    // Wrap the columns with the 'collection'...
    collection_type rows(columns.begin(), columns.end());
    //!code-end: wrap

    //!code-start: traverse-row
    // Traverse the tabular data in row-wise direction.
    for (const auto& cell : rows)
    {
        if (cell.index > 0)
            // Insert a column separator before each cell except for the ones in the first column.
            std::cout << " | ";

        switch (cell.type)
        {
            // In this example, we use two element types only.
            case mdds::mtv::element_type_int32:
                std::cout << cell.get<mdds::mtv::int32_element_block>();
                break;
            case mdds::mtv::element_type_string:
                std::cout << cell.get<mdds::mtv::string_element_block>();
                break;
            default:
                std::cout << "???"; // The default case should not hit in this example.
        }

        if (cell.index == 4)
            // We are in the last column. Insert a line break.
            std::cout << std::endl;
    }
    //!code-end: traverse-row

    //!code-start: limit-range
    rows.set_collection_range(1, 2); // only columns 2 and 3.
    rows.set_element_range(1, 10);   // only rows 2 through 11.
    //!code-end: limit-range

    std::cout << "--" << std::endl;

    //!code-start: traverse-row-range
    for (const auto& cell : rows)
    {
        if (cell.index > 1)
            // Insert a column separator before each cell except for the ones in the first column.
            std::cout << " | ";

        switch (cell.type)
        {
            // In this example, we use two element types only.
            case mdds::mtv::element_type_int32:
                std::cout << cell.get<mdds::mtv::int32_element_block>();
                break;
            case mdds::mtv::element_type_string:
                std::cout << cell.get<mdds::mtv::string_element_block>();
                break;
            default:
                std::cout << "???"; // The default case should not hit in this example.
        }

        if (cell.index == 2)
            // We are in the last column. Insert a line break.
            std::cout << std::endl;
    }
    //!code-end: traverse-row-range
}

int main() try
{
    example1();

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
