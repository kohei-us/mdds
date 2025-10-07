/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2025 Kohei Yoshida
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
