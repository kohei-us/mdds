
Removing segments
=================

So far we have covered how to insert segment values and perform searches, but
you can also remove values from the tree.  There are two ways to remove values:
one is to use an iterator from the search results object, and another is to
specify a match condition predicate and remove all values that the predicate
evaluates to true.

Removing with iterator
----------------------

Let's first cover how to remove a value with an iterator.  Our goal here is to
remove the segment value "A" that extends from 0 to 10.  To obtain an iterator,
you first need to perform a search then get an iterator from the results object.
Once you have an iterator, iterate through the result set until it finds the
right iterator position, then call :cpp:func:`~mdds::segment_tree::erase()` to
remove that value, as the following code illustrates:

.. literalinclude:: ../../example/segment_tree.cpp
   :language: C++
   :start-after: //!code-start: erase-by-iterator
   :end-before: //!code-end: erase-by-iterator
   :dedent: 4

Let's run the same search with the search point of 5:

.. literalinclude:: ../../example/segment_tree.cpp
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
-----------------------

Another way to remove values is to call :cpp:func:`~mdds::segment_tree::erase_if()`
with a predicate that matches the value to be removed.  The following code removes
all the segments that contains 5:

.. literalinclude:: ../../example/segment_tree.cpp
   :language: C++
   :start-after: //!code-start: erase-by-predicate
   :end-before: //!code-end: erase-by-predicate
   :dedent: 4

The predicate function takes three parameters that are start position, end
position, and a value of a segment.  Running this code produces the following
output:

.. code-block:: none

   --
   3 segments have been removed

indicating that a total of 3 segments have been removed with this call.  Running
the same search again after the value removal:

.. literalinclude:: ../../example/segment_tree.cpp
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
