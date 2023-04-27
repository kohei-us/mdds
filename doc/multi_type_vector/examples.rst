.. highlight:: cpp

Examples
========

Quick start
-----------

The following code demonstrates a simple use case of storing values of double
and :cpp:class:`std::string` types in a single container using :cpp:type:`~mdds::multi_type_vector`.

.. literalinclude:: ../../example/multi_type_vector/basic.cpp
   :language: C++
   :start-after: //!code-start
   :end-before: //!code-end

You'll see the following console output when you compile and execute this code:

.. code-block:: none

   numeric block of size 8
    * 1.1
    * 1.2
    * 1.3
    * 10.1
    * 10.2
    * 10.3
    * 10.4
    * 10.5
   empty block of size 2
    - no data -
   string block of size 3
    * Andy
    * Bruce
    * Charlie
   empty block of size 7
    - no data -

.. figure:: ../_static/images/mtv-block-structure.svg
   :align: center

   Logical structure between the primary array, blocks, and element blocks.

Each multi_type_vector instance maintains a logical storage structure of one
primary array containing one or more blocks each of which consists of ``type``,
``position``, ``size`` and ``data`` members:

* ``type`` - numeric value representing the block type.
* ``position`` - numeridc value representing the logical position of the first
  element of the block.
* ``size`` - number of elements present in the block a.k.a its logical size.
* ``data`` - pointer to the secondary storage (element block) storing the element
  values.

In this example code, the ``type`` member is referenced to determine its block
type and its logical size is determined from the ``size`` member.  For the
numeric and string blocks, their ``data`` members, which should point to the
memory addresses of their respective element blocks, are dereferenced in order
to print out their element values to stdout inside the ``print_block`` function.


.. _standard-element-blocks:

Standard element block types
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

It is worth noting that the two block types used in the previous example, namely
:cpp:type:`~mdds::mtv::double_element_block` and :cpp:type:`~mdds::mtv::string_element_block`
didn't come out of nowhere.  By default, including the header that defines multi_type_vector
implicitly also defines the following block types:

* :cpp:type:`mdds::mtv::boolean_element_block`
* :cpp:type:`mdds::mtv::int8_element_block`
* :cpp:type:`mdds::mtv::uint8_element_block`
* :cpp:type:`mdds::mtv::int16_element_block`
* :cpp:type:`mdds::mtv::uint16_element_block`
* :cpp:type:`mdds::mtv::int32_element_block`
* :cpp:type:`mdds::mtv::uint32_element_block`
* :cpp:type:`mdds::mtv::int64_element_block`
* :cpp:type:`mdds::mtv::uint64_element_block`
* :cpp:type:`mdds::mtv::float_element_block`
* :cpp:type:`mdds::mtv::double_element_block`
* :cpp:type:`mdds::mtv::string_element_block`

which respectively store elements of the following value types:

* ``bool``
* ``int8_t``
* ``uint8_t``
* ``int16_t``
* ``uint16_t``
* ``int32_t``
* ``uint32_t``
* ``int64_t``
* ``uint64_t``
* ``float``
* ``double``
* ``std::string``

The header also defines the :cpp:class:`mdds::mtv::standard_element_blocks_traits`
struct which you can pass to the :cpp:type:`~mdds::multi_type_vector` template
definition in order to have all of the above mentioned block types and their
respective value types available for use.


.. _custom-value-types:

Specifying custom types in element blocks
-----------------------------------------

There are times when you need to store a set of user-defined types in :cpp:type:`~mdds::multi_type_vector`.
That is what we are going to talk about in this section.

First, let's include the header:

.. literalinclude:: ../../example/multi_type_vector/custom_value_types.cpp
   :language: C++
   :start-after: //!code-start: header
   :end-before: //!code-end: header

then proceed to define some constant values to use as element types. We are going
to define three custom value types, so we need three element types defined:

.. literalinclude:: ../../example/multi_type_vector/custom_value_types.cpp
   :language: C++
   :start-after: //!code-start: element-types
   :end-before: //!code-end: element-types

