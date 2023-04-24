/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2023 Kohei Yoshida
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

#include <mdds/multi_type_vector/types.hpp>

#include <vector>
#include <iostream>

void mtv_test_element_blocks_range()
{
    stack_printer __stack_printer__(__func__);

    constexpr mdds::mtv::element_t element_type_int32 = mdds::mtv::element_type_user_start + 2;
    using block_type = mdds::mtv::default_element_block<element_type_int32, std::int32_t, std::vector>;

    const std::vector<std::int8_t> values = {1, 2, 3, 4, 5};
    auto blk = block_type(values.begin(), values.end());
    const auto& crblk = blk;

    assert(block_type::size(blk) == 5u);

    {
        auto it = block_type::begin(blk), it_end = block_type::end(blk);
        assert(std::distance(it, it_end) == 5u);
        assert(*it++ == 1);
        assert(*it++ == 2);
        assert(*it++ == 3);
        assert(*it++ == 4);
        assert(*it++ == 5);
        assert(it == it_end);
    }

    {
        // immutable range access
        auto it = block_type::begin(crblk);
        for (auto v : block_type::range(crblk))
        {
            std::cout << "v=" << v << std::endl;
            assert(v == *it++);
        }

        assert(it == block_type::end(crblk));
    }

    {
        // mutable range access
        auto it = block_type::begin(blk);
        for (auto v : block_type::range(blk))
        {
            std::cout << "v=" << v << std::endl;
            assert(v == *it);
            *it = v + 2;
            ++it;
        }

        assert(it == block_type::end(blk));

        it = block_type::begin(blk);
        assert(*it++ == 3);
        assert(*it++ == 4);
        assert(*it++ == 5);
        assert(*it++ == 6);
        assert(*it++ == 7);
        assert(it == block_type::end(blk));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
