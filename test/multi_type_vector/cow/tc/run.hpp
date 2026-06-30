/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "share.hpp"
#include "detach.hpp"
#include "lifetime.hpp"
#include "noncopyable.hpp"
#include "iterator_gate.hpp"

template<typename cow_mtv_type, typename non_cow_mtv_type>
void run_all_tests()
{
    test_cow_share<cow_mtv_type>();
    test_cow_detach_on_write<cow_mtv_type>();
    test_cow_both_side<cow_mtv_type>();
    test_cow_lifetime<cow_mtv_type>();
    test_cow_noncopyable<cow_mtv_type>();
    test_cow_iterator_gate<cow_mtv_type, non_cow_mtv_type>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
