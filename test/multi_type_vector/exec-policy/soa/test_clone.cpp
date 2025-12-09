/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_main.hpp"

using mtv_type = mdds::mtv::soa::multi_type_vector<seq_policy_traits>;
using mtv_type_noncopyable = mdds::mtv::soa::multi_type_vector<noncopyable_traits>;

void test_clone()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type src;

    src.push_back<int32_t>(12);
    src.push_back<int32_t>(23);
    src.push_back<float>(1.0f);
    src.push_back<float>(2.0f);
    src.push_back<float>(3.0f);

    auto cloned = src.clone();
    TEST_ASSERT(src == cloned);
}

void test_clone_noncopyable()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type_noncopyable src;

    src.push_back(new custom_str{"value1"});
    src.push_back(new custom_str{"value2"});
    src.push_back(new custom_str{"value3"});
    src.push_back(new custom_int{12});
    src.push_back(new custom_int{34});
    src.push_back(new custom_int{56});

    auto cloned = src.clone();
    TEST_ASSERT(src == cloned);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
