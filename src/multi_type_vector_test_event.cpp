/*************************************************************************
 *
 * Copyright (c) 2015 Kohei Yoshida
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

#include <iostream>

using namespace std;
using namespace mdds;

struct event_block_counter
{
    size_t block_count;

    event_block_counter() : block_count(0) {}

    void element_block_created(const mtv::base_element_block* block)
    {
        ++block_count;
    }

    void element_block_destroyed(const mtv::base_element_block* block)
    {
        --block_count;
    }
};

void mtv_test_block_counter()
{
    stack_printer __stack_printer__("::mtv_test_block_counter");

    typedef multi_type_vector<mtv::element_block_func, event_block_counter> mtv_type;

    {
        // Initializing with an empty block should not create any element block.
        mtv_type db(10);
        assert(db.event_handler().block_count == 0);
    }

    {
        // Initializing with one element block of size 10.
        mtv_type db(10, 1.2);
        assert(db.event_handler().block_count == 1);
    }
}

int main (int argc, char **argv)
{
    try
    {
        mtv_test_block_counter();
    }
    catch (const std::exception& e)
    {
        cout << "Test failed: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}
