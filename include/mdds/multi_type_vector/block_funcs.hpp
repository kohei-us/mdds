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

#pragma once

#include "./types.hpp"
#include "../global.hpp"

#include <unordered_map>
#include <functional>
#include <sstream>

namespace mdds { namespace mtv {

template<typename... Ts>
struct element_block_funcs
{
    static base_element_block* create_new_block(element_t type, std::size_t init_size)
    {
        static const std::unordered_map<element_t, std::function<base_element_block*(std::size_t)>> func_map{
            {Ts::block_type, Ts::create_block}...};

        auto it = func_map.find(type);
        if (it == func_map.end())
            throw general_error("create_new_block: failed to create a new block of unknown type.");

        return it->second(init_size);
    }

    static base_element_block* clone_block(const base_element_block& block)
    {
        static const std::unordered_map<element_t, std::function<base_element_block*(const base_element_block&)>>
            func_map{{Ts::block_type, Ts::clone_block}...};

        auto it = func_map.find(get_block_type(block));
        if (it == func_map.end())
            throw general_error("clone_block: failed to clone a block of unknown type.");

        return it->second(block);
    }

    static void delete_block(const base_element_block* p)
    {
        if (!p)
            return;

        static std::unordered_map<element_t, std::function<void(const base_element_block*)>> func_map{
            {Ts::block_type, Ts::delete_block}...};

        auto it = func_map.find(get_block_type(*p));
        if (it == func_map.end())
        {
#ifdef MDDS_MULTI_TYPE_VECTOR_DEBUG
            // We sould not throw an exception here as this gets called from a
            // destructor and destructors should not throw exceptions.
            std::ostringstream os;
            os << __FILE__ << "#" << __LINE__ << " (element_block_func_base:delete_block): "
               << "failed to delete a block of unknown type (" << get_block_type(*p) << ")" << std::endl;
            throw general_error(os.str());
#else
            throw general_error("delete_block: failed to delete a block of unknown type.");
#endif
        }

        it->second(p);
    }
};

}} // namespace mdds::mtv

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
