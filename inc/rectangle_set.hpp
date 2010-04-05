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
#include "global.hpp"

#include <boost/ptr_container/ptr_map.hpp>

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

    typedef ::std::pair<key_type, key_type>             interval_type;
    typedef ::boost::ptr_map<interval_type, inner_type> inner_segment_map_type;

    /** 
     * This data member stores pointers to the inner segment tree instances 
     * associated with outer intervals.  Used during searches.
     */
    outer_type              m_outer_segments;

    /**
     * This data member owns the inner segment_tree instances.
     */
    inner_segment_map_type  m_outer_map;
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
    // Check if internal x1 - x2 is already stored.
    interval_type outer_interval = interval_type(x1, x2);
    typename inner_segment_map_type::iterator itr = m_outer_map.find(outer_interval);
    if (itr == m_outer_map.end())
    {
        // this interval has not yet been stored.  Create a new inner segment 
        // tree instance for this interval.
        ::std::pair<typename inner_segment_map_type::iterator, bool> r =
            m_outer_map.insert(outer_interval, new inner_type);
        if (!r.second)
            throw general_error("inner segment tree insertion failed.");

        itr = r.first;

        // Register the pointer to this inner segment tree instance with the 
        // outer segment tree.
        if (!m_outer_segments.insert(x1, x2, itr->second))
            // This should never fail if my logic is correct.
            throw general_error("failed to insert an inner segment tree pointer into the outer segment tree.");
    }

    inner_type* inner_tree = itr->second;
    inner_tree->insert(y1, y2, data);
}

#ifdef UNIT_TEST
template<typename _Key, typename _Data>
void rectangle_set<_Key,_Data>::dump_rectangles() const
{
}
#endif

}

#endif
