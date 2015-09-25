.. highlight:: cpp

Segment tree
============

.. cpp:namespace:: mdds

.. cpp:class:: segment_tree<Key,Value>

   .. cpp:type:: Key key_type

   .. cpp:type:: Value value_type

   .. cpp:type:: size_t size_type

   .. cpp:type:: std::vector<value_type> search_result_type

   .. cpp:function:: segment_tree()

      Default constructor.

   .. cpp:function:: segment_tree(const segment_tree& r)

      Copy constructor only copies the leaf nodes.

   .. cpp:function:: ~segment_tree()

      Destructor.

   .. cpp:function:: bool operator==(const segment_tree& r) const

   .. cpp:function:: bool operator!=(const segment_tree& r) const

   .. cpp:function:: bool is_tree_valid() const

      Check whether or not the internal tree is in a valid state.  The tree
      must be valid in order to perform searches.

      :return: true if the tree is valid, false otherwise.

   .. cpp:function:: build_tree()

      Build or re-build tree based on the current set of segments.

   .. cpp:function:: bool insert(key_type begin_key, key_type end_key, value_type pdata)

      Insert a new segment.

      :param begin_key: begin point of the segment.  The value is inclusive.
      :param end_key: end point of the segment.  The value is non-inclusive.
      :param pdata: pointer to the data instance associated with this segment.
                    Note that *the caller must manage the life cycle of the
                    data instance.*

   .. cpp:function:: bool search(key_type point, search_result_type& result) const

      Search the tree and collect all segments that include a specified
      point.

      :param point: specified point value
      :param result: doubly-linked list of data instances associated with the
         segments that include the specified point.  *Note that the search
         result gets appended to the list; the list will not get emptied on
         each search.*  It is caller's responsibility to empty the list before
         passing it to this method in case the caller so desires.

      :return: true if the search is performed successfully, false if the
         search has ended prematurely due to error conditions.

   .. cpp:function:: search_result search(key_type point) const

      Search the tree and collect all segments that include a specified
      point.

      :param point: specified point value

      :return: object containing the result of the search, which can be
              accessed via iterator.

   .. cpp:function:: void remove(value_type pdata)

   .. cpp:function:: void clear()

   .. cpp:function:: size_t size() const

   .. cpp:function:: bool empty() const

   .. cpp:function:: size_t leaf_size() const
