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

#include "test_global.hpp"
#include "common_types.hpp"
#include <mdds/multi_type_vector/soa/main.hpp>

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
    test_clone();
    test_shrink_to_fit();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */


