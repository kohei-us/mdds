/*************************************************************************
 *
 * Copyright (c) 2016-2021 Kohei Yoshida
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

#include "test_global.hpp" // This must be the first header to be included.

#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#include <mdds/multi_type_vector/aos/main.hpp>
#include <mdds/multi_type_vector/trait.hpp>
#include <mdds/multi_type_vector/collection.hpp>

#include <iostream>
#include <vector>
#include <deque>
#include <memory>

using mtv_type = mdds::mtv::aos::multi_type_vector<mdds::mtv::element_block_func>;
using cols_type = mdds::mtv::collection<mtv_type>;

#include "all.inl"

int main()
{
    try
    {
        mtv_test_empty();
        mtv_test_pointer_size1();
        mtv_test_unique_pointer_size1();
        mtv_test_shared_pointer_size2();
        mtv_test_non_pointer_size1();
        mtv_test_invalid_collection();
        mtv_test_sub_element_ranges();
        mtv_test_sub_element_ranges_invalid();
        mtv_test_sub_collection_ranges_invalid();
        mtv_test_boolean_block();
    }
    catch (const std::exception& e)
    {
        cout << "Test failed: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}
