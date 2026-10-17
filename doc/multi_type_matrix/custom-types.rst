.. SPDX-FileCopyrightText: 2026 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

.. highlight:: cpp

Storing user-defined types
==========================

You can store values of your own type in the matrix by adding its element
block type to ``element_blocks``.  In the following example, we will store
error values in a matrix together with numeric, boolean and string values.

First, we define the value type to store:

.. literalinclude:: ../../example/multi_type_matrix_custom.cpp
   :language: C++
   :start-after: //!code-start: value-type
   :end-before: //!code-end: value-type

Next, we define the element block type for this value type, and the callback
functions for it.  This is the same as for
:cpp:type:`~mdds::multi_type_vector`; see :ref:`custom-value-types` for the
details:

.. literalinclude:: ../../example/multi_type_matrix_custom.cpp
   :language: C++
   :start-after: //!code-start: block-def
   :end-before: //!code-end: block-def

Now we define the matrix traits.  We use the standard double block as the
numeric block, and list the standard boolean and string blocks together with
our error block in ``element_blocks``:

.. literalinclude:: ../../example/multi_type_matrix_custom.cpp
   :language: C++
   :start-after: //!code-start: traits
   :end-before: //!code-end: traits

The following code creates a matrix with 3 rows and 2 columns, and stores
values of all four types.  The :cpp:func:`~mdds::multi_type_matrix::set`
method accepts the numeric type, and the value type of every block listed in
``element_blocks``:

.. literalinclude:: ../../example/multi_type_matrix_custom.cpp
   :language: C++
   :start-after: //!code-start: set
   :end-before: //!code-end: set
   :dedent: 4

Note that the string value is passed as ``std::string``, not as a string
literal.  The value type must exactly match the value type of one of the
listed blocks.

To read a value, check the element type with
:cpp:func:`~mdds::multi_type_matrix::get_type` first, then call
:cpp:func:`~mdds::multi_type_matrix::get` with the value type.  Both methods
have an overload that takes a row and a column, and an overload that takes a
position object.  Each call with a row and a column looks up the element
block that stores the element.  To avoid doing that lookup twice, the
following code gets the position object of the element once with
:cpp:func:`~mdds::multi_type_matrix::position`, and passes it to both
methods:

.. literalinclude:: ../../example/multi_type_matrix_custom.cpp
   :language: C++
   :start-after: //!code-start: get
   :end-before: //!code-end: get
   :dedent: 4

If the element is of a different type,
:cpp:func:`~mdds::multi_type_matrix::get` throws a
:cpp:class:`~mdds::type_error`.  If the element is empty, it returns the
empty value of the requested type.

:cpp:func:`~mdds::multi_type_matrix::get` returns a copy of the value.  To
get a reference to the stored value instead, use
:cpp:func:`~mdds::multi_type_matrix::get_ref`.  It throws a
:cpp:class:`~mdds::type_error` for an empty element, since there is no
stored value to reference.

Finally, the following code walks all element blocks in the matrix, and
prints their values.  It switches on the type of each block, and uses the
block type IDs as the ``case`` labels:

.. literalinclude:: ../../example/multi_type_matrix_custom.cpp
   :language: C++
   :start-after: //!code-start: walk
   :end-before: //!code-end: walk
   :dedent: 4

Running this code generates the following output:

.. code-block:: none

   error code at (0, 1): 7
   numeric: 1.5
   boolean: true
   string: text
   error: 7
   error: 9
   empty: 1 element(s)

The elements are visited in column-major order.  The two error values in the
second column are stored in one block.
