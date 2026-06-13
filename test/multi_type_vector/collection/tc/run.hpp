/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "all.hpp"

template<typename mtv_type>
void run_all_tests()
{
    mtv_test_empty<mtv_type>();
    mtv_test_pointer_size1<mtv_type>();
    mtv_test_unique_pointer_size1<mtv_type>();
    mtv_test_shared_pointer_size2<mtv_type>();
    mtv_test_non_pointer_size1<mtv_type>();
    mtv_test_invalid_collection<mtv_type>();
    mtv_test_sub_element_ranges<mtv_type>();
    mtv_test_sub_element_ranges_invalid<mtv_type>();
    mtv_test_sub_collection_ranges_invalid<mtv_type>();
    mtv_test_boolean_block<mtv_type>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
