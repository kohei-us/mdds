
API Incompatibility Notes
=========================

v2.1
----

* The following public template types have been put into ``mdds::detail`` namespace:

  * ``has_value_type``
  * ``const_or_not``
  * ``const_t``
  * ``get_iterator_type``
  * ``invalid_static_int``

multi_type_vector
^^^^^^^^^^^^^^^^^

* In 2.0, multi_type_vector took two template parameters: one for the element block
  functions and one for other traits.  In 2.1, multi_type_vector only takes one
  template parameter for the traits, and the traits now must include the element
  block functions.

* The following block function helpers have been removed:

  * ``mdds::mtv::custom_block_func1``
  * ``mdds::mtv::custom_block_func2``
  * ``mdds::mtv::custom_block_func3``

  They have been replaced with ``mdds::mtv::element_block_funcs`` which uses
  `template parameter pack <https://en.cppreference.com/w/cpp/language/parameter_pack>`_
  to allow unspecified number of standard and custom blocks.

  As a result of this change, the following headers have been removed:

  * ``include/mdds/multi_type_vector/custom_func1.hpp``
  * ``include/mdds/multi_type_vector/custom_func2.hpp``
  * ``include/mdds/multi_type_vector/custom_func3.hpp``
  * ``include/mdds/multi_type_vector/trait.hpp``
  * ``include/mdds/multi_type_vector_custom_func2.hpp``
  * ``include/mdds/multi_type_vector_custom_func3.hpp``
  * ``include/mdds/multi_type_vector_trait.hpp``

* ``mdds::mtv::default_trait`` has been renamed to ``mdds::mtv::default_traits``.

* The following element block types have an additional template parameter to specify
  the underlying storage type.  This can be used to specify, for instance, whether
  the element block uses ``std::vector``, ``std::deque`` or another custom container
  type with API compatible with the aforementioned two.

  * ``mdds::mtv::default_element_block``
  * ``mdds::mtv::managed_element_block``
  * ``mdds::mtv::noncopyable_managed_element_block``

  With this change, the compiler macro named ``MDDS_MULTI_TYPE_VECTOR_USE_DEQUE``,
  which was previously used to switch from ``std::vector`` to ``std::deque`` as the
  underlying storage type for all element blocks, has been removed.

multi_type_matrix
^^^^^^^^^^^^^^^^^

* ``mdds::mtm::std_string_trait`` has been renamed to ``mdds::mtm::std_string_traits``.

trie_map / packed_trie_map
^^^^^^^^^^^^^^^^^^^^^^^^^^

* The following public types have been renamed:

  * ``mdds::trie::std_container_trait`` -> ``mdds::trie::std_container_traits``
  * ``mdds::trie::std_string_trait`` -> ``mdds::trie::std_string_traits``


v2.0
----

* baseline C++ version has been set to C++17.

* deprecated ``rectangle_set`` data structure has been removed.

multi_type_vector
^^^^^^^^^^^^^^^^^

* The second template parameter is now a trait type that specifies custom event
  function type and loop-unrolling factor.  Prior to 2.0 the second template
  parameter was custom event function type.

* Due to the addition of the structure-of-arrays variant, the following header
  files have been relocated:

  .. list-table:: Relocated Headers
     :widths: 50 50
     :header-rows: 1

     * - Old header location
       - New header location
     * - mdds/multi_type_vector_types.hpp
       - mdds/multi_type_vector/types.hpp
     * - mdds/multi_type_vector_macro.hpp
       - mdds/multi_type_vector/macro.hpp
     * - mdds/multi_type_vector_trait.hpp
       - mdds/multi_type_vector/trait.hpp
     * - mdds/multi_type_vector_itr.hpp
       - mdds/multi_type_vector/aos/iterator.hpp
     * - mdds/multi_type_vector_custom_func1.hpp
       - mdds/multi_type_vector/custom_func1.hpp
     * - mdds/multi_type_vector_custom_func2.hpp
       - mdds/multi_type_vector/custom_func2.hpp
     * - mdds/multi_type_vector_custom_func3.hpp
       - mdds/multi_type_vector/custom_func3.hpp

  The old headers will continue to work for the time being, but consider them
  deprecated.

* Since now we have array-of-structures (AoS) and structure-of-arrays (SoA) variants
  of multi_type_vector, there are two instances of multi_type_vector class in two
  different headers and namespace locations. To use the AoS variant, include the header

  .. code-block:: c++

     #include <mdds/multi_type_vector/aos/main.hpp>

  and instantiate the template class as ``mdds::mtv::aos::multi_type_vector``.
  Likewise, to use the SoA variant, include the header

  .. code-block:: c++

     #include <mdds/multi_type_vector/soa/main.hpp>

  and instantiate the template class as ``mdds::mtv::soa::multi_type_vector``.

  If you include the original header

  .. code-block:: c++

     #include <mdds/multi_type_vector.hpp>

  it will include a template alias ``mdds::multi_type_vector`` that simply references
  ``mdds::mtv::soa::multi_type_vector``.


segment_tree
^^^^^^^^^^^^

* The following public types have been renamed:

  * ``search_result`` -> ``search_results``
  * ``search_result_type`` -> ``search_results_type``

v1.5
----

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

v1.4
----

multi_type_matrix
^^^^^^^^^^^^^^^^^

* The walk() methods previously took the function object by reference,
  but the newer versions now take the function object by value.  With
  this change, it is now possible to pass inline lambda function.
  However, if you were dependent on the old behavior, *this change may
  adversely affect the outcome of your code especially when your
  function object stores data members that are expected to be altered by
  the walk() methods*.

v1.2
----

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

v1.0
----

* Starting with version 1.0, mdds now requires support for C++11.  Stick with
  0.12 or earlier versions if you use a compiler that doesn't support C++11.

* data_type has been renamed to value_type for segment_tree, rectangle_set,
  and point_quad_tree.


v0.9
----

multi_type_vector
^^^^^^^^^^^^^^^^^

* The number of template parameters in custom_block_func1,
  custom_block_func2 and custom_block_func3 have been reduced by half,
  by deducing the numerical block type ID from the block type
  definition directly.  If you use the older variant, simply remove
  the template arguments that are numerical block IDs.

v0.8
----

flat_segment_tree
^^^^^^^^^^^^^^^^^

* The search_tree() method in 0.8.0 returns std::pair<const_iterator,
  bool> instead of just returning bool as of 0.7.1.  If you use this
  method and relies on the return value of the old version, use the
  second parameter of the new return value which is equivalent of the
  previous return value.

v0.5
----

flat_segment_tree
^^^^^^^^^^^^^^^^^

* The search() method now returns ::std::pair<const_iterator, bool>.
  This method previously returned only bool.  Use the second parameter of
  the new return value which is equivalent of the previous return value.
