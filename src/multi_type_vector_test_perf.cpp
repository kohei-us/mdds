/*************************************************************************
 *
 * Copyright (c) 2011-2013 Kohei Yoshida
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

#include <mdds/multi_type_vector.hpp>
#include <mdds/multi_type_vector_trait.hpp>

#include <cassert>
#include <sstream>
#include <vector>
#include <deque>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/noncopyable.hpp>

#define ARRAY_SIZE(x) sizeof(x)/sizeof(x[0])

using namespace std;
using namespace mdds;

namespace {

typedef mdds::multi_type_vector<mdds::mtv::element_block_func> mtv_type;

void mtv_perf_test_block_position_lookup()
{
    size_t n = 24000;

    {
        // Default insertion which always looks up the right element block
        // from the position of the first block.  As such, as the block size
        // grows, so does the time it takes to search for the right block.

        mtv_type db(n*2);
        double val1 = 1.1;
        int val2 = 23;
        stack_printer __stack_printer__("::mtv_perf_test_block_position_lookup::default insertion");
        for (size_t i = 0; i < n; ++i)
        {
            size_t pos1 = i*2, pos2 = i*2 + 1;
            db.set(pos1, val1);
            db.set(pos2, val2);
        }
    }

    {
        // As a solution for this, we can use an iterator to specify the start
        // position, which eliminates the above scalability problem nicely.

        mtv_type db(n*2);
        mtv_type::iterator pos_hint = db.begin();
        double val1 = 1.1;
        int val2 = 23;
        stack_printer __stack_printer__("::mtv_perf_test_block_position_lookup::insertion with position hint");
        for (size_t i = 0; i < n; ++i)
        {
            size_t pos1 = i*2, pos2 = i*2 + 1;
            pos_hint = db.set(pos_hint, pos1, val1);
            pos_hint = db.set(pos_hint, pos2, val2);
        }
    }
}

}

int main (int argc, char **argv)
{
    mtv_perf_test_block_position_lookup();
    return EXIT_SUCCESS;
}
