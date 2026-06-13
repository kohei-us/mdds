/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2022 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>

// `mtv_tmpl` is the storage-variant alias template (aos or soa); the element
// block definitions and `my_traits` come from no_standard_blocks_defs.hpp,
// which the including driver pulls in before the multi_type_vector header.
template<template<typename...> class mtv_tmpl>
void mtv_test_no_standard_blocks_basic()
{
    MDDS_TEST_FUNC_SCOPE;

    using this_mtv_type = mtv_tmpl<my_traits>;

    this_mtv_type db{};

    db.push_back(true);
    db.push_back(false);
    db.template push_back<std::int32_t>(123);
    db.template push_back<std::uint32_t>(456u);

    TEST_ASSERT(db.size() == 4);
    TEST_ASSERT(db.block_size() == 3);
    TEST_ASSERT(db.template get<bool>(0) == true);
    TEST_ASSERT(db.template get<bool>(1) == false);
    TEST_ASSERT(db.template get<std::int32_t>(2) == 123);
    TEST_ASSERT(db.template get<std::uint32_t>(3) == 456u);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
