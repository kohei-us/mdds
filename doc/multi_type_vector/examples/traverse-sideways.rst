
Traverse multiple multi_type_vector instances "sideways"
========================================================

In this section we will demonstrate a way to traverse multiple instances of
:cpp:type:`~mdds::multi_type_vector` "sideways" using the
:cpp:class:`mdds::mtv::collection` class.  What this class does is to wrap
multiple instances of :cpp:type:`~mdds::multi_type_vector` and generate
iterators that let you iterate the individual element values collectively in
the direction orthogonal to the direction of the individual vector instances.

The best way to explain this feature is to use a spreadsheet analogy.  Let's
say we are implementing a data store to store a 2-dimensional tabular data
where each cell in the data set is associated with row and column indices.
Each cell may store a value of string type, integer type, numeric type, etc.
And let's say that the data looks like the following spreadsheet data:

.. figure:: ../../_static/images/mtv_collection_sheet.png
   :align: center

It consists of five columns, with each column storing 21 rows of data.  The
first row is a header row, followed by 20 rows of values.  In this example, We
will be using one :cpp:type:`~mdds::multi_type_vector` instance for each
column thus creating five instances in total, and store them in a
``std::vector`` container.

The declaration of the data store will look like this:

.. literalinclude:: ../../../example/multi_type_vector/mtv_collection.cpp
   :language: C++
   :start-after: //!code-start: declare
   :end-before: //!code-end: declare
   :dedent: 4

The first two lines specify the concrete :cpp:type:`~mdds::multi_type_vector`
type used for each individual column and the collection type that wraps the
columns.  The third line instantiates the ``std::vector`` instance to store
the columns, and we are setting its size to five to accommodate for five
columns.  We will make use of the collection_type later in this example after
the columns have been populated.

Now, we need to populate the columns with values.  First, we are setting the
header row:

.. literalinclude:: ../../../example/multi_type_vector/mtv_collection.cpp
   :language: C++
   :start-after: //!code-start: header-row
   :end-before: //!code-end: header-row
   :dedent: 4

We are then filling each column individually from column 1 through column 5.
First up is column 1:

.. literalinclude:: ../../../example/multi_type_vector/mtv_collection.cpp
   :language: C++
   :start-after: //!code-start: column-1
   :end-before: //!code-end: column-1
   :dedent: 4

Hopefully this code is straight-forward.  It initializes an array of values
and push them to the column one at a time via
:cpp:func:`~mdds::mtv::soa::multi_type_vector::push_back`.  Next up is column 2:

.. literalinclude:: ../../../example/multi_type_vector/mtv_collection.cpp
   :language: C++
   :start-after: //!code-start: column-2
   :end-before: //!code-end: column-2
   :dedent: 4

This is similar to the code for column 1, except that because we are using an
array of string literals which implicitly becomes an initializer list of type
``const char*``, we need to explicitly specify the type for the
:cpp:func:`~mdds::mtv::soa::multi_type_vector::push_back` call to be ``std::string``.

The code for column 3 is very similar to this:

.. literalinclude:: ../../../example/multi_type_vector/mtv_collection.cpp
   :language: C++
   :start-after: //!code-start: column-3
   :end-before: //!code-end: column-3
   :dedent: 4

Populating column 4 needs slight pre-processing.  We are inserting a string
value of "unknown" in lieu of an integer value of -1.  Therefore the following
code will do:

.. literalinclude:: ../../../example/multi_type_vector/mtv_collection.cpp
   :language: C++
   :start-after: //!code-start: column-4
   :end-before: //!code-end: column-4
   :dedent: 4

Finally, the last column to fill, which uses the same logic as for columns 2
and 3:

.. literalinclude:: ../../../example/multi_type_vector/mtv_collection.cpp
   :language: C++
   :start-after: //!code-start: column-5
   :end-before: //!code-end: column-5
   :dedent: 4

At this point, the content we've put into the ``columns`` variable roughly
reflects the tabular data shown at the beginning of this section.  Now we can
use the collection type we've declared earlier to wrap the columns:

.. literalinclude:: ../../../example/multi_type_vector/mtv_collection.cpp
   :language: C++
   :start-after: //!code-start: wrap
   :end-before: //!code-end: wrap
   :dedent: 4

We are naming this variable ``rows`` since what we are doing with this wrapper
is to traverse the content of the tabular data in row-wise direction.  For
this reason, calling it ``rows`` is quite fitting.

The :cpp:class:`~mdds::mtv::collection` class offers some flexibility as to
how the instances that you are trying to traverse orthogonally are stored.
That being said, you must meet the following prerequisites when passing the
collection of vector instances to the constructor of the
:cpp:class:`~mdds::mtv::collection` class:

