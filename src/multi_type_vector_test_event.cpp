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
    size_t block_count;  // number of element (data) blocks

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
        db.clear();
        assert(db.event_handler().block_count == 0);

        db.push_back(5.5);  // create a new block.
        assert(db.event_handler().block_count == 1);
        db.push_back(6.6);  // no new block creation.
        assert(db.event_handler().block_count == 1);
        db.push_back(string("foo"));  // another new block.
        assert(db.event_handler().block_count == 2);

        // This should remove the last string block.
        db.resize(2);
        assert(db.event_handler().block_count == 1);

        // This should have no effect on the block count.
        db.resize(1);
        assert(db.event_handler().block_count == 1);

        // This should remove the last remaining block.
        db.resize(0);
        assert(db.event_handler().block_count == 0);
    }

    {
        mtv_type db(5);
        assert(db.event_handler().block_count == 0);

        db.set(0, true);
        assert(db.event_handler().block_count == 1);
        db.set(1, 12.2);
        assert(db.event_handler().block_count == 2);

        db.set(4, string("foo"));
        assert(db.event_handler().block_count == 3);
        db.set(3, string("bar"));
        assert(db.event_handler().block_count == 3);

        // This should delete the top two element blocks.
        db.set_empty(0, 1);
        assert(db.event_handler().block_count == 1);

        // Now, delete the bottom one.
        db.set_empty(3, 4);
        assert(db.event_handler().block_count == 0);

        // Create and delete a block in the middle.
        db.set(3, false);
        assert(db.event_handler().block_count == 1);
        db.set_empty(3, 3);
        assert(db.event_handler().block_count == 0);

        db.set(2, 10.5);
        db.set(3, string("hmm"));
        assert(db.event_handler().block_count == 2);
        db.set_empty(3, 3);
        assert(db.event_handler().block_count == 1);

        // Start over.
        db.clear();
        assert(db.event_handler().block_count == 0);

        db.push_back(1.1);
        db.push_back(1.2);
        db.push_back(1.3);
        assert(db.event_handler().block_count == 1);

        // Put empty block in the moddle.
        db.set_empty(1, 1);
        assert(db.event_handler().block_count == 2);
    }

    {
        mtv_type db(4, 1.2);
        assert(db.event_handler().block_count == 1);

        // Split the block into two.
        db.insert_empty(2, 2);
        assert(db.event_handler().block_count == 2);
    }

    {
        mtv_type db(2);
        db.set(1, 1.2);  // This creates a new element block.
        assert(db.event_handler().block_count == 1);
        db.set(0, 1.1);  // The element block count should not change.
        assert(db.event_handler().block_count == 1);
    }

    {
        mtv_type db(2);
        db.set(1, string("test"));
        assert(db.event_handler().block_count == 1);
        db.set(0, 1.1);
        assert(db.event_handler().block_count == 2);
        db.set(0, true);
        assert(db.event_handler().block_count == 2);
    }

    {
        mtv_type db(2);
        db.set(0, string("test")); // This creates a new string block.
        assert(db.event_handler().block_count == 1);
        db.set(1, string("foo"));  // This appends to the existing string block.
        assert(db.event_handler().block_count == 1);
    }

    {
        mtv_type db(3);
        db.set(0, string("test"));
        assert(db.event_handler().block_count == 1);
        db.set(2, string("foo"));
        assert(db.event_handler().block_count == 2);
        db.set(1, string("bar")); // This merges all data into a single string block.
        assert(db.event_handler().block_count == 1);
    }

    {
        mtv_type db(4);
        db.set(0, string("test"));
        assert(db.event_handler().block_count == 1);
        db.set(2, string("foo1"));
        assert(db.event_handler().block_count == 2);
        db.set(3, string("foo2"));
        assert(db.event_handler().block_count == 2);
        db.set(1, string("bar")); // This merges all data into a single string block.
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
