/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include <mdds/multi_type_vector/soa/main.hpp>

#include <iostream>
#include <string>

//!code-start: shape-block

/**
 * Class representing a drawing primitive.  A real-world example would contain
 * geometry data but it is kept minimal in this example.
 */
struct shape
{
    int id = 0;

    // geometry data may come here in a real-world situation...
};

// element block definition
constexpr mdds::mtv::element_t shape_id = mdds::mtv::element_type_user_start;
using shape_block = mdds::mtv::default_element_block<shape_id, shape>;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(shape, shape_id, shape{}, shape_block)
//!code-end: shape-block

//!code-start: handler
struct layer;

/**
 * Event handler holding a back-pointer to its parent layer.  It keeps the
 * layer's count of live element blocks up-to-date as block events arrive.
 */
struct block_counter
{
    layer* parent = nullptr;

    block_counter() = default;

    explicit block_counter(layer* l) : parent(l)
    {}

    void element_block_acquired(const mdds::mtv::base_element_block* /*block*/);
    void element_block_released(const mdds::mtv::base_element_block* /*block*/);
};
//!code-end: handler

//!code-start: traits
struct layer_traits : mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<shape_block>;
    using event_func = block_counter;
};

using shape_store = mdds::mtv::soa::multi_type_vector<layer_traits>;
//!code-end: traits

//!code-start: layer
/**
 * A drawing layer owns its shapes in a shape store.
 */
struct layer
{
    std::string name;
    int block_count = 0;
    shape_store shapes;

    /**
     * Creates a layer with a handler that points back to the layer being
     * created.
     */
    explicit layer(std::string layer_name) :
        name(std::move(layer_name)),
        shapes(block_counter(this)) {}

    /**
     * Clones another layer but uses a brand-new handler that points back to the
     * new layer.
     */
    layer(const layer& src, std::string layer_name) :
        name(std::move(layer_name)),
        shapes(src.shapes.clone(block_counter(this))) {}
};

// The handler's methods are defined below.

void block_counter::element_block_acquired(const mdds::mtv::base_element_block* /*block*/)
{
    if (parent)
        ++parent->block_count;
}

void block_counter::element_block_released(const mdds::mtv::base_element_block* /*block*/)
{
    if (parent)
        --parent->block_count;
}
//!code-end: layer

int main() try
{
    //!code-start: populate
    // Create a background layer and add a few shapes to it.  Creating the
    // element block reports an acquisition to the layer's handler, bumping its
    // block count.
    layer background("background");
    background.shapes.push_back(shape{1});
    background.shapes.push_back(shape{2});
    background.shapes.push_back(shape{3});

    std::cout << "background block count: " << background.block_count << std::endl;
    //!code-end: populate

    //!code-start: clone-layer
    // Clone the whole layer.  Its clone constructor clones the shape store with
    // a handler that points back to the new foreground layer.
    layer foreground(background, "foreground");
    //!code-end: clone-layer

    //!code-start: erase-shape
    // Delete the middle shape from the foreground layer only.  Deleting empties
    // its slot rather than shifting the others, so the hole splits the run into
    // two shape blocks, fragmenting the foreground's storage.
    foreground.shapes.set_empty(1, 1);
    //!code-end: erase-shape

    //!code-start: observe
    // The two layers' block counts now differ: the foreground's reflects the
    // extra block produced by the split, while the background's is untouched.
    std::cout << "foreground block count: " << foreground.block_count << std::endl;
    std::cout << "background block count after clone: " << background.block_count << std::endl;

    // Each store's handler points back to its own layer.
    std::cout << "background handler parent: " << background.shapes.event_handler().parent->name << std::endl;
    std::cout << "foreground handler parent: " << foreground.shapes.event_handler().parent->name << std::endl;
    //!code-end: observe

    return EXIT_SUCCESS;
}
catch (const std::exception&)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
