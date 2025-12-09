.. SPDX-FileCopyrightText: 2025 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

Quick start
===========

This section demonstrates a simple use case of storing non-overlapping ranged
values and performing queries using :cpp:class:`~mdds::flat_segment_tree`.

First, we need to instantiate a concrete type from the template:

.. literalinclude:: ../../example/flat_segment_tree.cpp
   :language: C++
   :start-after: //!code-start: type
   :end-before: //!code-end: type
   :dedent: 4

then create an instance of this type:

.. literalinclude:: ../../example/flat_segment_tree.cpp
   :language: C++
   :start-after: //!code-start: instance
   :end-before: //!code-end: instance
   :dedent: 4

Here, the first and second arguments specify the lower and upper boundaries of
the whole segment.  The third argument specifies the value for the empty
segments.  What this line does is to create a new instance and initializes it
with one initial segment ranging from 0 to 500 with a value of 0:

.. figure:: ../_static/images/fst-example1-initial.svg
   :align: center

   The new instance is initialized with an initial segment.

Internally, this initial range is represented by two leaf nodes, with the
first one storing the start key and the value for the segment both of which
happen to be 0 in this example, and the second one storing the end key of 500.
Note that the end key of a segment is not inclusive.

The following lines insert two new segments into this structure:

.. literalinclude:: ../../example/flat_segment_tree.cpp
   :language: C++
   :start-after: //!code-start: insert-1
   :end-before: //!code-end: insert-1
   :dedent: 4

The first line inserts a segment ranging from 10 to 20 with a value of 10, and
the second line from 50 to 70 with a value of 15:

.. figure:: ../_static/images/fst-example1-insert1.svg
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

.. literalinclude:: ../../example/flat_segment_tree.cpp
   :language: C++
   :start-after: //!code-start: insert-2
   :end-before: //!code-end: insert-2
   :dedent: 4

As this new segment overlaps with the existing segment of 50 to 70, it will
cut into a middle part of that segment to make room for the new segment.  At
this point, the tree contains a total of eight leaf nodes representing seven
segments:

.. figure:: ../_static/images/fst-example1-insert2.svg
   :align: center

   A new segment has been inserted that overlaps an existing non-empty segment.

Next, we are going to query the value associated with a key of 15 via
:cpp:func:`~mdds::flat_segment_tree::search`:

.. literalinclude:: ../../example/flat_segment_tree.cpp
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

.. literalinclude:: ../../example/flat_segment_tree.cpp
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
