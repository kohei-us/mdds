/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include <mdds/multi_type_vector/soa/main.hpp>

#include <iostream>
#include <vector>

//!code-start: shape-def
struct point
{
    double x;
    double y;
};

/**
 * Noncopyable: the vertex list can be large, so accidental copies are
 * forbidden.
 */
class shape
{
    std::vector<point> m_vertices;

public:
    shape(std::vector<point> vertices) : m_vertices(std::move(vertices))
    {}

    shape(const shape&) = delete;
    shape& operator=(const shape&) = delete;

    const std::vector<point>& vertices() const
    {
        return m_vertices;
    }
};
//!code-end: shape-def

//!code-start: elem-block-def
// static block ID for shape
constexpr mdds::mtv::element_t shape_id = mdds::mtv::element_type_user_start;

// block type for shape
using shape_block = mdds::mtv::noncopyable_managed_element_block<shape_id, shape>;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(shape, shape_id, nullptr, shape_block)
//!code-end: elem-block-def

//!code-start: clone-value-ts
namespace mdds { namespace mtv {

template<>
struct clone_value<shape*>
{
    shape* operator()(const shape* src) const
    {
        // Explicitly duplicate the vertices - the one place a copy is intended.
        return new shape(src->vertices());
    }
};

}}
//!code-end: clone-value-ts

//!code-start: cow-traits
struct mtv_traits : mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<shape_block>;
    static constexpr bool enable_cow = true;
};

using mtv_type = mdds::mtv::soa::multi_type_vector<mtv_traits>;
//!code-end: cow-traits

int main() try
{
    //!code-start: populate
    mtv_type store;
    store.push_back(new shape({{0.0, 0.0}, {4.0, 0.0}, {4.0, 3.0}, {0.0, 3.0}})); // rectangle
    store.push_back(new shape({{0.0, 0.0}, {2.0, 4.0}, {4.0, 0.0}})); // triangle
    //!code-end: populate

    //!code-start: copy-throws
    // Copy construction of a container holding a noncopyable block throws,
    // before any sharing happens, so the source is left intact.
    try
    {
        mtv_type copied(store);
    }
    catch (const mdds::mtv::element_block_error& e)
    {
        std::cout << "copy failed: " << e.what() << std::endl;
    }

    std::cout << "source still holds a shape with " << store.get<shape*>(0)->vertices().size() << " vertices"
              << std::endl;
    //!code-end: copy-throws

    //!code-start: clone-shares
    // clone() shares instead of copying, so it succeeds even for a noncopyable
    // block: the clone borrows the same element block.
    auto cloned = store.clone();

    std::cout << std::boolalpha;
    std::cout << "blocks shared after clone: " << (store.begin()->data == cloned.begin()->data) << std::endl;
    //!code-end: clone-shares

    //!code-start: detach-clones
    // Write a pentagon to position 0 to overwrite it.
    cloned.set(0, new shape({{0.0, 0.0}, {2.0, 1.0}, {4.0, 0.0}, {4.0, 3.0}, {0.0, 3.0}}));

    std::cout << "blocks shared after write: " << (store.begin()->data == cloned.begin()->data) << std::endl;
    std::cout << "source[0] vertices: " << store.get<shape*>(0)->vertices().size() << std::endl;
    std::cout << "clone[0] vertices: " << cloned.get<shape*>(0)->vertices().size() << std::endl;
    //!code-end: detach-clones

    return EXIT_SUCCESS;
}
catch (const std::exception&)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
