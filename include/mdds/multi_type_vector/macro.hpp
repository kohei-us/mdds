/*************************************************************************
 *
 * Copyright (c) 2012-2021 Kohei Yoshida
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

#ifndef INCLUDED_MDDS_MULTI_TYPE_VECTOR_DIR_MACRO_HPP
#define INCLUDED_MDDS_MULTI_TYPE_VECTOR_DIR_MACRO_HPP

/**
 * @def MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(type, type_id, empty_value,
 *      block_type)
 *
 * @param type element value type.
 * @param type_id constant value used as an ID for the value type. It should
 *                be of type mdds::mtv::element_t.
 * @param empty_value value that should be used as the default "false" value
 *                    for the value type.
 * @param block_type block type that stores the specified value type.
 *
 * Defines required callback functions for multi_type_vector.
 */
#define MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(type, type_id, empty_value, block_type) \
\
    inline mdds::mtv::element_t mdds_mtv_get_element_type(const type&) \
    { \
        return type_id; \
    } \
\
    inline void mdds_mtv_get_empty_value(type& val) \
    { \
        val = empty_value; \
    } \
\
    inline void mdds_mtv_set_value(mdds::mtv::base_element_block& block, size_t pos, const type& val) \
    { \
        block_type::set_value(block, pos, val); \
    } \
\
    inline void mdds_mtv_get_value(const mdds::mtv::base_element_block& block, size_t pos, type& val) \
    { \
        block_type::get_value(block, pos, val); \
    } \
\
    template<typename _Iter> \
    void mdds_mtv_set_values( \
        mdds::mtv::base_element_block& block, size_t pos, const type&, const _Iter& it_begin, const _Iter& it_end) \
    { \
        block_type::set_values(block, pos, it_begin, it_end); \
    } \
\
    inline void mdds_mtv_append_value(mdds::mtv::base_element_block& block, const type& val) \
    { \
        block_type::append_value(block, val); \
    } \
\
    inline void mdds_mtv_prepend_value(mdds::mtv::base_element_block& block, const type& val) \
    { \
        block_type::prepend_value(block, val); \
    } \
\
    template<typename _Iter> \
    void mdds_mtv_prepend_values( \
        mdds::mtv::base_element_block& block, const type&, const _Iter& it_begin, const _Iter& it_end) \
    { \
        block_type::prepend_values(block, it_begin, it_end); \
    } \
\
    template<typename _Iter> \
    void mdds_mtv_append_values( \
        mdds::mtv::base_element_block& block, const type&, const _Iter& it_begin, const _Iter& it_end) \
    { \
        block_type::append_values(block, it_begin, it_end); \
    } \
\
    template<typename _Iter> \
    void mdds_mtv_assign_values( \
        mdds::mtv::base_element_block& dest, const type&, const _Iter& it_begin, const _Iter& it_end) \
    { \
        block_type::assign_values(dest, it_begin, it_end); \
    } \
\
    template<typename _Iter> \
    void mdds_mtv_insert_values( \
        mdds::mtv::base_element_block& block, size_t pos, const type&, const _Iter& it_begin, const _Iter& it_end) \
    { \
        block_type::insert_values(block, pos, it_begin, it_end); \
    } \
\
    inline mdds::mtv::base_element_block* mdds_mtv_create_new_block(size_t init_size, const type& val) \
    { \
        return block_type::create_block_with_value(init_size, val); \
    } \
\
    template<typename _Iter> \
    mdds::mtv::base_element_block* mdds_mtv_create_new_block(const type&, const _Iter& it_begin, const _Iter& it_end) \
    { \
        return block_type::create_block_with_values(it_begin, it_end); \
    }

/**
 * @def MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(type, type_id, empty_value,
 *      block_type)
 *
 * A variant of MDDS_MTV_DEFINE_ELEMENT_CALLBACKS that should be used for a
 * pointer type.
 */
#define MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR(type, type_id, empty_value, block_type) \
\
    inline mdds::mtv::element_t mdds_mtv_get_element_type(const type*) \
    { \
        return type_id; \
    } \
\
    inline void mdds_mtv_get_empty_value(type*& val) \
    { \
        val = empty_value; \
    } \
\
    inline void mdds_mtv_set_value(mdds::mtv::base_element_block& block, size_t pos, type* val) \
    { \
        block_type::set_value(block, pos, val); \
    } \
\
    inline void mdds_mtv_get_value(const mdds::mtv::base_element_block& block, size_t pos, type*& val) \
    { \
        block_type::get_value(block, pos, val); \
    } \
\
    template<typename _Iter> \
    void mdds_mtv_set_values( \
        mdds::mtv::base_element_block& block, size_t pos, const type*, const _Iter& it_begin, const _Iter& it_end) \
    { \
        block_type::set_values(block, pos, it_begin, it_end); \
    } \
\
    inline void mdds_mtv_append_value(mdds::mtv::base_element_block& block, type* val) \
    { \
        block_type::append_value(block, val); \
    } \
\
    inline void mdds_mtv_prepend_value(mdds::mtv::base_element_block& block, type* val) \
    { \
        block_type::prepend_value(block, val); \
    } \
\
    template<typename _Iter> \
    void mdds_mtv_prepend_values( \
        mdds::mtv::base_element_block& block, const type*, const _Iter& it_begin, const _Iter& it_end) \
    { \
        block_type::prepend_values(block, it_begin, it_end); \
    } \
\
    template<typename _Iter> \
    void mdds_mtv_append_values( \
        mdds::mtv::base_element_block& block, const type*, const _Iter& it_begin, const _Iter& it_end) \
    { \
        block_type::append_values(block, it_begin, it_end); \
    } \
\
    template<typename _Iter> \
    void mdds_mtv_assign_values( \
        mdds::mtv::base_element_block& dest, const type*, const _Iter& it_begin, const _Iter& it_end) \
    { \
        block_type::assign_values(dest, it_begin, it_end); \
    } \
\
    template<typename _Iter> \
    void mdds_mtv_insert_values( \
        mdds::mtv::base_element_block& block, size_t pos, const type*, const _Iter& it_begin, const _Iter& it_end) \
    { \
        block_type::insert_values(block, pos, it_begin, it_end); \
    } \
\
    inline mdds::mtv::base_element_block* mdds_mtv_create_new_block(size_t init_size, type* val) \
    { \
        return block_type::create_block_with_value(init_size, val); \
    } \
\
    template<typename _Iter> \
    mdds::mtv::base_element_block* mdds_mtv_create_new_block(const type*, const _Iter& it_begin, const _Iter& it_end) \
    { \
        return block_type::create_block_with_values(it_begin, it_end); \
    }

#endif
