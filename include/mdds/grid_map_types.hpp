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

namespace mdds { namespace gridmap {

typedef int cell_t;

const int celltype_numeric = 0;
const int celltype_string  = 1;
const int celltype_index   = 2;
const int celltype_boolean = 3;

const int celltype_user_start = 50;

struct base_cell_block
{
    cell_t type;
    base_cell_block(cell_t _t) : type(_t) {}
};

template<typename _Self, cell_t _TypeId, typename _Data>
struct cell_block : public base_cell_block, public std::vector<_Data>
{
    cell_block() : base_cell_block(_TypeId), std::vector<_Data>(1) {}
    cell_block(size_t n) : base_cell_block(_TypeId), std::vector<_Data>(n) {}

    static _Self& get(base_cell_block& block)
    {
        if (block.type != _TypeId)
            throw general_error("incorrect block type.");

        return static_cast<_Self&>(block);
    }

    static const _Self& get(const base_cell_block& block)
    {
        if (block.type != _TypeId)
            throw general_error("incorrect block type.");

        return static_cast<const _Self&>(block);
    }
};

}}

#endif
