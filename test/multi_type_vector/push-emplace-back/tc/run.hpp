/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "push_back.hpp"

template<typename mtv_type>
void run_all_tests()
{
    test_push_back_copy<mtv_type>();
    test_push_back_move<mtv_type>();
    test_emplace_back<mtv_type>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
