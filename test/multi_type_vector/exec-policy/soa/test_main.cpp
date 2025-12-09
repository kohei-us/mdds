/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_main.hpp"

using mtv_type = mdds::mtv::soa::multi_type_vector<seq_policy_traits>;

void test_copy_construction_and_equal()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type src;
    src.push_back<int32_t>(12);
    src.push_back<int32_t>(23);
    src.push_back<float>(1.0f);
    src.push_back<float>(2.0f);
    src.push_back<float>(3.0f);
    mtv_type copied{src};

    TEST_ASSERT(src == copied);

    // keep the logical lengths of src and copied the same to trigger
    // equal_blocks function
    src.push_back<double>(1.0);
    src.push_back<double>(2.0);
    copied.push_back<double>(10.0);
    copied.push_back<double>(12.0);

    TEST_ASSERT(src != copied);
}

void test_shrink_to_fit()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type store;
    store.push_back<int32_t>(12);
    store.push_back<int32_t>(23);
    store.push_back<float>(1.0f);
    store.push_back<float>(2.0f);
    store.push_back<float>(3.0f);
    store.shrink_to_fit();
}

int main()
{
    test_copy_construction_and_equal();
    test_shrink_to_fit();
    test_clone();
    test_clone_noncopyable();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
