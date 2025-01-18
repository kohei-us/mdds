
Quick start
===========

Let's go through a very simple example to demonstrate how to use
:cpp:class:`~mdds::rtree`.  First, you need to specify a concrete type by
specifying the key type and value type to use:

.. literalinclude:: ../../example/rtree_simple.cpp
   :language: C++
   :start-after: //!code-start: type
   :end-before: //!code-end: type

You'll only need to specify the types of key and value here unless you want to
customize other properties of :cpp:class:`~mdds::rtree` including the number
of dimensions.  By default, :cpp:class:`~mdds::rtree` sets the number of
dimensions to 2.

.. literalinclude:: ../../example/rtree_simple.cpp
   :language: C++
   :start-after: //!code-start: instantiate
   :end-before: //!code-end: instantiate
   :dedent: 4

Instantiating an rtree instance should be no brainer as it requires no input
parameters.  Now, let's insert some data:

.. literalinclude:: ../../example/rtree_simple.cpp
   :language: C++
   :start-after: //!code-start: insert-1
   :end-before: //!code-end: insert-1
   :dedent: 4

This inserts a string value associated with a bounding rectangle of (0, 0) -
(15, 20).  Note that in the above code we are passing the bounding rectangle
parameter to rtree's :cpp:func:`~mdds::rtree::insert` method as a nested
initializer list, which implicitly gets converted to
:cpp:class:`~mdds::rtree::extent_type`.  You can also use the underlying type
directly as follows:

.. literalinclude:: ../../example/rtree_simple.cpp
   :language: C++
   :start-after: //!code-start: insert-2
   :end-before: //!code-end: insert-2
   :dedent: 4

which inserts a string value associated with a bounding rectangle of (-2, -1)
to (1, 2).  You may have noticed that this code also uses extent_type's
:cpp:func:`~mdds::rtree::extent_type::to_string` method which returns a string
representation of the bounding rectangle.  This may come in handy when
debugging your code.  This method should work as long as the key type used in
your rtree class overloads ``std::ostream``'s ``<<`` operator function.

Running this code will generate the following output:

.. code-block:: none

    inserting value for (-2, -1) - (1, 2)

As :cpp:class:`~mdds::rtree::extent_type` consists of two members called
``start`` and ``end`` both of which are of type
:cpp:class:`~mdds::rtree::point_type`, which in turn contains an array of keys
called ``d`` whose size equals the number of dimensions, you can modify the
extent directly:

.. literalinclude:: ../../example/rtree_simple.cpp
   :language: C++
   :start-after: //!code-start: insert-3
   :end-before: //!code-end: insert-3
   :dedent: 4

This code will insert a string value associated with a rectangle of (-1, -1)
to (1, 3), and will generate the following output:

.. code-block:: none

    inserting value for (-1, -1) - (1, 3)

So far we have only inserted data associated with rectangle shapes, but
:cpp:class:`~mdds::rtree` also allows data associated with points to co-exist
in the same tree.  The following code inserts a string value associated with a
point (5, 6):

.. literalinclude:: ../../example/rtree_simple.cpp
   :language: C++
   :start-after: //!code-start: insert-pt-1
   :end-before: //!code-end: insert-pt-1
   :dedent: 4

Like the very first rectangle data we inserted, we are passing the point data as
an initializer list of two elements (for 2-dimensional data storage), which will
implicitly get converted to :cpp:class:`~mdds::rtree::point_type` before it
enters into the call.

Now that some data have been inserted, it's time to run some queries.  Let's
query all objects that overlap with a certain rectangular region either
partially or fully.  The following code will do just that:

.. literalinclude:: ../../example/rtree_simple.cpp
   :language: C++
   :start-after: //!code-start: search-overlap
   :end-before: //!code-end: search-overlap
   :dedent: 8

In this query, we are specifying the search region to be (4, 4) to (7, 7)
which should overlap with the first rectangle data and the first point data.
Indeed, when you execute this code, you will see the following output:

.. code-block:: none

    value: first rectangle data
    value: first point data

indicating that the query region does overlap with two of the stored values.

Note that the :cpp:func:`~mdds::rtree::search` method takes exactly two
arguments; the first one specifies the search region while the second two
specifies the type of search to be performed.  In the above call we passed
:cpp:type:`~mdds::detail::rtree::search_type`'s ``overlap`` enum value which
picks up all values whose bounding rectangles overlap with the search region
either partially or fully.

Sometimes, however, you may need to find a value whose bounding rectangle
matches exactly the search region you specify in your query.  You can achieve
that by setting the search type to ``match``.

Here is an example:

.. literalinclude:: ../../example/rtree_simple.cpp
   :language: C++
   :start-after: //!code-start: search-match-1
   :end-before: //!code-end: search-match-1
   :dedent: 8

The search region is identical to that of the previous example, but the search
type is set to ``match`` instead.  Then the next line will count the number of
results and print it out.  The output you will see is as follows:

.. code-block:: none

    number of results: 0

indicating that the results are empty.  That is expected since none of the
objects stored in the tree have an exact bounding rectangle of (4, 4) - (7,
7).  When you change the search region to (0, 0) - (15, 20), however, you'll
get one object back.  Here is the actual code:

.. literalinclude:: ../../example/rtree_simple.cpp
   :language: C++
   :start-after: //!code-start: search-match-2
   :end-before: //!code-end: search-match-2
   :dedent: 8

which is identical to the previous one except for the search region.  This is
its output:

.. code-block:: none

    number of results: 1

indicating that it has found exactly one object whose bounding rectangle
exactly matches the search region.

It's worth mentioning that :cpp:class:`~mdds::rtree` supports storage of
multiple objects with identical bounding rectangle.  As such, searching with
the search type of ``match`` can return more than one result.

As you may have noticed in these example codes, the
:cpp:class:`~mdds::rtree::search_results` object does provide
:cpp:func:`~mdds::rtree::search_results::begin` and
:cpp:func:`~mdds::rtree::search_results::end` methods that return standard
iterators which you can plug into various iterator algorithms from the STL.
Dereferencing the iterator will return a reference to the stored value i.e.
this line:

.. literalinclude:: ../../example/rtree_simple.cpp
   :language: C++
   :start-after: //!code-start: iterator-deref
   :end-before: //!code-end: iterator-deref
   :dedent: 8

which immediately follows the previous code block will output:

.. code-block:: none

    value: first rectangle data

In addition to accessing the value that the iterator references, you can also
query from the same iterator object the bounding rectangle associated with the
value as well as its depth in the tree by calling its
:cpp:func:`~mdds::rtree::iterator_base::extent` and
:cpp:func:`~mdds::rtree::iterator_base::depth` methods, respectively:

.. literalinclude:: ../../example/rtree_simple.cpp
   :language: C++
   :start-after: //!code-start: iterator-attrs
   :end-before: //!code-end: iterator-attrs
   :dedent: 8

Running this code will produce the following output:

.. code-block:: none

    value: first rectangle data
    extent: (0, 0) - (15, 20)
    depth: 1

This depth value represents the distance of the node that stores the value
from the root node of the tree, and is technically 0-based.  However, you will
never see a depth of 0 in the search results since the root node of a R-tree
is always a directory node, and a directory node only stores other child nodes
and never stores a value, hence it never appears in the search results.
