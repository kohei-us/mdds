// SPDX-FileCopyrightText: 2011 - 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#pragma once

#include "./multi_type_vector/soa/main.hpp"

namespace mdds {

/**
 * Type alias for the concrete implementation to maintain backward API
 * compatibility.
 */
template<typename Traits = mtv::default_traits>
using multi_type_vector = mtv::soa::multi_type_vector<Traits>;

} // namespace mdds
