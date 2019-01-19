/*************************************************************************
 *
 * Copyright (c) 2010-2015 Kohei Yoshida
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

#ifndef INCLUDED_MDDS_RECTANGLE_SET_HPP
#define INCLUDED_MDDS_RECTANGLE_SET_HPP

#include "segment_tree.hpp"
#include "global.hpp"

namespace mdds {

template<typename _Key, typename _Value>
class MDDS_DEPRECATED rectangle_set
{
public:
    typedef _Key    key_type;
    typedef _Value  value_type;

#ifdef MDDS_UNIT_TEST
public:
#else
private:
#endif
    struct rectangle
    {
        key_type x1;
        key_type y1;
        key_type x2;
        key_type y2;

        rectangle(key_type _x1, key_type _y1, key_type _x2, key_type _y2) :
            x1(_x1), y1(_y1), x2(_x2), y2(_y2) {}

        rectangle(const rectangle& r) :
            x1(r.x1), y1(r.y1), x2(r.x2), y2(r.y2) {}

        rectangle& operator= (const rectangle& r)
        {
            x1 = r.x1;
            y1 = r.y1;
            x2 = r.x2;
            y2 = r.y2;
            return *this;
        }

        bool operator== (const rectangle& r) const
        {
            return x1 == r.x1 && y1 == r.y1 && x2 == r.x2 && y2 == r.y2;
        }

        bool operator!= (const rectangle& r) const
        {
            return !operator==(r);
        }
    };
    typedef std::unordered_map<value_type, rectangle>    dataset_type;
private:
    typedef segment_tree<key_type, value_type>   inner_type;
    typedef segment_tree<key_type, inner_type*>  outer_type;

    typedef std::pair<key_type, key_type>             interval_type;
    typedef std::map<interval_type, std::unique_ptr<inner_type>> inner_segment_map_type;

public:
    typedef typename inner_type::search_result_type search_result_type;

    /** 
     * Most of the implementation of search_result and its iterator is in 
     * segment_tree since the iteration logic is identical & depends on the
     * segment_tree internals. 
     */
    class search_result : public inner_type::search_result_base
    {
    public:
        typedef typename inner_type::search_result_base::res_chains_type res_chains_type;
        typedef typename inner_type::search_result_base::res_chains_ptr res_chains_ptr;
        typedef typename inner_type::data_chain_type data_chain_type;

    public:

        class iterator : public inner_type::iterator_base
        {
            friend class rectangle_set<_Key,_Value>::search_result;
        private:
            iterator(const res_chains_ptr& p) : inner_type::iterator_base(p) {}
        public:
            iterator() : inner_type::iterator_base() {}
        };

        search_result() : inner_type::search_result_base() {}
        search_result(const search_result& r) : inner_type::search_result_base(r) {}

        typename search_result::iterator begin()
        {
            typename search_result::iterator itr(
                inner_type::search_result_base::get_res_chains());
            itr.move_to_front();
            return itr;
        }

        typename search_result::iterator end()
        {
            typename search_result::iterator itr(
                inner_type::search_result_base::get_res_chains());
            itr.move_to_end();
            return itr;
        }
    };

    rectangle_set();
    rectangle_set(const rectangle_set& r);
    ~rectangle_set();

    rectangle_set& operator= (const rectangle_set& r);

    /** 
     * Equality between two instances of rectangle_set is evaluated based on 
     * the stored rectangle instances; their pointer values and geometries.
     */
    bool operator== (const rectangle_set& r) const;

    bool operator!= (const rectangle_set& r) const { return !operator==(r); }

    /** 
     * Insert a new rectangle (and data associated with it) into the set. 
     * Note that insertion of duplicate data instance is not allowed.  A data 
     * is considered a duplicate if its pointer value is identical to one of 
     * the data instances already stored within.  Also note that <i>the end
     * point of a rectangle is non-inclusive; a rectangle of (x1,y1) - (x2,y2) 
     * means that the rectangle spans x1 <= x < x2 and y1 <= y < y2.</i> 
     *  
     * @param x1 lower x coordinate of the rectangle.  Inclusive.
     * @param y1 lower y coordinate of the rectangle.  Inclusive.
     * @param x2 upper x coordinate of the rectangle.  Non-inclusive.
     * @param y2 upper y coordinate of the rectangle.  Non-inclusive.
     * @param data pointer to data instance associated with this rectangle. 
     *             <i>Note that the caller is responsible for managing the
     *             life cycle of the data instance</i>.
     * 
     * @return true if a rectangle successfully inserted, false otherwise.
     */
    bool insert(key_type x1, key_type y1, key_type x2, key_type y2, value_type data);

    /** 
     * Search and collect all rectangles that contains a given point.
     *  
     * @param x x coordinate of a query point.
     * @param y y coordinate of a query point.
     * @param result array of pointers to rectangle instances.
     * 
     * @return true if the search is successful, false otherwise.
     */
    bool search(key_type x, key_type y, search_result_type& result);

    /** 
     * Search and collect all rectangles containing a given point.
     *  
     * @param x x coordinate of a query point.
     * @param y y coordinate of a query point. 
     * 
     * @return object containing the result of the search, which can be 
     *         accessed via iterator.
     */
    search_result search(key_type x, key_type y);

    /** 
     * Remove a rectangle instance pointed to by a given pointer.
     *
     * @param data pointer that points to the rectangle instance you wish to 
     *             remove from the set.
     */
    void remove(value_type data);

    /** 
     * Clear all rectangles stored in the set.
     */
    void clear();

    /** 
     * Return the number of rectangles currently stored in the set.
     * 
     * @return number of stored rectangles.
     */
    size_t size() const;

    /** 
     * Check whether or not the set is empty.
     * 
     * @return true if the set is empty, false otherwise.
     */
    bool empty() const;

private:
    void build_inner_map(const inner_segment_map_type& r);
    void build_outer_segment_tree();

#ifdef MDDS_UNIT_TEST
public:
    void dump_rectangles() const;
    bool verify_rectangles(const dataset_type& expected) const;
#endif

private:

    /** 
     * This data member stores pointers to the inner segment tree instances 
     * associated with outer intervals.  Used to speed up searches.
     */
    outer_type m_outer_segments;

    /**
     * This data member owns the inner segment_tree instances.
     */
    inner_segment_map_type m_inner_map;

    /** 
     * Used to keep track of currently stored data instances, to prevent 
     * insertion of duplicates.  Duplicates are defined as data objects having 
     * identical pointer value. 
     */
    dataset_type m_dataset;
};

}

#include "rectangle_set_def.inl"

#endif
