/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "share.hpp"

template<typename cow_mtv_type, typename non_cow_mtv_type>
void run_all_tests()
{
    test_cow_share<cow_mtv_type>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
