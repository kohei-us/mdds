.. highlight:: cpp


Trie Maps
=========

Examples
--------

Populating Trie Map
^^^^^^^^^^^^^^^^^^^

This section illustrates how to use :cpp:class:`~mdds::trie_map` to build a
database of city populations and perform prefix searches.  In this example,
we will use the 2013 populations of cities in North Carolina, and use the city
names as keys.

Let's define the type first:

.. literalinclude:: ../example/trie_map.cpp
   :language: C++
   :start-after: //!code-start: type
   :end-before: //!code-end: type
   :dedent: 4

The first template argument specifies the key type, while the second template
argument specifies the value type.  In this example we are using ``std::string``
and ``int``, respectively.

Once the type is defined, the next step is instantiation:

.. literalinclude:: ../example/trie_map.cpp
   :language: C++
   :start-after: //!code-start: inst
   :end-before: //!code-end: inst
   :dedent: 4

It's pretty simple as you don't need to pass any arguments to the constructor.
Now, let's populate this data structure with some population data:

.. literalinclude:: ../example/trie_map.cpp
   :language: C++
   :start-after: //!code-start: populate
   :end-before: //!code-end: populate
   :dedent: 4

It's pretty straight-forward.  Each :cpp:func:`~mdds::trie_map::insert` call
expects a pair of string key and an integer value.  You can insert your data
in any order regardless of key's sort order.

Now that the data is in, let's perform prefix search to query all cities whose
name begins with "Cha":

.. literalinclude:: ../example/trie_map.cpp
   :language: C++
   :start-after: //!code-start: search-cha
   :end-before: //!code-end: search-cha
   :dedent: 4

You can perform prefix search via :cpp:func:`~mdds::trie_map::prefix_search`
method, which returns a results object that can be iterated over using a range-based
for loop.  Running this code will produce the following output:

.. code-block:: none

    Cities that start with 'Cha' and their populations:
      Chapel Hill: 59635
      Charlotte: 792862

Let's perform another prefix search, this time with a prefix of "W":

.. literalinclude:: ../example/trie_map.cpp
   :language: C++
   :start-after: //!code-start: search-w
   :end-before: //!code-end: search-w
   :dedent: 4

You'll see the following output when running this code:

.. code-block:: none

    Cities that start with 'W' and their populations:
      Wilmington: 112067
      Wilson: 49628
      Winston-Salem: 236441

Note that the results are sorted in key's ascending order.

.. note::

   Results from the prefix search are sorted in key's ascending order.


Creating Packed Trie Map from Trie Map
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There is also another variant of trie called :cpp:class:`~mdds::packed_trie_map`
which is designed to store all its data in contiguous memory region.  Unlike
:cpp:class:`~mdds::trie_map` which is mutable, :cpp:class:`~mdds::packed_trie_map`
is immutable; once populated, you can only perform queries and it is no longer
possible to add new entries into the container.

One way to create an instance of :cpp:class:`~mdds::packed_trie_map` is from
:cpp:class:`~mdds::trie_map` by calling its :cpp:func:`~mdds::trie_map::pack`
method:

.. literalinclude:: ../example/trie_map.cpp
   :language: C++
   :start-after: //!code-start: pack
   :end-before: //!code-end: pack
   :dedent: 4

When creating a :cpp:class:`~mdds::packed_trie_map` instance this way, however,
you need to be aware that the values get moved from the original instance to the
new instance.  As such, it is not advisable to keep the original instance around
afterward.  If you need to keep the original instance intact, you can first
create a copy of it and then call its :cpp:func:`~mdds::trie_map::pack` method
to create the packed variant.

.. warning::

   Calling :cpp:func:`~mdds::trie_map::pack` will move all the stored values to
   the packed variant.  Make a copy first if you need to keep the original instance
   intact.

The query methods of :cpp:class:`~mdds::packed_trie_map` are identical to those
of :cpp:class:`~mdds::trie_map`.  For instance, performing prefix search to find
all entries whose key begins with "C" can be done as follows:

.. literalinclude:: ../example/trie_map.cpp
   :language: C++
   :start-after: //!code-start: search-c
   :end-before: //!code-end: search-c
   :dedent: 4

Running this code will generate the following output:

.. code-block:: none

    Cities that start with 'C' and their populations:
      Cary: 151088
      Chapel Hill: 59635
      Charlotte: 792862
      Concord: 83506

You can also perform an exact-match query via :cpp:func:`~mdds::packed_trie_map::find`
method which returns an iterator associated with the key-value pair entry:

.. literalinclude:: ../example/trie_map.cpp
   :language: C++
   :start-after: //!code-start: find-one
   :end-before: //!code-end: find-one
   :dedent: 4

You'll see the following output with this code:

.. code-block:: none

    Population of Wilmington: 112067

