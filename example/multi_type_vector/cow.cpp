/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include <mdds/multi_type_vector/soa/main.hpp>
#include <mdds/multi_type_vector/standard_element_blocks.hpp>

#include <iostream>

//!code-start: cow-traits
struct mtv_traits : mdds::mtv::standard_element_blocks_traits
{
    static constexpr bool enable_cow = true;
};

using mtv_type = mdds::mtv::soa::multi_type_vector<mtv_traits>;
//!code-end: cow-traits

int main() try
{
    //!code-start: populate
    mtv_type original;
    original.push_back(1.1);
    original.push_back(1.2);
    original.push_back(1.3);
    //!code-end: populate

    //!code-start: copy-shares
    // Copying shares the source's element blocks - no duplication happens yet.
    mtv_type copied(original);

    std::cout << std::boolalpha;
    std::cout << "blocks shared right after copy: " << (original.begin()->data == copied.begin()->data) << std::endl;
    //!code-end: copy-shares

    //!code-start: detach-on-write
    // The first write to the copy detaches it: it makes its own private copy of
    // the block, so the two containers no longer share it.
    copied.set(0, 9.9);

    std::cout << "blocks shared after the write: " << (original.begin()->data == copied.begin()->data) << std::endl;
    //!code-end: detach-on-write

    //!code-start: original-intact
    // The original is untouched by the write to the copy.
    std::cout << "original[0] = " << original.get<double>(0) << std::endl;
    std::cout << "copied[0] = " << copied.get<double>(0) << std::endl;
    //!code-end: original-intact

    return EXIT_SUCCESS;
}
catch (const std::exception&)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
