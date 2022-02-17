/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2021 Kohei Yoshida
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

void mtv_test_misc_types()
{
    stack_printer __stack_printer__(__FUNCTION__);

    mdds::mtv::element_t ct;

    // Basic types
    ct = mtv_type::get_element_type(double(12.3));
    assert(ct == mdds::mtv::element_type_double);
    ct = mtv_type::get_element_type(std::string());
    assert(ct == mdds::mtv::element_type_string);
    ct = mtv_type::get_element_type(static_cast<uint64_t>(12));
    assert(ct == mdds::mtv::element_type_uint64);
    ct = mtv_type::get_element_type(true);
    assert(ct == mdds::mtv::element_type_boolean);
    ct = mtv_type::get_element_type(false);
    assert(ct == mdds::mtv::element_type_boolean);

    // Custom cell type
    user_cell* p = nullptr;
    ct = mtv_type::get_element_type(p);
    assert(ct == element_type_user_block && ct >= mdds::mtv::element_type_user_start);
    ct = mtv_type::get_element_type(static_cast<muser_cell*>(nullptr));
    assert(ct == element_type_muser_block && ct >= mdds::mtv::element_type_user_start);
    ct = mtv_fruit_type::get_element_type(unknown_fruit);
    assert(ct == element_type_fruit_block && ct >= mdds::mtv::element_type_user_start);
}

void mtv_test_misc_block_identifier()
{
    stack_printer __stack_printer__(__FUNCTION__);
    assert(user_cell_block::block_type == element_type_user_block);
    assert(muser_cell_block::block_type == element_type_muser_block);
    assert(fruit_block::block_type == element_type_fruit_block);
    assert(date_block::block_type == element_type_date_block);
}

void mtv_test_misc_custom_block_func1()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_fruit_type db(10);
    db.set(0, apple);
    db.set(1, orange);
    db.set(2, mango);
    db.set(3, peach);
    assert(db.block_size() == 2);
    assert(db.get_type(0) == element_type_fruit_block);
    assert(db.get<my_fruit_type>(0) == apple);
    assert(db.get<my_fruit_type>(1) == orange);
    assert(db.get<my_fruit_type>(2) == mango);
    assert(db.get<my_fruit_type>(3) == peach);
    db.set<int>(1, 234);
    assert(db.block_size() == 4);
    db.set(1, apple);
    assert(db.block_size() == 2);
}

void mtv_test_misc_custom_block_func3()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv3_type db(10);

    // Insert custom elements.
    db.set(0, new muser_cell(12.3));
    db.set(1, apple);
    db.set(2, date(1989, 12, 13));
    db.set(3, date(2011, 8, 7));
    assert(db.get_type(0) == element_type_muser_block);
    assert(db.get_type(1) == element_type_fruit_block);
    assert(db.get_type(2) == element_type_date_block);
    assert(db.get_type(3) == element_type_date_block);
    assert(db.get<muser_cell*>(0)->value == 12.3);
    assert(db.get<my_fruit_type>(1) == apple);
    assert(db.get<date>(2).year == 1989);
    assert(db.get<date>(2).month == 12);
    assert(db.get<date>(2).day == 13);
    assert(db.get<date>(3).year == 2011);
    assert(db.get<date>(3).month == 8);
    assert(db.get<date>(3).day == 7);
    assert(db.block_size() == 4);

    // We should still support the primitive types.
    db.set(8, 34.56);
    assert(db.get<double>(8) == 34.56);
}

void mtv_test_misc_release()
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
    assert(it->type == mdds::mtv::element_type_empty);
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
    assert(it->type == mdds::mtv::element_type_empty);
    assert(it->size == 4);
    ++it;
    assert(it == db.end());

    // It should be safe to delete these instances now that they've been released.
    delete p1;
    delete p2;
}

void mtv_test_misc_construction_with_array()
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
