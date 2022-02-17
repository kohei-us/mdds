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

/**
 * This test is to be run with valgrind, to ensure no memory leak occurs.
 */
void mtv_test_managed_block()
{
    stack_printer __stack_printer__(__FUNCTION__);
    {
        mtv_type db(1);
        db.set(0, new muser_cell(1.0));
        const muser_cell* p = db.get<muser_cell*>(0);
        assert(p->value == 1.0);
        db.set(0, new muser_cell(2.0)); // overwrite.
        p = db.get<muser_cell*>(0);
        assert(p->value == 2.0);
    }

    {
        // Overwrite with empty cells.
        mtv_type db(3);

        // Empty the upper part.
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));
        db.set_empty(0, 0);

        // Empty the lower part.
        db.set(0, new muser_cell(4.0));
        db.set_empty(2, 2);

        // Empty the middle part.
        db.set(2, new muser_cell(5.0));
        db.set_empty(1, 1);
    }

    {
        // More overwrite with empty cells.
        mtv_type db(3);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, 3.0);
        db.set_empty(1, 2);

        db.set(0, std::string("foo"));
        db.set(1, new muser_cell(4.0));
        db.set(2, new muser_cell(5.0));
        db.set_empty(0, 1);

        db.set(0, new muser_cell(6.0));
        db.set(1, static_cast<uint64_t>(12));
        db.set_empty(0, 2);
    }

    {
        // Another case for set_empty().
        mtv_type db(5);
        db.set(0, 1.2);
        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));
        db.set(3, new muser_cell(4.0));
        db.set(4, new muser_cell(5.0));
        db.set_empty(2, 4);

        db.set(2, new muser_cell(3.0));
        db.set(3, new muser_cell(4.0));
        db.set(4, new muser_cell(5.0));
        db.set_empty(1, 2);

        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));
        db.set_empty(2, 3);
    }

    {
        // Test for cloning.
        mtv_type db(3);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));

        // swap
        mtv_type db2;
        db2.swap(db);
        assert(db.empty());
        assert(db2.get<muser_cell*>(0)->value == 1.0);
        assert(db2.get<muser_cell*>(1)->value == 2.0);
        assert(db2.get<muser_cell*>(2)->value == 3.0);
        db.swap(db2);
        assert(db2.empty());
        assert(db.get<muser_cell*>(0)->value == 1.0);
        assert(db.get<muser_cell*>(1)->value == 2.0);
        assert(db.get<muser_cell*>(2)->value == 3.0);

        // copy constructor
        mtv_type db_copied(db);
        assert(db_copied.size() == 3);
        assert(db_copied.get<muser_cell*>(0)->value == 1.0);
        assert(db_copied.get<muser_cell*>(1)->value == 2.0);
        assert(db_copied.get<muser_cell*>(2)->value == 3.0);

        // Assignment.
        mtv_type db_assigned = db;
        assert(db_assigned.size() == 3);
        assert(db_assigned.get<muser_cell*>(0)->value == 1.0);
        assert(db_assigned.get<muser_cell*>(1)->value == 2.0);
        assert(db_assigned.get<muser_cell*>(2)->value == 3.0);
    }

    {
        // Resize and clear
        mtv_type db(3);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));
        db.resize(1);
        assert(db.get<muser_cell*>(0)->value == 1.0);

        db.clear();
    }

    {
        // Overwrite with a cell of different type.
        mtv_type db(3);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));
        db.set(1, 4.5);
    }

    {
        // Erase (single block)
        mtv_type db(3);

        // Erase the whole thing.
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));
        db.erase(0, 2);
        assert(db.empty());

        // Erase top.
        db.resize(3);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));
        db.erase(0, 1);
        assert(db.size() == 1);

        // Erase bottom.
        db.resize(3);
        db.set(1, new muser_cell(4.0));
        db.set(2, new muser_cell(5.0));
        db.erase(1, 2);
        assert(db.size() == 1);

        // Erase middle.
        db.resize(3);
        db.set(1, new muser_cell(4.0));
        db.set(2, new muser_cell(5.0));
        db.erase(1, 1);
        assert(db.size() == 2);
    }

    {
        // Erase (single block with preceding block)
        mtv_type db(4);

        // Erase the whole thing.
        db.set(0, 1.1);
        db.set(1, new muser_cell(1.0));
        db.set(2, new muser_cell(2.0));
        db.set(3, new muser_cell(3.0));
        db.erase(1, 3);
        assert(db.size() == 1);

        // Erase top.
        db.resize(4);
        db.set(1, new muser_cell(1.0));
        db.set(2, new muser_cell(2.0));
        db.set(3, new muser_cell(3.0));
        db.erase(1, 2);
        assert(db.size() == 2);

        // Erase bottom.
        db.resize(4);
        db.set(2, new muser_cell(4.0));
        db.set(3, new muser_cell(5.0));
        db.erase(2, 3);
        assert(db.size() == 2);

        // Erase middle.
        db.resize(4);
        db.set(2, new muser_cell(4.0));
        db.set(3, new muser_cell(5.0));
        db.erase(2, 2);
        assert(db.size() == 3);
    }

    {
        // Erase (multi-block 1)
        mtv_type db(6);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));
        db.set(3, 4.1);
        db.set(4, 4.2);
        db.set(5, 4.3);
        db.erase(1, 4);
    }

    {
        // Erase (multi-block 2)
        mtv_type db(6);
        db.set(0, 4.1);
        db.set(1, 4.2);
        db.set(2, 4.3);
        db.set(3, new muser_cell(5.0));
        db.set(4, new muser_cell(6.0));
        db.set(5, new muser_cell(7.0));
        db.erase(1, 4);
    }

    {
        // Erase (multi-block 3)
        mtv_type db(6);
        db.set(0, 1.0);
        db.set(1, 2.0);
        db.set(2, new muser_cell(3.0));
        db.set(3, new muser_cell(4.0));
        db.set(4, 5.0);
        db.set(5, 6.0);
        db.erase(1, 4);
    }

    {
        // Insert into the middle of block.  This one shouldn't overwrite any
        // cells, but just to be safe...
        mtv_type db(2);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.insert_empty(1, 2);
        assert(db.size() == 4);
        assert(db.get<muser_cell*>(0)->value == 1.0);
        assert(db.get<muser_cell*>(3)->value == 2.0);
    }

    {
        // set_cells (simple overwrite)
        mtv_type db(2);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));

        std::vector<muser_cell*> vals;
        vals.push_back(new muser_cell(3.0));
        vals.push_back(new muser_cell(4.0));
        db.set(0, vals.begin(), vals.end());
        assert(db.get<muser_cell*>(0)->value == 3.0);
        assert(db.get<muser_cell*>(1)->value == 4.0);
    }

    {
        // set_cells (overwrite upper)
        mtv_type db(2);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        double vals[] = {3.0};
        const double* p = &vals[0];
        db.set(0, p, p + 1);
        assert(db.get<double>(0) == 3.0);
        assert(db.get<muser_cell*>(1)->value == 2.0);
    }

    {
        // set_cells (overwrite lower)
        mtv_type db(2);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        double vals[] = {3.0};
        const double* p = &vals[0];
        db.set(1, p, p + 1);
        assert(db.get<muser_cell*>(0)->value == 1.0);
        assert(db.get<double>(1) == 3.0);
    }

    {
        // set_cells (overwrite middle)
        mtv_type db(4);
        db.set(0, 1.1);
        db.set(1, new muser_cell(1.0));
        db.set(2, new muser_cell(2.0));
        db.set(3, new muser_cell(3.0));
        double vals[] = {4.0};
        const double* p = &vals[0];
        db.set(2, p, p + 1);
        assert(db.get<muser_cell*>(1)->value == 1.0);
        assert(db.get<double>(2) == 4.0);
        assert(db.get<muser_cell*>(3)->value == 3.0);
    }
    {
        // insert_empty() to split the block into two.
        mtv_type db(3);
        db.set(0, 1.1);
        db.set(1, new muser_cell(1.0));
        db.set(2, new muser_cell(2.0));
        db.insert_empty(2, 2);
        assert(db.size() == 5);
        assert(db.get<muser_cell*>(1)->value == 1.0);
        assert(db.get<muser_cell*>(4)->value == 2.0);
    }

    {
        // erase() to merge two blocks.
        mtv_type db(4);
        db.set(0, 1.1);
        db.set(1, new muser_cell(1.0));
        db.set(2, static_cast<uint64_t>(2));
        db.set(3, new muser_cell(3.0));
        assert(db.block_size() == 4);
        assert(db.size() == 4);

        db.erase(2, 2);
        assert(db.block_size() == 2);
        assert(db.size() == 3);
        assert(db.get<double>(0) == 1.1);
        assert(db.get<muser_cell*>(1)->value == 1.0);
        assert(db.get<muser_cell*>(2)->value == 3.0);
    }

    {
        // set_cells() across multiple blocks.
        mtv_type db(5);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, 1.2);
        db.set(3, new muser_cell(3.0));
        db.set(4, new muser_cell(4.0));
        uint64_t vals[] = {5, 6, 7};
        const uint64_t* p = &vals[0];
        db.set(1, p, p + 3);
    }

    {
        // set_cells() across multiple blocks, part 2.
        mtv_type db(6);
        db.set(0, static_cast<uint64_t>(12));
        db.set(1, new muser_cell(1.0));
        db.set(2, new muser_cell(2.0));
        db.set(3, 1.2);
        db.set(4, new muser_cell(3.0));
        db.set(5, new muser_cell(4.0));
        assert(db.block_size() == 4);

        std::vector<muser_cell*> vals;
        vals.push_back(new muser_cell(5.0));
        vals.push_back(new muser_cell(6.0));
        vals.push_back(new muser_cell(7.0));
        db.set(2, vals.begin(), vals.end());
        assert(db.block_size() == 2);
    }

    {
        // set_cell() to merge 3 blocks.
        mtv_type db(6);
        db.set(0, static_cast<uint64_t>(12));
        db.set(1, new muser_cell(1.0));
        db.set(2, new muser_cell(2.0));
        db.set(3, 1.2);
        db.set(4, new muser_cell(3.0));
        db.set(5, new muser_cell(4.0));
        assert(db.block_size() == 4);
        assert(db.get<uint64_t>(0) == 12);
        assert(db.get<muser_cell*>(1)->value == 1.0);
        assert(db.get<muser_cell*>(2)->value == 2.0);
        assert(db.get<double>(3) == 1.2);
        assert(db.get<muser_cell*>(4)->value == 3.0);
        assert(db.get<muser_cell*>(5)->value == 4.0);

        db.set(3, new muser_cell(5.0)); // merge blocks.
        assert(db.block_size() == 2);
        assert(db.get<uint64_t>(0) == 12);
        assert(db.get<muser_cell*>(1)->value == 1.0);
        assert(db.get<muser_cell*>(2)->value == 2.0);
        assert(db.get<muser_cell*>(3)->value == 5.0);
        assert(db.get<muser_cell*>(4)->value == 3.0);
        assert(db.get<muser_cell*>(5)->value == 4.0);
    }

    {
        // set_cell() to merge 2 blocks.
        mtv_type db(3);
        db.set(0, static_cast<uint64_t>(23));
        db.set(1, new muser_cell(2.1));
        db.set(2, new muser_cell(3.1));

        db.set(0, new muser_cell(4.2)); // merge
        assert(db.block_size() == 1);
        assert(db.get<muser_cell*>(0)->value == 4.2);
        assert(db.get<muser_cell*>(1)->value == 2.1);
        assert(db.get<muser_cell*>(2)->value == 3.1);
    }

    {
        // insert_cells() to split block into two.
        mtv_type db(2);
        db.set(0, new muser_cell(2.1));
        db.set(1, new muser_cell(2.2));
        double vals[] = {3.1, 3.2};
        const double* p = &vals[0];
        db.insert(1, p, p + 2);
    }

    {
        // set_cells() - merge new data block with existing block below.
        mtv_type db(6);
        db.set(0, std::string("foo"));
        db.set(1, std::string("baa"));
        db.set(2, 1.1);
        db.set(3, 1.2);
        db.set(4, new muser_cell(2.2));
        db.set(5, new muser_cell(2.3));
        assert(db.block_size() == 3);

        std::vector<muser_cell*> vals;
        vals.push_back(new muser_cell(2.4));
        vals.push_back(new muser_cell(2.5));
        vals.push_back(new muser_cell(2.6));
        db.set(1, vals.begin(), vals.end());
        assert(db.block_size() == 2);

        assert(db.get<std::string>(0) == "foo");
        assert(db.get<muser_cell*>(1)->value == 2.4);
        assert(db.get<muser_cell*>(2)->value == 2.5);
        assert(db.get<muser_cell*>(3)->value == 2.6);
        assert(db.get<muser_cell*>(4)->value == 2.2);
        assert(db.get<muser_cell*>(5)->value == 2.3);
    }

    {
        // set_cells() - merge new data block with existing block below, but
        // it overwrites the upper cell.
        mtv_type db(6);
        db.set(0, std::string("foo"));
        db.set(1, std::string("baa"));
        db.set(2, 1.1);
        db.set(3, new muser_cell(2.1));
        db.set(4, new muser_cell(2.2));
        db.set(5, new muser_cell(2.3));
        std::vector<muser_cell*> vals;
        vals.push_back(new muser_cell(2.4));
        vals.push_back(new muser_cell(2.5));
        vals.push_back(new muser_cell(2.6));
        db.set(1, vals.begin(), vals.end());
        assert(db.block_size() == 2);

        assert(db.get<std::string>(0) == "foo");
        assert(db.get<muser_cell*>(1)->value == 2.4);
        assert(db.get<muser_cell*>(2)->value == 2.5);
        assert(db.get<muser_cell*>(3)->value == 2.6);
        assert(db.get<muser_cell*>(4)->value == 2.2);
        assert(db.get<muser_cell*>(5)->value == 2.3);
    }

    {
        mtv_type db(3);
        db.set(0, new muser_cell(1.0));
        db.set(2, new muser_cell(1.0));
        db.set(1, new muser_cell(1.0));
        assert(db.block_size() == 1);
    }

    {
        mtv_type db(10);
        for (size_t i = 0; i < 10; ++i)
            db.set(i, new muser_cell(1.1));

        std::vector<double> doubles(3, 2.2);
        db.set(3, doubles.begin(), doubles.end());
        assert(db.block_size() == 3);

        std::vector<muser_cell*> cells;
        cells.push_back(new muser_cell(2.1));
        cells.push_back(new muser_cell(2.2));
        cells.push_back(new muser_cell(2.3));
        db.set(3, cells.begin(), cells.end());
        assert(db.block_size() == 1);
        assert(db.get<muser_cell*>(0)->value == 1.1);
        assert(db.get<muser_cell*>(1)->value == 1.1);
        assert(db.get<muser_cell*>(2)->value == 1.1);
        assert(db.get<muser_cell*>(3)->value == 2.1);
        assert(db.get<muser_cell*>(4)->value == 2.2);
        assert(db.get<muser_cell*>(5)->value == 2.3);
        assert(db.get<muser_cell*>(6)->value == 1.1);
        assert(db.get<muser_cell*>(7)->value == 1.1);
        assert(db.get<muser_cell*>(8)->value == 1.1);
        assert(db.get<muser_cell*>(9)->value == 1.1);
    }

    {
        mtv_type db(3);
        db.set(0, new muser_cell(1.0));
        db.set(1, new muser_cell(2.0));
        db.set(2, new muser_cell(3.0));
        db.set_empty(1, 1);
        assert(db.block_size() == 3);
        assert(db.get<muser_cell*>(0)->value == 1.0);
        assert(db.is_empty(1));
        assert(db.get<muser_cell*>(2)->value == 3.0);
    }

    {
        mtv_type db(3);
        db.set(1, new muser_cell(3.3));
        assert(db.block_size() == 3);
        db.set_empty(1, 1);
        assert(db.block_size() == 1);
    }

    {
        // Release an element.
        mtv_type db(1);
        muser_cell* p1 = new muser_cell(4.5);
        db.set(0, p1);
        muser_cell* p2 = db.release<muser_cell*>(0);
        assert(p1 == p2);
        assert(p2->value == 4.5);
        assert(db.is_empty(0));
        delete p2;

        db = mtv_type(2);
        db.set(0, new muser_cell(23.3));
        assert(db.block_size() == 2);
        p2 = db.release<muser_cell*>(0);
        assert(db.is_empty(0));
        assert(db.is_empty(1));
        assert(db.block_size() == 1);
        delete p2;

        db = mtv_type(2);
        db.set(0, new muser_cell(1.2));
        db.set(1, new muser_cell(1.3));

        p2 = db.release<muser_cell*>(0);
        assert(db.is_empty(0));
        assert(!db.is_empty(1));
        assert(p2->value == 1.2);
        delete p2;

        db.set(0, new muser_cell(1.4));
        p2 = db.release<muser_cell*>(1);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(p2->value == 1.3);
        delete p2;

        db = mtv_type(3);
        db.set(0, new muser_cell(2.1));
        db.set(1, new muser_cell(2.2));
        db.set(2, new muser_cell(2.3));

        p2 = db.release<muser_cell*>(1);
        assert(p2->value == 2.2);
        assert(!db.is_empty(0));
        assert(db.is_empty(1));
        assert(!db.is_empty(2));

        delete p2;

        db = mtv_type(3);
        db.set(0, new muser_cell(2.1));
        db.set(1, new muser_cell(2.2));
        db.set(2, new muser_cell(2.3));
        db.set_empty(0, 2); // Make sure this doesn't release anything.

        // Release with position hint.
        db = mtv_type(4);
        db.set(0, new muser_cell(4.5));
        db.set(1, new muser_cell(4.6));
        db.set(3, new muser_cell(5.1));

        mtv_type::iterator pos = db.release(0, p1);
        assert(pos == db.begin());
        pos = db.release(pos, 3, p2);
        ++pos;
        assert(pos == db.end());
        assert(p1->value == 4.5);
        assert(p2->value == 5.1);
        assert(db.block_size() == 3);
        assert(db.is_empty(0));
        assert(db.get<muser_cell*>(1)->value == 4.6);
        assert(db.is_empty(2));
        assert(db.is_empty(3));
        delete p1;
        delete p2;
    }

    {
        mtv_type db(5);

        db.set(1, new muser_cell(1.1));
        db.set(2, new muser_cell(1.2));
        db.set(3, new muser_cell(1.3));

        db.set(1, 2.1); // Don't leak the overwritten muser_cell instance.
        db.set(2, 2.2); // ditto
    }

    {
        mtv_type db(4);
        db.set(0, new muser_cell(1.1));
        db.set(1, new muser_cell(1.2));
        db.set(2, new muser_cell(1.3));

        db.set(2, 2.1); // Don't leak the overwritten muser_cell instance.
        db.set(1, 2.0); // ditto
    }

    {
        mtv_type db(8);
        db.set(3, new muser_cell(1.1));
        db.set(4, new muser_cell(1.2));
        db.set(5, 1.3);

        db.set(4, 2.2); // Overwrite muser_cell and don't leak.
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