Here, you need to ensure that the values used will not collide with the values
that may be used for the standard value types.  The best way to ensure that is
to assign the values that are greater than or equal to :cpp:var:`~mdds::mtv::element_type_user_start`
as the code above does.  Values less than :cpp:var:`~mdds::mtv::element_type_user_start`
are reserved for use either for the standard value types or any other internal uses
in the future.

Now, let's define the first two custom value types, and their respective block types:

.. literalinclude:: ../../example/multi_type_vector/custom_value_types.cpp
   :language: C++
   :start-after: //!code-start: custom-values
   :end-before: //!code-end: custom-values

Here, we are using the :cpp:type:`~mdds::mtv::default_element_block` as the basis
to define their block types.  At minimum, you need to specify the element type constant
and the value type as its template arguments.  There is a third optional template
argument you can specify which will become the underlying storage type.  By
default, :cpp:class:`~mdds::mtv::delayed_delete_vector` is used when the third
argument is not given.  But you can specify other types such as :cpp:type:`std::vector`
or :cpp:type:`std::deque` instead, or any other types that have similar interfaces
to :cpp:type:`std::vector`.

Once the block types are defined, it's time to define callback functions for them.
This should be as simple as using the :c:macro:`MDDS_MTV_DEFINE_ELEMENT_CALLBACKS` with
all necessary parameters provided:

.. literalinclude:: ../../example/multi_type_vector/custom_value_types.cpp
   :language: C++
   :start-after: //!code-start: custom-values-macro
   :end-before: //!code-end: custom-values-macro

Our third type is defined in a namespace ``ns``, and its associated block type is
also defined in the same namespace.  One thing to keep in mind is that, when the
custom type is defined in a namespace, its callback functions must also be defined
in the same namespace in order for them to be discovered per argument dependent lookup
during overload resolution.  This means that you must place the macro that defines
the callback functions in the same namespace as the namespace that encompasses the
original value type:

.. literalinclude:: ../../example/multi_type_vector/custom_value_types.cpp
   :language: C++
   :start-after: //!code-start: custom-value-ns
   :end-before: //!code-end: custom-value-ns

.. warning::

   If the original value type is defined inside a namespace, its associated callback
   functions must also be defined in the same namespace, due to the way argument
   dependent lookup works during overload resolution.

The next step is to define a trait type that specifies these block types.  The
easiest way is to have your trait inherit from :cpp:struct:`mdds::mtv::default_traits`
and overwrite the :cpp:type:`~mdds::mtv::default_traits::block_funcs` static member
type with an instance of :cpp:type:`mdds::mtv::element_block_funcs` with one or
more block types specified as its template arguments:

.. literalinclude:: ../../example/multi_type_vector/custom_value_types.cpp
   :language: C++
   :start-after: //!code-start: trait
   :end-before: //!code-end: trait

Now we are ready to define the final :cpp:class:`~mdds::mtv::multi_type_vector` type
with the trait we just defined:

.. literalinclude:: ../../example/multi_type_vector/custom_value_types.cpp
   :language: C++
   :start-after: //!code-start: mtv-type
   :end-before: //!code-end: mtv-type

And that's it!  With this in place, you can write a code like the following:

.. literalinclude:: ../../example/multi_type_vector/custom_value_types.cpp
   :language: C++
   :start-after: //!code-start: main
   :end-before: //!code-end: main
   :dedent: 4

to put some values of the custom types into your container and accessing them.  This
code should generate the following output:

.. code-block:: none

   is this custom_value1? 1
   is this custom_value2? 1
   is this ns::custom_value3? 1


.. _custom-value-types-custom-store:

Specifying different storage type
---------------------------------

By default, :cpp:class:`mdds::mtv::default_element_block` uses :cpp:class:`mdds::mtv::delayed_delete_vector`
as its underlying storage type to store its elements starting with version 2.1.  Prior to
2.1, :cpp:class:`std::vector` was used as the only storage type of choice.  If you use 2.1 or newer
versions of the library, you can specify your own storage type as the third template argument to
:cpp:class:`mdds::mtv::default_element_block`.

Let's tweak the previous example to specify :cpp:class:`std::vector` and :cpp:class:`std::deque` as the
storage types for ``custom_value1_block`` and ``custom_value2_block``, respectively:

