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

    // Snapshot the album for undo.  A noncopyable block can't be copied, so we
    // clone(); under COW that is cheap: the snapshot borrows the same blocks
    // instead of duplicating every image.
    auto snapshot = album.clone();
    //!code-end: setup

    //!code-start: release-throws
    // We now want to pull the first two images out of the album to hand them
    // off elsewhere.  release_range() refuses, because the album is still
    // sharing its blocks with the snapshot.
    try
    {
        album.release_range(0, 1);
    }
    catch (const mdds::mtv::shared_block_error& e)
    {
        std::cout << "release failed: " << e.what() << std::endl;
    }
    //!code-end: release-throws

    //!code-start: detach-then-release
    // Take sole ownership first.  detach() gives the album its own private
    // copies of the images while the snapshot keeps the originals, after which
    // release_range() succeeds.
    album.detach();

    image* first = album.get<image*>(0);
    image* second = album.get<image*>(1);
    album.release_range(0, 1);

    // We own these two images now.  Rename one to show it is a distinct object
    // from the snapshot's: the snapshot's copy keeps its original name.
    first->set_name("beach_edited.jpg");

    std::cout << "extracted and renamed: " << first->name() << " and " << second->name() << std::endl;
    std::cout << "snapshot still has: " << snapshot.get<image*>(0)->name() << std::endl;

    delete first;
    delete second;
    //!code-end: detach-then-release

    return EXIT_SUCCESS;
}
catch (const std::exception&)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
