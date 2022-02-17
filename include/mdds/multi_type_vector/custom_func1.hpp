/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2021 Kohei Yoshida
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

#ifndef INCLUDED_MDDS_MULTI_TYPE_VECTOR_DIR_CUSTOM_FUNC1_HPP
#define INCLUDED_MDDS_MULTI_TYPE_VECTOR_DIR_CUSTOM_FUNC1_HPP

#include "types.hpp"
#include "trait.hpp"

namespace mdds { namespace mtv {

/**
 * Block function template for multi_type_vector with 1 user-defined block.
 */
template<typename _Block>
struct custom_block_func1
{
    static base_element_block* create_new_block(element_t type, size_t init_size)
    {
        switch (type)
        {
            case _Block::block_type:
                return _Block::create_block(init_size);
            default:;
        }

        return element_block_func::create_new_block(type, init_size);
    }

    static base_element_block* clone_block(const base_element_block& block)
    {
        switch (get_block_type(block))
        {
            case _Block::block_type:
                return _Block::clone_block(block);
            default:;
        }

        return element_block_func::clone_block(block);
    }

    static void delete_block(const base_element_block* p)
    {
        if (!p)
            return;

        switch (get_block_type(*p))
        {
            case _Block::block_type:
                _Block::delete_block(p);
                break;
            default:
                element_block_func::delete_block(p);
        }
    }

    static void resize_block(base_element_block& block, size_t new_size)
    {
        switch (get_block_type(block))
        {
            case _Block::block_type:
                _Block::resize_block(block, new_size);
                break;
            default:
                element_block_func::resize_block(block, new_size);
        }
    }

    static void print_block(const base_element_block& block)
    {
        switch (get_block_type(block))
        {
            case _Block::block_type:
                _Block::print_block(block);
                break;
            default:
                element_block_func::print_block(block);
        }
    }

    static void erase(base_element_block& block, size_t pos)
    {
        switch (get_block_type(block))
        {
            case _Block::block_type:
                _Block::erase_block(block, pos);
                break;
            default:
                element_block_func::erase(block, pos);
        }
    }

    static void erase(base_element_block& block, size_t pos, size_t size)
    {
        switch (get_block_type(block))
        {
            case _Block::block_type:
                _Block::erase_block(block, pos, size);
                break;
            default:
                element_block_func_base::erase(block, pos, size);
        }
    }

    static void append_values_from_block(base_element_block& dest, const base_element_block& src)
    {
        switch (get_block_type(dest))
        {
            case _Block::block_type:
                _Block::append_values_from_block(dest, src);
                break;
            default:
                element_block_func_base::append_values_from_block(dest, src);
        }
    }

    static void append_values_from_block(
        base_element_block& dest, const base_element_block& src, size_t begin_pos, size_t len)
    {
        switch (get_block_type(dest))
        {
            case _Block::block_type:
                _Block::append_values_from_block(dest, src, begin_pos, len);
                break;
            default:
                element_block_func_base::append_values_from_block(dest, src, begin_pos, len);
        }
    }

    static void assign_values_from_block(
        base_element_block& dest, const base_element_block& src, size_t begin_pos, size_t len)
    {
        switch (get_block_type(dest))
        {
            case _Block::block_type:
                _Block::assign_values_from_block(dest, src, begin_pos, len);
                break;
            default:
                element_block_func_base::assign_values_from_block(dest, src, begin_pos, len);
        }
    }

    static void prepend_values_from_block(
        base_element_block& dest, const base_element_block& src, size_t begin_pos, size_t len)
    {
        switch (get_block_type(dest))
        {
            case _Block::block_type:
                _Block::prepend_values_from_block(dest, src, begin_pos, len);
                break;
            default:
                element_block_func_base::prepend_values_from_block(dest, src, begin_pos, len);
        }
    }

    static void swap_values(base_element_block& blk1, base_element_block& blk2, size_t pos1, size_t pos2, size_t len)
    {
        switch (get_block_type(blk1))
        {
            case _Block::block_type:
                _Block::swap_values(blk1, blk2, pos1, pos2, len);
                break;
            default:
                element_block_func_base::swap_values(blk1, blk2, pos1, pos2, len);
        }
    }

    static bool equal_block(const base_element_block& left, const base_element_block& right)
    {
        if (get_block_type(left) == _Block::block_type)
        {
            if (get_block_type(right) != _Block::block_type)
                return false;

            return _Block::get(left) == _Block::get(right);
        }
        else if (mtv::get_block_type(right) == _Block::block_type)
            return false;

        return element_block_func::equal_block(left, right);
    }

    static void overwrite_values(base_element_block& block, size_t pos, size_t len)
    {
        switch (get_block_type(block))
        {
            case _Block::block_type:
                _Block::overwrite_values(block, pos, len);
                break;
            default:
                element_block_func::overwrite_values(block, pos, len);
        }
    }

    static void shrink_to_fit(base_element_block& block)
    {
        switch (get_block_type(block))
        {
            case _Block::block_type:
                _Block::shrink_to_fit(block);
                break;
            default:
                element_block_func::shrink_to_fit(block);
        }
    }

    static size_t size(const base_element_block& block)
    {
        switch (get_block_type(block))
        {
            case _Block::block_type:
                return _Block::size(block);
            default:
                return element_block_func::size(block);
        }
    }
};

}} // namespace mdds::mtv

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
