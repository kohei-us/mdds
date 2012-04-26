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

#ifndef __MDDS_GRID_MAP_TRAIT_HPP__
#define __MDDS_GRID_MAP_TRAIT_HPP__

#include "grid_map_types.hpp"

#include <vector>

namespace mdds { namespace gridmap {

typedef default_cell_block<celltype_numeric, double>     numeric_cell_block;
typedef default_cell_block<celltype_string, std::string> string_cell_block;
typedef default_cell_block<celltype_index, size_t>       index_cell_block;
typedef default_cell_block<celltype_boolean, bool>       boolean_cell_block;

cell_t get_cell_type(double)
{
    return celltype_numeric;
}

cell_t get_cell_type(const std::string&)
{
    return celltype_string;
}

cell_t get_cell_type(size_t)
{
    return celltype_index;
}

cell_t get_cell_type(bool)
{
    return celltype_boolean;
}

void set_value(base_cell_block& block, size_t pos, double val)
{
    numeric_cell_block& blk = numeric_cell_block::get(block);
    blk[pos] = val;
}

void set_value(base_cell_block& block, size_t pos, const std::string& val)
{
    string_cell_block& blk = string_cell_block::get(block);
    blk[pos] = val;
}

void set_value(base_cell_block& block, size_t pos, size_t val)
{
    index_cell_block& blk = index_cell_block::get(block);
    blk[pos] = val;
}

void set_value(base_cell_block& block, size_t pos, bool val)
{
    boolean_cell_block& blk = boolean_cell_block::get(block);
    blk[pos] = val;
}

void get_value(const base_cell_block& block, size_t pos, double& val)
{
    const numeric_cell_block& blk = numeric_cell_block::get(block);
    val = blk[pos];
}

void get_value(const base_cell_block& block, size_t pos, std::string& val)
{
    const string_cell_block& blk = string_cell_block::get(block);
    val = blk[pos];
}

void get_value(const base_cell_block& block, size_t pos, size_t& val)
{
    const index_cell_block& blk = index_cell_block::get(block);
    val = blk[pos];
}

void get_value(const base_cell_block& block, size_t pos, bool& val)
{
    const boolean_cell_block& blk = boolean_cell_block::get(block);
    val = blk[pos];
}

void append_value(base_cell_block& block, double val)
{
    numeric_cell_block& blk = numeric_cell_block::get(block);
    blk.push_back(val);
}

void append_value(base_cell_block& block, const std::string& val)
{
    string_cell_block& blk = string_cell_block::get(block);
    blk.push_back(val);
}

void append_value(base_cell_block& block, size_t val)
{
    index_cell_block& blk = index_cell_block::get(block);
    blk.push_back(val);
}

void append_value(base_cell_block& block, bool val)
{
    boolean_cell_block& blk = boolean_cell_block::get(block);
    blk.push_back(val);
}

void prepend_value(base_cell_block& block, double val)
{
    numeric_cell_block& blk = numeric_cell_block::get(block);
    blk.insert(blk.begin(), val);
}

void prepend_value(base_cell_block& block, const std::string& val)
{
    string_cell_block& blk = string_cell_block::get(block);
    blk.insert(blk.begin(), val);
}

void prepend_value(base_cell_block& block, size_t val)
{
    index_cell_block& blk = index_cell_block::get(block);
    blk.insert(blk.begin(), val);
}

void prepend_value(base_cell_block& block, bool val)
{
    boolean_cell_block& blk = boolean_cell_block::get(block);
    blk.insert(blk.begin(), val);
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
    static base_cell_block* create_new_block(cell_t type, size_t init_size);

    static base_cell_block* clone_block(const base_cell_block& block);

    static void delete_block(base_cell_block* p);

    static void resize_block(base_cell_block& block, size_t new_size);

    static void print_block(const base_cell_block& block);

    static void erase(base_cell_block& block, size_t pos);

    static void erase(base_cell_block& block, size_t pos, size_t size);

    static void append_values_from_block(base_cell_block& dest, const base_cell_block& src);

    static void append_values_from_block(
        base_cell_block& dest, const base_cell_block& src, size_t begin_pos, size_t len);

    static void assign_values_from_block(
        base_cell_block& dest, const base_cell_block& src, size_t begin_pos, size_t len);

    static bool equal_block(const base_cell_block& left, const base_cell_block& right);

    /**
     * This method gets called when cell values are being overwritten by new
     * values.  This provides the client code an opportunity to delete
     * overwritten instances in case the block stores pointers to managed
     * objects.  For blocks that don't need to manage their stored objects (or
     * store primitive values), this method can be left empty.
     */
    static void overwrite_cells(base_cell_block& block, size_t pos, size_t len);
};

base_cell_block* cell_block_func_base::create_new_block(cell_t type, size_t init_size)
{
    switch (type)
    {
        case celltype_numeric:
            return numeric_cell_block::create_block(init_size);
        case celltype_string:
            return string_cell_block::create_block(init_size);
        case celltype_index:
            return index_cell_block::create_block(init_size);
        case celltype_boolean:
            return boolean_cell_block::create_block(init_size);
        default:
            throw general_error("create_new_block: failed to create a new block of unknown type.");
    }
    return NULL;
}

base_cell_block* cell_block_func_base::clone_block(const base_cell_block& block)
{
    switch (get_block_type(block))
    {
        case celltype_numeric:
            return numeric_cell_block::clone_block(block);
        case celltype_string:
            return string_cell_block::clone_block(block);
        case celltype_index:
            return index_cell_block::clone_block(block);
        case celltype_boolean:
            return boolean_cell_block::clone_block(block);
        default:
            throw general_error("clone_block: failed to clone a block of unknown type.");
    }
    return NULL;
}

void cell_block_func_base::delete_block(base_cell_block* p)
{
    if (!p)
        return;

    switch (get_block_type(*p))
    {
        case celltype_numeric:
            numeric_cell_block::delete_block(p);
        break;
        case celltype_string:
            string_cell_block::delete_block(p);
        break;
        case celltype_index:
            index_cell_block::delete_block(p);
        break;
        case celltype_boolean:
            boolean_cell_block::delete_block(p);
        break;
        default:
            throw general_error("delete_block: failed to delete a block of unknown type.");
    }
}

void cell_block_func_base::resize_block(base_cell_block& block, size_t new_size)
{
    switch (get_block_type(block))
    {
        case celltype_numeric:
            numeric_cell_block::resize_block(block, new_size);
        break;
        case celltype_string:
            string_cell_block::resize_block(block, new_size);
        break;
        case celltype_index:
            index_cell_block::resize_block(block, new_size);
        break;
        case celltype_boolean:
            boolean_cell_block::resize_block(block, new_size);
        break;
        default:
            throw general_error("resize_block: failed to resize a block of unknown type.");
    }
}

void cell_block_func_base::print_block(const base_cell_block& block)
{
    switch (get_block_type(block))
    {
        case celltype_numeric:
            numeric_cell_block::print_block(block);
        break;
        case celltype_string:
            string_cell_block::print_block(block);
        break;
        case celltype_index:
            index_cell_block::print_block(block);
        break;
        case celltype_boolean:
            boolean_cell_block::print_block(block);
        break;
        default:
            throw general_error("print_block: failed to print a block of unknown type.");
    }
}

void cell_block_func_base::erase(base_cell_block& block, size_t pos)
{
    switch (get_block_type(block))
    {
        case celltype_numeric:
            numeric_cell_block::erase_block(block, pos);
        break;
        case celltype_string:
            string_cell_block::erase_block(block, pos);
        break;
        case celltype_index:
            index_cell_block::erase_block(block, pos);
        break;
        case celltype_boolean:
            boolean_cell_block::erase_block(block, pos);
        break;
        default:
            throw general_error("erase: failed to erase an element from a block of unknown type.");
    }
}

void cell_block_func_base::erase(base_cell_block& block, size_t pos, size_t size)
{
    switch (get_block_type(block))
    {
        case celltype_numeric:
            numeric_cell_block::erase_block(block, pos, size);
        break;
        case celltype_string:
            string_cell_block::erase_block(block, pos, size);
        break;
        case celltype_index:
            index_cell_block::erase_block(block, pos, size);
        break;
        case celltype_boolean:
            boolean_cell_block::erase_block(block, pos, size);
        break;
        default:
            throw general_error("erase: failed to erase elements from a block of unknown type.");
    }
}

template<typename _Iter>
void set_values(
    base_cell_block& block, size_t pos, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block& d = numeric_cell_block::get(block);
    for (_Iter it = it_begin; it != it_end; ++it, ++pos)
        d[pos] = *it;
}

template<typename _Iter>
void set_values(
    base_cell_block& block, size_t pos, std::string, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block& d = string_cell_block::get(block);
    for (_Iter it = it_begin; it != it_end; ++it, ++pos)
        d[pos] = *it;
}

template<typename _Iter>
void set_values(
    base_cell_block& block, size_t pos, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block& d = index_cell_block::get(block);
    for (_Iter it = it_begin; it != it_end; ++it, ++pos)
        d[pos] = *it;
}

template<typename _Iter>
void set_values(
    base_cell_block& block, size_t pos, bool, const _Iter& it_begin, const _Iter& it_end)
{
    boolean_cell_block& d = boolean_cell_block::get(block);
    for (_Iter it = it_begin; it != it_end; ++it, ++pos)
        d[pos] = *it;
}

template<typename _Iter>
void prepend_values(base_cell_block& block, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block& d = numeric_cell_block::get(block);
    d.insert(d.begin(), it_begin, it_end);
}

template<typename _Iter>
void prepend_values(base_cell_block& block, const std::string&, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block& d = string_cell_block::get(block);
    d.insert(d.begin(), it_begin, it_end);
}

template<typename _Iter>
void prepend_values(base_cell_block& block, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block& d = index_cell_block::get(block);
    d.insert(d.begin(), it_begin, it_end);
}

template<typename _Iter>
void prepend_values(base_cell_block& block, bool, const _Iter& it_begin, const _Iter& it_end)
{
    boolean_cell_block& d = boolean_cell_block::get(block);
    d.insert(d.begin(), it_begin, it_end);
}

void cell_block_func_base::append_values_from_block(base_cell_block& dest, const base_cell_block& src)
{
    switch (get_block_type(dest))
    {
        case celltype_numeric:
        {
            numeric_cell_block& d = numeric_cell_block::get(dest);
            const numeric_cell_block& s = numeric_cell_block::get(src);
            d.insert(d.end(), s.begin(), s.end());
        }
        break;
        case celltype_string:
        {
            string_cell_block& d = string_cell_block::get(dest);
            const string_cell_block& s = string_cell_block::get(src);
            d.insert(d.end(), s.begin(), s.end());
        }
        break;
        case celltype_index:
        {
            index_cell_block& d = index_cell_block::get(dest);
            const index_cell_block& s = index_cell_block::get(src);
            d.insert(d.end(), s.begin(), s.end());
        }
        break;
        case celltype_boolean:
        {
            boolean_cell_block& d = boolean_cell_block::get(dest);
            const boolean_cell_block& s = boolean_cell_block::get(src);
            d.insert(d.end(), s.begin(), s.end());
        }
        break;
        default:
            throw general_error("append_values: failed to append values to a block of unknown type.");
    }
}

void cell_block_func_base::append_values_from_block(
    base_cell_block& dest, const base_cell_block& src, size_t begin_pos, size_t len)
{
    switch (get_block_type(dest))
    {
        case celltype_numeric:
        {
            numeric_cell_block& d = numeric_cell_block::get(dest);
            const numeric_cell_block& s = numeric_cell_block::get(src);
            numeric_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            numeric_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.reserve(d.size() + len);
            std::copy(it, it_end, std::back_inserter(d));
        }
        break;
        case celltype_string:
        {
            string_cell_block& d = string_cell_block::get(dest);
            const string_cell_block& s = string_cell_block::get(src);
            string_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            string_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.reserve(d.size() + len);
            std::copy(it, it_end, std::back_inserter(d));
        }
        break;
        case celltype_index:
        {
            index_cell_block& d = index_cell_block::get(dest);
            const index_cell_block& s = index_cell_block::get(src);
            index_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            index_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.reserve(d.size() + len);
            std::copy(it, it_end, std::back_inserter(d));
        }
        break;
        case celltype_boolean:
        {
            boolean_cell_block& d = boolean_cell_block::get(dest);
            const boolean_cell_block& s = boolean_cell_block::get(src);
            boolean_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            boolean_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.reserve(d.size() + len);
            std::copy(it, it_end, std::back_inserter(d));
        }
        break;
        default:
            throw general_error("append_values: failed to append values to a block of unknown type.");
    }
}

template<typename _Iter>
void append_values(base_cell_block& block, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block& d = numeric_cell_block::get(block);
    numeric_cell_block::iterator it = d.end();
    d.insert(it, it_begin, it_end);
}

template<typename _Iter>
void append_values(base_cell_block& block, std::string, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block& d = string_cell_block::get(block);
    string_cell_block::iterator it = d.end();
    d.insert(it, it_begin, it_end);
}

template<typename _Iter>
void append_values(base_cell_block& block, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block& d = index_cell_block::get(block);
    index_cell_block::iterator it = d.end();
    d.insert(it, it_begin, it_end);
}

template<typename _Iter>
void append_values(base_cell_block& block, bool, const _Iter& it_begin, const _Iter& it_end)
{
    boolean_cell_block& d = boolean_cell_block::get(block);
    boolean_cell_block::iterator it = d.end();
    d.insert(it, it_begin, it_end);
}

void cell_block_func_base::assign_values_from_block(
    base_cell_block& dest, const base_cell_block& src, size_t begin_pos, size_t len)
{
    switch (get_block_type(dest))
    {
        case celltype_numeric:
        {
            numeric_cell_block& d = numeric_cell_block::get(dest);
            const numeric_cell_block& s = numeric_cell_block::get(src);
            numeric_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            numeric_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.assign(it, it_end);
        }
        break;
        case celltype_string:
        {
            string_cell_block& d = string_cell_block::get(dest);
            const string_cell_block& s = string_cell_block::get(src);
            string_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            string_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.assign(it, it_end);
        }
        break;
        case celltype_index:
        {
            index_cell_block& d = index_cell_block::get(dest);
            const index_cell_block& s = index_cell_block::get(src);
            index_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            index_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.assign(it, it_end);
        }
        break;
        case celltype_boolean:
        {
            boolean_cell_block& d = boolean_cell_block::get(dest);
            const boolean_cell_block& s = boolean_cell_block::get(src);
            boolean_cell_block::const_iterator it = s.begin();
            std::advance(it, begin_pos);
            boolean_cell_block::const_iterator it_end = it;
            std::advance(it_end, len);
            d.assign(it, it_end);
        }
        break;
        default:
            throw general_error("assign_values: failed to assign values to a block of unknown type.");
    }
}

template<typename _Iter>
void assign_values(base_cell_block& dest, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block& d = numeric_cell_block::get(dest);
    d.assign(it_begin, it_end);
}

template<typename _Iter>
void assign_values(base_cell_block& dest, const std::string&, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block& d = string_cell_block::get(dest);
    d.assign(it_begin, it_end);
}

template<typename _Iter>
void assign_values(base_cell_block& dest, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block& d = index_cell_block::get(dest);
    d.assign(it_begin, it_end);
}

template<typename _Iter>
void assign_values(base_cell_block& dest, bool, const _Iter& it_begin, const _Iter& it_end)
{
    boolean_cell_block& d = boolean_cell_block::get(dest);
    d.assign(it_begin, it_end);
}

template<typename _Iter>
void insert_values(
    base_cell_block& block, size_t pos, double, const _Iter& it_begin, const _Iter& it_end)
{
    numeric_cell_block& blk = numeric_cell_block::get(block);
    blk.insert(blk.begin()+pos, it_begin, it_end);
}

template<typename _Iter>
void insert_values(
    base_cell_block& block, size_t pos, std::string, const _Iter& it_begin, const _Iter& it_end)
{
    string_cell_block& blk = string_cell_block::get(block);
    blk.insert(blk.begin()+pos, it_begin, it_end);
}

template<typename _Iter>
void insert_values(
    base_cell_block& block, size_t pos, size_t, const _Iter& it_begin, const _Iter& it_end)
{
    index_cell_block& blk = index_cell_block::get(block);
    blk.insert(blk.begin()+pos, it_begin, it_end);
}

template<typename _Iter>
void insert_values(
    base_cell_block& block, size_t pos, bool, const _Iter& it_begin, const _Iter& it_end)
{
    boolean_cell_block& blk = boolean_cell_block::get(block);
    blk.insert(blk.begin()+pos, it_begin, it_end);
}

bool cell_block_func_base::equal_block(const base_cell_block& left, const base_cell_block& right)
{
    cell_t block_type = get_block_type(left);
    if (block_type != get_block_type(right))
        return false;

    switch (block_type)
    {
        case celltype_numeric:
            return numeric_cell_block::get(left) == numeric_cell_block::get(right);
        case celltype_string:
            return string_cell_block::get(left) == string_cell_block::get(right);
        case celltype_index:
            return index_cell_block::get(left) == index_cell_block::get(right);
        case celltype_boolean:
            return boolean_cell_block::get(left) == boolean_cell_block::get(right);
        default:
            ;
    }
    return false;
}

void cell_block_func_base::overwrite_cells(base_cell_block&, size_t, size_t)
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
    static mdds::gridmap::cell_t get_cell_type(const T& cell)
    {
        return mdds::gridmap::get_cell_type(cell);
    }

