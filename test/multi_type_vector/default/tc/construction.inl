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

void mtv_test_construction()
{
    stack_printer __stack_printer__(__FUNCTION__);

    {
        mtv_type db; // default constructor.
        assert(db.size() == 0);
        assert(db.empty());
        assert(db.block_size() == 0);
    }

    {
        // Create an empty segment of size 7.
        mtv_type db(7);
        assert(db.size() == 7);
        assert(db.block_size() == 1);
    }

    {
        // Create with initial value and size.
        mtv_type db(10, 1.0);
        assert(db.size() == 10);
        assert(db.block_size() == 1);
        assert(db.get<double>(0) == 1.0);
        assert(db.get<double>(9) == 1.0);
    }

    {
        // Create with initial value and size.
        mtv_type db(10, std::string("foo"));
        assert(db.size() == 10);
        assert(db.block_size() == 1);
        assert(db.get<std::string>(0) == "foo");
        assert(db.get<std::string>(9) == "foo");
    }

    {
        // Create with an array of values.
        std::vector<double> vals;
        vals.push_back(1.1);
        vals.push_back(1.2);
        vals.push_back(1.3);
        mtv_type db(vals.size(), vals.begin(), vals.end());
        assert(db.size() == 3);
        assert(db.block_size() == 1);
        assert(db.get<double>(0) == 1.1);
        assert(db.get<double>(1) == 1.2);
        assert(db.get<double>(2) == 1.3);
    }

    {
        std::vector<std::string> vals;
        mtv_type db_empty(0, vals.begin(), vals.end());
        assert(db_empty.size() == 0);
        assert(db_empty.block_size() == 0);

        vals.push_back("Andy");
        vals.push_back("Bruce");

        mtv_type db(2, vals.begin(), vals.end());
        assert(db.size() == 2);
        assert(db.block_size() == 1);
        assert(db.get<std::string>(0) == "Andy");
        assert(db.get<std::string>(1) == "Bruce");
    }

    {
        std::vector<int32_t> vals(10, 1);
        try
        {
            mtv_type db(20, vals.begin(), vals.end());
            assert(!"This construction should have failed due to incorrect initial array size.");
        }
        catch (const mdds::invalid_arg_error&)
        {
            // good.
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
