
API Incompatibility Notes
=========================

1.5
---

multi_type_vector
^^^^^^^^^^^^^^^^^

* The standard integer blocks previously used non-standard integer types,
  namely:

  * short
  * unsigned short
  * int
  * unsigned int
  * long
  * unsigned long
  * char
  * unsigned char

  Starting with this version, the integer blocks now use:

  * (u)int8_t
  * (u)int16_t
  * (u)int32_t
  * (u)int64_t

* The numeric_element_block type has been renamed to `double_element_block`,
  to make room for a new element block for float type named
  `float_element_block`.

1.4
---

multi_type_matrix
^^^^^^^^^^^^^^^^^

* The walk() methods previously took the function object by reference,
  but the newer versions now take the function object by value.  With
  this change, it is now possible to pass inline lambda function.
  However, if you were dependent on the old behavior, *this change may
  adversely affect the outcome of your code especially when your
  function object stores data members that are expected to be altered by
  the walk() methods*.

1.2
---

trie_map / packed_trie_map
^^^^^^^^^^^^^^^^^^^^^^^^^^

* The find() method now returns a const_iterator instance rather than a value
  type.  It returns an end position iterator when the method fails to find a
  match.

* The prefix_search() method now returns a search_results instance that has
  begin() and end() methods to allow iterating through the result set.

* The constructor no longer takes a null value parameter.

* Some nested type names have been renamed:

  * string_type -> key_type
  * char_type -> key_unit_type
  * string_buffer_type -> key_buffer_type

* Some functions expected from the key trait class have been renamed:

  * init_buffer() -> to_key_buffer()
  * to_string() -> to_key()

* The kay trait class now expects the following additional static methods:

  * key_buffer_type to_key_buffer(const key_type& key)
  * key_unit_type* buffer_data(const key_buffer_type& buf)
  * size_t buffer_size(const key_buffer_type& buf)

quad_point_tree
^^^^^^^^^^^^^^^

* The search_result nested class has been renamed to search_results, to keep
  the name consistent with that of the same name in trie_map and
  packed_trie_map.

multi_type_matrix
^^^^^^^^^^^^^^^^^

* The matrix trait structure (formerly known as the string trait structure)
  now needs to specify the type of block that stores integer values as its
  **integer_element_block** member.

1.0
---

* Starting with version 1.0, mdds now requires support for C++11.  Stick with
  0.12 or earlier versions if you use a compiler that doesn't support C++11.

* data_type has been renamed to value_type for segment_tree, rectangle_set,
  and point_quad_tree.


0.9
---

multi_type_vector
^^^^^^^^^^^^^^^^^

* The number of template parameters in custom_block_func1,
  custom_block_func2 and custom_block_func3 have been reduced by half,
  by deducing the numerical block type ID from the block type
  definition directly.  If you use the older variant, simply remove
  the template arguments that are numerical block IDs.

0.8
---

flat_segment_tree
^^^^^^^^^^^^^^^^^

* The search_tree() method in 0.8.0 returns std::pair<const_iterator,
  bool> instead of just returning bool as of 0.7.1.  If you use this
  method and relies on the return value of the old version, use the
  second parameter of the new return value which is equivalent of the
  previous return value.

0.5
---

flat_segment_tree
^^^^^^^^^^^^^^^^^

* The search() method now returns ::std::pair<const_iterator, bool>.
  This method previously returned only bool.  Use the second parameter of
  the new return value which is equivalent of the previous return value.
