
Populating trie_map
===================

This section illustrates how to use :cpp:class:`~mdds::trie_map` to build a
database of city populations and perform prefix searches.  In this example,
we will use the 2013 populations of cities in North Carolina, and use the city
names as keys.

Let's define the type first:

.. literalinclude:: ../../example/trie_map.cpp
   :language: C++
   :start-after: //!code-start: type
   :end-before: //!code-end: type
   :dedent: 4

The first template argument specifies the key type, while the second template
argument specifies the value type.  In this example we are using ``std::string``
and ``int``, respectively.

Once the type is defined, the next step is instantiation:

.. literalinclude:: ../../example/trie_map.cpp
   :language: C++
   :start-after: //!code-start: inst
   :end-before: //!code-end: inst
   :dedent: 4

It's pretty simple as you don't need to pass any arguments to the constructor.
Now, let's populate this data structure with some population data:

.. literalinclude:: ../../example/trie_map.cpp
   :language: C++
   :start-after: //!code-start: populate
   :end-before: //!code-end: populate
   :dedent: 4

It's pretty straight-forward.  Each :cpp:func:`~mdds::trie_map::insert` call
expects a pair of string key and an integer value.  You can insert your data
in any order regardless of key's sort order.

Now that the data is in, let's perform prefix search to query all cities whose
name begins with "Cha":

.. literalinclude:: ../../example/trie_map.cpp
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

.. literalinclude:: ../../example/trie_map.cpp
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
