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

void mtv_test_swap()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv3_type db1(5), db2(2);
    db1.set(0, new muser_cell(1.1));
    db1.set(1, new muser_cell(1.2));
    db1.set(2, new muser_cell(1.3));
    db1.set(3, new muser_cell(1.4));
    db1.set(4, new muser_cell(1.5));

    db2.set(0, string("A"));
    db2.set(1, string("B"));

    db1.swap(2, 3, db2, 0);

    // swap blocks of equal size, one managed, and one default.

    db1.clear();
    db1.resize(10);
    db2.clear();
    db2.resize(10);

    db1.set(3, 2.1);
    db1.set(4, 2.2);
    db1.set(5, 2.3);

    db2.set(3, new muser_cell(3.1));
    db2.set(4, new muser_cell(3.2));
    db2.set(5, new muser_cell(3.3));

    db2.swap(3, 5, db1, 3);

    assert(db1.size() == 10);
    assert(db1.block_size() == 3);
    assert(db2.size() == 10);
    assert(db2.block_size() == 3);

    assert(db1.get<muser_cell*>(3)->value == 3.1);
    assert(db1.get<muser_cell*>(4)->value == 3.2);
    assert(db1.get<muser_cell*>(5)->value == 3.3);
    assert(db2.get<double>(3) == 2.1);
    assert(db2.get<double>(4) == 2.2);
    assert(db2.get<double>(5) == 2.3);

    db2.swap(3, 5, db1, 3);

    assert(db1.get<double>(3) == 2.1);
    assert(db1.get<double>(4) == 2.2);
    assert(db1.get<double>(5) == 2.3);
    assert(db2.get<muser_cell*>(3)->value == 3.1);
    assert(db2.get<muser_cell*>(4)->value == 3.2);
    assert(db2.get<muser_cell*>(5)->value == 3.3);

    // Same as above, except that the source segment splits the block into 2.

    db1.clear();
    db1.resize(10);
    db2.clear();
    db2.resize(10);

    db1.set(3, 2.1);
    db1.set(4, 2.2);

    db2.set(3, new muser_cell(3.1));
    db2.set(4, new muser_cell(3.2));
    db2.set(5, new muser_cell(3.3));

    db2.swap(3, 4, db1, 3);

    // Another scenario that used to crash on double delete.

    db1.clear();
    db1.resize(10);
    db2.clear();
    db2.resize(10);

    db1.set(2, new muser_cell(4.1));
    db1.set(3, 4.2);
    db1.set(4, new muser_cell(4.3));

    db2.set(3, new muser_cell(6.1));
    db2.set(4, 6.2);
    db2.set(5, 6.3);

    assert(db1.get<muser_cell*>(2)->value == 4.1);
    assert(db1.get<double>(3) == 4.2);
    assert(db1.get<muser_cell*>(4)->value == 4.3);

    assert(db2.get<muser_cell*>(3)->value == 6.1);
    assert(db2.get<double>(4) == 6.2);
    assert(db2.get<double>(5) == 6.3);

    db2.swap(4, 4, db1, 4);

    assert(db1.get<muser_cell*>(2)->value == 4.1);
    assert(db1.get<double>(3) == 4.2);
    assert(db1.get<double>(4) == 6.2);

    assert(db2.get<muser_cell*>(3)->value == 6.1);
    assert(db2.get<muser_cell*>(4)->value == 4.3);
    assert(db2.get<double>(5) == 6.3);

    // One more on double deletion...

    db1.clear();
    db1.resize(10);
    db2.clear();
    db2.resize(10);

    db1.set(0, 2.1);
    db1.set(1, 2.2);
    db1.set(2, 2.3);
    db1.set(3, new muser_cell(4.5));

    db2.set(2, new muser_cell(3.1));
    db2.set(3, new muser_cell(3.2));
    db2.set(4, new muser_cell(3.3));

    db1.swap(2, 2, db2, 3);

    assert(db1.get<double>(0) == 2.1);
    assert(db1.get<double>(1) == 2.2);
    assert(db1.get<muser_cell*>(2)->value == 3.2);
    assert(db1.get<muser_cell*>(3)->value == 4.5);

    assert(db2.get<muser_cell*>(2)->value == 3.1);
    assert(db2.get<double>(3) == 2.3);
    assert(db2.get<muser_cell*>(4)->value == 3.3);

    assert(db1.check_block_integrity());
    assert(db2.check_block_integrity());
}

void mtv_test_swap_2()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv3_type db1(3), db2(3);

    db1.set(0, new muser_cell(1.1));
    db1.set(1, new muser_cell(1.2));

    db2.set(0, 1.2);
    db2.set(1, std::string("foo"));

    // Repeat the same swap twice.
    db1.swap(0, 1, db2, 0);
    assert(db1.check_block_integrity());
    assert(db2.check_block_integrity());
    assert(db2.get<muser_cell*>(0)->value == 1.1);
    assert(db2.get<muser_cell*>(1)->value == 1.2);
    assert(db1.get<double>(0) == 1.2);
    assert(db1.get<std::string>(1) == "foo");

    db1.swap(0, 1, db2, 0);
    assert(db1.check_block_integrity());
    assert(db2.check_block_integrity());
    assert(db1.get<muser_cell*>(0)->value == 1.1);
    assert(db1.get<muser_cell*>(1)->value == 1.2);
    assert(db2.get<double>(0) == 1.2);
    assert(db2.get<std::string>(1) == "foo");
}

void mtv_test_release()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db(3);
    muser_cell c1(1.2), c2(1.3); // allocated on the stack.
    db.set(0, &c1);
    db.set(2, &c2);
    db.release(); // Prevent invalid free when db goes out of scope.

    // Variant with no argument should make the container empty after the call.
    assert(db.empty());

    db.push_back(new muser_cell(10.0));
    db.push_back(&c1);
    db.push_back(&c2);
    db.push_back(new muser_cell(10.1));

    assert(db.size() == 4);
    assert(db.block_size() == 1);

    // Release those allocated on the stack to avoid double deletion.
    mtv_type::iterator it = db.release_range(1, 2);

    // Check the integrity of the returned iterator.
    assert(it->type == mtv::element_type_empty);
    assert(it->size == 2);
    mtv_type::iterator check = it;
    --check;
    assert(check == db.begin());
    check = it;
    ++check;
    ++check;
    assert(check == db.end());

    db.push_back(new muser_cell(10.2));
    assert(db.size() == 5);

    muser_cell* p1 = db.get<muser_cell*>(3);
    muser_cell* p2 = db.get<muser_cell*>(4);
    assert(p1->value == 10.1);
    assert(p2->value == 10.2);

    // Pass iterator as a position hint.
    it = db.release_range(it, 3, 4);
    assert(db.block_size() == 2);

    // Check the returned iterator.
    check = it;
    --check;
    assert(check == db.begin());
    assert(check->type == element_type_muser_block);
    assert(check->size == 1);
    assert(it->type == mtv::element_type_empty);
    assert(it->size == 4);
    ++it;
    assert(it == db.end());

    // It should be safe to delete these instances now that they've been released.
    delete p1;
    delete p2;
}

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
        mtv_test_basic();
        mtv_test_basic_equality();
        mtv_test_managed_block();
        mtv_test_transfer();
        mtv_test_swap();
        mtv_test_swap_2();
        mtv_test_release();
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
