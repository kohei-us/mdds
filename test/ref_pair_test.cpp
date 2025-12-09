/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2020 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include "test_global.hpp" // This must be the first header to be included.
#include "mdds/ref_pair.hpp"

#include <iostream>

void test_basic()
{
    MDDS_TEST_FUNC_SCOPE;

    using rp_type = mdds::detail::ref_pair<const int, int>;
    using pair_type = std::pair<int, int>;

    int v1 = 11;
    int v2 = 22;
    rp_type rp(v1, v2);

    rp.second = 34;

    TEST_ASSERT(rp == pair_type(11, 34));
    TEST_ASSERT(rp != pair_type(12, 34));
    TEST_ASSERT(rp != pair_type(11, 32));
    TEST_ASSERT(rp == rp);

    int v3 = 23;
    int v4 = 33;
    rp_type rp2(v3, v4);
    TEST_ASSERT(rp != rp2);
    TEST_ASSERT(rp2 == pair_type(23, 33));

    rp_type rp3(rp);
    TEST_ASSERT(rp3 == rp);
}

void test_used_in_iterator()
{
    MDDS_TEST_FUNC_SCOPE;

    using rp_type = mdds::detail::ref_pair<int, int>;

    struct fake_iterator
    {
        int m_v1 = 222;
        int m_v2 = 456;

        fake_iterator()
        {}

        rp_type operator*()
        {
            return rp_type(m_v1, m_v2);
        }

        rp_type operator->()
        {
            return rp_type(m_v1, m_v2);
        }
    };

    fake_iterator it;

    TEST_ASSERT((*it).first == 222);
    TEST_ASSERT((*it).second == 456);
    TEST_ASSERT(it->first == 222);
    TEST_ASSERT(it->second == 456);

    // Make sure the member values can be modified.
    it->second = 897;
    TEST_ASSERT(it->second == 897);
    (*it).first = -23;
    TEST_ASSERT(it->first == -23);
}

int main()
{
    test_basic();
    test_used_in_iterator();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
