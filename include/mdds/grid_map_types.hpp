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

#ifndef __MDDS_GRID_MAP_TYPES_HPP__
#define __MDDS_GRID_MAP_TYPES_HPP__

#include <vector>

namespace mdds { namespace gridmap {

typedef int cell_t;

const cell_t celltype_numeric = 0;
const cell_t celltype_string  = 1;
const cell_t celltype_index   = 2;
const cell_t celltype_boolean = 3;

const cell_t celltype_user_start = 50;

struct base_cell_block;
cell_t get_block_type(const base_cell_block&);

struct base_cell_block
{
    friend cell_t get_block_type(const base_cell_block&);
protected:
    cell_t type;
    base_cell_block(cell_t _t) : type(_t) {}
};

template<typename _Self, cell_t _TypeId, typename _Data>
class cell_block : public base_cell_block, public std::vector<_Data>
{
protected:
    cell_block() : base_cell_block(_TypeId), std::vector<_Data>() {}
    cell_block(size_t n) : base_cell_block(_TypeId), std::vector<_Data>(n) {}

public:
    static _Self& get(base_cell_block& block)
    {
        if (get_block_type(block) != _TypeId)
            throw general_error("incorrect block type.");

        return static_cast<_Self&>(block);
    }

    static const _Self& get(const base_cell_block& block)
    {
        if (get_block_type(block) != _TypeId)
            throw general_error("incorrect block type.");

        return static_cast<const _Self&>(block);
    }

    static _Self* create(size_t init_size)
    {
        return new _Self(init_size);
    }

    static _Self* clone(const base_cell_block& blk)
    {
        return new _Self(get(blk));
    }
};

/**
 * Get the numerical block type ID from a given cell block instance.
 *
 * @param blk cell block instance
 *
 * @return numerical value representing the ID of a cell block.
 */
inline cell_t get_block_type(const base_cell_block& blk)
{
    return blk.type;
}

/**
 * Template for default, unmanaged cell block for use in grid_map.
 */
template<cell_t _TypeId, typename _Data>
struct default_cell_block : public cell_block<default_cell_block<_TypeId,_Data>, _TypeId, _Data>
{
    typedef cell_block<default_cell_block, _TypeId, _Data> base_type;

    default_cell_block() : base_type() {}
    default_cell_block(size_t n) : base_type(n) {}
};

/**
 * Template for cell block that stores pointers to objects whose life cycles
 * are managed by the block.
 */
template<cell_t _TypeId, typename _Data>
struct managed_cell_block : public cell_block<managed_cell_block<_TypeId,_Data>, _TypeId, _Data*>
{
    typedef cell_block<managed_cell_block<_TypeId,_Data>, _TypeId, _Data*> base_type;

    using base_type::begin;
    using base_type::end;
    using base_type::reserve;
    using base_type::push_back;

    managed_cell_block() : base_type() {}
    managed_cell_block(size_t n) : base_type(n) {}
    managed_cell_block(const managed_cell_block& r)
    {
        reserve(r.size());
        typename managed_cell_block::const_iterator it = r.begin(), it_end = r.end();
        for (; it != it_end; ++it)
            push_back(new _Data(**it));
    }

    ~managed_cell_block()
    {
        std::for_each(begin(), end(), default_deleter<_Data>());
    }
};

}}

#endif