.. literalinclude:: ../../example/multi_type_vector/custom_value_types_custom_store.cpp
   :language: C++
   :start-after: //!code-start: blocks-1-2
   :end-before: //!code-end: blocks-1-2

For ``custom_value3_block``, we will leave it as the default storage type, namely,
:cpp:class:`mdds::mtv::delayed_delete_vector`:

.. literalinclude:: ../../example/multi_type_vector/custom_value_types_custom_store.cpp
   :language: C++
   :start-after: //!code-start: block-3
   :end-before: //!code-end: block-3

You can specify different storage types for different block types as you can see above. But unless you have a
good reason to do so, you may want to stick with the same storage type for all of your blocks in order to have
consistent performance characteristics.

With this now in place, let's run the following code:

.. literalinclude:: ../../example/multi_type_vector/custom_value_types_custom_store.cpp
   :language: C++
   :start-after: //!code-start: main
   :end-before: //!code-end: main
   :dedent: 4

which should generate the following output:

.. code-block:: none

   custom_value1 stored in std::vector? 1
   custom_value2 stored in std::deque? 1
   ns::custom_value3 stored in delayed_delete_vector? 1

One thing to note is that, in order for a class to be usable as the storage type for
:cpp:class:`~mdds::mtv::default_element_block`, it must be a template class
with two parameters: the first one being the value type while the second one
is the allocator type just like how :cpp:class:`std::vector` or :cpp:class:`std::deque`
are defined.


Different storage types in standard element blocks
--------------------------------------------------

Now, what if you need to specify different storage types in the blocks already defined for the
standard value types, given that, as explained in the :ref:`standard-element-blocks` section,
those standard element blocks are automagically defined?

The answer is that it is possible to do such a thing, but it will require that you follow a certain
set of steps, as outlined below:

First, manually define the element type constants, block types, and their respecitve callback functions
for the standard value types you need to use as if they were user-defined types. When doing so, specify
the non-default storage types you need to use for these blocks.

Include the header for the multi_type_vector definition with the special macro value named
:c:macro:`MDDS_MTV_USE_STANDARD_ELEMENT_BLOCKS` defined and its value is set to 0.  This bypasses the
automatic inclusion of the block types for the standard value types when this header is included.

Lastly, define a custom trait type and overwrite the ``block_funcs`` member type to specify the block types
defined in the first step.  This is essentially the same step you would take when you define custom
block types for user-defined value types.

Let's do this step-by-step.  First, include the necessary headers:

.. literalinclude:: ../../example/multi_type_vector/standard_custom_store.cpp
   :language: C++
   :start-after: //!code-start: header
   :end-before: //!code-end: header

The ``types.hpp`` header is required for the :cpp:type:`~mdds::mtv::element_t` and
:cpp:class:`~mdds::mtv::default_element_block`, and the ``macro.hpp`` header is required
for the :c:macro:`MDDS_MTV_DEFINE_ELEMENT_CALLBACKS` macro.  The ``<deque>`` header is so
that we can use :cpp:class:`std::deque` as storage types in our block types.

Next, let's define the element and block types as well as their callback functions:

.. literalinclude:: ../../example/multi_type_vector/standard_custom_store.cpp
   :language: C++
   :start-after: //!code-start: block-defs
   :end-before: //!code-end: block-defs

This is very similar to how it is done in the :ref:`custom-value-types-custom-store` section.
The only difference is that, this part needs to happen *before* the header for the
multi_type_vector type gets included, in order for the multi_type_vector implementation code
to reference the callback functions now that the callback functions for the standard value
types will no longer be included.

Let's proceed to include the multi_type_vector header:

.. literalinclude:: ../../example/multi_type_vector/standard_custom_store.cpp
   :language: C++
   :start-after: //!code-start: mtv-header
   :end-before: //!code-end: mtv-header

Here, we define the :c:macro:`MDDS_MTV_USE_STANDARD_ELEMENT_BLOCKS` macro and set its value to
0, to skip the inclusion of the standard element blocks.  It is also worth noting that we
are including the ``mdds/multi_type_vector/soa/main.hpp`` header directly instead of
``mdds/multi_type_vector.hpp``, which indirectly includes the first header.

