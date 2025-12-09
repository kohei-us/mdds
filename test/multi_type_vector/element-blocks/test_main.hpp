/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2022 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "test_global.hpp" // This must be the first header to be included.

// We don't include any multi_type_vector specific headers here in order to
// test features controlled by preprocessor defines.

void mtv_test_element_blocks_buildability();
void mtv_test_element_blocks_std_vector();
void mtv_test_element_blocks_std_deque();
void mtv_test_element_blocks_std_vector_bool();
void mtv_test_element_blocks_std_deque_bool();
void mtv_test_element_blocks_delayed_delete_vector_bool();
void mtv_test_element_blocks_range();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
