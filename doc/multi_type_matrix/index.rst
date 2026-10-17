.. SPDX-FileCopyrightText: 2025 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

.. highlight:: cpp

Multi Type Matrix
=================

:cpp:class:`~mdds::multi_type_matrix` is a two-dimensional matrix that can
store elements of different types.  Internally it uses
:cpp:type:`~mdds::multi_type_vector` as its value store, and stores the
element values in column-major order.

Defining the matrix traits
--------------------------

The matrix traits type, passed as the template argument, specifies which
element types the matrix can store.  It has two members:

* ``numeric_element_block`` specifies the element block type that stores the
  numeric elements.  This member is required.
* ``element_blocks`` is a ``std::tuple`` that lists the element block types
  of all the other element types.  This member is optional.  When it is
  absent, the matrix can store only numeric and empty elements.

The ``std::tuple`` is used only as a list of types.  The matrix never creates
an instance of it.

The numeric element type is the only type that has a special meaning to the
matrix.  Methods such as :cpp:func:`~mdds::multi_type_matrix::get_numeric`
and :cpp:func:`~mdds::multi_type_matrix::numeric` deal with the numeric
elements only.  All the other element types, including boolean, integer and
string, are handled through the generic
:cpp:func:`~mdds::multi_type_matrix::set` and
:cpp:func:`~mdds::multi_type_matrix::get` methods.  To check whether a matrix
contains only elements of a given set of types, use
:cpp:func:`~mdds::multi_type_matrix::all_of` with the element block types
to accept.

The library provides :cpp:struct:`~mdds::mtm::std_string_traits`, which uses
``double`` as its numeric type, and stores ``bool``, ``int32_t`` and
``std::string`` values in addition to the numeric values.  It is defined as
follows:

.. code-block:: cpp

   struct std_string_traits
   {
       using numeric_element_block = mdds::mtv::double_element_block;

       using element_blocks = std::tuple<
           mdds::mtv::boolean_element_block,
           mdds::mtv::int32_element_block,
           mdds::mtv::string_element_block>;
   };

The traits type must satisfy the :cpp:concept:`~mdds::mtm::matrix_traits`
concept.  All element block types must have distinct block type IDs, and the
numeric element block type must not also appear in ``element_blocks``.

The type of each element is identified by the block type ID of the element
block that stores it.  :cpp:func:`~mdds::multi_type_matrix::get_type` returns
this ID.  An empty element has the type
:cpp:var:`mdds::mtm::element_empty`, and a numeric element has the type
:cpp:member:`~mdds::multi_type_matrix::element_numeric`.

.. toctree::
   :maxdepth: 1

   custom-types.rst
   api.rst