Lastly, let's define the trait type to specify the block types to use, and instantiate the final
multi_type_vector type:

.. literalinclude:: ../../example/multi_type_vector/standard_custom_store.cpp
   :language: C++
   :start-after: //!code-start: mtv-def
   :end-before: //!code-end: mtv-def

Now that the concrete multi_type_vector is defined, we can use it to store some values of the
specified types:

.. literalinclude:: ../../example/multi_type_vector/standard_custom_store.cpp
   :language: C++
   :start-after: //!code-start: main
   :end-before: //!code-end: main
   :dedent: 4

If you inspect the storage types of the element blocks like the following:

.. literalinclude:: ../../example/multi_type_vector/standard_custom_store.cpp
   :language: C++
   :start-after: //!code-start: main-block-type
   :end-before: //!code-end: main-block-type
   :dedent: 4

you should see the following output:

.. code-block:: none

   my_double_block: is std::deque its store type? 1
   my_int32_block: is std::deque its store type? 1

which indicates that they are indeed :cpp:class:`std::deque`.

Use custom event handlers
-------------------------

It is also possible to define custom event handlers that get called when
certain events take place.  To define custom event handlers, you need to
define either a class or a struct that has the following methods:

* **void element_block_acquired(mdds::mtv::base_element_block* block)**
* **void element_block_released(mdds::mtv::base_element_block* block)**

as its public methods, specify it as type named ``event_func`` in a trait struct,
and pass it as the second template argument when instantiating your
:cpp:type:`~mdds::multi_type_vector` type.  Refer to :cpp:type:`mdds::mtv::empty_event_func`
for the detail on when each event handler method gets triggered.

The following code example demonstrates how this all works:

.. literalinclude:: ../../example/multi_type_vector/event1.cpp
   :language: C++
   :start-after: //!code-start
   :end-before: //!code-end

You'll see the following console output when you compile and execute this code:

.. code-block:: none

   inserting string 'foo'...
     * element block acquired
   inserting string 'bah'...
   inserting int 100...
     * element block acquired
   emptying the container...
     * element block released
     * element block released
   exiting program...

In this example, the **element_block_acquired** handler gets triggered each
time the container creates (thus acquires) a new element block to store a value.
It does *not* get called when a new value is appended to a pre-existing element
block.  Similarly, the **element_block_releasd** handler gets triggered each
time an existing element block storing non-empty values gets deleted.  One
thing to keep in mind is that since these two handlers respond to events related
to element blocks which are owned by non-empty blocks in the primary array,
and empty blocks don't store any element block instances, creations or deletions
of empty blocks don't trigger these event handlers.

The trait also allows you to configure other behaviors of :cpp:type:`~mdds::multi_type_vector`.
Refer to :cpp:type:`mdds::mtv::default_traits` for all available parameters.


Get raw pointer to element block array
--------------------------------------

Sometimes you need to expose a pointer to an element block array especially
when you need to pass such an array pointer to C API that requires one.  You
can do this by calling the ``data`` method of the element_block template
class.  This works since the element block internally just wraps
:cpp:class:`std::vector` or one that acts like it, such as
:cpp:class:`std::deque` or :cpp:class:`~mdds::mtv::delayed_delete_vector`, and
its ``data`` method simply exposes the internal storage types's own ``data``
method which returns the memory location of its internal buffer.

The following code demonstrates this by exposing raw array pointers to the
internal arrays of numeric and string element blocks, and printing their
element values directly from these array pointers.

.. literalinclude:: ../../example/multi_type_vector/element_block1.cpp
   :language: C++
   :start-after: //!code-start
   :end-before: //!code-end

Compiling and execute this code produces the following output:

.. code-block:: none

   block size: 2
   --
   1.1
   1.2
   1.3
   1.4
   1.5
   --
   A
   B
   C
   D
   E


Traverse multiple multi_type_vector instances "sideways"
--------------------------------------------------------

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

.. figure:: ../_static/images/mtv_collection_sheet.png
   :align: center