What if you performed an exact-match query with a key that doesn't exist in the
container?  You will basically get the end iterator position as its return value.
Thus, running this code:

.. literalinclude:: ../example/trie_map.cpp
   :language: C++
   :start-after: //!code-start: find-none
   :end-before: //!code-end: find-none
   :dedent: 4

will generate the following output:

.. code-block:: none

    Population of Asheboro: not found

The complete source code for the examples in these two sections is available
`here <https://gitlab.com/mdds/mdds/-/blob/master/example/trie_map.cpp>`__.


Using Packed Trie Map directly
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In the previous example, we showed a way to create an instance of :cpp:class:`~mdds::packed_trie_map`
from a populated instance of :cpp:class:`~mdds::trie_map`.  There is also a way
to instantiate and populate an instance of :cpp:class:`~mdds::packed_trie_map`
directly, and that is what we will cover in this section.

First, declare the type:

.. literalinclude:: ../example/packed_trie_map.cpp
   :language: C++
   :start-after: //!code-start: type
   :end-before: //!code-end: type
   :dedent: 4

Once again, we are using ``std::string`` as its key, and ``int`` as its value
type.  The next step is to prepare its entries ahead of time:

.. literalinclude:: ../example/packed_trie_map.cpp
   :language: C++
   :start-after: //!code-start: populate
   :end-before: //!code-end: populate
   :dedent: 4

We need to do this since :cpp:class:`~mdds::packed_trie_map` is immutable, and
the only time we can populate its content is at instantiation time.  Here, we
are using the :c:macro:`MDDS_ASCII` macro to expand a string literal to its
pointer value and size.  Note that you need to ensure that *the entries are sorted
by the key in ascending order.*

.. warning::

   When instantiating :cpp:class:`~mdds::packed_trie_map` directly with a static
   set of entries, the entries must be sorted by the key in ascending order.

You can then pass this list of entries to construct the instance:

.. literalinclude:: ../example/packed_trie_map.cpp
   :language: C++
   :start-after: //!code-start: inst
   :end-before: //!code-end: inst
   :dedent: 4

Once it's instantiated, the rest of the example for performing searches will be
the same as in the previous section, which we will not repeat here.

The complete source code for the example in this section is available
`here <https://gitlab.com/mdds/mdds/-/blob/master/example/packed_trie_map.cpp>`__.


Saving and loading Packed Trie Map instances
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are times when you need to save the state of a :cpp:class:`~mdds::packed_trie_map`
instance to a file, or an in-memory buffer, and load it back later.  Doing that
is now possible by using the :cpp:func:`~mdds::packed_trie_map::save_state` and
:cpp:func:`~mdds::packed_trie_map::load_state` member methods of the
:cpp:class:`~mdds::packed_trie_map` class.

First, let's define the type of use:

.. literalinclude:: ../example/packed_trie_state_int.cpp
   :language: C++
   :start-after: //!code-start: type
   :end-before: //!code-end: type
   :dedent: 4

As with the previous examples, we will use ``std::string`` as the key type and
``int`` as the value type.  In this example, we are going to use `the world's
largest cities and their 2018 populations
<https://en.wikipedia.org/wiki/List_of_largest_cities>`__ as the data to store
in the container.

The following code defines the entries:

.. literalinclude:: ../example/packed_trie_state_int.cpp
   :language: C++
   :start-after: //!code-start: entries
   :end-before: //!code-end: entries
   :dedent: 4

It's a bit long as it contains entries for 81 cities.  We are then going to
create an instance of the :cpp:class:`~mdds::packed_trie_map` class directly:

.. literalinclude:: ../example/packed_trie_state_int.cpp
   :language: C++
   :start-after: //!code-start: inst
   :end-before: //!code-end: inst
   :dedent: 4

Let's print the size of the container to make sure the container has been
successfully populated:

.. literalinclude:: ../example/packed_trie_state_int.cpp
   :language: C++
   :start-after: //!code-start: print-n-cities
   :end-before: //!code-end: print-n-cities
   :dedent: 4

You will see the following output:

.. code-block:: none

    Number of cities: 81

if the container has been successfully populated.  Now, let's run a prefix
search on names beginning with an 'S':

.. literalinclude:: ../example/packed_trie_state_int.cpp
   :language: C++
   :start-after: //!code-start: search-s
   :end-before: //!code-end: search-s
   :dedent: 8

to make sure you get the following ten cities and their populations as the
output:

.. code-block:: none

    Cities that begin with 'S':
      * Saint Petersburg: 5383000
      * Santiago: 6680000
      * Sao Paulo: 21650000
      * Seoul: 9963000
      * Shanghai: 25582000
      * Shenyang: 6921000
      * Shenzhen: 11908000
      * Singapore: 5792000
      * Surat: 6564000
      * Suzhou: 6339000

