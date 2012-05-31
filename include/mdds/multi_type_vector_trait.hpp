/*************************************************************************
 *
 * Copyright (c) 2012 Kohei Yoshida
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

#ifndef __MDDS_MULTI_TYPE_VECTOR_TRAIT_HPP__
#define __MDDS_MULTI_TYPE_VECTOR_TRAIT_HPP__

#include "multi_type_vector_types.hpp"

#include <vector>

namespace mdds { namespace mtv {

typedef default_element_block<element_type_numeric, double>     numeric_cell_block;
typedef default_element_block<element_type_string, std::string> string_cell_block;
typedef default_element_block<element_type_index, size_t>       index_cell_block;
typedef default_element_block<element_type_boolean, bool>       boolean_cell_block;

void get_value(const base_element_block& block, size_t pos, double& val)
{
    numeric_cell_block::get_value(block, pos, val);
}

void get_value(const base_element_block& block, size_t pos, std::string& val)
{
    string_cell_block::get_value(block, pos, val);
}

void get_value(const base_element_block& block, size_t pos, size_t& val)
{
    index_cell_block::get_value(block, pos, val);
}

void get_value(const base_element_block& block, size_t pos, bool& val)
{
    boolean_cell_block::get_value(block, pos, val);
}

void append_value(base_element_block& block, double val)
{
    numeric_cell_block::append_value(block, val);
}

void append_value(base_element_block& block, const std::string& val)
{
    string_cell_block::append_value(block, val);
}

void append_value(base_element_block& block, size_t val)
{
    index_cell_block::append_value(block, val);
}

void append_value(base_element_block& block, bool val)
{
    boolean_cell_block::append_value(block, val);
}

void prepend_value(base_element_block& block, double val)
{
    numeric_cell_block::prepend_value(block, val);
}

void prepend_value(base_element_block& block, const std::string& val)
{
    string_cell_block::prepend_value(block, val);
}

void prepend_value(base_element_block& block, size_t val)
{
    index_cell_block::prepend_value(block, val);
}

void prepend_value(base_element_block& block, bool val)
{
    boolean_cell_block::prepend_value(block, val);
}

void get_empty_value(double& val)
{
    val = 0.0;
}

void get_empty_value(std::string& val)
{
    val = std::string();
}

void get_empty_value(size_t& val)
{
    val = 0;
}

void get_empty_value(bool& val)
{
    val = false;
}

struct cell_block_func_base
{
    static base_element_block* create_new_block(element_t type, size_t init_size);

    static base_element_block* clone_block(const base_element_block& block);

    static void delete_block(base_element_block* p);

    static void resize_block(base_element_block& block, size_t new_size);

    static void print_block(const base_element_block& block);

    static void erase(base_element_block& block, size_t pos);

    static void erase(base_element_block& block, size_t pos, size_t size);

    static void append_values_from_block(base_element_block& dest, const base_element_block& src);

    static void append_values_from_block(
        base_element_block& dest, const base_element_block& src, size_t begin_pos, size_t len);

    static void assign_values_from_block(
        base_element_block& dest, const base_element_block& src, size_t begin_pos, size_t len);

    static bool equal_block(const base_element_block& left, const base_element_block& right);

    /**
     * This method gets called when cell values are being overwritten by new
     * values.  This provides the client code an opportunity to delete
     * overwritten instances in case the block stores pointers to managed
     * objects.  For blocks that don't need to manage their stored objects (or
     * store primitive values), this method can be left empty.
     */
    static void overwrite_values(base_element_block& block, size_t pos, size_t len);
};

base_element_block* cell_block_func_base::create_new_block(element_t type, size_t init_size)
{
    switch (type)
    {
        case element_type_numeric:
            return numeric_cell_block::create_block(init_size);
        case element_type_string:
            return string_cell_block::create_block(init_size);
        case element_type_index:
            return index_cell_block::create_block(init_size);
        case element_type_boolean:
            return boolean_cell_block::create_block(init_size);
        default:
            throw general_error("create_new_block: failed to create a new block of unknown type.");
    }
    return NULL;
}

base_element_block* cell_block_func_base::clone_block(const base_element_block& block)
{
    switch (get_block_type(block))
    {
        case element_type_numeric:
            return numeric_cell_block::clone_block(block);
        case element_type_string:
            return string_cell_block::clone_block(block);
        case element_type_index:
            return index_cell_block::clone_block(block);
        case element_type_boolean:
            return boolean_cell_block::clone_block(block);
        default:
            throw general_error("clone_block: failed to clone a block of unknown type.");
    }
    return NULL;
}

void cell_block_func_base::delete_block(base_element_block* p)
{
    if (!p)
        return;

    switch (get_block_type(*p))
    {
        case element_type_numeric:
            numeric_cell_block::delete_block(p);
        break;
        case element_type_string:
            string_cell_block::delete_block(p);
        break;
        case element_type_index:
            index_cell_block::delete_block(p);
        break;
        case element_type_boolean:
            boolean_cell_block::delete_block(p);
        break;
        default:
            throw general_error("delete_block: failed to delete a block of unknown type.");
    }
}

