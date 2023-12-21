.. highlight:: cpp

Segment Tree
============

Overview
--------

`Segment tree <https://en.wikipedia.org/wiki/Segment_tree>`_ is a data structure
designed for storing one-dimensional intervals or segments, either overlapping
or non-overlapping.  It is useful for detecting all the segments that contain a
specific point.  Each segment has start and end points where the start point is
inclusive and the end point is not.  The version of segment tree implemented in
mdds allows associating a value with each segment.

Quickstart
----------

In this section we are going to demonstrate how to use
:cpp:class:`~mdds::segment_tree` with some code examples.  First, let's include
some headers:

.. literalinclude:: ../example/segment_tree.cpp
   :language: C++
   :start-after: //!code-start: header
   :end-before: //!code-end: header

The ``mdds/segment_tree.hpp`` header defines the :cpp:class:`mdds::segment_tree`
template class, and we'll use ``std::string`` as its value type.  We also need
to include the ``iostream`` header so that we can print some stuff out.  The
next step then is to define a type alias for the concrete type instance of
:cpp:class:`~mdds::segment_tree`:

.. literalinclude:: ../example/segment_tree.cpp
   :language: C++
   :start-after: //!code-start: type
   :end-before: //!code-end: type

Here, we are using ``long`` as the key type and ``std::string`` as the value type.
Let's create an instance of this and insert some values:

.. literalinclude:: ../example/segment_tree.cpp
   :language: C++
   :start-after: //!code-start: insert
   :end-before: //!code-end: insert
   :dedent: 4

We have created a new instance called ``tree``, and inserted five segment values
into it using the :cpp:func:`~mdds::segment_tree::insert()` method, which takes:

* start position of the segment,
* end position of the segment, and
* the value associated with the segment

as its arguments.  Note that, as mentioned earlier, the start position of a
segment is inclusive but the end position isn't.  Once all the segment data have
been inserted, the next step is to build the tree by simply calling the
:cpp:func:`~mdds::segment_tree::build_tree()` method:

.. literalinclude:: ../example/segment_tree.cpp
   :language: C++
   :start-after: //!code-start: build
   :end-before: //!code-end: build
   :dedent: 4

Hopefully this needs no explanation.  Building the tree is required before you
can perform any queries.  Now that the tree is built, let's run some queries.
But first we are going to define the following function:

.. literalinclude:: ../example/segment_tree.cpp
   :language: C++
   :start-after: //!code-start: search-and-print
   :end-before: //!code-end: search-and-print

This helper function takes the segment tree instance and a search point,
performs a search by calling :cpp:func:`~mdds::segment_tree::search()`, and
prints its results.  The return value from
:cpp:func:`~mdds::segment_tree::search()` is of type
:cpp:class:`~mdds::segment_tree::search_results`, which provides the
:cpp:func:`~mdds::segment_tree::search_results::size()` method to quickly find
the total number of the results.  It also allows you to iterate through the
results through its :cpp:func:`~mdds::segment_tree::search_results::begin()`
:cpp:func:`~mdds::segment_tree::search_results::end()` methods, or simply by plugging
it into a `range-based for loop <https://en.cppreference.com/w/cpp/language/range-for>`_
as you see in this function.

With this help function in place, let's find all the segments that contain 5 by calling:

.. literalinclude:: ../example/segment_tree.cpp
   :language: C++
   :start-after: //!code-start: search-5
   :end-before: //!code-end: search-5
   :dedent: 4

which will print the following output:

.. code-block:: none

   --
   search at 5
   number of results: 3
   range: [2:15); value: B
   range: [0:10); value: A
   range: [5:22); value: D

It's worth noting that the results here don't include the "C" segment, which
extends from -2 to 5, because the end point is not inclusive.  On the other
hand, the results *do* include the "D" segment because the start point *is*
inclusive.

Let's do another search, and this time let's find all the segments that contain 0:

