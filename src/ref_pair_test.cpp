/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2020 Kohei Yoshida
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

#include "test_global.hpp" // This must be the first header to be included.
#include "mdds/ref_pair.hpp"

#include <iostream>

using namespace std;

void test_basic()
{
    stack_printer __stack_printer__("::test_basic");

    using rp_type = mdds::detail::ref_pair<const int, int>;
    using pair_type = std::pair<int, int>;

    int v1 = 11;
    int v2 = 22;
    rp_type rp(v1, v2);

    rp.second = 34;

    assert(rp == pair_type(11, 34));
    assert(rp != pair_type(12, 34));
    assert(rp != pair_type(11, 32));
    assert(rp == rp);

    int v3 = 23;
    int v4 = 33;
    rp_type rp2(v3, v4);
    assert(rp != rp2);
    assert(rp2 == pair_type(23, 33));

    rp_type rp3(rp);
    assert(rp3 == rp);
}

void test_used_in_iterator()
{
    stack_printer __stack_printer__("::test_used_in_iterator");

    using rp_type = mdds::detail::ref_pair<int, int>;

    struct fake_iterator
    {
        int m_v1 = 222;
        int m_v2 = 456;

        fake_iterator() {}

        rp_type operator* ()
        {
            return rp_type(m_v1, m_v2);
        }

        rp_type operator->()
        {
            return rp_type(m_v1, m_v2);
        }
    };

    fake_iterator it;

    assert((*it).first == 222);
    assert((*it).second == 456);
    assert(it->first == 222);
    assert(it->second == 456);

    // Make sure the member values can be modified.
    it->second = 897;
    assert(it->second == 897);
    (*it).first = -23;
    assert(it->first = -23);
}

int main(int argc, char** argv)
{
    test_basic();
    test_used_in_iterator();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
