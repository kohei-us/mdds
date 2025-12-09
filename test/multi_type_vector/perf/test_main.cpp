// SPDX-FileCopyrightText: 2011 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.

#include <mdds/multi_type_vector.hpp>

#include <cassert>
#include <sstream>
#include <vector>
#include <deque>

using namespace std;
using namespace mdds;

namespace {

typedef mdds::multi_type_vector<mdds::mtv::standard_element_blocks_traits> mtv_type;

void mtv_perf_test_block_position_lookup()
{
    size_t n = 24000;

    {
        // Default insertion which always looks up the right element block
        // from the position of the first block.  As such, as the block size
        // grows, so does the time it takes to search for the right block.

        mtv_type db(n * 2);
        double val1 = 1.1;
        int val2 = 23;
        stack_printer __stack_printer__("::mtv_perf_test_block_position_lookup::default insertion");
        for (size_t i = 0; i < n; ++i)
        {
            size_t pos1 = i * 2, pos2 = i * 2 + 1;
            db.set(pos1, val1);
            db.set(pos2, val2);
        }
    }

    {
        // As a solution for this, we can use an iterator to specify the start
        // position, which eliminates the above scalability problem nicely.

        mtv_type db(n * 2);
        mtv_type::iterator pos_hint = db.begin();
        double val1 = 1.1;
        int val2 = 23;
        stack_printer __stack_printer__("::mtv_perf_test_block_position_lookup::insertion with position hint");
        for (size_t i = 0; i < n; ++i)
        {
            size_t pos1 = i * 2, pos2 = i * 2 + 1;
            pos_hint = db.set(pos_hint, pos1, val1);
            pos_hint = db.set(pos_hint, pos2, val2);
        }
    }
}

void mtv_perf_test_insert_via_position_object()
{
    size_t data_size = 80000;
    mtv_type db(data_size);
    {
        stack_printer __stack_printer__("::mtv_perf_test_insert_via_position_object initialize mtv.");
        mtv_type::iterator it = db.begin();
        for (size_t i = 0, n = db.size() / 2; i < n; ++i)
        {
            it = db.set(it, i * 2, 1.1);
        }
    }

    mtv_type db2 = db;
    {
        stack_printer __stack_printer__("::mtv_perf_test_insert_via_position_object insert with position hint.");
        mtv_type::iterator it = db2.begin();
        for (size_t i = 0, n = db2.size(); i < n; ++i)
        {
            it = db2.set(it, i, string("foo"));
        }
    }

    db2 = db;
    {
        stack_printer __stack_printer__("::mtv_perf_test_insert_via_position_object insert via position object.");
        mtv_type::position_type pos = db2.position(0);
        for (; pos.first != db2.end(); pos = mtv_type::next_position(pos))
        {
            size_t log_pos = mtv_type::logical_position(pos);
            pos.first = db2.set(pos.first, log_pos, string("foo"));
            pos.second = log_pos - pos.first->position;
        }
    }
}

} // namespace

int main()
try
{
    mtv_perf_test_block_position_lookup();
    mtv_perf_test_insert_via_position_object();

    return EXIT_SUCCESS;
}
catch (...)
{
    return EXIT_FAILURE;
}
