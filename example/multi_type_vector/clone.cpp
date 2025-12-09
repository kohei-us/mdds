/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2025 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include <mdds/multi_type_vector/soa/main.hpp>

#include <iostream>
#include <random>

//!code-start: stream-store
/**
 * Fictional class to store a stream of data from an external location.
 */
class stream_store
{
    std::vector<uint8_t> m_buffer;

public:
    stream_store() = default;
    stream_store(std::vector<uint8_t> data) : m_buffer(std::move(data))
    {
        std::cout << "storing a buffer of size " << m_buffer.size() << std::endl;
    }

    stream_store(const stream_store&) = delete;

    ~stream_store()
    {
        std::cout << "disposing of the buffer of size " << m_buffer.size() << std::endl;
    }

    const std::vector<uint8_t>& get_buffer() const { return m_buffer; }
};
//!code-end: stream-store

//!code-start: elem-block-def
// static block ID for stream_store
constexpr mdds::mtv::element_t stream_store_id = mdds::mtv::element_type_user_start;

// block type for stream_store
using stream_store_block_type =
    mdds::mtv::noncopyable_managed_element_block<stream_store_id, stream_store>;

// macro that defines for callbacks for pointer type
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(
    stream_store,
    stream_store_id,
    nullptr,
    stream_store_block_type
)
//!code-end: elem-block-def

//!code-start: mtv-def
struct mtv_traits : mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<stream_store_block_type>;
};

using mtv_type = mdds::mtv::soa::multi_type_vector<mtv_traits>;
//!code-end: mtv-def

//!code-start: fetch-buffer
/**
 * Fictional function to fetch a stream of data from an external location.
 */
std::vector<uint8_t> fetch_buffer();
//!code-end: fetch-buffer

//!code-start: clone-value-ts
namespace mdds { namespace mtv {

template<>
struct clone_value<stream_store*>
{
    stream_store* operator()(const stream_store* src) const
    {
        return new stream_store(src->get_buffer());
    }
};

}}
//!code-end: clone-value-ts

int main() try
{
    //!code-start: populate-store
    mtv_type store;

    store.push_back(new stream_store(fetch_buffer()));
    store.push_back(new stream_store(fetch_buffer()));
    store.push_back(new stream_store(fetch_buffer()));
    store.push_back(new stream_store(fetch_buffer()));
    store.push_back(new stream_store(fetch_buffer()));
    //!code-end: populate-store

    //!code-start: clone-store
    std::cout << "cloning the container..." << std::endl;
    auto cloned = store.clone();
    std::cout << "done cloning" << std::endl;
    //!code-end: clone-store

    return EXIT_SUCCESS;
}
catch (const std::exception&)
{
    return EXIT_FAILURE;
}

std::vector<uint8_t> fetch_buffer()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());

    std::uniform_int_distribution<std::size_t> size_dist(0, 1048576);
    std::vector<uint8_t> buffer(size_dist(gen));

    std::uniform_int_distribution<std::uint8_t> value_dist(0, 255);
    for (auto& byte : buffer)
        byte = value_dist(gen);

    return buffer;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

