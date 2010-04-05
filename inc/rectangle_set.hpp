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

#ifndef __MDDS_RECTANGLE_SET_HPP__
#define __MDDS_RECTANGLE_SET_HPP__

#include "segmenttree.hpp"
#include <boost/ptr_container/ptr_vector.hpp>

namespace mdds {

template<typename _Key, typename _Data>
class rectangle_set
{
public:
    typedef _Key    key_type;
    typedef _Data   data_type;

    rectangle_set();
    rectangle_set(const rectangle_set& r);
    ~rectangle_set();

    void insert(key_type x1, key_type y1, key_type x2, key_type y2, data_type* data);

private:
#ifdef UNIT_TEST
    void dump_rectangles() const;
#endif

private:
    typedef segment_tree<key_type, data_type>   inner_type;
    typedef segment_tree<key_type, inner_type>  outer_type;
    outer_type m_outer_segments;

};

template<typename _Key, typename _Data>
rectangle_set<_Key,_Data>::rectangle_set()
{
}

template<typename _Key, typename _Data>
rectangle_set<_Key,_Data>::rectangle_set(const rectangle_set& r)
{
}

template<typename _Key, typename _Data>
rectangle_set<_Key,_Data>::~rectangle_set()
{
}

template<typename _Key, typename _Data>
void rectangle_set<_Key,_Data>::insert(key_type x1, key_type y1, key_type x2, key_type y2, data_type* data)
{
}

#ifdef UNIT_TEST
template<typename _Key, typename _Data>
void rectangle_set<_Key,_Data>::dump_rectangles() const
{
}
#endif

}

#endif
