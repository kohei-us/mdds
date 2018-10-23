
.. highlight:: cpp

R-tree
======

Overview
--------

`R-tree <https://en.wikipedia.org/wiki/R-tree>`_ is a tree-based data
structure designed for optimal query performance on multi-dimensional spatial
objects with rectangular bounding shapes.  The R-tree implementation included
in this library is a variant of R-tree known as `R*-tree
<https://en.wikipedia.org/wiki/R*_tree>`_ which differs from the original
R-tree in that it may re-insert an object if it causes the original target
directory to overflow in order to keep the overall tree more balanced.

Our implementation of R-tree theoretically supports any number of dimensions
although certain functionalities, especially those related to visualization,
are only supported for 2-dimensional cases.


Code Example
------------

Quick Start
^^^^^^^^^^^

Let's go through a very simple example to demonstrate how to use
:cpp:class:`~mdds::rtree`.  First, you need to specify a concrete type by
specifying the key type and value type to use::

    #include <mdds/rtree.hpp>

    #include <string>
    #include <iostream>

    // key values are of type double, and we are storing std::string as a
    // value for each spatial object.  By default, tree becomes 2-dimensional
    // object store unless otherwise specified.
    using rt_type = mdds::rtree<double, std::string>;

You'll only need to specify the types of key and value here unless you want to
customize other properties of :cpp:class:`~mdds::rtree` including the number
of dimensions.  By default, :cpp:class:`~mdds::rtree` sets the number of
dimensions to 2.

::

    rt_type tree;

Instantiating an rtree instance should be no brainer as it requires no input
parameters.  Now, let's insert some data::

    tree.insert({{0.0, 0.0}, {15.0, 20.0}}, "first rectangle data");

This inserts a string value associated with a bounding rectangle of (0, 0) -
(15, 20).  Note that in the above code we are passing the bounding rectangle
parameter to rtree's :cpp:func:`~mdds::rtree::insert` method as a nested
initializer list, which implicitly gets converted to
:cpp:class:`~mdds::rtree::extent_type`.  You can also use the underlying type
directly as follows::

    rt_type::extent_type bounds({-2.0, -1.0}, {1.0, 2.0});
    std::cout << "inserting value for " << bounds.to_string() << std::endl;
    tree.insert(bounds, "second rectangle data");

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
extent directly::

    bounds.start.d[0] = -1.0; // Change the first dimension value of the start rectangle point.
    bounds.end.d[1] += 1.0; // Increment the second dimension value of the end rectangle point.
    std::cout << "inserting value for " << bounds.to_string() << std::endl;
    tree.insert(bounds, "third rectangle data");

This code will insert a string value associated with a rectangle of (-1, -1)
to (1, 3), and will generate the following output:

.. code-block:: none

    inserting value for (-1, -1) - (1, 3)

So far we have only inserted data associated with rectangle shapes, but
:cpp:class:`~mdds::rtree` also allows data associated with points to co-exist
in the same tree.  The following code inserts a string value associated with a
point (5, 6)::

    tree.insert({5.0, 6.0}, "first point data");

Like the verfy first rectangle data we've inserted, we are passing the point
data as an initializer list of two elements (for 2-dimensional data storage),
which will implicitly get converted to :cpp:class:`~mdds::rtree::point_type`
before it enters into the call.

Now that some data have been inserted, it's time to run some queries.  Let's
query all objects that overlap with a certain rectangular region either
partially or fully.  The following code will do just that::

    // Search for all objects that overlap with a (4, 4) - (7, 7) rectangle.
    auto results = tree.search({{4.0, 4.0}, {7.0, 7.0}}, rt_type::search_type::overlap);

    for (const std::string& v : results)
        std::cout << "value: " << v << std::endl;

In this query, we are specifying the search region to be (4, 4) to (7, 7)
which should overlap with the first rectangle data and the first point data.
Indeed, when you execute this code, you will see the following output:

.. code-block:: none

    value: first rectangle data
    value: first point data

indicating that the query region does overlap with two of the stored values

Note that the :cpp:func:`~mdds::rtree::search` method takes exactly two
arguments; the first one specifies the search region while the second two
specifies the type of search to be performed.  In the above call we passed
:cpp:type:`~mdds::detail::rtree::search_type`'s ``overlap`` enum value which
picks up all values whose bounding rectangles overlap with the search region
either partially or fully.

Sometimes, however, you may need to find a value whose bounding rectangle
matches exactly the search region you specify in your query.  You can achieve
that by setting the search type to ``match``.

Here is an example::

    // Search for all objects whose bounding rectangles are exactly (4, 4) - (7, 7).
    auto results = tree.search({{4.0, 4.0}, {7.0, 7.0}}, rt_type::search_type::match);
    std::cout << "number of results: " << std::distance(results.begin(), results.end()) << std::endl;

The search region is identical to that of the previous example, but the search
type is set to ``match`` instead.  Then the next line will count the number of
results and print it out.  The output you will see is as follows:

.. code-block:: none

    number of results: 0

indicating that the results are empty.  That is expected since none of the
objects stored in the tree have an exact bounding rectangle of (4, 4) - (7,
7).  When you change the search region to (0, 0) - (15, 20), however, you'll
get one object back.  Here is the actual code::

    // Search for all objects whose bounding rectangles are exactly (0, 0) - (15, 20).
    auto results = tree.search({{0.0, 0.0}, {15.0, 20.0}}, rt_type::search_type::match);
    std::cout << "number of results: " << std::distance(results.begin(), results.end()) << std::endl;

which is identical to the previous one except for the search resion.  This is
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
this line::

    std::cout << "value: " << *results.begin() << std::endl;

which immediately comes after the previous search will output:

.. code-block:: none

    value: first rectangle data



API Reference
-------------

.. doxygenclass:: mdds::rtree
   :members:

.. doxygenstruct:: mdds::detail::rtree::default_rtree_trait
   :members:

.. doxygenstruct:: mdds::detail::rtree::integrity_check_properties
   :members:

.. doxygenenum:: mdds::detail::rtree::export_tree_type
   :project: mdds

.. doxygenenum:: mdds::detail::rtree::search_type
   :project: mdds
