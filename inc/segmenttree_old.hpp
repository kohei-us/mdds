/*************************************************************************
 *
 * Copyright (c) 2008-2009 Kohei Yoshida
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

#ifndef __MDDS_SEGMENTTREE_HXX__
#define __MDDS_SEGMENTTREE_HXX__

#include <boost/shared_ptr.hpp>
#include <memory>
#include <exception>
#include <list>

namespace mdds {

class segment_tree_impl;

class segment_tree
{
public:
    typedef long value_type;

    /** 
     * Base class for data accompanying each segment.  The client code needs 
     * to write a concrete class from this to store meta data for inserted 
     * segments. 
     */
    struct segment_data_type
    {
        virtual ~segment_data_type() = 0;

        /** 
         * Generate a hash value in case this struct is stored in a hash table
         * container. 
         */
        virtual size_t hash() const = 0;

        /** 
         * Return a descriptive string for debug display.
         */
        virtual const char* what() const = 0;
    };

    segment_tree();
    ~segment_tree();

    /** 
     * Build the internal tree with all segments that have been inserted up to
     * this point.  Note that once the tree is built, no new segments can be 
     * inserted via insertSegment method. 
     */
    void build();

    /** 
     * Insert a single segment into the tree.  Note that once the internal 
     * tree is built, no new interval can be inserted, or it will throw a 
     * {@link TreeLockedException}. 
     *
     * @param low lower bound of the interval
     * @param high upper bound of the interval
     * @param data pointer to the segment data instance.  Note that this class 
     *             does not delete this instance.  The caller must manage its
     *             life cycle.
     */
    void insert_segment(value_type low, value_type high, const segment_data_type* data);

    void search(value_type point, ::std::list<const segment_data_type*>& segments);

private:
    ::std::auto_ptr<segment_tree_impl> mpImpl;
};

}

#endif