1. All :cpp:type:`~mdds::multi_type_vector` instances that comprise the
   collection must be of the same logical length i.e. their
   :cpp:func:`~mdds::mtv::soa::multi_type_vector::size` methods must all return the same
   value.
2. The instances in the collection must be stored in the source container
   either as

   * concrete instances (as in this example),
   * as pointers, or
   * as heap instances wrapped within smart pointer class such as
     ``std::shared_ptr`` or ``std::unique_ptr``.

Although we are storing the vector instances in a ``std::vector`` container in
this example, you have the flexibility to pick a different type of container
to store the individual vector instances as long as it provides STL-compatible
standard iterator functionality.

Additionally, when using the :cpp:class:`~mdds::mtv::collection` class, you
must ensure that the content of the vector instances that it references will
not change for the duration of its use.

Finally, here is the code that does the traversing:

.. literalinclude:: ../../../example/multi_type_vector/mtv_collection.cpp
   :language: C++
   :start-after: //!code-start: traverse-row
   :end-before: //!code-end: traverse-row
   :dedent: 4

It's a simple for-loop, and in each iteration you get a single cell node that
contains metadata about that cell including its value.  The node contains the
following members:

* ``type`` - an integer value representing the type of the value.
* ``index`` -  a 0-based index of the :cpp:type:`~mdds::multi_type_vector`
  instance within the collection.  You can think of this as column index in
  this example.
* ``position`` - a 0-based logical element position within each
  :cpp:type:`~mdds::multi_type_vector` instance.  You can think of this as
  row index in this example.

In the current example we are only making use of the ``type`` and ``index``
members, but the ``position`` member will be there if you need it.

The node also provides a convenient ``get()`` method to fetch the value of the
cell.  This method is a template method, and you need to explicitly specify
the element block type in order to access the value.

When executing this code, you will see the following outout:

.. code-block:: none

    ID | Make | Model | Year | Color
    1 | Nissan | Frontier | 1998 | Turquoise
    2 | Mercedes-Benz | W201 | 1986 | Fuscia
    3 | Nissan | Frontier | 2009 | Teal
    4 | Suzuki | Equator | unknown | Fuscia
    5 | Saab | 9-5 | unknown | Green
    6 | Subaru | Tribeca | 2008 | Khaki
    7 | GMC | Yukon XL 2500 | 2009 | Pink
    8 | Mercedes-Benz | E-Class | 2008 | Goldenrod
    9 | Toyota | Camry Hybrid | 2010 | Turquoise
    10 | Nissan | Frontier | 2001 | Yellow
    11 | Mazda | MX-5 | 2008 | Orange
    12 | Dodge | Ram Van 1500 | 2000 | Goldenrod
    13 | Ford | Edge | unknown | Fuscia
    14 | Bentley | Azure | 2009 | Goldenrod
    15 | GMC | Sonoma Club Coupe | 1998 | Mauv
    16 | Audi | S4 | 2013 | Crimson
    17 | GMC | 3500 Club Coupe | 1994 | Turquoise
    18 | Mercury | Villager | 2000 | Teal
    19 | Pontiac | Sunbird | 1990 | Indigo
    20 | BMW | 3 Series | 1993 | LKhaki

which clearly shows that the code has traversed the content of the tabular
data horizontally across columns as intended.

Now, one feature that may come in handy is the ability to limit the iteration
range within the collection.  You can do that by calling either
:cpp:func:`~mdds::mtv::collection::set_collection_range` to limit the column
range or :cpp:func:`~mdds::mtv::collection::set_element_range` to limit the
row range, or perhaps both.

Let's see how this works in the current example.  Here, we are going to limit
the iteration range to only columns 2 and 3, and rows 2 through 11.  The following
code will set this limit:

.. literalinclude:: ../../../example/multi_type_vector/mtv_collection.cpp
   :language: C++
   :start-after: //!code-start: limit-range
   :end-before: //!code-end: limit-range
   :dedent: 4

Then iterate through the collection once again:

.. literalinclude:: ../../../example/multi_type_vector/mtv_collection.cpp
   :language: C++
   :start-after: //!code-start: traverse-row-range
   :end-before: //!code-end: traverse-row-range
   :dedent: 4

This code is nearly identical to the previous one except for the index values
used to control when to insert column separators and line breaks at the top
and bottom of each iteration.  When executing this code, you'll see the
following output:

.. code-block:: none

    Nissan | Frontier
    Mercedes-Benz | W201
    Nissan | Frontier
    Suzuki | Equator
    Saab | 9-5
    Subaru | Tribeca
    GMC | Yukon XL 2500
    Mercedes-Benz | E-Class
    Toyota | Camry Hybrid
    Nissan | Frontier

which clearly shows that your iteration range did indeed shrink as expected.
