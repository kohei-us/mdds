/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "common_types.hpp"

/**
 * COW does not relax the copy ctor's copyability contract - copy ctor should
 * still throw when at least one noncopyable element block is present,
 * just like the non-COW counterpart does.  The contract extends to the
 * assignment operator as well.
 */
template<typename mtv_type>
void test_cow_noncopyable()
{
    MDDS_TEST_FUNC_SCOPE;

    {
        // clone() of a container holding non-copyable blocks shares, then
        // detaches correctly (deep-clones the managed pointees) on first write.
        mtv_type src;
        src.template push_back<custom_num>(1.1);
        src.push_back(new custom_str1{"a"});
        src.push_back(new custom_str2{"b"});

        auto cloned = src.clone();
        TEST_ASSERT(src == cloned);
        TEST_ASSERT(same_block_pointers(src, cloned)); // shared

        // First write detaches: the managed pointees are deep-cloned and the
        // two sides become independent.
        cloned.template set<custom_num>(0, 9.9);
        TEST_ASSERT(!same_block_pointers(src, cloned));
        TEST_ASSERT(cloned.template get<custom_num>(0).value == 9.9);
        TEST_ASSERT(src.template get<custom_num>(0).value == 1.1);

        // The managed blocks were cloned, so the pointees are distinct objects
        // holding equal values.
        TEST_ASSERT(src.template get<custom_str1*>(1) != cloned.template get<custom_str1*>(1));
        TEST_ASSERT(src.template get<custom_str1*>(1)->value == cloned.template get<custom_str1*>(1)->value);
        TEST_ASSERT(src.template get<custom_str2*>(2) != cloned.template get<custom_str2*>(2));
        TEST_ASSERT(src.template get<custom_str2*>(2)->value == cloned.template get<custom_str2*>(2)->value);
    }

    {
        // Copy-constructing a container that holds a non-copyable block throws
        // element_block_error, before the source's blocks are shared.
        mtv_type src;
        src.template push_back<custom_num>(1.1);
        src.push_back(new custom_str1{"a"});

        bool threw = false;
        try
        {
            mtv_type copied(src);
        }
        catch (const mdds::mtv::element_block_error&)
        {
            threw = true;
        }
        TEST_ASSERT(threw);

        // The source is intact.
        TEST_ASSERT(src.template get<custom_num>(0).value == 1.1);
        TEST_ASSERT(src.template get<custom_str1*>(1)->value == "a");

        // The source's blocks were never shared: it is still a sole owner, so
        // mutating it modifies its block in place rather than detaching.
        const mdds::mtv::base_element_block* p_num = src.begin()->data;
        src.template set<custom_num>(0, 2.2);
        TEST_ASSERT(src.begin()->data == p_num);
        TEST_ASSERT(src.template get<custom_num>(0).value == 2.2);
    }

    {
        // Copy-assignment routes through the copy ctor, so it throws too and
        // leaves the destination unchanged.
        mtv_type src;
        src.template push_back<custom_num>(1.1);
        src.push_back(new custom_str2{"b"});

        mtv_type dest;
        dest.template push_back<custom_num>(5.5);

        bool threw = false;
        try
        {
            dest = src;
        }
        catch (const mdds::mtv::element_block_error&)
        {
            threw = true;
        }
        TEST_ASSERT(threw);

        TEST_ASSERT(dest.size() == 1);
        TEST_ASSERT(dest.template get<custom_num>(0).value == 5.5);
    }

    {
        // clone() of a container holding only a non-copyable block succeeds; it
        // is meant to clone such blocks.
        mtv_type src;
        src.push_back(new custom_str1{"x"});

        auto cloned = src.clone();
        TEST_ASSERT(src == cloned);
        TEST_ASSERT(same_block_pointers(src, cloned));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
