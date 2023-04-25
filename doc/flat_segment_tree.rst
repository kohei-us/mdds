.. highlight:: cpp

Flat Segment Tree
=================

Overview
--------

Flat segment tree is a derivative of `segment tree
<https://en.wikipedia.org/wiki/Segment_tree>`_, and is designed to store
non-overlapping 1-dimensional range values such that *the values of the
neighboring ranges are guaranteed to be different.*  An insertion of a range
value into this structure will always overwrite one or more existing ranges
that overlap with the new range.  If an insertion of a new range would cause
any adjacent ranges to have the equal value, those ranges will be merged into
one range.

An instance of this structure is initialized with fixed lower and upper
bounaries, which will not change throughout the life time of the instance.

The flat segment tree structure consists of two parts: the leaf-node part
which also forms a doubly-linked list, and the non-leaf-node part which forms
a balanced-binary tree and is used only when performing tree-based queries.
The range values are stored in the leaf-nodes, while the non-leaf nodes are
used only for queries.


Quick start
-----------

This section demonstrates a simple use case of storing non-overlapping ranged
values and performing queries using :cpp:class:`~mdds::flat_segment_tree`.

First, we need to instantiate a concrete type from the template:

.. literalinclude:: ../example/flat_segment_tree.cpp
   :language: C++
   :start-after: //!code-start: type
   :end-before: //!code-end: type
   :dedent: 4

then create an instance of this type:

.. literalinclude:: ../example/flat_segment_tree.cpp
   :language: C++
   :start-after: //!code-start: instance
   :end-before: //!code-end: instance
   :dedent: 4

Here, the first and second arguments specify the lower and upper boundaries of
the whole segment.  The third argument specifies the value for the empty
segments.  What this line does is to create a new instance and initializes it
with one initial segment ranging from 0 to 500 with a value of 0:

.. figure:: _static/images/fst-example1-initial.svg
   :align: center

   The new instance is initialized with an initial segment.

Internally, this initial range is represented by two leaf nodes, with the
first one storing the start key and the value for the segment both of which
happen to be 0 in this example, and the second one storing the end key of 500.
Note that the end key of a segment is not inclusive.

The following lines insert two new segments into this structure:

.. literalinclude:: ../example/flat_segment_tree.cpp
   :language: C++
   :start-after: //!code-start: insert-1
   :end-before: //!code-end: insert-1
   :dedent: 4

The first line inserts a segment ranging from 10 to 20 with a value of 10, and
the second line from 50 to 70 with a value of 15:

.. figure:: _static/images/fst-example1-insert1.svg
   :align: center

   Two new segments have been inserted.

You can insert a new segment either via :cpp:func:`~mdds::flat_segment_tree::insert_front`
or :cpp:func:`~mdds::flat_segment_tree::insert_back`.  The end result will be
the same regardless of which method you use; the difference is that
:cpp:func:`~mdds::flat_segment_tree::insert_front` begins its search for
the insertion point from the first node associated with the minimum key value,
whereas :cpp:func:`~mdds::flat_segment_tree::insert_back` starts its search
from the last node associated with the maximum key value.

After the insertions, the tree now contains a total of six leaf nodes to
represent all stored segments.  Note that one leaf node typically represents
both the end of a segment and the start of the adjacent segment that comes
after it, unless it's either the first or the last node.

The next line inserts another segment ranging from 60 to 65 having a value of
5:

.. literalinclude:: ../example/flat_segment_tree.cpp
   :language: C++
   :start-after: //!code-start: insert-2
   :end-before: //!code-end: insert-2
   :dedent: 4

As this new segment overlaps with the existing segment of 50 to 70, it will
cut into a middle part of that segment to make room for the new segment.  At
this point, the tree contains a total of eight leaf nodes representing seven
segments:

.. figure:: _static/images/fst-example1-insert2.svg
   :align: center

   A new segment has been inserted that overlaps an existing non-empty segment.

Next, we are going to query the value associated with a key of 15 via
:cpp:func:`~mdds::flat_segment_tree::search`:

.. literalinclude:: ../example/flat_segment_tree.cpp
   :language: C++
   :start-after: //!code-start: linear-search
   :end-before: //!code-end: linear-search
   :dedent: 4

Executing this code will yield the following output:

.. code-block:: none

    The value at 15 is 10, and this segment spans from 10 to 20

One thing to note is that the :cpp:func:`~mdds::flat_segment_tree::search`
method performs a linear search which involves traversing only through the
leaf nodes in this data structure in order to find the target segment.  As
such, the worst-case lookup performance is directly proportional to the number
of leaf nodes.

There is another way to perform the query with better worse-case performance,
that is through :cpp:func:`~mdds::flat_segment_tree::search_tree` as seen in
the following code:

.. literalinclude:: ../example/flat_segment_tree.cpp
   :language: C++
   :start-after: //!code-start: tree-search
   :end-before: //!code-end: tree-search
   :dedent: 4

The signature of the :cpp:func:`~mdds::flat_segment_tree::search_tree` method
is identical to that of the :cpp:func:`~mdds::flat_segment_tree::search` method
except for the name.  This code generates the following output:

.. code-block:: none

    The value at 62 is 5, and this segment spans from 60 to 65

Query via :cpp:func:`~mdds::flat_segment_tree::search_tree` generally performs
better since it traverses through the search tree to find the target segment.
But it does require the search tree to be built ahead of time by calling
:cpp:func:`~mdds::flat_segment_tree::build_tree`.  Please be aware that if the
segments have been modified after the tree was last built, you will have to rebuild
the tree by calling :cpp:func:`~mdds::flat_segment_tree::build_tree`.

.. warning::

   You need to build the tree by calling :cpp:func:`~mdds::flat_segment_tree::build_tree`
   before performing a tree-based search via :cpp:func:`~mdds::flat_segment_tree::search_tree`.
   If the segments have been modified after the tree was last built, you will have to
   rebuild the tree by calling :cpp:func:`~mdds::flat_segment_tree::build_tree` again.


Iterate through stored segments
-------------------------------

:cpp:class:`~mdds::flat_segment_tree` supports two types of iterators to allow
you to iterate through the segments stored in your tree.  The first way is to
iterate through the individual leaf nodes one at a time by using
:cpp:func:`~mdds::flat_segment_tree::begin` and :cpp:func:`~mdds::flat_segment_tree::end`:

.. literalinclude:: ../example/flat_segment_tree_itrs.cpp
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

.. literalinclude:: ../example/flat_segment_tree_itrs.cpp
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

.. literalinclude:: ../example/flat_segment_tree_itrs.cpp
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

.. literalinclude:: ../example/flat_segment_tree_itrs.cpp
   :language: C++
   :start-after: //!code-start: loop-segments
   :end-before: //!code-end: loop-segments
   :dedent: 4

This code should generate output identical to that of the previous code.


API Reference
-------------

.. doxygenclass:: mdds::flat_segment_tree
   :members:
