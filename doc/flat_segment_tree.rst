.. highlight:: cpp

Flat Segment Tree
=================

Code Example
------------

Simple use case
^^^^^^^^^^^^^^^

The following code demonstrates a simple use case of storing non-overlapping
ranged values and performing queries using :cpp:class:`~mdds::flat_segment_tree`::

    #include <mdds/flat_segment_tree.hpp>
    #include <iostream>

    using namespace std;

    using fst_type = mdds::flat_segment_tree<long, int>;

    int main()
    {
        // Define the begin and end points of the whole segment, and the default
        // value.
        fst_type db(0, 500, 0);

        db.insert_front(10, 20, 10);
        db.insert_back(50, 70, 15);
        db.insert_back(60, 65, 5);

        int value = -1;
        long beg = -1, end = -1;

        // Perform linear search.  This doesn't require the tree to be built
        // beforehand.  Note that the begin and end point parameters are optional.
        db.search(15, value, &beg, &end);
        cout << "The value at 15 is " << value << ", and this segment spans from " << beg << " to " << end << endl;;

        // Don't forget to build tree before calling search_tree().
        db.build_tree();

        // Perform tree search.  Tree search is generally a lot faster than linear
        // search, but requires the tree to be built beforehand.
        db.search_tree(62, value, &beg, &end);
        cout << "The value at 62 is " << value << ", and this segment spans from " << beg << " to " << end << endl;;
    }

Let's walk through this code step-by-step.  The first step is to declare the
instance::

    // Define the begin and end points of the whole segment, and the default
    // value.
    fst_type db(0, 500, 0);

Here, the first and second arguments specify the lower and upper boundaries of
the whole segment.  The third argument specifies the value for the empty
segments.  What this line does is to create a new instance and initializes it
with one initial segment ranging from 0 to 500 with a value of 0.

.. figure:: _static/images/fst_example1_initial.png
   :align: center

The following lines insert two new segments into this structure::

    db.insert_front(10, 20, 10);
    db.insert_back(50, 70, 15);

The first line inserts a segment ranging from 10 to 20 with a value of 10, and
the second line from 50 to 70 with a value of 15.

.. figure:: _static/images/fst_example1_insert1.png
   :align: center

You can insert a new segment either via :cpp:func:`~mdds::flat_segment_tree::insert_front`
or :cpp:func:`~mdds::flat_segment_tree::insert_back`.  The end result will be
the same regardless of which method you use; the difference is that
:cpp:func:`~mdds::flat_segment_tree::insert_front` begins its search for
the insertion point from the first node associated with the minimum key value,
whereas :cpp:func:`~mdds::flat_segment_tree::insert_back` starts its search
from the last node associated with the maximum key value.

The next line inserts another segment ranging from 60 to 65 having a value of
5::

    db.insert_back(60, 65, 5);

As this new segment overlaps with the existing segment of 50 to 70, it will
cut into a middle part of that segment to make room for itself.

.. figure:: _static/images/fst_example1_insert2.png
   :align: center

The next part queries the value associated with a key value of 15 via
:cpp:func:`~mdds::flat_segment_tree::search`::

    int value = -1;
    long beg = -1, end = -1;

    // Perform linear search.  This doesn't require the tree to be built
    // beforehand.  Note that the begin and end point parameters are optional.
    db.search(15, value, &beg, &end);
    cout << "The value at 15 is " << value << ", and this segment spans from " << beg << " to " << end << endl;;

When executing this code, you will see the following output:

.. code-block:: none

    The value at 15 is 10, and this segment spans from 10 to 20

One thing to note is that the :cpp:func:`~mdds::flat_segment_tree::search`
method performs a linear search which involves traversing only through
the leaf nodes of the structure in order to find the target segment.  As such,
the worst-case lookup performance is directly proportional to the number of
linear nodes.

There is another way to perform the query with better worse-case performance,
that is through :cpp:func:`~mdds::flat_segment_tree::search_tree` as seen in
the following code::

    // Don't forget to build tree before calling search_tree().
    db.build_tree();

    // Perform tree search.  Tree search is generally a lot faster than linear
    // search, but requires the tree to be built beforehand.
    db.search_tree(62, value, &beg, &end);
    cout << "The value at 62 is " << value << ", and this segment spans from " << beg << " to " << end << endl;;

The signature of the :cpp:func:`~mdds::flat_segment_tree::search_tree` method
is identical to that of the :cpp:func:`~mdds::flat_segment_tree::search` method
except for the name.  This code generate the following output:

.. code-block:: none

    The value at 62 is 5, and this segment spans from 60 to 65

Query via :cpp:func:`~mdds::flat_segment_tree::search_tree` generally performs
better since it traverses through the search tree to find the target segment.
But it does require the search tree to be built ahead of time by calling
:cpp:func:`~mdds::flat_segment_tree::build_tree`.

API Reference
-------------

.. doxygenclass:: mdds::flat_segment_tree
   :members:
