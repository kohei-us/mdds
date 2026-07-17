/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "clone.hpp"

template<typename mtv_type>
void run_all_tests()
{
    test_clone<mtv_type>();
    test_clone_stateful<mtv_type>();
    test_clone_group_payload<mtv_type>();
    test_clone_throw_mid_block<mtv_type>();
    test_noclone<mtv_type>(); // now runs for both variants (previously aos-only)
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
