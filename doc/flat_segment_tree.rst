.. highlight:: cpp

Flat Segment Tree
=================

.. cpp:namespace:: mdds

.. cpp:class:: flat_segment_tree<Key,Value>

   Write something about this class.

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

      :param start_key:
      :param end_key:
      :param val:

   .. cpp:function:: std::pair<const_iterator, bool> insert_back(key_type start_key, key_type end_key, value_type val)

      :param start_key:
      :param end_key:
      :param val:

   .. cpp:function:: std::pair<const_iterator, bool> insert(const const_iterator &pos, key_type start_key, key_type end_key, value_type val)

      :param pos:
      :param start_key:
      :param end_key:
      :param val:

   .. cpp:function:: void shift_left(key_type start_key, key_type end_key)

      :param start_key:
      :param end_key:

   .. cpp:function:: void shift_right(key_type pos, key_type size, bool skip_start_node)

      :param start_key:
      :param end_key:
      :param skip_start_node:

   .. cpp:function:: std::pair<const_iterator, bool> search(key_type key, value_type& value, key_type* start_key=nullptr, key_type* end_key=nullptr) const

      :param key:
      :param value:
      :param start_key:
      :param end_key:

   .. cpp:function:: std::pair<const_iterator, bool> search(const const_iterator& pos, key_type key, value_type& value, key_type* start_key=nullptr, key_type* end_key=nullptr) const

      :param pos:
      :param key:
      :param value:
      :param start_key:
      :param end_key:

   .. cpp:function:: std::pair<const_iterator, bool> search_tree(key_type key, value_type& value, key_type* start_key=nullptr, key_type* end_key=nullptr) const

      :param key:
      :param value:
      :param start_key:
      :param end_key:

   .. cpp:function:: void build_tree()

   .. cpp:function:: bool is_tree_valid() const

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
