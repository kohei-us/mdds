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

#pragma once

#include <iostream>
#include <memory>
#include <mdds/multi_type_vector/types.hpp>
#include <mdds/multi_type_vector/macro.hpp>
#include <mdds/multi_type_vector/util.hpp>

struct data_block {};

class user_cell
{
    std::unique_ptr<data_block> data;

public:
    static std::size_t copy_count;
    static std::size_t move_count;

    static void reset_counter()
    {
        copy_count = 0;
        move_count = 0;
    }

    static void print_counters()
    {
        std::cout << "user_cell: copy count=" << user_cell::copy_count << "; move count=" << user_cell::move_count << std::endl;
    }

    user_cell() : data(std::make_unique<data_block>())
    {
        std::cout << "default constructor" << std::endl;
    }

    user_cell(const user_cell& other) : data(std::make_unique<data_block>(*other.data))
    {
        ++user_cell::copy_count;
        std::cout << "copy constructor" << std::endl;
    }

    user_cell(user_cell&& other) noexcept : data(std::move(other.data))
    {
        ++user_cell::move_count;
        std::cout << "move constructor" << std::endl;
    }

    user_cell& operator=(const user_cell& other)
    {
        std::cout << "copy assignment" << std::endl;
        data = std::make_unique<data_block>(*other.data);
        return *this;
    }

    user_cell& operator=(user_cell&& other)
    {
        std::cout << "move assignment" << std::endl;
        data = std::move(other.data);
        return *this;
    }
};

inline std::size_t user_cell::copy_count = 0;
inline std::size_t user_cell::move_count = 0;

constexpr mdds::mtv::element_t element_type_user_block = mdds::mtv::element_type_user_start;

using user_block = mdds::mtv::default_element_block<element_type_user_block, user_cell>;

struct user_traits : public mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<user_block>;
};

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(user_cell, element_type_user_block, user_cell(), user_block)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
