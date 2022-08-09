/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2022 Kohei Yoshida
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

#include "test_main.hpp"

// Settting this to 0 should make the multi_type_vector code to NOT include
// the header for the standard element blocks.
#define MDDS_MTV_USE_STANDARD_ELEMENT_BLOCKS 0
#include <mdds/multi_type_vector/aos/main.hpp>
#include <mdds/global.hpp>

namespace mdds { namespace mtv {

// This variable is defined in the header, so if it's included it should cause
// a compiler error.
constexpr element_t element_type_boolean = element_type_reserved_start;

struct standard_element_blocks_trait;
static_assert(!mdds::is_complete<standard_element_blocks_trait>::value,
    "The standard_element_blocks_trait struct should not have been defined.");

}}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