    template<typename T>
    static void set_value(mdds::gridmap::base_cell_block& block, size_t pos, const T& val)
    {
        mdds::gridmap::set_value(block, pos, val);
    }

    template<typename T>
    static void set_values(mdds::gridmap::base_cell_block& block, size_t pos, const T& it_begin, const T& it_end)
    {
        assert(it_begin != it_end);
        mdds::gridmap::set_values(block, pos, *it_begin, it_begin, it_end);
    }

    template<typename T>
    static void get_value(const mdds::gridmap::base_cell_block& block, size_t pos, T& val)
    {
        mdds::gridmap::get_value(block, pos, val);
    }

    template<typename T>
    static void append_value(base_cell_block& block, const T& val)
    {
        mdds::gridmap::append_value(block, val);
    }

    template<typename T>
    static void insert_values(
        base_cell_block& block, size_t pos, const T& it_begin, const T& it_end)
    {
        assert(it_begin != it_end);
        mdds::gridmap::insert_values(block, pos, *it_begin, it_begin, it_end);
    }

    template<typename T>
    static void append_values(base_cell_block& block, const T& it_begin, const T& it_end)
    {
        assert(it_begin != it_end);
        mdds::gridmap::append_values(block, *it_begin, it_begin, it_end);
    }

    template<typename T>
    static void assign_values(base_cell_block& dest, const T& it_begin, const T& it_end)
    {
        assert(it_begin != it_end);
        mdds::gridmap::assign_values(dest, *it_begin, it_begin, it_end);
    }

    template<typename T>
    static void prepend_value(base_cell_block& block, const T& val)
    {
        mdds::gridmap::prepend_value(block, val);
    }

    template<typename T>
    static void prepend_values(base_cell_block& block, const T& it_begin, const T& it_end)
    {
        assert(it_begin != it_end);
        mdds::gridmap::prepend_values(block, *it_begin, it_begin, it_end);
    }

    template<typename T>
    static void get_empty_value(T& val)
    {
        mdds::gridmap::get_empty_value(val);
    }
};

}}

#endif