So far so good.  Next, we will use the :cpp:func:`~mdds::packed_trie_map::save_state`
method to dump the internal state of this container to a file named **cities.bin**:

.. literalinclude:: ../example/packed_trie_state_int.cpp
   :language: C++
   :start-after: //!code-start: save-state-cities
   :end-before: //!code-end: save-state-cities
   :dedent: 8

This will create a file named **cities.bin** which contains a binary blob
representing the content of this container in the current working directory.
Run the ``ls -l cities.bin`` command to make sure the file has been created:

.. code-block:: none

    -rw-r--r-- 1 kohei kohei 17713 Jun 20 12:49 cities.bin

Now that the state of the container has been fully serialized to a file, let's
work on restoring its content in another, brand-new instance of
:cpp:class:`~mdds::packed_trie_map`.

.. literalinclude:: ../example/packed_trie_state_int.cpp
   :language: C++
   :start-after: //!code-start: load-state-cities
   :end-before: //!code-end: load-state-cities
   :dedent: 4

Here, we used the :cpp:func:`~mdds::packed_trie_map::load_state` method to
restore the state from the file we have previously created.  Let's make sure
that this new instance has content equivalent to that of the original:

.. literalinclude:: ../example/packed_trie_state_int.cpp
   :language: C++
   :start-after: //!code-start: check-equal
   :end-before: //!code-end: check-equal
   :dedent: 4

If you see the following output:

.. code-block:: none

    Equal to the original? true

then this new instance has equivalent contant as the original one.  Let's also
make sure that it contains the same number of entries as the original:

.. literalinclude:: ../example/packed_trie_state_int.cpp
   :language: C++
   :start-after: //!code-start: print-n-cities-2
   :end-before: //!code-end: print-n-cities-2
   :dedent: 4

Hopefully you will see the following output:

.. code-block:: none

    Number of cities: 81

Lastly, let's run on this new instance the same prefix search we did on the
original instance, to make sure we still get the same results:

.. literalinclude:: ../example/packed_trie_state_int.cpp
   :language: C++
   :start-after: //!code-start: search-s-2
   :end-before: //!code-end: search-s-2
   :dedent: 4

You should see the following output:

.. code-block:: none

    Cities that begin with 'S':
      * Saint Petersburg: 5383000
      * Santiago: 6680000
      * Sao Paulo: 21650000
      * Seoul: 9963000
      * Shanghai: 25582000
      * Shenyang: 6921000
      * Shenzhen: 11908000
      * Singapore: 5792000
      * Surat: 6564000
      * Suzhou: 6339000

which is the same output we saw in the first prefix search.

The complete source code for this example is found
`here <https://gitlab.com/mdds/mdds/-/blob/master/example/packed_trie_state_int.cpp>`__.


Saving Packed Trie Map with custom value type
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In the previos example, you didn't have to explicitly specify the serializer type
to the :cpp:func:`~mdds::packed_trie_map::save_state` and
:cpp:func:`~mdds::packed_trie_map::load_state` methods, even though these two
methods require the serializer type as their template arguments.  That's because
the library provides default serializer types for

* numeric value types i.e. integers, float and double,
* ``std::string``, and
* the standard sequence types, such as ``std::vector``, whose elements are of
  numeric value types,

and the previous example used ``int`` as the value type.

In this section, we are going to illustrate how you can write your own custom
serializer to allow serialization of your own custom value type.  In this example,
we are going to use `the list of presidents of the United States
<https://en.wikipedia.org/wiki/List_of_presidents_of_the_United_States>`__,
with the names of the presidents as the keys, and their years of inauguration
and political affiliations as the values.

We will use the following structure to store the values:

.. literalinclude:: ../example/packed_trie_state_custom.cpp
   :language: C++
   :start-after: //!code-start: value-type
   :end-before: //!code-end: value-type
   :dedent: 0

Each entry stores the year as a 16-bit integer and the affiliated party as an enum
value of 8-bit width.

Next, let's define the container type:

.. literalinclude:: ../example/packed_trie_state_custom.cpp
   :language: C++
   :start-after: //!code-start: trie-type
   :end-before: //!code-end: trie-type
   :dedent: 4

As with the previous example, the first step is to define the entries that are
sorted by the keys, which in this case are the president's names:

.. literalinclude:: ../example/packed_trie_state_custom.cpp
   :language: C++
   :start-after: //!code-start: entries
   :end-before: //!code-end: entries
   :dedent: 4

Note that we need to add numeric suffixes to the entries for Grover Cleveland,
who became president twice in two separate periods, in order to make the keys
for his entries unique.

Now, proceed to create an instance of :cpp:class:`~mdds::packed_trie_map`:

.. literalinclude:: ../example/packed_trie_state_custom.cpp
   :language: C++
   :start-after: //!code-start: inst
   :end-before: //!code-end: inst
   :dedent: 4

