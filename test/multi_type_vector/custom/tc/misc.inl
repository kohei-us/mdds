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
    MDDS_TEST_FUNC_SCOPE;

    mdds::mtv::element_t ct;

    // Basic types
    ct = mtv_type::get_element_type(double(12.3));
    TEST_ASSERT(ct == mdds::mtv::element_type_double);
    ct = mtv_type::get_element_type(std::string());
    TEST_ASSERT(ct == mdds::mtv::element_type_string);
    ct = mtv_type::get_element_type(static_cast<uint64_t>(12));
    TEST_ASSERT(ct == mdds::mtv::element_type_uint64);
    ct = mtv_type::get_element_type(true);
    TEST_ASSERT(ct == mdds::mtv::element_type_boolean);
    ct = mtv_type::get_element_type(false);
    TEST_ASSERT(ct == mdds::mtv::element_type_boolean);

    // Custom cell type
    user_cell* p = nullptr;
    ct = mtv_type::get_element_type(p);
    TEST_ASSERT(ct == element_type_user_block && ct >= mdds::mtv::element_type_user_start);
    ct = mtv_type::get_element_type(static_cast<muser_cell*>(nullptr));
    TEST_ASSERT(ct == element_type_muser_block && ct >= mdds::mtv::element_type_user_start);
    ct = mtv_fruit_type::get_element_type(unknown_fruit);
    TEST_ASSERT(ct == element_type_fruit_block && ct >= mdds::mtv::element_type_user_start);
}

void mtv_test_misc_block_identifier()
{
    MDDS_TEST_FUNC_SCOPE;

    TEST_ASSERT(user_cell_block::block_type == element_type_user_block);
    TEST_ASSERT(muser_cell_block::block_type == element_type_muser_block);
    TEST_ASSERT(fruit_block::block_type == element_type_fruit_block);
    TEST_ASSERT(date_block::block_type == element_type_date_block);
}

void mtv_test_misc_custom_block_func1()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_fruit_type db(10);
    db.set(0, apple);
    db.set(1, orange);
    db.set(2, mango);
    db.set(3, peach);
    TEST_ASSERT(db.block_size() == 2);
    TEST_ASSERT(db.get_type(0) == element_type_fruit_block);
    TEST_ASSERT(db.get<my_fruit_type>(0) == apple);
    TEST_ASSERT(db.get<my_fruit_type>(1) == orange);
    TEST_ASSERT(db.get<my_fruit_type>(2) == mango);
    TEST_ASSERT(db.get<my_fruit_type>(3) == peach);
    db.set<int>(1, 234);
    TEST_ASSERT(db.block_size() == 4);
    db.set(1, apple);
    TEST_ASSERT(db.block_size() == 2);
}

void mtv_test_misc_custom_block_func3()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv3_type db(10);

    // Insert custom elements.
    db.set(0, new muser_cell(12.3));
    db.set(1, apple);
    db.set(2, date(1989, 12, 13));
    db.set(3, date(2011, 8, 7));
    TEST_ASSERT(db.get_type(0) == element_type_muser_block);
    TEST_ASSERT(db.get_type(1) == element_type_fruit_block);
    TEST_ASSERT(db.get_type(2) == element_type_date_block);
    TEST_ASSERT(db.get_type(3) == element_type_date_block);
    TEST_ASSERT(db.get<muser_cell*>(0)->value == 12.3);
    TEST_ASSERT(db.get<my_fruit_type>(1) == apple);
    TEST_ASSERT(db.get<date>(2).year == 1989);
    TEST_ASSERT(db.get<date>(2).month == 12);
    TEST_ASSERT(db.get<date>(2).day == 13);
    TEST_ASSERT(db.get<date>(3).year == 2011);
    TEST_ASSERT(db.get<date>(3).month == 8);
    TEST_ASSERT(db.get<date>(3).day == 7);
    TEST_ASSERT(db.block_size() == 4);

    // We should still support the primitive types.
    db.set(8, 34.56);
    TEST_ASSERT(db.get<double>(8) == 34.56);
}

void mtv_test_misc_release()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type db(3);
    muser_cell c1(1.2), c2(1.3); // allocated on the stack.
    db.set(0, &c1);
    db.set(2, &c2);
    db.release(); // Prevent invalid free when db goes out of scope.

    // Variant with no argument should make the container empty after the call.
    TEST_ASSERT(db.empty());

    db.push_back(new muser_cell(10.0));
    db.push_back(&c1);
    db.push_back(&c2);
    db.push_back(new muser_cell(10.1));

    TEST_ASSERT(db.size() == 4);
    TEST_ASSERT(db.block_size() == 1);

    // Release those allocated on the stack to avoid double deletion.
    mtv_type::iterator it = db.release_range(1, 2);

    // Check the integrity of the returned iterator.
    TEST_ASSERT(it->type == mdds::mtv::element_type_empty);
    TEST_ASSERT(it->size == 2);
    mtv_type::iterator check = it;
    --check;
    TEST_ASSERT(check == db.begin());
    check = it;
    ++check;
    ++check;
    TEST_ASSERT(check == db.end());

    db.push_back(new muser_cell(10.2));
    TEST_ASSERT(db.size() == 5);

    muser_cell* p1 = db.get<muser_cell*>(3);
    muser_cell* p2 = db.get<muser_cell*>(4);
    TEST_ASSERT(p1->value == 10.1);
    TEST_ASSERT(p2->value == 10.2);

    // Pass iterator as a position hint.
    it = db.release_range(it, 3, 4);
    TEST_ASSERT(db.block_size() == 2);

    // Check the returned iterator.
    check = it;
    --check;
    TEST_ASSERT(check == db.begin());
    TEST_ASSERT(check->type == element_type_muser_block);
    TEST_ASSERT(check->size == 1);
    TEST_ASSERT(it->type == mdds::mtv::element_type_empty);
    TEST_ASSERT(it->size == 4);
    ++it;
    TEST_ASSERT(it == db.end());

    // It should be safe to delete these instances now that they've been released.
    delete p1;
    delete p2;
}

void mtv_test_misc_construction_with_array()
{
    MDDS_TEST_FUNC_SCOPE;
    {
        std::vector<muser_cell*> vals;
        vals.push_back(new muser_cell(2.1));
        vals.push_back(new muser_cell(2.2));
        vals.push_back(new muser_cell(2.3));
        mtv_type db(vals.size(), vals.begin(), vals.end());

        db.set(1, 10.2); // overwrite.
        TEST_ASSERT(db.size() == 3);
        TEST_ASSERT(db.block_size() == 3);
        TEST_ASSERT(db.get<muser_cell*>(0)->value == 2.1);
        TEST_ASSERT(db.get<double>(1) == 10.2);
        TEST_ASSERT(db.get<muser_cell*>(2)->value == 2.3);

        // Now those heap objects are owned by the container.  Clearing the
        // array shouldn't leak.
        vals.clear();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
