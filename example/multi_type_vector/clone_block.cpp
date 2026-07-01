/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

// SPDX-FileCopyrightText: 2026 Kohei Yoshida
//
// SPDX-License-Identifier: MIT

#include <mdds/multi_type_vector/soa/main.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <vector>

//!code-start: pooled-string
/**
 * A pool of strings shared by every element stored in a single block.
 */
using string_pool = std::vector<std::string>;

/**
 * References a string in a shared pool by its index.  All elements in a block
 * share one pool, so it should be cloned once per block, not once per element.
 * Copying is disabled to force duplication through the clone machinery.
 */
class pooled_string
{
    std::shared_ptr<string_pool> m_pool;
    std::size_t m_index;

public:
    pooled_string(std::shared_ptr<string_pool> pool, std::size_t index) :
        m_pool(std::move(pool)), m_index(index) {}

    pooled_string(const pooled_string&) = delete;

    const std::shared_ptr<string_pool>& pool() const
    {
        return m_pool;
    }

    std::size_t index() const
    {
        return m_index;
    }

    const std::string& value() const
    {
        return (*m_pool)[m_index];
    }
};
//!code-end: pooled-string

//!code-start: elem-block-def
// static block ID for pooled_string
constexpr mdds::mtv::element_t pooled_string_id = mdds::mtv::element_type_user_start;

// block type for pooled_string
using pooled_string_block_type = mdds::mtv::noncopyable_managed_element_block<pooled_string_id, pooled_string>;

// macro that defines the callbacks for pointer type
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(pooled_string, pooled_string_id, nullptr, pooled_string_block_type)
//!code-end: elem-block-def

//!code-start: mtv-def
struct mtv_traits : mdds::mtv::default_traits
{
    using block_funcs = mdds::mtv::element_block_funcs<pooled_string_block_type>;
};

using mtv_type = mdds::mtv::soa::multi_type_vector<mtv_traits>;
//!code-end: mtv-def

//!code-start: clone-block-ts
namespace mdds { namespace mtv {

template<>
struct clone_block<pooled_string_block_type>
{
    pooled_string_block_type* operator()(const pooled_string_block_type& src) const
    {
        auto dest = std::make_unique<pooled_string_block_type>();

        // store() exposes the block's underlying element container; for a
        // managed block it holds the raw pointers to the stored elements.
        const auto& elements = src.store();
        if (elements.size() == 0)
            return dest.release();

        // Every element in the block shares the same pool; clone it just once
        // and let all cloned elements reference the new pool.
        auto cloned_pool = std::make_shared<string_pool>(*elements[0]->pool());
        std::cout << "cloning a pool of " << cloned_pool->size() << " strings shared by " << elements.size()
                  << " elements" << std::endl;

        auto& cloned = dest->store();
        cloned.reserve(elements.size());
        for (const pooled_string* elem : elements)
            cloned.push_back(new pooled_string(cloned_pool, elem->index()));

        return dest.release();
    }
};

}} // namespace mdds::mtv
//!code-end: clone-block-ts

int main() try
{
    //!code-start: populate-store
    auto pool = std::make_shared<string_pool>(string_pool{"alpha", "beta", "gamma", "delta"});

    mtv_type store;
    store.push_back(new pooled_string(pool, 0));
    store.push_back(new pooled_string(pool, 2));
    store.push_back(new pooled_string(pool, 1));
    store.push_back(new pooled_string(pool, 3));
    //!code-end: populate-store

    //!code-start: clone-store
    std::cout << "cloning the container..." << std::endl;
    auto cloned = store.clone();
    std::cout << "done cloning" << std::endl;
    //!code-end: clone-store

    //!code-start: inspect-store
    // The original and the clone reference separate pools.
    for (std::size_t i = 0; i < store.size(); ++i)
    {
        const pooled_string* orig = store.get<pooled_string*>(i);
        const pooled_string* copy = cloned.get<pooled_string*>(i);
        std::cout << "element " << i << ": \"" << orig->value() << "\" (pool is "
                  << (orig->pool() == copy->pool() ? "shared" : "separate") << ")" << std::endl;
    }
    //!code-end: inspect-store

    return EXIT_SUCCESS;
}
catch (const std::exception&)
{
    return EXIT_FAILURE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
