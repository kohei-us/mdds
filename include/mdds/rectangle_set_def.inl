/*************************************************************************
 *
 * Copyright (c) 2011-2015 Kohei Yoshida
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

#include <sstream>

namespace mdds {

template<typename _Key, typename _Value>
rectangle_set<_Key,_Value>::rectangle_set()
{
}

template<typename _Key, typename _Value>
rectangle_set<_Key,_Value>::rectangle_set(const rectangle_set& r) :
    m_dataset(r.m_dataset)
{
    build_inner_map(r.m_inner_map);
    build_outer_segment_tree();
}

template<typename _Key, typename _Value>
rectangle_set<_Key,_Value>::~rectangle_set()
{
}

template<typename _Key, typename _Value>
rectangle_set<_Key,_Value>& rectangle_set<_Key,_Value>::operator= (const rectangle_set& r)
{
    clear(); // Don't forget to clear the internal state beforehands.

    m_dataset = r.m_dataset;
    build_inner_map(r.m_inner_map);
    build_outer_segment_tree();
    return *this;
}

template<typename _Key, typename _Value>
bool rectangle_set<_Key,_Value>::operator== (const rectangle_set& r) const
{
    if (m_dataset.size() != r.m_dataset.size())
        return false;

    typename dataset_type::const_iterator itr = m_dataset.begin(), itr_end = m_dataset.end();
    for (; itr != itr_end; ++itr)
    {
        typename dataset_type::const_iterator itr_rhs = r.m_dataset.find(itr->first);
        if (itr_rhs == r.m_dataset.end())
            return false;

        if (itr->second != itr_rhs->second)
            return false;
    }

    return true;
}

template<typename _Key, typename _Value>
bool rectangle_set<_Key,_Value>::insert(key_type x1, key_type y1, key_type x2, key_type y2, value_type data)
{
    if (x1 >= x2 || y1 >= y2)
    {
        std::ostringstream os;
        os << "specified range coordinates are invalid (x1=" << x1 << ",y1=" << y1 << ",x2=" << x2 << ",y2=" << y2 << ')';
        throw invalid_arg_error(os.str());
    }

    // Make sure this is not a duplicate.
    if (m_dataset.find(data) != m_dataset.end())
        return false;

    // Check if interval x1 - x2 is already stored.
    interval_type outer_interval = interval_type(x1, x2);
    typename inner_segment_map_type::iterator itr = m_inner_map.find(outer_interval);
    if (itr == m_inner_map.end())
    {
        // this interval has not yet been stored.  Create a new inner segment
        // tree instance for this interval.
        auto r = m_inner_map.insert(
            typename inner_segment_map_type::value_type(
                outer_interval, make_unique<inner_type>()));
        if (!r.second)
            throw general_error("inner segment tree insertion failed.");

        itr = r.first;

        // Register the pointer to this inner segment tree instance with the
        // outer segment tree.
        if (!m_outer_segments.insert(x1, x2, itr->second.get()))
            // This should never fail if my logic is correct.
            throw general_error("failed to insert an inner segment tree pointer into the outer segment tree.");
    }

    inner_type* inner_tree = itr->second.get();
    inner_tree->insert(y1, y2, data);
    m_dataset.insert(typename dataset_type::value_type(data, rectangle(x1, y1, x2, y2)));

    return true;
}

template<typename _Key, typename _Value>
bool rectangle_set<_Key,_Value>::search(key_type x, key_type y, search_result_type& result)
{
    typename outer_type::search_result_type inner_trees;
    if (!m_outer_segments.is_tree_valid())
        m_outer_segments.build_tree();

    if (!m_outer_segments.search(x, inner_trees))
        return false;

    typename outer_type::search_result_type::iterator itr_tree = inner_trees.begin(), itr_tree_end = inner_trees.end();
    for (; itr_tree != itr_tree_end; ++itr_tree)
    {
        inner_type* inner_tree = *itr_tree;
        if (!inner_tree->is_tree_valid())
            inner_tree->build_tree();

        // Search all relevant inner trees and aggregate results.
        if (!inner_tree->search(y, result))
            return false;
    }
    return true;
}

template<typename _Key, typename _Value>
typename rectangle_set<_Key,_Value>::search_result
rectangle_set<_Key,_Value>::search(key_type x, key_type y)
{
    search_result result;
    typename outer_type::search_result_type inner_trees;
    if (!m_outer_segments.is_tree_valid())
        m_outer_segments.build_tree();

    if (!m_outer_segments.search(x, inner_trees))
        return result;

    typename outer_type::search_result_type::iterator itr_tree = inner_trees.begin(), itr_tree_end = inner_trees.end();
    for (; itr_tree != itr_tree_end; ++itr_tree)
    {
        inner_type* inner_tree = *itr_tree;
        if (!inner_tree->is_tree_valid())
            inner_tree->build_tree();

        // Search all relevant inner trees and aggregate results.
        inner_tree->search(y, result);
    }
    return result;
}

template<typename _Key, typename _Value>
void rectangle_set<_Key,_Value>::remove(value_type data)
{
    typename dataset_type::iterator itr_data = m_dataset.find(data);
    if (itr_data == m_dataset.end())
        // The data is not stored in this data structure.
        return;

    const rectangle& rect = itr_data->second;

    // Find the corresponding inner segment tree for this outer interval.
    interval_type outer(rect.x1, rect.x2);
    typename inner_segment_map_type::iterator itr_seg = m_inner_map.find(outer);
    if (itr_seg == m_inner_map.end())
        throw general_error("inconsistent internal state: failed to find an internal segment tree for an existing interval.");

    // Remove data from the inner segment tree.
    inner_type* inner_tree = itr_seg->second.get();
    inner_tree->remove(data);
    if (inner_tree->empty())
    {
        // This inner tree is now empty.  Erase it.
        m_outer_segments.remove(inner_tree);
        m_inner_map.erase(itr_seg);
    }

    // Remove it from the data set as well.
    m_dataset.erase(data);
}

template<typename _Key, typename _Value>
void rectangle_set<_Key,_Value>::clear()
{
    m_outer_segments.clear();
    m_inner_map.clear();
    m_dataset.clear();
}

template<typename _Key, typename _Value>
size_t rectangle_set<_Key,_Value>::size() const
{
    return m_dataset.size();
}

template<typename _Key, typename _Value>
bool rectangle_set<_Key,_Value>::empty() const
{
    return m_dataset.empty();
}

template<typename _Key, typename _Value>
void rectangle_set<_Key,_Value>::build_inner_map(const inner_segment_map_type& r)
{
    auto it = r.begin(), ite = r.end();
    for (; it != ite; ++it)
    {
        m_inner_map.insert(
            typename inner_segment_map_type::value_type(
                it->first, make_unique<inner_type>(*it->second)));
    }
}

template<typename _Key, typename _Value>
void rectangle_set<_Key,_Value>::build_outer_segment_tree()
{
    // Re-construct the outer segment tree from the authoritative inner tree
    // map.
    typename inner_segment_map_type::iterator itr = m_inner_map.begin(), itr_end = m_inner_map.end();
    for (; itr != itr_end; ++itr)
    {
        const interval_type& interval = itr->first;
        inner_type* tree = itr->second.get();
        m_outer_segments.insert(interval.first, interval.second, tree);
    }
}

#ifdef MDDS_UNIT_TEST
template<typename _Key, typename _Value>
void rectangle_set<_Key,_Value>::dump_rectangles() const
{
    using namespace std;
    cout << "dump rectangles ------------------------------------------------" << endl;
    if (m_dataset.empty())
    {
        cout << "No rectangles in the data set." << endl;
        return;
    }

    typename dataset_type::const_iterator itr = m_dataset.begin(), itr_end = m_dataset.end();
    for (; itr != itr_end; ++itr)
    {
        const rectangle& rect = itr->second;
        cout << itr->first->name << ": (x1,y1,x2,y2) = "
            << "(" << rect.x1 << "," << rect.y1 << "," << rect.x2 << "," << rect.y2 << ")"
            << endl;
    }
}

template<typename _Key, typename _Value>
bool rectangle_set<_Key,_Value>::verify_rectangles(const dataset_type& expected) const
{
    if (m_dataset.size() != expected.size())
        // Data sizes differ.
        return false;

    typename dataset_type::const_iterator itr_data = m_dataset.begin(), itr_data_end = m_dataset.end();
    for (; itr_data != itr_data_end; ++itr_data)
    {
        const value_type data = itr_data->first;
        typename dataset_type::const_iterator itr_test = expected.find(data);
        if (itr_test == expected.end())
            // Pointer in one container but not in the other.
            return false;

        if (itr_data->second != itr_test->second)
            // Rectangle positions and/or sizes differ.
            return false;
    }

    return true;
}
#endif

}