and inspect its size to make sure it is instantiated properly:

.. literalinclude:: ../example/packed_trie_state_custom.cpp
   :language: C++
   :start-after: //!code-start: print-n-entries
   :end-before: //!code-end: print-n-entries
   :dedent: 4

You should see the following output:

.. code-block:: none

    Number of entries: 45

Before we proceed to save the state of this instance, let's define the custom
serializer type first:

.. literalinclude:: ../example/packed_trie_state_custom.cpp
   :language: C++
   :start-after: //!code-start: custom-serializer
   :end-before: //!code-end: custom-serializer
   :dedent: 0

A custom value type can be either variable-size or fixed-size.  For a variable-size
value type, each value segment is preceded by the byte length of that segment.
For a fixed-size value type, the byte length of all of the value segments
is written only once up-front, followed by one or more value segments of equal
byte length.

Since the value type in this example is fixed-size, we set the value of the
``variable_size`` static constant to false, and define the size of the value to 3 (bytes)
as the ``value_size`` static constant.  Keep in mind that you need to define
the ``value_size`` constant *only* for fixed-size value types; if your value
type is variable-size, you can leave it out.

Additionally, you need to define two static methods - one for writing to the
output stream, and one for reading from the input stream.  The ``write`` method
must have the following signature::

    static void write(std::ostream& os, const T& v);

where the ``T`` is the value type.  In the body of this method you write to the
output stream the bytes that represent the value.  The length of the bytes you
write must match the size specified by the ``value_size`` constant.

The ``read`` method must have the following signature::

    static void read(std::istream& is, size_t n, T& v);

where the ``T`` is the value type, and the ``n`` specifies the length of the
bytes you need to read for the value.  For a fixed-size value type, the value
of ``n`` should equal the ``value_size`` constant.  Your job is to read the
bytes off of the input stream for the length specified by the ``n``, and
populate the value instance passed to the method as the third argument.

Now that we have defined the custom serializer type, let's proceed to save the
state to a file:

.. literalinclude:: ../example/packed_trie_state_custom.cpp
   :language: C++
   :start-after: //!code-start: save-state
   :end-before: //!code-end: save-state
   :dedent: 8

This time around, we are specifying the serializer type explicitly as the template
argument to the :cpp:func:`~mdds::packed_trie_map::save_state` method.  Otherwise
it is no different than what we did in the previous example.

Let's create another instance of :cpp:class:`~mdds::packed_trie_map` and restore
the state back from the file we just created:

.. literalinclude:: ../example/packed_trie_state_custom.cpp
   :language: C++
   :start-after: //!code-start: load-state
   :end-before: //!code-end: load-state
   :dedent: 4

Once again, aside from explicitly specifying the serializer type as the template
argument to the :cpp:func:`~mdds::packed_trie_map::load_state` method, it is
identical to the way we did in the previous example.

Let's compare the new instance with the old one to see if the two are equal:

.. literalinclude:: ../example/packed_trie_state_custom.cpp
   :language: C++
   :start-after: //!code-start: compare
   :end-before: //!code-end: compare
   :dedent: 4

The output says:

.. code-block:: none

    Equal to the original? true

They are.  While we are at it, let's run a simple prefix search to find out
all the US presidents whose first name is 'John':

.. literalinclude:: ../example/packed_trie_state_custom.cpp
   :language: C++
   :start-after: //!code-start: search-john
   :end-before: //!code-end: search-john
   :dedent: 4

Here is the output:

.. code-block:: none

    Presidents whose first name is 'John':
      * John Adams (1797; Federalist)
      * John F. Kennedy (1961; Democratic)
      * John Quincy Adams (1825; Democratic Republican)
      * John Tyler (1841; Whig)

This looks like the correct results!

You can find the complete source code for this example `here
<https://gitlab.com/mdds/mdds/-/blob/master/example/packed_trie_state_custom.cpp>`__.


API Reference
-------------

Trie Map
^^^^^^^^

.. doxygenclass:: mdds::trie_map
   :members:


Packed Trie Map
^^^^^^^^^^^^^^^

.. doxygenclass:: mdds::packed_trie_map
   :members:


Value Serializers
^^^^^^^^^^^^^^^^^

.. doxygenstruct:: mdds::trie::value_serializer
   :members:

.. doxygenstruct:: mdds::trie::numeric_value_serializer
   :members:

.. doxygenstruct:: mdds::trie::variable_value_serializer
   :members:

.. doxygenstruct:: mdds::trie::numeric_sequence_value_serializer
   :members:


Other Types
^^^^^^^^^^^

.. doxygenstruct:: mdds::trie::default_traits
   :members:

.. doxygenenum:: mdds::trie::dump_structure_type
