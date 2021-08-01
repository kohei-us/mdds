/*************************************************************************
 *
 * Copyright (c) 2012-2021 Kohei Yoshida
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
#include "test_main.hpp"

#include <memory>
#include <sstream>
#include <vector>

using namespace std;
using namespace mdds;

void mtv_test_construction_with_array()
{
    stack_printer __stack_printer__(__FUNCTION__);
    {
        std::vector<muser_cell*> vals;
        vals.push_back(new muser_cell(2.1));
        vals.push_back(new muser_cell(2.2));
        vals.push_back(new muser_cell(2.3));
        mtv_type db(vals.size(), vals.begin(), vals.end());

        db.set(1, 10.2); // overwrite.
        assert(db.size() == 3);
        assert(db.block_size() == 3);
        assert(db.get<muser_cell*>(0)->value == 2.1);
        assert(db.get<double>(1) == 10.2);
        assert(db.get<muser_cell*>(2)->value == 2.3);

        // Now those heap objects are owned by the container.  Clearing the
        // array shouldn't leak.
        vals.clear();
    }
}

int main (int argc, char** argv)
{
    try
    {
        mtv_test_misc_types();
        mtv_test_misc_block_identifier();
        mtv_test_misc_custom_block_func1();
        mtv_test_misc_custom_block_func3();
        mtv_test_misc_release();
        mtv_test_basic();
        mtv_test_basic_equality();
        mtv_test_managed_block();
        mtv_test_transfer();
        mtv_test_swap();
        mtv_test_swap_2();
        mtv_test_construction_with_array();
    }
    catch (const std::exception& e)
    {
        cout << "Test failed: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
