/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include <mdds/multi_type_vector/soa/main.hpp>

#include <iostream>
#include <string>
#include <vector>

//!code-start: image-def
/**
 * Noncopyable: an image owns a potentially large pixel buffer.
 */
class image
{
    std::string m_name;
    std::vector<unsigned char> m_pixels;

public:
    image(std::string name, std::vector<unsigned char> pixels) :
        m_name(std::move(name)), m_pixels(std::move(pixels)) {}

    image(const image&) = delete;
    image& operator=(const image&) = delete;

    const std::string& name() const
    {
        return m_name;
    }

    void set_name(std::string name)
    {
        m_name = std::move(name);
    }

    const std::vector<unsigned char>& pixels() const
    {
        return m_pixels;
    }
};

// static block ID
constexpr mdds::mtv::element_t image_id = mdds::mtv::element_type_user_start;

// element block type for images
using image_block = mdds::mtv::noncopyable_managed_element_block<image_id, image>;

MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(image, image_id, nullptr, image_block)

namespace mdds { namespace mtv {

template<>
struct clone_value<image*>
{
    image* operator()(const image* src) const
    {
        return new image(src->name(), src->pixels());
    }
};

}}
//!code-end: image-def

//!code-start: cow-traits
struct mtv_traits : mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<image_block>;
    static constexpr bool enable_cow = true;
};

using mtv_type = mdds::mtv::soa::multi_type_vector<mtv_traits>;
//!code-end: cow-traits

int main() try
{
    //!code-start: setup
    // A photo album stores decoded images.
    mtv_type album;
    album.push_back(new image("beach.jpg", std::vector<unsigned char>(4096)));
    album.push_back(new image("sunset.jpg", std::vector<unsigned char>(8192)));
    album.push_back(new image("forest.jpg", std::vector<unsigned char>(2048)));

    // Snapshot the album for undo; the snapshot borrows the album's blocks.
    auto snapshot = album.clone();
    //!code-end: setup

    //!code-start: edit-throws
    // Rename the first image in place; the non-const position() throws while
    // the album is sharing its blocks.
    try
    {
        auto pos = album.position(0);
        image_block::at(*pos.first->data, pos.second)->set_name("beach_edited.jpg");
    }
    catch (const mdds::mtv::shared_block_error& e)
    {
        std::cout << "mutable access failed: " << e.what() << std::endl;
    }
    //!code-end: edit-throws

    //!code-start: detach-then-edit
    // Detach to take sole ownership; the same edit now succeeds.
    album.detach();

    auto pos = album.position(0);
    image_block::at(*pos.first->data, pos.second)->set_name("beach_edited.jpg");

    std::cout << "album now has: " << album.get<image*>(0)->name() << std::endl;
    std::cout << "snapshot still has: " << snapshot.get<image*>(0)->name() << std::endl;
    //!code-end: detach-then-edit

    //!code-start: release-clone
    // Pull an image out of the album to hand it off elsewhere. This internally
    // detaches.
    image* extracted = album.release<image*>(1);

    std::cout << std::boolalpha;
    std::cout << "extracted: " << extracted->name() << std::endl;
    std::cout << "extracted is the snapshot's object: " << (extracted == snapshot.get<image*>(1)) << std::endl;

    delete extracted;
    //!code-end: release-clone

    return EXIT_SUCCESS;
}
catch (const std::exception&)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
