/*************************************************************************
 *
 * Copyright (c) 2016 Kohei Yoshida
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

#include "test_global.hpp"

#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#include <mdds/multi_type_vector.hpp>
#include <mdds/multi_type_vector_trait.hpp>
#include <mdds/multi_type_vector/side_iterator.hpp>

#include <iostream>
#include <vector>

using namespace std;
using namespace mdds;

typedef multi_type_vector<mtv::element_block_func> mtv_type;
typedef mtv::side_iterator<mtv_type> side_iterator_type;

void mtv_test_pointer_size1()
{
    stack_printer __stack_printer__("::mtv_test_pointer_size1");

    vector<mtv_type*> vectors;
    for (size_t i = 0; i < 2; ++i)
        vectors.push_back(new mtv_type(1));

    side_iterator_type side_iterator(vectors.begin(), vectors.end());

    for_each(vectors.begin(), vectors.end(), [](const mtv_type* p) { delete p; });
}

void mtv_test_non_pointer_size1()
{
    stack_printer __stack_printer__("::mtv_test_non_pointer_size1");

    vector<mtv_type> vectors;
    vectors.reserve(2);
    vectors.emplace_back(1);
    vectors.emplace_back(1);

    side_iterator_type side_iterator(vectors.begin(), vectors.end());
}

int main (int argc, char **argv)
{
    try
    {
        mtv_test_pointer_size1();
        mtv_test_non_pointer_size1();
    }
    catch (const std::exception& e)
    {
        cout << "Test failed: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}

