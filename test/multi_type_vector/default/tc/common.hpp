/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2021 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

template<typename mtv_type, typename ValT>
bool test_cell_insertion(mtv_type& col_db, std::size_t row, ValT val)
{
    ValT test;
    col_db.set(row, val);
    col_db.get(row, test);

    if (val == test)
        return true;

    std::cout << "row " << row << ": value stored = " << val << "; value retrieved = " << test << std::endl;
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
