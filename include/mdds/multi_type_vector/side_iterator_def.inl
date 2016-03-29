/*************************************************************************
 *
 * Copyright (c) 2016 Kohei Yoshida
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

#include <iostream>

namespace mdds { namespace mtv {

template<typename _MtvT>
template<typename _T>
side_iterator<_MtvT>::side_iterator(const _T& begin, const _T& end)
{
    _T it = begin;
    for (; it != end; ++it)
    {
        init_insert_vector(*it);
    }
}

template<typename _MtvT>
template<typename _T>
void side_iterator<_MtvT>::init_insert_vector(
    _T t,  typename std::enable_if<std::is_pointer<_T>::value>::type*)
{
    std::cout << "pointer: " << t << std::endl;
}

template<typename _MtvT>
template<typename _T>
void side_iterator<_MtvT>::init_insert_vector(
    _T t,  typename std::enable_if<!std::is_pointer<_T>::value>::type*)
{
    std::cout << "non-pointer: " << &t << std::endl;
}

}}
