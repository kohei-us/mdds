/*************************************************************************
 *
 * Copyright (c) 2010 Kohei Yoshida
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

#ifndef __MDDS_QUAD_TYPE_MATRIX_HPP__
#define __MDDS_QUAD_TYPE_MATRIX_HPP__

#include <cstdlib>

namespace mdds {

/**
 * This data structure represents a matrix where each individual element may
 * be of one of four types: value, boolean, string, or empty.
 */
template<typename _ValueType, typename _BoolType, typename _StringType>
class quad_type_matrix
{
public:
    typedef _ValueType  value_type;
    typedef _BoolType   bool_type;
    typedef _StringType string_type;

    enum data_density_t
    {
        data_filled,
        data_sparse
    };

    quad_type_matrix();
    ~quad_type_matrix();

private:
    class storage_base
    {
    };

    class storage_filled : public storage_base
    {
    };

private:
    storage_base* mp_storage;
};

template<typename _ValueType, typename _BoolType, typename _StringType>
quad_type_matrix<_ValueType,_BoolType,_StringType>::quad_type_matrix() :
    mp_storage(NULL)
{
    mp_storage = new storage_filled;
}

template<typename _ValueType, typename _BoolType, typename _StringType>
quad_type_matrix<_ValueType,_BoolType,_StringType>::~quad_type_matrix()
{
    delete mp_storage;
}

}

#endif
