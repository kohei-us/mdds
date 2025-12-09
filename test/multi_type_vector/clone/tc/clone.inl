/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

void test_clone()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type store;
    store.push_back<custom_num>(1.1);
    store.push_back<custom_num>(1.2);
    store.push_back<custom_num>(1.3);
    store.push_back_empty();
    store.push_back(new custom_str1{"custom str1 1"});
    store.push_back(new custom_str1{"custom str1 2"});
    store.push_back(new custom_str1{"custom str1 3"});
    store.push_back(new custom_str1{"custom str1 4"});
    store.push_back(new custom_str2{"custom str2 1"});
    store.push_back(new custom_str2{"custom str2 2"});
    store.push_back(new custom_str2{"custom str2 3"});
    store.push_back(new custom_str2{"custom str2 4"});

    auto cloned = store.clone();

    TEST_ASSERT(store == cloned);
}

void test_noclone()
{
    MDDS_TEST_FUNC_SCOPE;

    mtv_type store;
    store.push_back(new custom_noclone{"no-clone string"});

    try
    {
        [[maybe_unused]] auto cloned = store.clone();
        TEST_ASSERT(!"exception was not thrown as expected");
    }
    catch (const mdds::mtv::element_block_error&)
    {
        // expected
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
