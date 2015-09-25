.. highlight:: cpp

Flat segment tree
=================

.. cpp:namespace:: mdds

.. cpp:class:: flat_segment_tree<Key,Value>

   .. cpp:type:: Key key_type

   .. cpp:type:: Value value_type

   .. cpp:type:: size_t size_type

   .. cpp:type:: iterator

   .. cpp:type:: const_iterator

   .. cpp:type:: const_reverse_iterator

   .. cpp:function:: flat_segment_tree(key_type min_val, key_type max_val, value_type init_val)

      Constructor.

      :param min_val: minimum value
      :param max_val: maximum value
      :param init_val: initial value

   .. cpp:function:: flat_segment_tree(const flat_segment_tree<key_type,value_type>& r)

      Copy constructor only copies the leaf nodes.

   .. cpp:function:: ~flat_segment_tree()

      Destructor.

   .. cpp:function:: void swap(flat_segment_tree<key_type,value_type>& other)

      :param other: the other instance to swap contents with.

   .. cpp:function:: const_iterator begin() const

   .. cpp:function:: const_iterator end() const

   .. cpp:function:: const_reverse_iterator rbegin() const

   .. cpp:function:: const_reverse_iterator rend() const

   .. cpp:function:: void clear()

   .. cpp:function:: std::pair<const_iterator, bool> insert_front(key_type start_key, key_type end_key, value_type val)

      Insert a new segment into the tree.  It searches for the point of
      insertion from the first leaf node.

      :param start_key: start value of the segment being inserted.  The value
         is inclusive.
      :param end_key: end value of the segment being inserted.  The value is
         not inclusive.
      :param val: value associated with this segment.

      :return: pair of const_iterator corresponding to the start position of
         the inserted segment, and a boolean value indicating whether or not
         the insertion has modified the tree.

   .. cpp:function:: std::pair<const_iterator, bool> insert_back(key_type start_key, key_type end_key, value_type val)

      Insert a new segment into the tree.  Unlike the :cpp:func:`~insert_front()`
      counterpart, this method searches for the point of insertion from the
      last leaf node toward the first.

      :param start_key: start value of the segment being inserted.  The value
         is inclusive.
      :param end_key: end value of the segment being inserted.  The value is
         not inclusive.
      :param val: value associated with this segment.

      :return: pair of const_iterator corresponding to the start position of
         the inserted segment, and a boolean value indicating whether or not
         the insertion has modified the tree.

   .. cpp:function:: std::pair<const_iterator, bool> insert(const const_iterator &pos, key_type start_key, key_type end_key, value_type val)

      Insert a new segment into the tree at or after specified point of
      insertion.

      :param pos: specified insertion point
      :param start_key: start value of the segment being inserted.  The value
         is inclusive.
      :param end_key: end value of the segment being inserted.  The value is
         not inclusive.
      :param val: value associated with this segment.

      :return: pair of const_iterator corresponding to the start position of
         the inserted segment, and a boolean value indicating whether or not
         the insertion has modified the tree.

   .. cpp:function:: void shift_left(key_type start_key, key_type end_key)

      Remove a segment specified by the start and end key values, and shift
      the remaining segments (i.e. those segments that come after the removed
      segment) to left.  Note that the start and end positions of the segment
      being removed **must** be within the base segment span.

      :param start_key: start position of the segment being removed.
      :param end_key: end position of the segment being removed.

   .. cpp:function:: void shift_right(key_type pos, key_type size, bool skip_start_node)

      Shift all segments that occur at or after the specified start position
      to right by the size specified.

      :param pos: position where the right-shift occurs.
      :param size: amount of shift (must be greater than 0)
      :param skip_start_node: if true, and the specified position is at an
         existing node position, that node will *not* be shifted.  This
         argument has no effect if the position specified does not coincide
         with any of the existing nodes.

   .. cpp:function:: std::pair<const_iterator, bool> search(key_type key, value_type& value, key_type* start_key=nullptr, key_type* end_key=nullptr) const

      Perform leaf-node search for a value associated with a key.

      :param key: key value
      :param value: value associated with key specified gets stored upon
         successful search.
      :param start_key: pointer to a variable where the start key value of the
         segment that contains the key gets stored upon successful search.
      :param end_key: pointer to a varaible where the end key value of the
         segment that contains the key gets stored upon successful search.
      :return: a pair of const_iterator corresponding to the start position of
         the segment containing the key, and a boolean value indicating
         whether or not the search has been successful.

   .. cpp:function:: std::pair<const_iterator, bool> search(const const_iterator& pos, key_type key, value_type& value, key_type* start_key=nullptr, key_type* end_key=nullptr) const

      Perform leaf-node search for a value associated with a key.

      :param pos: position from which the search should start.  When the
         position is invalid, it falls back to the normal search.
      :param key: key value
      :param value: value associated with key specified gets stored upon
         successful search.
      :param start_key: pointer to a variable where the start key value of the
         segment that contains the key gets stored upon successful search.
      :param end_key: pointer to a varaible where the end key value of the
         segment that contains the key gets stored upon successful search.
      :return: a pair of const_iterator corresponding to the start position of
         the segment containing the key, and a boolean value indicating
         whether or not the search has been successful.

   .. cpp:function:: std::pair<const_iterator, bool> search_tree(key_type key, value_type& value, key_type* start_key=nullptr, key_type* end_key=nullptr) const

      Perform tree search for a value associated with a key.  This method
      assumes that the tree is valid.

      :param key: key value
      :param value: value associated with key specified gets stored upon
         successful search.
      :param start_key: pointer to a variable where the start key value of the
         segment that contains the key gets stored upon successful search.
      :param end_key: pointer to a varaible where the end key value of the
         segment that contains the key gets stored upon successful search.
      :return: a pair of const_iterator corresponding to the start position of
         the segment containing the key, and a boolean value indicating
         whether or not the search has been successful.

   .. cpp:function:: void build_tree()

      Build a tree of non-leaf nodes based on the values stored in the leaf
      nodes.  The tree must be valid before you can call the
      :cpp:func:`~search_tree()` method.

   .. cpp:function:: bool is_tree_valid() const

      :return: true if the tree is valid, otherwise false.  The tree must be
         valid before you can call the :cpp:func:`~search_tree()` method.

   .. cpp:function:: flat_segment_tree<key_type,value_type>& operator =(const flat_segment_tree<key_type, value_type>& other)

      Assignment only copies the leaf nodes.

      :param other: the other instance to assign content from.

   .. cpp:function:: bool operator ==(const flat_segment_tree<key_type, value_type>& r) const

      :param r: the other instance to check equality against.

   .. cpp:function:: bool operator !=(const flat_segment_tree<key_type, value_type>& r) const

      :param r: the other instance to check in-equality against.

   .. cpp:function:: key_type min_key() const

      :return: minimum key value.

   .. cpp:function:: key_type max_key() const

      :return: maximum key value.

   .. cpp:function:: value_type default_value() const

      :return: default value.

   .. cpp:function:: size_type leaf_size() const

      :return: number of leaf nodes.


Example
-------

::

    #include <mdds/flat_segment_tree.hpp>
    #include <string>
    #include <iostream>

    using namespace std;

    typedef mdds::flat_segment_tree<long, int> fst_type;

    int main()
    {
        // Define the begin and end points of the whole segment, and the default
        // value.
        fst_type db(0, 500, 0);

        db.insert_front(10, 20, 10);
        db.insert_back(50, 70, 15);
        db.insert_back(60, 65, 5);

        int value = -1;
        long beg = -1, end = -1;

        // Perform linear search.  This doesn't require the tree to be built
        // beforehand.  Note that the begin and end point parameters are optional.
        db.search(15, value, &beg, &end);
        cout << "The value at 15 is " << value << ", and this segment spans from " << beg << " to " << end << endl;;

        // Don't forget to build tree before calling search_tree().
        db.build_tree();

        // Perform tree search.  Tree search is generally a lot faster than linear
        // search, but requires the tree to be built beforehand.
        db.search_tree(62, value, &beg, &end);
        cout << "The value at 62 is " << value << ", and this segment spans from " << beg << " to " << end << endl;;
    }
