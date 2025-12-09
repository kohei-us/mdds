.. SPDX-FileCopyrightText: 2025 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

Iterate through stored segments
===============================

:cpp:class:`~mdds::flat_segment_tree` supports two types of iterators to allow
you to iterate through the segments stored in your tree.  The first way is to
iterate through the individual leaf nodes one at a time by using
:cpp:func:`~mdds::flat_segment_tree::begin` and :cpp:func:`~mdds::flat_segment_tree::end`:

.. literalinclude:: ../../example/flat_segment_tree_itrs.cpp
   :language: C++
   :start-after: //!code-start: iterate-nodes
   :end-before: //!code-end: iterate-nodes
   :dedent: 4

Each iterator value contains a pair of two values named ``first`` and ``second``,
with the first one being the key of the segment that the node initiates, and the
second one being the value associated with that segment.  When executing this
code with the tree from the example code above, you'll get the following output:

.. code-block:: none

    key: 0; value: 0
    key: 10; value: 10
    key: 20; value: 0
    key: 50; value: 15
    key: 60; value: 5
    key: 65; value: 15
    key: 70; value: 0
    key: 500; value: 0

Each node stores the start key and the value of the segment it initiates, and
the key stored in each node is also the end key of the segment that the
previous node initiates except for the first node.  Note that the value stored
in the last node is currently not used.  It is set to be the zero value of the
value type, but this may change in the future.

One thing to keep in mind is that :cpp:class:`~mdds::flat_segment_tree` does
not support mutable iterators that let you modify the stored keys or values.

.. note::

   :cpp:class:`~mdds::flat_segment_tree` does not support mutable iterators;
   you can only traverse the values in a read-only fashion.

You can also use range-based for loop to iterate through the leaf nodes in a
similar fashion:

.. literalinclude:: ../../example/flat_segment_tree_itrs.cpp
   :language: C++
   :start-after: //!code-start: loop-nodes
   :end-before: //!code-end: loop-nodes
   :dedent: 4

The output from this code is identical to that from the previous one.

Now, one major inconvenience of navigating through the individual leaf nodes
is that you need to manually keep track of the start and end points of each
segment if you need to operate on the segments rather than the nodes that
comprise the segments.  The good news is that
:cpp:class:`~mdds::flat_segment_tree` does provide a way to iterate through
the segments directly as the following code demonstrates:

.. literalinclude:: ../../example/flat_segment_tree_itrs.cpp
   :language: C++
   :start-after: //!code-start: iterate-segments
   :end-before: //!code-end: iterate-segments
   :dedent: 4

This code uses :cpp:func:`~mdds::flat_segment_tree::begin_segment` and
:cpp:func:`~mdds::flat_segment_tree::end_segment` to iterate through one
segment at a time with the value of each iterator containing ``start``,
``end`` and ``value`` members that correspond with the start key, end key and
the value of the segment, respectively.  Running this code produces the
following output:

.. code-block:: none

    start: 0; end: 10; value: 0
    start: 10; end: 20; value: 10
    start: 20; end: 50; value: 0
    start: 50; end: 60; value: 15
    start: 60; end: 65; value: 5
    start: 65; end: 70; value: 15
    start: 70; end: 500; value: 0

It's also possible to iterate through the segments in a range-based for loop, by
calling :cpp:func:`~mdds::flat_segment_tree::segment_range()`:

.. literalinclude:: ../../example/flat_segment_tree_itrs.cpp
   :language: C++
   :start-after: //!code-start: loop-segments
   :end-before: //!code-end: loop-segments
   :dedent: 4

This code should generate output identical to that of the previous code.
