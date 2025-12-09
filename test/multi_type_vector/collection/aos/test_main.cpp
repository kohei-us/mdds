// SPDX-FileCopyrightText: 2016 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.

#define MDDS_MULTI_TYPE_VECTOR_DEBUG 1
#include <mdds/multi_type_vector/aos/main.hpp>
#include <mdds/multi_type_vector/collection.hpp>

#include <iostream>
#include <vector>
#include <deque>
#include <memory>

using mtv_type = mdds::mtv::aos::multi_type_vector<mdds::mtv::standard_element_blocks_traits>;
using cols_type = mdds::mtv::collection<mtv_type>;

#include "all.inl"

int main()
{
    try
    {
        mtv_test_empty();
        mtv_test_pointer_size1();
        mtv_test_unique_pointer_size1();
        mtv_test_shared_pointer_size2();
        mtv_test_non_pointer_size1();
        mtv_test_invalid_collection();
        mtv_test_sub_element_ranges();
        mtv_test_sub_element_ranges_invalid();
        mtv_test_sub_collection_ranges_invalid();
        mtv_test_boolean_block();
    }
    catch (const std::exception& e)
    {
        cout << "Test failed: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    cout << "Test finished successfully!" << endl;
    return EXIT_SUCCESS;
}
