
Saving packed_trie_map with custom value type
=============================================

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

.. literalinclude:: ../../example/packed_trie_state_custom.cpp
   :language: C++
   :start-after: //!code-start: value-type
   :end-before: //!code-end: value-type
   :dedent: 0

Each entry stores the year as a 16-bit integer and the affiliated party as an enum
value of 8-bit width.

Next, let's define the container type:

.. literalinclude:: ../../example/packed_trie_state_custom.cpp
   :language: C++
   :start-after: //!code-start: trie-type
   :end-before: //!code-end: trie-type
   :dedent: 4

As with the previous example, the first step is to define the entries that are
sorted by the keys, which in this case are the president's names:

.. literalinclude:: ../../example/packed_trie_state_custom.cpp
   :language: C++
   :start-after: //!code-start: entries
   :end-before: //!code-end: entries
   :dedent: 4

Note that we need to add numeric suffixes to the entries for Grover Cleveland,
who became president twice in two separate periods, in order to make the keys
for his entries unique.

Now, proceed to create an instance of :cpp:class:`~mdds::packed_trie_map`:

.. literalinclude:: ../../example/packed_trie_state_custom.cpp
   :language: C++
   :start-after: //!code-start: inst
   :end-before: //!code-end: inst
   :dedent: 4

and inspect its size to make sure it is instantiated properly:

.. literalinclude:: ../../example/packed_trie_state_custom.cpp
   :language: C++
   :start-after: //!code-start: print-n-entries
   :end-before: //!code-end: print-n-entries
   :dedent: 4

You should see the following output:

.. code-block:: none

    Number of entries: 45

Before we proceed to save the state of this instance, let's define the custom
serializer type first:

.. literalinclude:: ../../example/packed_trie_state_custom.cpp
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

.. literalinclude:: ../../example/packed_trie_state_custom.cpp
   :language: C++
   :start-after: //!code-start: save-state
   :end-before: //!code-end: save-state
   :dedent: 8

This time around, we are specifying the serializer type explicitly as the template
argument to the :cpp:func:`~mdds::packed_trie_map::save_state` method.  Otherwise
it is no different than what we did in the previous example.

Let's create another instance of :cpp:class:`~mdds::packed_trie_map` and restore
the state back from the file we just created:

.. literalinclude:: ../../example/packed_trie_state_custom.cpp
   :language: C++
   :start-after: //!code-start: load-state
   :end-before: //!code-end: load-state
   :dedent: 4

Once again, aside from explicitly specifying the serializer type as the template
argument to the :cpp:func:`~mdds::packed_trie_map::load_state` method, it is
identical to the way we did in the previous example.

Let's compare the new instance with the old one to see if the two are equal:

.. literalinclude:: ../../example/packed_trie_state_custom.cpp
   :language: C++
   :start-after: //!code-start: compare
   :end-before: //!code-end: compare
   :dedent: 4

The output says:

.. code-block:: none

    Equal to the original? true

They are.  While we are at it, let's run a simple prefix search to find out
all the US presidents whose first name is 'John':

.. literalinclude:: ../../example/packed_trie_state_custom.cpp
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