It consists of five columns, with each column storing 21 rows of data.  The
first row is a header row, followed by 20 rows of values.  In this example, We
will be using one :cpp:type:`~mdds::multi_type_vector` instance for each
column thus creating five instances in total, and store them in a
``std::vector`` container.

The declaration of the data store will look like this:

.. literalinclude:: ../../example/multi_type_vector/mtv_collection.cpp
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

.. literalinclude:: ../../example/multi_type_vector/mtv_collection.cpp
   :language: C++
   :start-after: //!code-start: header-row
   :end-before: //!code-end: header-row
   :dedent: 4

We are then filling each column individually from column 1 through column 5.
First up is column 1:

.. literalinclude:: ../../example/multi_type_vector/mtv_collection.cpp
   :language: C++
   :start-after: //!code-start: column-1
   :end-before: //!code-end: column-1
   :dedent: 4

Hopefully this code is straight-forward.  It initializes an array of values
and push them to the column one at a time via
:cpp:func:`~mdds::mtv::soa::multi_type_vector::push_back`.  Next up is column 2:

.. literalinclude:: ../../example/multi_type_vector/mtv_collection.cpp
   :language: C++
   :start-after: //!code-start: column-2
   :end-before: //!code-end: column-2
   :dedent: 4

This is similar to the code for column 1, except that because we are using an
array of string literals which implicitly becomes an initializer list of type
``const char*``, we need to explicitly specify the type for the
:cpp:func:`~mdds::mtv::soa::multi_type_vector::push_back` call to be ``std::string``.

The code for column 3 is very similar to this:

.. literalinclude:: ../../example/multi_type_vector/mtv_collection.cpp
   :language: C++
   :start-after: //!code-start: column-3
   :end-before: //!code-end: column-3
   :dedent: 4

Populating column 4 needs slight pre-processing.  We are inserting a string
value of "unknown" in lieu of an integer value of -1.  Therefore the following
code will do:

.. literalinclude:: ../../example/multi_type_vector/mtv_collection.cpp
   :language: C++
   :start-after: //!code-start: column-4
   :end-before: //!code-end: column-4
   :dedent: 4

Finally, the last column to fill, which uses the same logic as for columns 2
and 3:

.. literalinclude:: ../../example/multi_type_vector/mtv_collection.cpp
   :language: C++
   :start-after: //!code-start: column-5
   :end-before: //!code-end: column-5
   :dedent: 4

At this point, the content we've put into the ``columns`` variable roughly
reflects the tabular data shown at the beginning of this section.  Now we can
use the collection type we've declared earlier to wrap the columns:

.. literalinclude:: ../../example/multi_type_vector/mtv_collection.cpp
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

.. literalinclude:: ../../example/multi_type_vector/mtv_collection.cpp
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

.. literalinclude:: ../../example/multi_type_vector/mtv_collection.cpp
   :language: C++
   :start-after: //!code-start: limit-range
   :end-before: //!code-end: limit-range
   :dedent: 4

Then iterate through the collection once again:

.. literalinclude:: ../../example/multi_type_vector/mtv_collection.cpp
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


Performance Considerations
==========================

Select SoA or AoS storage types
-------------------------------

If you instantiate a multi_type_vector instance via
:cpp:type:`mdds::multi_type_vector`, which is an alias type for
:cpp:class:`mdds::mtv::soa::multi_type_vector`, you will be using the
structure-of-arrays (SoA) variant of its implementation which is new in 2.0.
Prior to 2.0, multi_type_vector used the array-of-structures (AoS) layout which
is still available post 2.0 via :cpp:class:`mdds::mtv::aos::multi_type_vector`
in case you need it.

Note, however, that the SoA variant generally yields better overall performance
since it can make more efficient use of CPU caches.  It is therefore highly
recommended that you stick with the SoA variant unless you have a specific
reason not to.

Also note that both variants are API compatibile with each other.


Use of position hints to avoid the cost of block position lookup
----------------------------------------------------------------

Consider the following example code:

