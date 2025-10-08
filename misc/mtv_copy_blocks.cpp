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
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <execution>
#include <functional>

#include <mdds/multi_type_vector/types.hpp>

constexpr mdds::mtv::element_t block_id = mdds::mtv::element_type_user_start;
using value_type = double;
const value_type zero = 1.2345;
const std::size_t n_elements = 10'000'000;
using element_block_type = mdds::mtv::default_element_block<block_id, value_type, std::vector>;

struct block_deleter
{
    void operator()(element_block_type* p) const
    {
        element_block_type::delete_block(p);
    }
};

using block_store_type = std::vector<element_block_type*>;

using clone_func_type = std::function<void(block_store_type&)>;

void clone_blocks(const std::vector<element_block_type*>& src, const char* label, clone_func_type func)
{
    std::vector<element_block_type*> blocks_cloned(src);

    {
        stack_printer __stack_printer__(label);
        func(blocks_cloned);
    }

    for (auto* p : blocks_cloned)
        element_block_type::delete_block(p);
}

int main()
{
    std::vector<value_type> src;
    {
        std::vector<value_type> tmp(n_elements, zero);
        src.swap(tmp);
    }

    element_block_type src_block;
    element_block_type::assign_values(src_block, src.begin(), src.end());

    std::vector<element_block_type*> src_blocks;

    {
        stack_printer __stack_printer__("create 30 blocks");
        for (int i = 0; i < 30; ++i)
        {
            auto* p = element_block_type::clone_block(src_block);
            src_blocks.push_back(p);
        }
    }

    clone_blocks(src_blocks, "clone blocks (serial)",
        [](block_store_type& blocks)
        {
            for (auto*& p : blocks)
            {
                if (p)
                    p = element_block_type::clone_block(*p);
            }
        }
    );

    clone_blocks(src_blocks, "clone blocks (transform)",
        [](block_store_type& blocks)
        {
            std::transform(blocks.begin(), blocks.end(), blocks.begin(), [](auto* p) {
                if (p)
                    p = element_block_type::clone_block(*p);
                return p;
            });
        }
    );

    clone_blocks(src_blocks, "clone blocks (transform-par)",
        [](block_store_type& blocks)
        {
            std::transform(std::execution::par, blocks.begin(), blocks.end(), blocks.begin(), [](auto* p) {
                if (p)
                    p = element_block_type::clone_block(*p);
                return p;
            });
        }
    );

    clone_blocks(src_blocks, "clone blocks (transform-par-unseq)",
        [](block_store_type& blocks)
        {
            std::transform(std::execution::par_unseq, blocks.begin(), blocks.end(), blocks.begin(), [](auto* p) {
                if (p)
                    p = element_block_type::clone_block(*p);
                return p;
            });
        }
    );

    for (auto* p : src_blocks)
        element_block_type::delete_block(p);

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
