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
