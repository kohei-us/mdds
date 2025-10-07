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

#include <mdds/multi_type_vector/types.hpp>
#include <mdds/multi_type_vector/soa/main.hpp>
#include <mdds/multi_type_vector/aos/main.hpp>

namespace {

struct custom_num
{
    double value;

    custom_num() : value(0.0) {}
    custom_num(double v) : value(v) {}
    operator double() const { return value; }

    bool operator==(const custom_num& other) const
    {
        return value == other.value;
    }

    bool operator!=(const custom_num& other) const
    {
        return !operator==(other);
    }
};

struct custom_str1
{
    std::string value;

    operator const char*() const
    {
        return value.c_str();
    }

    bool operator==(const custom_str1& other) const
    {
        return value == other.value;
    }

    bool operator!=(const custom_str1& other) const
    {
        return !operator==(other);
    }
};

struct custom_str2
{
    std::string value;

    operator const char*() const
    {
        return value.c_str();
    }

    bool operator==(const custom_str2& other) const
    {
        return value == other.value;
    }

    bool operator!=(const custom_str2& other) const
    {
        return !operator==(other);
    }
};

constexpr mdds::mtv::element_t block1_id = mdds::mtv::element_type_user_start;
constexpr mdds::mtv::element_t block2_id = mdds::mtv::element_type_user_start + 1;
constexpr mdds::mtv::element_t block3_id = mdds::mtv::element_type_user_start + 2;

using block1_type = mdds::mtv::default_element_block<block1_id, custom_num, std::vector>;
using block2_type = mdds::mtv::noncopyable_managed_element_block<block2_id, custom_str1, std::vector>;
using block3_type = mdds::mtv::noncopyable_managed_element_block<block3_id, custom_str2, std::vector>;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(custom_num, block1_id, custom_num{}, block1_type)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(custom_str1, block2_id, nullptr, block2_type)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(custom_str2, block3_id, nullptr, block3_type)

struct mtv_traits : mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<block1_type, block2_type, block3_type>;
};

using soa_mtv_type = mdds::mtv::soa::multi_type_vector<mtv_traits>;
using aos_mtv_type = mdds::mtv::aos::multi_type_vector<mtv_traits>;

}

namespace mdds { namespace mtv {

template<>
struct clone_block<block2_type>
{
    block2_type* operator()(const block2_type& src) const
    {
        std::unique_ptr<block2_type> cloned_blk = std::make_unique<block2_type>();
        auto cloned(src.store());
        std::transform(cloned.begin(), cloned.end(), cloned.begin(),
            [] (const custom_str1* p){ return new custom_str1(*p); });

        cloned_blk->store().swap(cloned);
        return cloned_blk.release();
    }
};

template<>
struct clone_value<custom_str2*>
{
    custom_str2* operator()(const custom_str2* src) const
    {
        return new custom_str2(*src);
    }
};

}}

void test_block1()
{
    MDDS_TEST_FUNC_SCOPE;
    using blk_type = block1_type;

    blk_type src;
    blk_type::append_value(src, 1.1);
    auto* cloned = blk_type::clone_block(src);

    auto n = blk_type::size(*cloned);
    std::cout << "size=" << n << std::endl;
    for (std::size_t i = 0; i < n; ++i)
        std::cout << "v(" << i << ")=" << blk_type::at(*cloned, i) << std::endl;

    blk_type::delete_block(cloned);
}

void test_block2()
{
    MDDS_TEST_FUNC_SCOPE;
    using blk_type = block2_type;

    blk_type src;
    blk_type::append_value(src, new custom_str1{"foo1"});
    blk_type::append_value(src, new custom_str1{"foo2"});
    blk_type::append_value(src, new custom_str1{"foo3"});
    blk_type::append_value(src, new custom_str1{"foo4"});
    blk_type::append_value(src, new custom_str1{"foo5"});
    auto* cloned = blk_type::clone_block(src);

    auto n = blk_type::size(*cloned);
    std::cout << "size=" << n << std::endl;
    for (std::size_t i = 0; i < n; ++i)
        std::cout << "v(" << i << ")=" << *blk_type::at(*cloned, i) << std::endl;

    blk_type::delete_block(cloned);
}

void test_block3()
{
    MDDS_TEST_FUNC_SCOPE;
    using blk_type = block3_type;

    blk_type src;
    blk_type::append_value(src, new custom_str2{"custom foo1"});
    blk_type::append_value(src, new custom_str2{"custom foo2"});
    blk_type::append_value(src, new custom_str2{"custom foo3"});
    blk_type::append_value(src, new custom_str2{"custom foo4"});
    auto* cloned = blk_type::clone_block(src);

    auto n = blk_type::size(*cloned);
    std::cout << "size=" << n << std::endl;
    for (std::size_t i = 0; i < n; ++i)
        std::cout << "v(" << i << ")=" << *blk_type::at(*cloned, i) << std::endl;

    blk_type::delete_block(cloned);
}

void test_mtv_soa()
{
    MDDS_TEST_FUNC_SCOPE;
    soa_mtv_type store;
    store.push_back<custom_num>(1.2);
    store.push_back<custom_num>(1.3);
    store.push_back_empty();
    store.push_back(new custom_str1{"custom str1"});
    store.push_back(new custom_str2{"custom str2"});

    auto cloned = store.clone();
    std::cout << "size=" << cloned.size() << std::endl;
    std::cout << "block-size=" << cloned.block_size() << std::endl;
    std::cout << "v(0)=" << cloned.get<custom_num>(0) << std::endl;
    std::cout << "v(1)=" << cloned.get<custom_num>(1) << std::endl;
    std::cout << "v(3)=" << *cloned.get<custom_str1*>(3) << std::endl;
    std::cout << "v(4)=" << *cloned.get<custom_str2*>(4) << std::endl;

    std::cout << "store == cloned ? " << std::boolalpha << (store == cloned) << std::endl;
}

void test_mtv_aos()
{
    MDDS_TEST_FUNC_SCOPE;
    aos_mtv_type store;
    store.push_back<custom_num>(1.2);
    store.push_back<custom_num>(1.3);
    store.push_back_empty();
    store.push_back(new custom_str1{"custom str1"});
    store.push_back(new custom_str2{"custom str2"});

    auto cloned = store.clone();
    std::cout << "size=" << cloned.size() << std::endl;
    std::cout << "block-size=" << cloned.block_size() << std::endl;
    std::cout << "v(0)=" << cloned.get<custom_num>(0) << std::endl;
    std::cout << "v(1)=" << cloned.get<custom_num>(1) << std::endl;
    std::cout << "v(3)=" << *cloned.get<custom_str1*>(3) << std::endl;
    std::cout << "v(4)=" << *cloned.get<custom_str2*>(4) << std::endl;

    std::cout << "store == cloned ? " << std::boolalpha << (store == cloned) << std::endl;
}

int main()
{
    test_block1();
    test_block2();
    test_block3();
    test_mtv_soa();
    test_mtv_aos();

    return EXIT_SUCCESS;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

