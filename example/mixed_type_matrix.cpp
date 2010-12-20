/*************************************************************************
 *
 * Copyright (c) 2010 Kohei Yoshida
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

#include <mdds/mixed_type_matrix.hpp>

#include <string>
#include <iostream>
#include <sstream>

using namespace mdds;
using namespace std;

typedef ::mdds::mixed_type_matrix<string, int> mx_type;

string str(const mx_type::element& e)
{
    ostringstream os;
    switch (e.m_type)
    {
        case element_empty:
            os << "empty";
        break;
        case element_numeric:
            os << "numeric (" << e.m_numeric << ")";
        break;
        case element_boolean:
            os << "boolean";
        break;
        case element_string:
            os << "string";
        break;
        default:
            os << "<unkwown>";
    }
    return os.str();
}

void print_element(const mx_type& mx, size_t row, size_t col)
{
    cout << "(" << row << "," << col << ") = ";
    switch (mx.get_type(row, col))
    {
        case element_boolean:
            cout << "boolean: " << (mx.get_boolean(row, col) ? "true" : "false");
        break;
        case element_numeric:
            cout << "numeric: " << mx.get_numeric(row, col);
        break;
        case element_string:
            cout << "string: " << *mx.get_string(row, col);
        break;
        case element_empty:
            cout << "empty";
        break;
        default:
            ;
    }
    cout << endl;
}

int main()
{
    // Create a matrix instance initialized with numeric zero values.
    mx_type mx(3, 3, matrix_density_filled_zero);

    // Insert values of various types.
    mx.set(0, 0, 5.5);
    mx.set(1, 0, true);
    mx.set(2, 1, new string("string value"));
    mx.set_empty(1, 2);

    // sizes.first contains row size, and sizes.second contains column size.
    mx_type::size_pair_type sizes = mx.size();

    // print all elements.
    for (size_t i = 0; i < sizes.first; ++i)
        for (size_t j = 0; j < sizes.second; ++j)
            print_element(mx, i, j);
}
