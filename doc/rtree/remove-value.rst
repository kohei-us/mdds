
.. highlight:: cpp

Removing a value from tree
==========================

Removing an existing value from the tree first requires you to perform a search to
obtain search results, then from the search results get an iterator and advance
it to the position of the value you want to remove.  Once you have your iterator
set to the right position, pass it to the :cpp:func:`~mdds::rtree::erase` method
to remove that value.

Note that you can only remove one value at a time, and the iterator becomes
invalid each time you call the :cpp:func:`~mdds::rtree::erase` method to
remove a value.

.. note::

   The iterator becomes invalid each time you call the :cpp:func:`~mdds::rtree::erase`
   method to remove a value.

Here is a contrived example to demonstrate how erasing a value works:

.. literalinclude:: ../../example/rtree_erase.cpp
   :language: C++
   :start-after: //!code-start: remove-value
   :end-before: //!code-end: remove-value
   :dedent: 4

In this code, we are intentionally putting 5 values to the same 2-dimensional
point (1, 1), then removing one of them based on matching criteria (of being
equal to "C").

Compiling and running this code will generate the following output:

.. code-block:: none

    A
    B
    C
    D
    E
    'C' has been erased.
    A
    B
    D
    E

which clearly shows that the 'C' has been successfully erased.
