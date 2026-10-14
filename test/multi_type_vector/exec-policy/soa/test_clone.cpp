/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_main.hpp"

using mtv_type = mdds::mtv::soa::multi_type_vector<seq_policy_traits>;
using mtv_type_noncopyable = mdds::mtv::soa::multi_type_vector<noncopyable_traits>;

/**
 * Whether clone() of container type T accepts an instance of CV.  A
 * requires-expression only evaluates to false inside a template, hence the
 * variable template.
 */
template<typename T, typename CV>
constexpr bool clone_accepts = requires(const T& s, CV& c) { s.clone(c); };

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

void test_clone_passed_cloner()
{
    MDDS_TEST_FUNC_SCOPE;

    using str_cloner = mdds::mtv::clone_value<custom_str*>;
    using int_cloner = mdds::mtv::clone_value<custom_int*>;

    // A cloner with an exec_policy is stateless by contract, so it cannot be
    // passed.
    static_assert(mdds::mtv::stateful_clone_value<str_cloner>);
    static_assert(!mdds::mtv::stateful_clone_value<int_cloner>);
    static_assert(clone_accepts<mtv_type_noncopyable, str_cloner>);
    static_assert(!clone_accepts<mtv_type_noncopyable, int_cloner>);

    mtv_type_noncopyable src;

    src.push_back(new custom_str{"value1"});
    src.push_back(new custom_str{"value2"});
    src.push_back(new custom_int{12});
    src.push_back(new custom_str{"value3"});
    src.push_back(new custom_int{34});

    // With a cloner passed, the blocks are cloned sequentially through it
    // despite the exec_policy of the traits.
    str_cloner cloner;
    auto cloned = src.clone(cloner);
    TEST_ASSERT(src == cloned);
    TEST_ASSERT(cloner.visits == 3);
    TEST_ASSERT(cloned.get<custom_str*>(0) != src.get<custom_str*>(0));
    TEST_ASSERT(cloned.get<custom_str*>(3) != src.get<custom_str*>(3));
    TEST_ASSERT(cloned.get<custom_int*>(2) != src.get<custom_int*>(2));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