.. literalinclude:: ../../example/multi_type_vector/pos_hint.cpp
   :language: C++
   :start-after: //!code-start: no-pos-hint
   :end-before: //!code-end: no-pos-hint
   :dedent: 4

which, when executed, may take quite sometime to complete especially when you
are using an older version of mdds.  This particular example exposes one
weakness that multi_type_vector has; because it needs to first look up the
position of the block to operate with, and that lookup *always* starts from the
first block, the time it takes to find the correct block increases as the number
of blocks goes up.  This example demonstrates the worst case scenario of such
lookup complexity since it always inserts the next value at the last block
position.

Fortunately, there is a simple solution to this which the following code
demonstrates:

.. literalinclude:: ../../example/multi_type_vector/pos_hint.cpp
   :language: C++
   :start-after: //!code-start: pos-hint
   :end-before: //!code-end: pos-hint
   :dedent: 4

Compiling and executing this code should take only a fraction of a second.

The only difference between the second example and the first one is that the
second one uses an interator as a position hint to keep track of the position of
the last modified block.  Each
:cpp:func:`~mdds::mtv::soa::multi_type_vector::set` method call returns an
iterator which can then be passed to the next
:cpp:func:`~mdds::mtv::soa::multi_type_vector::set` call as the position hint.
Because an iterator object internally stores the location of the block the value
was inserted to, this lets the method to start the block position lookup process
from the last modified block, which in this example is always one block behind
the one the new value needs to go.  Using the big-O notation, the use of the
position hint essentially turns the complexity of O(n^2) in the first example
into O(1) in the second one if you are using an older version of mdds where the
block position lookup had a linear complexity.

This strategy should work with any methods in :cpp:type:`~mdds::multi_type_vector`
that take a position hint as the first argument.

Note that, if you are using a more recent version of mdds (1.6.0 or newer), the
cost of block position lookup is significantly lessoned thanks to the switch to
binary search in performing the lookup.

.. note::

   If you are using mdds 1.6.0 or newer, the cost of block position lookup is
   much less significant even without the use of position hints. But the benefit
   of using position hints may still be there.  It's always a good idea to profile
   your specific use case and decide whether the use of position hints is worth
   it.

One important thing to note is that, as a user, you must ensure that the position
hint you pass stays valid between the calls.  A position hint becomes invalid when
the content of the container changes.  A good strategy to maintain a valid position
hint is to always receive the iterator returned from the mutator method you called
to which you passed the previous position hint, which is what the code above does.
Passing an invalid position hint to a method that takes one may result in invalid
memory access or otherwise in some sort of undefined behavior.

.. warning::

   You must ensure that the position hint you pass stays valid. Passing an invalid
   position hint to a method that takes one may result in invalid memory access
   or otherwise in some sort of undefined behavior.


Block shifting performance and loop-unrolling factor
----------------------------------------------------

The introduction of binary search in the block position lookup implementation
in version 1.6 has significantly improved its lookup performance, but has
also resulted in slight performance hit when shifting blocks during value
insertion.  This is because when shifting the logical positions of the blocks
below the insertion point, their head positions need to be re-calculated to
account for their new positions.

The good news is that the switch to the structure-of-arrays (SoA) storage
layout in 2.0 alone may bring subtle but measurable improvement in the
block position adjustment performance due to the logical block positions now
being stored in a separate array thereby improving its cache efficiency.  In
reality, however, this was somewhat dependent on the CPU types since some CPU's
didn't show any noticeable improvements or even showed worse performance, while
other CPU types showed consistent improvements with SoA over AoS.

Another factor that may play a role is `loop unrolling <https://en.wikipedia.org/wiki/Loop_unrolling>`_
factor which can be configured via the :cpp:var:`~mdds::mtv::default_traits::loop_unrolling`
variable in your custom trait type if you use version 2.0 or newer.  This variable
is an enum class of type :cpp:type:`mdds::mtv::lu_factor_t` which enumerates
several pre-defined loop-unrolling factors as well as some SIMD features.

