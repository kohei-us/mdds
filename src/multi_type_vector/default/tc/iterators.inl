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

void mtv_test_iterators()
{
    stack_printer __stack_printer__(__FUNCTION__);
    {
        mtv_type db(5);
        mtv_type::const_iterator it;
        it = db.begin();
        mtv_type::const_iterator it_end = db.end();
        size_t len = std::distance(it, it_end);
        assert(len == 1);
        assert(it != it_end);
        assert(it->type == mdds::mtv::element_type_empty);
        assert(it->size == 5);
        const mtv_type::const_iterator::value_type& val = *it;
        assert(val.type == it->type);
        assert(val.size == it->size);

        ++it;
        assert(it == it_end);
    }

    {
        mtv_type db(6);
        db.set(0, 1.1);
        db.set(1, 2.2);
        db.set(4, std::string("boo"));
        db.set(5, std::string("hoo"));
        assert(db.block_size() == 3);
        {
            // Forward iterator
            mtv_type::const_iterator it = db.begin(), it_end = db.end();
            size_t len = std::distance(it, it_end);
            assert(len == 3);
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_double);
            assert(it->size == 2);

            ++it;
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_empty);
            assert(it->size == 2);

            ++it;
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_string);
            assert(it->size == 2);

            ++it;
            assert(it == it_end);
        }

        {
            // Reverse iterator
            mtv_type::const_reverse_iterator it = db.rbegin(), it_end = db.rend();
            size_t len = std::distance(it, it_end);
            assert(len == 3);
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_string);
            assert(it->size == 2);

            ++it;
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_empty);
            assert(it->size == 2);

            ++it;
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_double);
            assert(it->size == 2);

            ++it;
            assert(it == it_end);
        }

        {
            // Reverse iterator (C++11)
            mtv_type::const_reverse_iterator it = db.crbegin(), it_end = db.crend();
            size_t len = std::distance(it, it_end);
            assert(len == 3);
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_string);
            assert(it->size == 2);

            ++it;
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_empty);
            assert(it->size == 2);

            ++it;
            assert(it != it_end);
            assert(it->type == mdds::mtv::element_type_double);
            assert(it->size == 2);

            ++it;
            assert(it == it_end);
        }
    }

    {
        // Make sure that decrementing the iterator calculates the position correctly.
        mtv_type db(10);
        db.set(0, true);
        mtv_type::const_iterator it = db.begin();
        assert(it->position == 0);
        assert(it->size == 1);
        ++it;
        assert(it->position == 1);
        assert(it->size == 9);
        --it;
        assert(it->position == 0);
        assert(it->size == 1);
        assert(it == db.begin());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

