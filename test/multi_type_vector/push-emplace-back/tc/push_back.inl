/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2025 Kohei Yoshida
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

void test_push_back_copy()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type vec;
    user_cell::reset_counter();

    user_cell v;
    const user_cell& vref = v;

    // first push_back() call should trigger 1 copy and 0 moves
    vec.push_back(vref);
    user_cell::print_counters();
    assert(user_cell::copy_count == 1);
    assert(!user_cell::move_count);

    // second push_back() call should trigger another copy, it also may cause
    // one move due to reallocation of the buffer in the destination storage,
    // so we don't check the move counter.
    vec.push_back(vref);
    user_cell::print_counters();
    assert(user_cell::copy_count == 2);
}

void test_push_back_move()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type vec;
    user_cell::reset_counter();

    {
        // first push_back() call should trigger 0 copies and 1 move
        user_cell v;
        vec.push_back(std::move(v));
        user_cell::print_counters();
        assert(!user_cell::copy_count);
        assert(user_cell::move_count == 1);
    }

    {
        // second push_back() call should still trigger 0 copies and may trigger
        // move than 2 moves due to buffer reallocation of the destination
        // storage
        user_cell v;
        vec.push_back(std::move(v));
        user_cell::print_counters();
        assert(!user_cell::copy_count);
        assert(user_cell::move_count >= 2);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
