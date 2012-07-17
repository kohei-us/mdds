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

namespace mdds {

inline mtv::element_t mdds_mtv_get_element_type(double)
{
    return mtv::element_type_numeric;
}

inline mtv::element_t mdds_mtv_get_element_type(const std::string&)
{
    return mtv::element_type_string;
}

inline mtv::element_t mdds_mtv_get_element_type(unsigned long)
{
    return mtv::element_type_ulong;
}

inline mtv::element_t mdds_mtv_get_element_type(bool)
{
    return mtv::element_type_boolean;
}

inline void mdds_mtv_get_empty_value(double& val)
{
    val = 0.0;
}

inline void mdds_mtv_get_empty_value(std::string& val)
{
    val = std::string();
}

inline void mdds_mtv_get_empty_value(unsigned long& val)
{
    val = 0;
}

inline void mdds_mtv_get_empty_value(bool& val)
{
    val = false;
}

inline void mdds_mtv_set_value(mtv::base_element_block& block, size_t pos, double val)
{
    mtv::numeric_element_block::set_value(block, pos, val);
}

inline void mdds_mtv_set_value(mtv::base_element_block& block, size_t pos, const std::string& val)
{
    mtv::string_element_block::set_value(block, pos, val);
}

inline void mdds_mtv_set_value(mtv::base_element_block& block, size_t pos, unsigned long val)
{
    mtv::index_element_block::set_value(block, pos, val);
}

inline void mdds_mtv_set_value(mtv::base_element_block& block, size_t pos, bool val)
{
    mtv::boolean_element_block::set_value(block, pos, val);
}

inline void mdds_mtv_get_value(const mtv::base_element_block& block, size_t pos, double& val)
{
    mtv::numeric_element_block::get_value(block, pos, val);
}

inline void mdds_mtv_get_value(const mtv::base_element_block& block, size_t pos, std::string& val)
{
    mtv::string_element_block::get_value(block, pos, val);
}

inline void mdds_mtv_get_value(const mtv::base_element_block& block, size_t pos, unsigned long& val)
{
    mtv::index_element_block::get_value(block, pos, val);
}

inline void mdds_mtv_get_value(const mtv::base_element_block& block, size_t pos, bool& val)
{
    mtv::boolean_element_block::get_value(block, pos, val);
}

template<typename _Iter>
void mdds_mtv_set_values(
    mtv::base_element_block& block, size_t pos, double, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::numeric_element_block::set_values(block, pos, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_set_values(
    mtv::base_element_block& block, size_t pos, std::string, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::string_element_block::set_values(block, pos, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_set_values(
    mtv::base_element_block& block, size_t pos, unsigned long, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::index_element_block::set_values(block, pos, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_set_values(
    mtv::base_element_block& block, size_t pos, bool, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::boolean_element_block::set_values(block, pos, it_begin, it_end);
}

inline void mdds_mtv_append_value(mtv::base_element_block& block, double val)
{
    mtv::numeric_element_block::append_value(block, val);
}

inline void mdds_mtv_append_value(mtv::base_element_block& block, const std::string& val)
{
    mtv::string_element_block::append_value(block, val);
}

inline void mdds_mtv_append_value(mtv::base_element_block& block, unsigned long val)
{
    mtv::index_element_block::append_value(block, val);
}

inline void mdds_mtv_append_value(mtv::base_element_block& block, bool val)
{
    mtv::boolean_element_block::append_value(block, val);
}

inline void mdds_mtv_prepend_value(mtv::base_element_block& block, double val)
{
    mtv::numeric_element_block::prepend_value(block, val);
}

inline void mdds_mtv_prepend_value(mtv::base_element_block& block, const std::string& val)
{
    mtv::string_element_block::prepend_value(block, val);
}

inline void mdds_mtv_prepend_value(mtv::base_element_block& block, unsigned long val)
{
    mtv::index_element_block::prepend_value(block, val);
}

inline void mdds_mtv_prepend_value(mtv::base_element_block& block, bool val)
{
    mtv::boolean_element_block::prepend_value(block, val);
}

template<typename _Iter>
void mdds_mtv_prepend_values(mtv::base_element_block& block, double, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::numeric_element_block::prepend_values(block, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_prepend_values(mtv::base_element_block& block, const std::string&, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::string_element_block::prepend_values(block, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_prepend_values(mtv::base_element_block& block, unsigned long, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::index_element_block::prepend_values(block, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_prepend_values(mtv::base_element_block& block, bool, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::index_element_block::prepend_values(block, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_append_values(mtv::base_element_block& block, double, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::numeric_element_block::append_values(block, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_append_values(mtv::base_element_block& block, std::string, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::string_element_block::append_values(block, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_append_values(mtv::base_element_block& block, unsigned long, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::index_element_block::append_values(block, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_append_values(mtv::base_element_block& block, bool, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::boolean_element_block::append_values(block, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_assign_values(mtv::base_element_block& dest, double, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::numeric_element_block::assign_values(dest, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_assign_values(mtv::base_element_block& dest, const std::string&, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::string_element_block::assign_values(dest, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_assign_values(mtv::base_element_block& dest, unsigned long, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::index_element_block::assign_values(dest, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_assign_values(mtv::base_element_block& dest, bool, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::boolean_element_block::assign_values(dest, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_insert_values(
    mtv::base_element_block& block, size_t pos, double, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::numeric_element_block::insert_values(block, pos, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_insert_values(
    mtv::base_element_block& block, size_t pos, std::string, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::string_element_block::insert_values(block, pos, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_insert_values(
    mtv::base_element_block& block, size_t pos, unsigned long, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::index_element_block::insert_values(block, pos, it_begin, it_end);
}

template<typename _Iter>
void mdds_mtv_insert_values(
    mtv::base_element_block& block, size_t pos, bool, const _Iter& it_begin, const _Iter& it_end)
{
    mtv::boolean_element_block::insert_values(block, pos, it_begin, it_end);
}

inline mtv::base_element_block* mdds_mtv_create_new_block(size_t init_size, double val)
{
    return mtv::numeric_element_block::create_block_with_value(init_size, val);
}

inline mtv::base_element_block* mdds_mtv_create_new_block(size_t init_size, const std::string& val)
{
    return mtv::string_element_block::create_block_with_value(init_size, val);
}

inline mtv::base_element_block* mdds_mtv_create_new_block(size_t init_size, unsigned long val)
{
    return mtv::index_element_block::create_block_with_value(init_size, val);
}

inline mtv::base_element_block* mdds_mtv_create_new_block(size_t init_size, bool val)
{
    return mtv::boolean_element_block::create_block_with_value(init_size, val);
}

}

