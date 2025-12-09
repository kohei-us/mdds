/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2018 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.
#include "test_global_rtree.hpp"

int main()
{
    try
    {
        rtree_test_intersection();
        rtree_test_square_distance();
        rtree_test_center_point();
        rtree_test_area_enlargement();
        rtree_test_basic_search();
        rtree_test_basic_erase();
        rtree_test_node_split();
        rtree_test_directory_node_split();
        rtree_test_erase_directories();
        rtree_test_forced_reinsertion();
        rtree_test_move();
        rtree_test_move_custom_type();
        rtree_test_copy();
        rtree_test_point_objects();
        rtree_test_only_copyable();
        rtree_test_exact_search_by_extent();
        rtree_test_exact_search_by_point();
    }
    catch (const std::exception& e)
    {
        std::cout << "Test failed: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Test finished successfully!" << std::endl;

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