void cell_block_func_base::resize_block(base_element_block& block, size_t new_size)
{
    switch (get_block_type(block))
    {
        case element_type_numeric:
            numeric_cell_block::resize_block(block, new_size);
        break;
        case element_type_string:
            string_cell_block::resize_block(block, new_size);
        break;
        case element_type_index:
            index_cell_block::resize_block(block, new_size);
        break;
        case element_type_boolean:
            boolean_cell_block::resize_block(block, new_size);
        break;
        default:
            throw general_error("resize_block: failed to resize a block of unknown type.");
    }
}

void cell_block_func_base::print_block(const base_element_block& block)
{
    switch (get_block_type(block))
    {
        case element_type_numeric:
            numeric_cell_block::print_block(block);
        break;
        case element_type_string:
            string_cell_block::print_block(block);
        break;
        case element_type_index:
            index_cell_block::print_block(block);
        break;
        case element_type_boolean:
            boolean_cell_block::print_block(block);
        break;
        default:
            throw general_error("print_block: failed to print a block of unknown type.");
    }
}

void cell_block_func_base::erase(base_element_block& block, size_t pos)
{
    switch (get_block_type(block))
    {
        case element_type_numeric:
            numeric_cell_block::erase_block(block, pos);
        break;
        case element_type_string:
            string_cell_block::erase_block(block, pos);
        break;
        case element_type_index:
            index_cell_block::erase_block(block, pos);
        break;
        case element_type_boolean:
            boolean_cell_block::erase_block(block, pos);
        break;
        default:
            throw general_error("erase: failed to erase an element from a block of unknown type.");
    }
}

void cell_block_func_base::erase(base_element_block& block, size_t pos, size_t size)
{
    switch (get_block_type(block))
    {
        case element_type_numeric:
            numeric_cell_block::erase_block(block, pos, size);
        break;
        case element_type_string:
            string_cell_block::erase_block(block, pos, size);
        break;
        case element_type_index:
            index_cell_block::erase_block(block, pos, size);
        break;
        case element_type_boolean:
            boolean_cell_block::erase_block(block, pos, size);
        break;
        default:
            throw general_error("erase: failed to erase elements from a block of unknown type.");
    }
}

template<typename _Iter>
void set_values(
    base_element_block& block, size_t pos, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block::set_values(block, pos, it_begin, it_end);
}

template<typename _Iter>
void set_values(
    base_element_block& block, size_t pos, std::string, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block::set_values(block, pos, it_begin, it_end);
}

template<typename _Iter>
void set_values(
    base_element_block& block, size_t pos, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block::set_values(block, pos, it_begin, it_end);
}

template<typename _Iter>
void set_values(
    base_element_block& block, size_t pos, bool, const _Iter& it_begin, const _Iter& it_end)
{
    boolean_cell_block::set_values(block, pos, it_begin, it_end);
}

template<typename _Iter>
void prepend_values(base_element_block& block, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block::prepend_values(block, it_begin, it_end);
}

template<typename _Iter>
void prepend_values(base_element_block& block, const std::string&, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block::prepend_values(block, it_begin, it_end);
}

template<typename _Iter>
void prepend_values(base_element_block& block, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block::prepend_values(block, it_begin, it_end);
}

template<typename _Iter>
void prepend_values(base_element_block& block, bool, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block::prepend_values(block, it_begin, it_end);
}

void cell_block_func_base::append_values_from_block(base_element_block& dest, const base_element_block& src)
{
    switch (get_block_type(dest))
    {
        case element_type_numeric:
            numeric_cell_block::append_values_from_block(dest, src);
        break;
        case element_type_string:
            string_cell_block::append_values_from_block(dest, src);
        break;
        case element_type_index:
            index_cell_block::append_values_from_block(dest, src);
        break;
        case element_type_boolean:
            boolean_cell_block::append_values_from_block(dest, src);
        break;
        default:
            throw general_error("append_values: failed to append values to a block of unknown type.");
    }
}

void cell_block_func_base::append_values_from_block(
    base_element_block& dest, const base_element_block& src, size_t begin_pos, size_t len)
{
    switch (get_block_type(dest))
    {
        case element_type_numeric:
            numeric_cell_block::append_values_from_block(dest, src, begin_pos, len);
        break;
        case element_type_string:
            string_cell_block::append_values_from_block(dest, src, begin_pos, len);
        break;
        case element_type_index:
            index_cell_block::append_values_from_block(dest, src, begin_pos, len);
        break;
        case element_type_boolean:
            boolean_cell_block::append_values_from_block(dest, src, begin_pos, len);
        break;
        default:
            throw general_error("append_values: failed to append values to a block of unknown type.");
    }
}

template<typename _Iter>
void append_values(base_element_block& block, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block::append_values(block, it_begin, it_end);
}

template<typename _Iter>
void append_values(base_element_block& block, std::string, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block::append_values(block, it_begin, it_end);
}