.. literalinclude:: ../example/segment_tree.cpp
   :language: C++
   :start-after: //!code-start: search-0
   :end-before: //!code-end: search-0
   :dedent: 4

This will print the following output:

.. code-block:: none

   --
   search at 0
   number of results: 2
   range: [-2:5); value: C
   range: [0:10); value: A

The results look just about right.  Note that it is entirely safe to call
:cpp:func:`~mdds::segment_tree::search()` with points that are below the minimum
position or above the maximum position in the tree; you will simply get empty
results in such cases.

Removing segments
-----------------

So far we have covered how to insert segment values and perform searches, but
you can also remove values from the tree.  There are two ways to remove values:
one is to use an iterator from the search results object, and another is to
specify a match condition predicate and remove all values that the predicate
evaluates to true.

Removing with iterator
^^^^^^^^^^^^^^^^^^^^^^

Let's first cover how to remove a value with an iterator.  Our goal here is to
remove the segment value "A" that extends from 0 to 10.  To obtain an iterator,
you first need to perform a search then get an iterator from the results object.
Once you have an iterator, iterate through the result set until it finds the
right iterator position, then call :cpp:func:`~mdds::segment_tree::erase()` to
remove that value, as the following code illustrates:

.. literalinclude:: ../example/segment_tree.cpp
   :language: C++
   :start-after: //!code-start: erase-by-iterator
   :end-before: //!code-end: erase-by-iterator
   :dedent: 4

Let's run the same search with the search point of 5:

.. literalinclude:: ../example/segment_tree.cpp
   :language: C++
   :start-after: //!code-start: search-5-after-erase-by-iterator
   :end-before: //!code-end: search-5-after-erase-by-iterator
   :dedent: 4

This time it will produce:

.. code-block:: none

   --
   search at 5
   number of results: 2
   range: [2:15); value: B
   range: [5:22); value: D

As you can see, the "A" segment has been removed.

One thing to note is that removing a value does *not* invalidate the tree
itself; you can continue to perform follow-up searches without having to
re-build the tree.  However, *it does invalidate the iterators*, which
necessitates you to exit your iteration once a value has been removed using an
iterator.  Note that the search results object itself remains valid even after
the value removal.

Even though removing a value does not invalidate the tree, if you remove a large
enough number of values re-building it may reduce the overall size of the tree,
as the size of the tree is dependent upon the number of unique end points of all
the stored segments.  And smaller the tree, the better the search performance.

Removing with predicate
^^^^^^^^^^^^^^^^^^^^^^^

Another way to remove values is to call :cpp:func:`~mdds::segment_tree::erase_if()`
with a predicate that matches the value to be removed.  The following code removes
all the segments that contains 5:

.. literalinclude:: ../example/segment_tree.cpp
   :language: C++
   :start-after: //!code-start: erase-by-predicate
   :end-before: //!code-end: erase-by-predicate
   :dedent: 4

The predicate function takes a struct value of contains ``start``, ``end``, and
``value`` as its data members.  The ``start`` and ``end`` members hold const
references to values of the same type as the
:cpp:type:`~mdds::segment_tree::key_type`, while the ``value`` member holds a
const reference to a value of the same type as the
:cpp:type:`~mdds::segment_tree::value_type`.  Running this code produces the
following output:

.. code-block:: none

   --
   3 segments have been removed

indicating that a total of 3 segments have been removed with this call.  Running
the same search again after the value removal:

.. literalinclude:: ../example/segment_tree.cpp
   :language: C++
   :start-after: //!code-start: search-5-after-erase-by-predicate
   :end-before: //!code-end: search-5-after-erase-by-predicate
   :dedent: 4

yields the following output:

.. code-block:: none

   --
   search at 5
   number of results: 0

indicating that the tree no longer stores segments that contain 5.

API Reference
-------------

.. doxygenclass:: mdds::segment_tree
   :members:

