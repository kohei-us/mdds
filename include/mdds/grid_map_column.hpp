/*************************************************************************
 *
 * Copyright (c) 2011 Kohei Yoshida
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

#ifndef __MDDS_GRID_MAP_COLUMN_HPP__
#define __MDDS_GRID_MAP_COLUMN_HPP__

#include <vector>
#include <algorithm>

namespace mdds { namespace __gridmap {

/**
 * Each column consists of a series of blocks, and each block stores a
 * series of non-empty cells of identical type.
 */
template<typename _Trait>
class column
{
    typedef typename _Trait::cell_type cell_type;
    typedef typename _Trait::cell_category_type cell_category_type;
    typedef typename _Trait::row_key_type row_key_type;

    struct block
    {
        cell_category_type m_type;
        std::vector<cell_type*> m_cells;

        block(cell_category_type _type, size_t _init_size = 0);
        ~block();
    };
public:

    column();
    ~column();

private:
    std::vector<block*> m_blocks;
};

template<typename _Trait>
column<_Trait>::block::block(cell_category_type _type, size_t _init_size) :
    m_type(_type), m_cells(_init_size, NULL) {}

template<typename _Trait>
column<_Trait>::block::~block()
{
    std::for_each(m_cells.begin(), m_cells.end(), default_deleter<cell_type>());
}

template<typename _Trait>
column<_Trait>::column() {}

template<typename _Trait>
column<_Trait>::~column()
{
    std::for_each(m_blocks.begin(), m_blocks.end(), default_deleter<block>());
}

}}

#endif
