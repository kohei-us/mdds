/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <mdds/multi_type_vector/macro.hpp>
#include <mdds/multi_type_vector/standard_element_blocks.hpp>

/** Caller manages the life cycle of these cells. */
struct user_cell
{
    double value;

    user_cell() : value(0.0)
    {}
    user_cell(double _v) : value(_v)
    {}
};

/**
 * Managed user cell: the storing block manages the life cycle of these
 * cells.
 */
struct muser_cell
{
    double value;

    muser_cell() : value(0.0)
    {}
    muser_cell(const muser_cell& r) : value(r.value)
    {}
    muser_cell(double _v) : value(_v)
    {}
    ~muser_cell()
    {}
};

enum my_fruit_type
{
    unknown_fruit = 0,
    apple,
    orange,
    mango,
    peach
};

struct date
{
    int year;
    int month;
    int day;

    date() : year(0), month(0), day(0)
    {}
    date(int _year, int _month, int _day) : year(_year), month(_month), day(_day)
    {}
};

/** custom cell type definition. */
constexpr mdds::mtv::element_t element_type_user_block = mdds::mtv::element_type_user_start;
constexpr mdds::mtv::element_t element_type_muser_block = mdds::mtv::element_type_user_start + 1;
constexpr mdds::mtv::element_t element_type_fruit_block = mdds::mtv::element_type_user_start + 2;
constexpr mdds::mtv::element_t element_type_date_block = mdds::mtv::element_type_user_start + 3;

using user_cell_block = mdds::mtv::default_element_block<element_type_user_block, user_cell*>;
using muser_cell_block = mdds::mtv::managed_element_block<element_type_muser_block, muser_cell>;
using fruit_block = mdds::mtv::default_element_block<element_type_fruit_block, my_fruit_type>;
using date_block = mdds::mtv::default_element_block<element_type_date_block, date>;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(user_cell, element_type_user_block, nullptr, user_cell_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(muser_cell, element_type_muser_block, nullptr, muser_cell_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(my_fruit_type, element_type_fruit_block, unknown_fruit, fruit_block)
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(date, element_type_date_block, date(), date_block)

template<typename T>
class cell_pool
{
    std::vector<std::unique_ptr<T>> m_pool;

public:
    cell_pool() = default;
    cell_pool(const cell_pool&) = delete;
    cell_pool& operator=(const cell_pool&) = delete;

    T* construct()
    {
        m_pool.emplace_back(new T);
        return m_pool.back().get();
    }

    void clear()
    {
        m_pool.clear();
    }
};

class user_cell_pool : public cell_pool<user_cell>
{
public:
    user_cell* construct(double val)
    {
        user_cell* p = cell_pool<user_cell>::construct();
        p->value = val;
        return p;
    }
};

struct user_muser_trait : public mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<
        mdds::mtv::boolean_element_block, mdds::mtv::int8_element_block, mdds::mtv::uint8_element_block,
        mdds::mtv::int16_element_block, mdds::mtv::uint16_element_block, mdds::mtv::int32_element_block,
        mdds::mtv::uint32_element_block, mdds::mtv::int64_element_block, mdds::mtv::uint64_element_block,
        mdds::mtv::float_element_block, mdds::mtv::double_element_block, mdds::mtv::string_element_block,
        user_cell_block, muser_cell_block>;
};

struct fruit_trait : public mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<
        mdds::mtv::boolean_element_block, mdds::mtv::int8_element_block, mdds::mtv::uint8_element_block,
        mdds::mtv::int16_element_block, mdds::mtv::uint16_element_block, mdds::mtv::int32_element_block,
        mdds::mtv::uint32_element_block, mdds::mtv::int64_element_block, mdds::mtv::uint64_element_block,
        mdds::mtv::float_element_block, mdds::mtv::double_element_block, mdds::mtv::string_element_block, fruit_block>;
};

struct muser_fruit_date_trait : public mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<
        mdds::mtv::boolean_element_block, mdds::mtv::int8_element_block, mdds::mtv::uint8_element_block,
        mdds::mtv::int16_element_block, mdds::mtv::uint16_element_block, mdds::mtv::int32_element_block,
        mdds::mtv::uint32_element_block, mdds::mtv::int64_element_block, mdds::mtv::uint64_element_block,
        mdds::mtv::float_element_block, mdds::mtv::double_element_block, mdds::mtv::string_element_block,
        muser_cell_block, fruit_block, date_block>;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
