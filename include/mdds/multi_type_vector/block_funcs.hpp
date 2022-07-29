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

namespace detail {

inline void throw_unknown_block(const char* func, const mdds::mtv::element_t type)
{
    std::ostringstream os;
    os << func << ": failed to map to a element block function (type=" << type << ")";
    throw general_error(os.str());
}

template<typename Ret, typename... Args>
auto& find_func(
    const std::unordered_map<element_t, std::function<Ret(Args...)>>& func_map, element_t type,
    const char* src_func_name)
{
    auto it = func_map.find(type);
    if (it == func_map.end())
        detail::throw_unknown_block(src_func_name, type);

    return it->second;
}

} // namespace detail

template<typename... Ts>
struct element_block_funcs
{
    static base_element_block* create_new_block(element_t type, std::size_t init_size)
    {
        static const std::unordered_map<element_t, std::function<base_element_block*(std::size_t)>> func_map{
            {Ts::block_type, Ts::create_block}...};

        auto& f = detail::find_func(func_map, type, __func__);
        return f(init_size);
    }

    static base_element_block* clone_block(const base_element_block& block)
    {
        static const std::unordered_map<element_t, std::function<base_element_block*(const base_element_block&)>>
            func_map{{Ts::block_type, Ts::clone_block}...};

        auto& f = detail::find_func(func_map, get_block_type(block), __func__);
        return f(block);
    }

    static void delete_block(const base_element_block* p)
    {
        if (!p)
            return;

        static const std::unordered_map<element_t, std::function<void(const base_element_block*)>> func_map{
            {Ts::block_type, Ts::delete_block}...};

        // TODO: We should not throw an exception here as this gets called
        // from a destructor and destructors should not throw exceptions.
        auto& f = detail::find_func(func_map, get_block_type(*p), __func__);
        f(p);
    }

    static void resize_block(base_element_block& block, std::size_t new_size)
    {
        static const std::unordered_map<element_t, std::function<void(base_element_block&, std::size_t)>> func_map{
            {Ts::block_type, Ts::resize_block}...};

        auto& f = detail::find_func(func_map, get_block_type(block), __func__);
        f(block, new_size);
    }

    static void print_block(const base_element_block& block)
    {
        static const std::unordered_map<element_t, std::function<void(const base_element_block&)>> func_map{
            {Ts::block_type, Ts::print_block}...};

        auto& f = detail::find_func(func_map, get_block_type(block), __func__);
        f(block);
    }

    static void erase(base_element_block& block, std::size_t pos)
    {
        static const std::unordered_map<element_t, std::function<void(base_element_block&, std::size_t)>> func_map{
            {Ts::block_type, Ts::erase_value}...};

        auto& f = detail::find_func(func_map, get_block_type(block), __func__);
        f(block, pos);
    }

    static void erase(base_element_block& block, std::size_t pos, std::size_t size)
    {
        static const std::unordered_map<element_t, std::function<void(base_element_block&, std::size_t, std::size_t)>>
            func_map{{Ts::block_type, Ts::erase_values}...};

        auto& f = detail::find_func(func_map, get_block_type(block), __func__);
        f(block, pos, size);
    }

    static void append_block(base_element_block& dest, const base_element_block& src)
    {
        static const std::unordered_map<element_t, std::function<void(base_element_block&, const base_element_block&)>>
            func_map{{Ts::block_type, Ts::append_block}...};

        auto& f = detail::find_func(func_map, get_block_type(dest), __func__);
        f(dest, src);
    }

    static void append_values_from_block(
        base_element_block& dest, const base_element_block& src, std::size_t begin_pos, std::size_t len)
    {
        using func_type = std::function<void(base_element_block&, const base_element_block&, std::size_t, std::size_t)>;
        static const std::unordered_map<element_t, func_type> func_map{
            {Ts::block_type, Ts::append_values_from_block}...};

        auto& f = detail::find_func(func_map, get_block_type(dest), __func__);
        f(dest, src, begin_pos, len);
    }

    static void assign_values_from_block(
        base_element_block& dest, const base_element_block& src, std::size_t begin_pos, std::size_t len)
    {
        using func_type = std::function<void(base_element_block&, const base_element_block&, std::size_t, std::size_t)>;
        static const std::unordered_map<element_t, func_type> func_map{
            {Ts::block_type, Ts::assign_values_from_block}...};

        auto& f = detail::find_func(func_map, get_block_type(dest), __func__);
        f(dest, src, begin_pos, len);
    }

    static void prepend_values_from_block(
        base_element_block& dest, const base_element_block& src, std::size_t begin_pos, std::size_t len)
    {
        using func_type = std::function<void(base_element_block&, const base_element_block&, std::size_t, std::size_t)>;
        static const std::unordered_map<element_t, func_type> func_map{
            {Ts::block_type, Ts::prepend_values_from_block}...};

        auto& f = detail::find_func(func_map, get_block_type(dest), __func__);
        f(dest, src, begin_pos, len);
    }

    static void swap_values(
        base_element_block& blk1, base_element_block& blk2, std::size_t pos1, std::size_t pos2, std::size_t len)
    {
        element_t blk1_type = get_block_type(blk1);
        assert(blk1_type == get_block_type(blk2));

        using func_type =
            std::function<void(base_element_block&, base_element_block&, std::size_t, std::size_t, std::size_t)>;
        static const std::unordered_map<element_t, func_type> func_map{{Ts::block_type, Ts::swap_values}...};

        auto& f = detail::find_func(func_map, blk1_type, __func__);
        f(blk1, blk2, pos1, pos2, len);
    }

    static bool equal_block(const base_element_block& left, const base_element_block& right)
    {
        element_t block_type = get_block_type(left);
        if (block_type != get_block_type(right))
            return false;

        using func_type = std::function<bool(const base_element_block&, const base_element_block&)>;
        static const std::unordered_map<element_t, func_type> func_map{{Ts::block_type, Ts::equal_block}...};

        auto& f = detail::find_func(func_map, block_type, __func__);
        return f(left, right);
    }

    static void overwrite_values(base_element_block& block, std::size_t pos, std::size_t len)
    {
        using func_type = std::function<void(base_element_block&, std::size_t, std::size_t)>;
        static const std::unordered_map<element_t, func_type> func_map{{Ts::block_type, Ts::overwrite_values}...};

        auto& f = detail::find_func(func_map, get_block_type(block), __func__);
        f(block, pos, len);
    }

    static void shrink_to_fit(base_element_block& block)
    {
        using func_type = std::function<void(base_element_block&)>;
        static const std::unordered_map<element_t, func_type> func_map{{Ts::block_type, Ts::shrink_to_fit}...};

        auto& f = detail::find_func(func_map, get_block_type(block), __func__);
        f(block);
    }

    static std::size_t size(const base_element_block& block)
    {
        using func_type = std::function<std::size_t(const base_element_block&)>;
        static const std::unordered_map<element_t, func_type> func_map{{Ts::block_type, Ts::size}...};

        auto& f = detail::find_func(func_map, get_block_type(block), __func__);
        return f(block);
    }
};

}} // namespace mdds::mtv

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
