
Quick start
===========

In this section we are going to demonstrate how to use
:cpp:class:`~mdds::segment_tree` with some code examples.  First, let's include
some headers:

.. literalinclude:: ../../example/segment_tree.cpp
   :language: C++
   :start-after: //!code-start: header
   :end-before: //!code-end: header

The ``<mdds/segment_tree.hpp>`` header defines the
:cpp:class:`mdds::segment_tree` template class, and we need to include the
``<string>`` so that we can use ``std::string`` as its value type.  We also need
the ``<iostream>`` header to write some outputs.  The next step is to define a
type alias for the concrete type instance of :cpp:class:`~mdds::segment_tree`:

.. literalinclude:: ../../example/segment_tree.cpp
   :language: C++
   :start-after: //!code-start: type
   :end-before: //!code-end: type

Here, we are using ``long`` as the key type and ``std::string`` as the value type.
Let's create an instance of this type and insert some segments with values:

.. literalinclude:: ../../example/segment_tree.cpp
   :language: C++
   :start-after: //!code-start: insert
   :end-before: //!code-end: insert
   :dedent: 4

We have created a new instance called ``tree``, and inserted five segment values
into it using the :cpp:func:`~mdds::segment_tree::insert()` method, which takes:

* start position of the segment,
* end position of the segment, and
* the value associated with the segment

as its arguments.  As mentioned earlier, the start position of a segment is
inclusive but the end position isn't.  Once all the segment data have been
inserted, the next step is to build the tree by simply calling the
:cpp:func:`~mdds::segment_tree::build_tree()` method:

.. literalinclude:: ../../example/segment_tree.cpp
   :language: C++
   :start-after: //!code-start: build
   :end-before: //!code-end: build
   :dedent: 4

Building the tree is required before you can perform any queries.  Now that the
tree is built, let's run some queries.  But first we are going to define the
following helper function:

.. literalinclude:: ../../example/segment_tree.cpp
   :language: C++
   :start-after: //!code-start: search-and-print
   :end-before: //!code-end: search-and-print

This helper function takes the segment tree instance and a search point,
performs a search by calling :cpp:func:`~mdds::segment_tree::search()`, and
iterates through and prints its results.  The return value from
:cpp:func:`~mdds::segment_tree::search()` is of type
:cpp:class:`~mdds::segment_tree::search_results`, which provides the
:cpp:func:`~mdds::segment_tree::search_results::size()` method to quickly find
the total number of the results.  It also allows you to iterate through the
results through its :cpp:func:`~mdds::segment_tree::search_results::begin()`
:cpp:func:`~mdds::segment_tree::search_results::end()` methods, or simply by
plugging it into a `range-based for loop
<https://en.cppreference.com/w/cpp/language/range-for>`_ as you see in this
function.

With this help function in place, let's find all the segments that contain 5 by
calling:

.. literalinclude:: ../../example/segment_tree.cpp
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

It's worth pointing out that the results here don't include the "C" segment,
which extends from -2 to 5, because the end point is not inclusive.  Given a
search point of ``x``, For a segment to be included in the results, it must
satisfy ``start <= x < end``.  On the other hand, the results *do* include the
"D" segment because the start point *is* inclusive.

Let's do another search, and this time let's find all the segments that contain
0:

.. literalinclude:: ../../example/segment_tree.cpp
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

The results look just about right.  As an aside, it is entirely safe to call
:cpp:func:`~mdds::segment_tree::search()` with points that are below the minimum
position or above the maximum position in the tree; you will simply get empty
results in such cases.
