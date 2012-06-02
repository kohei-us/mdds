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

#ifndef __MULTI_TYPE_MATRIX_BLOCK_FUNC_HPP__
#define __MULTI_TYPE_MATRIX_BLOCK_FUNC_HPP__

#include "multi_type_vector_types.hpp"

namespace mdds { namespace __mtm {

const mdds::mtv::element_t element_type_mtx_string = mdds::mtv::element_type_user_start;

template<typename _StringType>
struct trait
{
    typedef _StringType string_type;
    typedef mdds::mtv::default_element_block<element_type_mtx_string, string_type> string_cell_block;

    mdds::mtv::element_t mdds_mtv_get_element_type(const string_type&)
    {
        return element_type_mtx_string;
    }

    void mdds_mtv_set_value(mtv::base_element_block& block, size_t pos, const string_type& val)
    {
        string_cell_block::set_value(block, pos, val);
    }

    void mdds_mtv_get_value(const mtv::base_element_block& block, size_t pos, string_type& val)
    {
        string_cell_block::get_value(block, pos, val);
    }

    template<typename _Iter>
    void mdds_mtv_set_values(
        mtv::base_element_block& block, size_t pos, const string_type&, const _Iter& it_begin, const _Iter& it_end)
    {
        string_cell_block::set_values(block, pos, it_begin, it_end);
    }

    void mdds_mtv_append_value(mtv::base_element_block& block, const string_type& val)
    {
        string_cell_block::append_value(block, val);
    }

    void mdds_mtv_prepend_value(mtv::base_element_block& block, const string_type& val)
    {
        string_cell_block::prepend_value(block, val);
    }

    template<typename _Iter>
    void mdds_mtv_prepend_values(mtv::base_element_block& block, const string_type&, const _Iter& it_begin, const _Iter& it_end)
    {
        string_cell_block::prepend_values(block, it_begin, it_end);
    }

    template<typename _Iter>
    void mdds_mtv_append_values(mtv::base_element_block& block, const string_type&, const _Iter& it_begin, const _Iter& it_end)
    {
        string_cell_block::append_values(block, it_begin, it_end);
    }

    template<typename _Iter>
    void mdds_mtv_assign_values(mtv::base_element_block& dest, const string_type&, const _Iter& it_begin, const _Iter& it_end)
    {
        string_cell_block::assign_values(dest, it_begin, it_end);
    }

    void mdds_mtv_get_empty_value(string_type& val)
    {
        val = string_type();
    }

    template<typename _Iter>
    void mdds_mtv_insert_values(
        mtv::base_element_block& block, size_t pos, string_type, const _Iter& it_begin, const _Iter& it_end)
    {
        string_cell_block::insert_values(block, pos, it_begin, it_end);
    }
};

}}

#endif
