/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "share.hpp"
#include "detach.hpp"
#include "hints.hpp"
#include "lifetime.hpp"
#include "noncopyable.hpp"
#include "mutable_access.hpp"
#include "reclaim.hpp"
#include "clone_hdl.hpp"

template<typename cow_mtv_type, typename non_cow_mtv_type>
void run_all_tests()
{
    test_cow_share<cow_mtv_type>();
    test_cow_is_shared<cow_mtv_type, non_cow_mtv_type>();
    test_cow_detach_on_write<cow_mtv_type>();
    test_cow_both_side<cow_mtv_type>();
    test_cow_release_detaches<cow_mtv_type>();
    test_cow_release_pos_hint_detaches<cow_mtv_type>();
    test_cow_release_clone_identity<cow_mtv_type>();
    test_cow_nonreturning_release_requires_sole_ownership<cow_mtv_type>();
    test_cow_detach_noop_when_sole_owner<cow_mtv_type>();
    test_cow_hinted_mutation<cow_mtv_type>();
    test_cow_lifetime<cow_mtv_type>();
    test_cow_noncopyable<cow_mtv_type>();
    test_cow_mutable_accessor_types<cow_mtv_type, non_cow_mtv_type>();
    test_cow_mutable_access_throws_while_shared<cow_mtv_type>();
    test_cow_mutable_access_after_detach<cow_mtv_type>();
    test_cow_const_access_never_detaches<cow_mtv_type>();
    test_cow_last_sharer_reclaim<cow_mtv_type>();
    test_cow_last_sharer_reclaim_on_write<cow_mtv_type>();
    test_cow_clone_hdl();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