The hardest part is to figure out which loop unrolling factor is the best option
in your runtime environment, since it is highly dependent on the environment.
Luckily mdds comes with a tool called `runtime-env <https://gitlab.com/mdds/mdds/-/tree/master/tools/runtime-env>`_
which, when run, will perform some benchmarks and give you the best loop-unrolling
factor in your runtime environment.  Be sure to build this tool with the same
compiler and compiler flags as your target program in order for this tool to give
you a representative answer.


Debugging
=========

Tracing of public methods
-------------------------

When using :cpp:class:`~mdds::mtv::soa::multi_type_vector` to handle a series
of data reads and writes in an non-trivial code base, sometimes you may find
yourself needing to track which methods are getting called when following a
certain code path during a debugging session.  In such a situation, you can enable
an optional trace method which gets called whenever a public method of :cpp:class:`~mdds::mtv::soa::multi_type_vector`
is called.

First, you need to define a preprocessor macro named
``MDDS_MULTI_TYPE_VECTOR_DEBUG`` before including the header for
:cpp:class:`~mdds::mtv::soa::multi_type_vector`:

.. literalinclude:: ../../example/multi_type_vector/debug_trace.cpp
   :language: C++
   :start-after: //!code-start: header
   :end-before: //!code-end: header

to enable additional debug code.  In this example the value of the macro is
set to 1, but it doesn't matter what the value of the macro is, as long as it
is defined.  You can also define one as a compiler option as well.

Once defined, the next step is to add a ``trace`` method as a static function to
the trait type you pass as a template argument of multi_type_vector:

.. literalinclude:: ../../example/multi_type_vector/debug_trace.cpp
   :language: C++
   :start-after: //!code-start: types
   :end-before: //!code-end: types

Here, we are simply inheriting our trait type from the
:cpp:class:`~mdds::mtv::default_traits` type and simply adding a static ``trace``
function to it, and passing this trait type to the mtv_type definition below.
This trace function must take one argument of type
:cpp:class:`mdds::mtv::trace_method_properties_t` which includes various
properties of the traced call.  In this example, we are simply printing the
properties named
:cpp:member:`~mdds::mtv::trace_method_properties_t::function_name` and
:cpp:member:`~mdds::mtv::trace_method_properties_t::function_args` each time a
traced method is called.  Both of these properties are printable string types.

Note that this ``trace`` function is entirely optional; the code will compile
fine even when it's not defined.  Also, it must be declared as static for it to
be called.

Let's instantiate an object of ``mtv_type``, call some of its methods and see
what happens.  When executing the following code:

.. literalinclude:: ../../example/multi_type_vector/debug_trace.cpp
   :language: C++
   :start-after: //!code-start: main
   :end-before: //!code-end: main
   :dedent: 4

You will see the following output:

.. code-block:: text

    function:
      name: multi_type_vector
      args: init_size=10
    function:
      name: set
      args: pos=0; value=? (type=5)
    function:
      name: set
      args: pos=2; value=? (type=1)
    function:
      name: set
      args: pos=4; value=? (type=3)
    function:
      name: ~multi_type_vector
      args:

The :cpp:member:`~mdds::mtv::trace_method_properties_t::function_name`
property is hopefully self-explanatory.  The
:cpp:member:`~mdds::mtv::trace_method_properties_t::function_args` property is
a single string value containing the information about the function's
arguments and optionally their values if their values are known to be
printable.  If the value of an argument cannot be printed, ``?`` is placed
instead.  For some argument types, an additional information is displayed e.g.
``(type=5)`` in the above output which indicates that the type of the value
being passed to the function is :cpp:var:`~mdds::mtv::element_type_int32`.

If you want to limit your tracing to a specific function type or types, you
can make use of the :cpp:member:`~mdds::mtv::trace_method_properties_t::type`
property which specifies the type of the traced method.  Likewise, if you want
to only trace methods of a certain instance, use
:cpp:member:`~mdds::mtv::trace_method_properties_t::instance` to filter the
incoming trace calls based on the memory addresses of the instances whose
methods are being traced.

Note that this feature is available for version 2.0.2 and newer, and currently
only available for the SoA variant of :cpp:class:`~mdds::mtv::soa::multi_type_vector`.

.. note::

   This feature is only available for version 2.0.2 and newer, and only for the
   SoA variant.

