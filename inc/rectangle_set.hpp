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

#include <vector>
#include <unordered_map>
#include <boost/ptr_container/ptr_map.hpp>

namespace mdds {

template<typename _Key, typename _Data>
class rectangle_set
{
public:
    typedef _Key    key_type;
    typedef _Data   data_type;
    typedef ::std::vector<const data_type*> search_result_type;

    rectangle_set();
    rectangle_set(const rectangle_set& r);
    ~rectangle_set();

    bool insert(key_type x1, key_type y1, key_type x2, key_type y2, data_type* data);

    bool search(key_type x, key_type y, search_result_type& result) const;

    void remove(data_type* data);

    void clear();

    size_t size() const;

    bool empty() const;

private:
#ifdef UNIT_TEST
    void dump_rectangles() const;
#endif

private:
    struct rectangle
    {
        key_type x1;
        key_type y1;
        key_type x2;
        key_type y2;

        rectangle(key_type _x1, key_type _y1, key_type _x2, key_type _y2) :
            x1(_x1), y1(_y1), x2(_x2), y2(_y2) {}
    };
    typedef ::std::unordered_map<data_type*, rectangle>    dataset_type;

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

    /** 
     * Used to keep track of currently stored data instances, to prevent 
     * insertion of duplicates.  Duplicates are defined as data objects having 
     * identical pointer value. 
     */
    dataset_type m_dataset;
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
bool rectangle_set<_Key,_Data>::insert(key_type x1, key_type y1, key_type x2, key_type y2, data_type* data)
{
    // Make sure this is not a duplicate.
    if (m_dataset.find(data) != m_dataset.end())
        return false;

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
    m_dataset.insert(typename dataset_type::value_type(data, rectangle(x1, y1, x2, y2)));

    return true;
}

template<typename _Key, typename _Data>
bool rectangle_set<_Key,_Data>::search(key_type x, key_type y, search_result_type& result) const
{
    return true;
}

template<typename _Key, typename _Data>
void rectangle_set<_Key,_Data>::remove(data_type* data)
{
}

template<typename _Key, typename _Data>
void rectangle_set<_Key,_Data>::clear()
{
}

template<typename _Key, typename _Data>
size_t rectangle_set<_Key,_Data>::size() const
{
    return m_dataset.size();
}

template<typename _Key, typename _Data>
bool rectangle_set<_Key,_Data>::empty() const
{
    return m_dataset.empty();
}


#ifdef UNIT_TEST
template<typename _Key, typename _Data>
void rectangle_set<_Key,_Data>::dump_rectangles() const
{
    using namespace std;
    cout << "dump rectangles ------------------------------------------------" << endl;
    typename dataset_type::const_iterator itr = m_dataset.begin(), itr_end = m_dataset.end();
    for (; itr != itr_end; ++itr)
    {
        const rectangle& rect = itr->second;
        cout << itr->first->name << ": (x1,y1,x2,y2) = "
            << "(" << rect.x1 << "," << rect.y1 << "," << rect.x2 << "," << rect.y2 << ")" 
            << endl;
    }
}
#endif

}

#endif
