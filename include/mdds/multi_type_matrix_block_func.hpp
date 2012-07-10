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
#include "multi_type_vector_trait.hpp"

namespace mdds { namespace __mtm {

const mdds::mtv::element_t element_type_custom_string = mdds::mtv::element_type_user_start;

template<typename _StrType>
struct trait
{
    typedef _StrType string_type;
    typedef mdds::mtv::default_element_block<element_type_custom_string, string_type> string_elem_block;

    static mdds::mtv::element_t mdds_mtv_get_element_type(const string_type&)
    {
        return element_type_custom_string;
    }

    static void mdds_mtv_set_value(mtv::base_element_block& block, size_t pos, const string_type& val)
    {
        string_elem_block::set_value(block, pos, val);
    }

    static void mdds_mtv_get_value(const mtv::base_element_block& block, size_t pos, string_type& val)
    {
        string_elem_block::get_value(block, pos, val);
    }

    template<typename _Iter>
    static void mdds_mtv_set_values(
        mtv::base_element_block& block, size_t pos, const string_type&, const _Iter& it_begin, const _Iter& it_end)
    {
        string_elem_block::set_values(block, pos, it_begin, it_end);
    }

    static void mdds_mtv_append_value(mtv::base_element_block& block, const string_type& val)
    {
        string_elem_block::append_value(block, val);
    }

    static void mdds_mtv_prepend_value(mtv::base_element_block& block, const string_type& val)
    {
        string_elem_block::prepend_value(block, val);
    }

    template<typename _Iter>
    static void mdds_mtv_prepend_values(mtv::base_element_block& block, const string_type&, const _Iter& it_begin, const _Iter& it_end)
    {
        string_elem_block::prepend_values(block, it_begin, it_end);
    }

    template<typename _Iter>
    static void mdds_mtv_append_values(mtv::base_element_block& block, const string_type&, const _Iter& it_begin, const _Iter& it_end)
    {
        string_elem_block::append_values(block, it_begin, it_end);
    }

    template<typename _Iter>
    static void mdds_mtv_assign_values(mtv::base_element_block& dest, const string_type&, const _Iter& it_begin, const _Iter& it_end)
    {
        string_elem_block::assign_values(dest, it_begin, it_end);
    }

    static void mdds_mtv_get_empty_value(string_type& val)
    {
        val = string_type();
    }

    template<typename _Iter>
    static void mdds_mtv_insert_values(
        mtv::base_element_block& block, size_t pos, string_type, const _Iter& it_begin, const _Iter& it_end)
    {
        string_elem_block::insert_values(block, pos, it_begin, it_end);
    }

    struct elem_block_func
    {
        static mdds::mtv::base_element_block* create_new_block(
            mdds::mtv::element_t type, size_t init_size)
        {
            switch (type)
            {
                case element_type_custom_string:
                    return string_elem_block::create_block(init_size);
                default:
                    return mdds::mtv::cell_block_func_base::create_new_block(type, init_size);
            }
        }

        static mdds::mtv::base_element_block* clone_block(const mdds::mtv::base_element_block& block)
        {
            switch (mtv::get_block_type(block))
            {
                case element_type_custom_string:
                    return string_elem_block::clone_block(block);
                default:
                    return mdds::mtv::cell_block_func_base::clone_block(block);
            }
        }

        static void delete_block(mdds::mtv::base_element_block* p)
        {
            if (!p)
                return;

            switch (mtv::get_block_type(*p))
            {
                case element_type_custom_string:
                    string_elem_block::delete_block(p);
                break;
                default:
                    mdds::mtv::cell_block_func_base::delete_block(p);
            }
        }

        static void resize_block(mdds::mtv::base_element_block& block, size_t new_size)
        {
            switch (mtv::get_block_type(block))
            {
                case element_type_custom_string:
                    string_elem_block::resize_block(block, new_size);
                break;
                default:
                    mdds::mtv::cell_block_func_base::resize_block(block, new_size);
            }
        }

        static void print_block(const mdds::mtv::base_element_block& block)
        {
            switch (mtv::get_block_type(block))
            {
                case element_type_custom_string:
                    string_elem_block::print_block(block);
                break;
                default:
                    mdds::mtv::cell_block_func_base::print_block(block);
            }
        }

        static void erase(mdds::mtv::base_element_block& block, size_t pos)
        {
            switch (mtv::get_block_type(block))
            {
                case element_type_custom_string:
                    string_elem_block::erase_block(block, pos);
                break;
                default:
                    mdds::mtv::cell_block_func_base::erase(block, pos);
            }
        }

        static void erase(mdds::mtv::base_element_block& block, size_t pos, size_t size)
        {
            switch (mtv::get_block_type(block))
            {
                case element_type_custom_string:
                    string_elem_block::erase_block(block, pos, size);
                break;
                default:
                    mdds::mtv::cell_block_func_base::erase(block, pos, size);
            }
        }

        static void append_values_from_block(
            mdds::mtv::base_element_block& dest, const mdds::mtv::base_element_block& src)
        {
            switch (mtv::get_block_type(dest))
            {
                case element_type_custom_string:
                    string_elem_block::append_values_from_block(dest, src);
                break;
                default:
                    mdds::mtv::cell_block_func_base::append_values_from_block(dest, src);
            }
        }

        static void append_values_from_block(
            mdds::mtv::base_element_block& dest, const mdds::mtv::base_element_block& src,
            size_t begin_pos, size_t len)
        {
            switch (mtv::get_block_type(dest))
            {
                case element_type_custom_string:
                    string_elem_block::append_values_from_block(dest, src, begin_pos, len);
                break;
                default:
                    mdds::mtv::cell_block_func_base::append_values_from_block(dest, src, begin_pos, len);
            }
        }

        static void assign_values_from_block(
            mdds::mtv::base_element_block& dest, const mdds::mtv::base_element_block& src,
            size_t begin_pos, size_t len)
        {
            switch (mtv::get_block_type(dest))
            {
                case element_type_custom_string:
                    string_elem_block::assign_values_from_block(dest, src, begin_pos, len);
                break;
                default:
                    mdds::mtv::cell_block_func_base::assign_values_from_block(dest, src, begin_pos, len);
            }
        }

        static bool equal_block(
            const mdds::mtv::base_element_block& left, const mdds::mtv::base_element_block& right)
        {
            if (mtv::get_block_type(left) == element_type_custom_string)
            {
                if (mtv::get_block_type(right) != element_type_custom_string)
                    return false;

                return string_elem_block::get(left) == string_elem_block::get(right);
            }
            else if (mtv::get_block_type(right) == element_type_custom_string)
                return false;

            return mdds::mtv::cell_block_func_base::equal_block(left, right);
        }

        static void overwrite_values(mdds::mtv::base_element_block& block, size_t pos, size_t len)
        {
            switch (mtv::get_block_type(block))
            {
                case element_type_custom_string:
                    // Do nothing.  The client code manages the life cycle of these cells.
                break;
                default:
                    mdds::mtv::cell_block_func_base::overwrite_values(block, pos, len);
            }
        }
    };
};

template<>
struct trait<std::string>
{
    typedef std::string string_type;
    typedef mdds::mtv::string_cell_block string_elem_block;
    typedef mdds::mtv::cell_block_func elem_block_func;
};

}}

#endif
