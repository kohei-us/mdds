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

void mtv_test_position()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db(10, false);
    mtv_type::iterator check;
    mtv_type::const_iterator const_check;
    db.set(6, 1.1);
    db.set(7, 1.2);
    db.set(8, 1.3);
    db.set(9, 1.4);

    mtv_type::position_type pos = db.position(0);
    assert(pos.first == db.begin());
    assert(pos.second == 0);

    pos = db.position(1);
    assert(pos.first == db.begin());
    assert(pos.second == 1);

    pos = db.position(5);
    assert(pos.first == db.begin());
    assert(pos.second == 5);

    check = db.begin();
    ++check;

    // These positions should be on the 2nd block.

    pos = db.position(6);
    assert(pos.first == check);
    assert(pos.second == 0);

    pos = db.position(7);
    assert(pos.first == check);
    assert(pos.second == 1);

    pos = db.position(9);
    assert(pos.first == check);
    assert(pos.second == 3);

    {
        // Make sure you get the right element.
        mtv_type::iterator it = pos.first;
        assert(it->type == mdds::mtv::element_type_double);
        assert(it->data);
        mdds::mtv::double_element_block::iterator it_elem = mdds::mtv::double_element_block::begin(*it->data);
        advance(it_elem, pos.second);
        assert(*it_elem == 1.4);
    }

    // Quick check for the const variant.
    const mtv_type& db_ref = db;
    mtv_type::const_position_type const_pos = db_ref.position(3);
    assert(const_pos.first == db_ref.begin());
    assert(const_pos.second == 3);
    const_pos = db_ref.position(const_pos.first, 7);
    const_check = db_ref.begin();
    ++const_check;
    assert(const_pos.first == const_check);
    assert(const_pos.second == 1);

    // Check for the variant that takes position hint.
    pos = db.position(0);
    assert(pos.first == db.begin());
    assert(pos.second == 0);

    pos = db.position(pos.first, 6);
    check = db.begin();
    ++check;
    assert(pos.first == check);
    assert(pos.second == 0);

    pos = db.position(pos.first, 7);
    assert(pos.first == check);
    assert(pos.second == 1);

    pos = db.position(pos.first, 9);
    assert(pos.first == check);
    assert(pos.second == 3);
}

void mtv_test_position_next()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db(10);
    db.set(2, 1.1);
    db.set(3, 1.2);
    db.set(4, std::string("A"));
    db.set(5, std::string("B"));
    db.set(6, std::string("C"));
    db.set(7, true);
    db.set(8, false);

    mtv_type::position_type pos = db.position(0);
    assert(mtv_type::logical_position(pos) == 0);
    assert(pos.first->type == mdds::mtv::element_type_empty);

    pos = mtv_type::next_position(pos);
    assert(mtv_type::logical_position(pos) == 1);
    assert(pos.first->type == mdds::mtv::element_type_empty);

    pos = mtv_type::next_position(pos);
    assert(mtv_type::logical_position(pos) == 2);
    assert(pos.first->type == mdds::mtv::element_type_double);
    assert(mtv_type::get<mdds::mtv::double_element_block>(pos) == 1.1);

    pos = mtv_type::next_position(pos);
    assert(mtv_type::logical_position(pos) == 3);
    assert(pos.first->type == mdds::mtv::element_type_double);
    assert(mtv_type::get<mdds::mtv::double_element_block>(pos) == 1.2);

    pos = mtv_type::next_position(pos);
    assert(mtv_type::logical_position(pos) == 4);
    assert(pos.first->type == mdds::mtv::element_type_string);
    assert(mtv_type::get<mdds::mtv::string_element_block>(pos) == "A");

    pos = mtv_type::next_position(pos);
    assert(mtv_type::logical_position(pos) == 5);
    assert(pos.first->type == mdds::mtv::element_type_string);
    assert(mtv_type::get<mdds::mtv::string_element_block>(pos) == "B");

    pos = mtv_type::next_position(pos);
    assert(mtv_type::logical_position(pos) == 6);
    assert(pos.first->type == mdds::mtv::element_type_string);
    assert(mtv_type::get<mdds::mtv::string_element_block>(pos) == "C");

    pos = mtv_type::next_position(pos);
    assert(mtv_type::logical_position(pos) == 7);
    assert(pos.first->type == mdds::mtv::element_type_boolean);
    assert(mtv_type::get<mdds::mtv::boolean_element_block>(pos) == true);

    pos = mtv_type::next_position(pos);
    assert(mtv_type::logical_position(pos) == 8);
    assert(pos.first->type == mdds::mtv::element_type_boolean);
    assert(mtv_type::get<mdds::mtv::boolean_element_block>(pos) == false);

    pos = mtv_type::next_position(pos);
    assert(mtv_type::logical_position(pos) == 9);
    assert(pos.first->type == mdds::mtv::element_type_empty);

    pos = mtv_type::next_position(pos);
    assert(pos.first == db.end());
}

void mtv_test_position_advance()
{
    stack_printer __stack_printer__(__FUNCTION__);
    mtv_type db(10);
    db.set(2, 1.1);
    db.set(3, 1.2);
    db.set(4, std::string("A"));
    db.set(5, std::string("B"));
    db.set(6, std::string("C"));

    mtv_type::position_type pos = db.position(0);
    assert(mtv_type::logical_position(pos) == 0);

    pos = mtv_type::advance_position(pos, 4);
    assert(mtv_type::logical_position(pos) == 4);

    pos = mtv_type::advance_position(pos, 5);
    assert(mtv_type::logical_position(pos) == 9);

    pos = mtv_type::advance_position(pos, -3);
    assert(mtv_type::logical_position(pos) == 6);

    pos = mtv_type::advance_position(pos, -6);
    assert(mtv_type::logical_position(pos) == 0);

    pos = mtv_type::advance_position(pos, 10);
    assert(pos.first == db.end());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