template<typename _Iter>
void append_values(base_element_block& block, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block::append_values(block, it_begin, it_end);
}

template<typename _Iter>
void append_values(base_element_block& block, bool, const _Iter& it_begin, const _Iter& it_end)
{
    boolean_cell_block::append_values(block, it_begin, it_end);
}

void cell_block_func_base::assign_values_from_block(
    base_element_block& dest, const base_element_block& src, size_t begin_pos, size_t len)
{
    switch (get_block_type(dest))
    {
        case element_type_numeric:
            numeric_cell_block::assign_values_from_block(dest, src, begin_pos, len);
        break;
        case element_type_string:
            string_cell_block::assign_values_from_block(dest, src, begin_pos, len);
        break;
        case element_type_index:
            index_cell_block::assign_values_from_block(dest, src, begin_pos, len);
        break;
        case element_type_boolean:
            boolean_cell_block::assign_values_from_block(dest, src, begin_pos, len);
        break;
        default:
            throw general_error("assign_values: failed to assign values to a block of unknown type.");
    }
}

template<typename _Iter>
void assign_values(base_element_block& dest, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block::assign_values(dest, it_begin, it_end);
}

template<typename _Iter>
void assign_values(base_element_block& dest, const std::string&, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block::assign_values(dest, it_begin, it_end);
}

template<typename _Iter>
void assign_values(base_element_block& dest, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block::assign_values(dest, it_begin, it_end);
}

template<typename _Iter>
void assign_values(base_element_block& dest, bool, const _Iter& it_begin, const _Iter& it_end)
{
    boolean_cell_block::assign_values(dest, it_begin, it_end);
}

template<typename _Iter>
void insert_values(
    base_element_block& block, size_t pos, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block::insert_values(block, pos, it_begin, it_end);
}

template<typename _Iter>
void insert_values(
    base_element_block& block, size_t pos, std::string, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block::insert_values(block, pos, it_begin, it_end);
}

template<typename _Iter>
void insert_values(
    base_element_block& block, size_t pos, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block::insert_values(block, pos, it_begin, it_end);
}

template<typename _Iter>
void insert_values(
    base_element_block& block, size_t pos, bool, const _Iter& it_begin, const _Iter& it_end)
{
    boolean_cell_block::insert_values(block, pos, it_begin, it_end);
}

bool cell_block_func_base::equal_block(const base_element_block& left, const base_element_block& right)
{
    element_t block_type = get_block_type(left);
    if (block_type != get_block_type(right))
        return false;

    switch (block_type)
    {
        case element_type_numeric:
            return numeric_cell_block::get(left) == numeric_cell_block::get(right);
        case element_type_string:
            return string_cell_block::get(left) == string_cell_block::get(right);
        case element_type_index:
            return index_cell_block::get(left) == index_cell_block::get(right);
        case element_type_boolean:
            return boolean_cell_block::get(left) == boolean_cell_block::get(right);
        default:
            ;
    }
    return false;
}

void cell_block_func_base::overwrite_values(base_element_block&, size_t, size_t)
{
    // Do nothing for the standard types.
}

/**
 * Default cell block function definitions.  Implementation can use this if
 * it only uses the default block types implemented by the library.
 */
struct cell_block_func : public cell_block_func_base
{
    template<typename T>
    static void set_values(mdds::mtv::base_element_block& block, size_t pos, const T& it_begin, const T& it_end)
    {
        assert(it_begin != it_end);
        mdds::mtv::set_values(block, pos, *it_begin, it_begin, it_end);
    }

    template<typename T>
    static void get_value(const mdds::mtv::base_element_block& block, size_t pos, T& val)
    {
        mdds::mtv::get_value(block, pos, val);
    }

    template<typename T>
    static void append_value(base_element_block& block, const T& val)
    {
        mdds::mtv::append_value(block, val);
    }

    template<typename T>
    static void insert_values(
        base_element_block& block, size_t pos, const T& it_begin, const T& it_end)
    {
        assert(it_begin != it_end);
        mdds::mtv::insert_values(block, pos, *it_begin, it_begin, it_end);
    }

    template<typename T>
    static void append_values(base_element_block& block, const T& it_begin, const T& it_end)
    {
        assert(it_begin != it_end);
        mdds::mtv::append_values(block, *it_begin, it_begin, it_end);
    }

    template<typename T>
    static void assign_values(base_element_block& dest, const T& it_begin, const T& it_end)
    {
        assert(it_begin != it_end);
        mdds::mtv::assign_values(dest, *it_begin, it_begin, it_end);
    }

    template<typename T>
    static void prepend_value(base_element_block& block, const T& val)
    {
        mdds::mtv::prepend_value(block, val);
    }

    template<typename T>
    static void prepend_values(base_element_block& block, const T& it_begin, const T& it_end)
    {
        assert(it_begin != it_end);
        mdds::mtv::prepend_values(block, *it_begin, it_begin, it_end);
    }

    template<typename T>
    static void get_empty_value(T& val)
    {
        mdds::mtv::get_empty_value(val);
    }
};

}}

#endif
