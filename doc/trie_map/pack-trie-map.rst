
Creating packed_trie_map from trie_map
======================================

There is also another variant of trie called :cpp:class:`~mdds::packed_trie_map`
which is designed to store all its data in contiguous memory region.  Unlike
:cpp:class:`~mdds::trie_map` which is mutable, :cpp:class:`~mdds::packed_trie_map`
is immutable; once populated, you can only perform queries and it is no longer
possible to add new entries into the container.

One way to create an instance of :cpp:class:`~mdds::packed_trie_map` is from
:cpp:class:`~mdds::trie_map` by calling its :cpp:func:`~mdds::trie_map::pack`
method:

.. literalinclude:: ../../example/trie_map.cpp
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

.. literalinclude:: ../../example/trie_map.cpp
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

.. literalinclude:: ../../example/trie_map.cpp
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

.. literalinclude:: ../../example/trie_map.cpp
   :language: C++
   :start-after: //!code-start: find-none
   :end-before: //!code-end: find-none
   :dedent: 4

will generate the following output:

.. code-block:: none

    Population of Asheboro: not found

The complete source code for the examples in these two sections is available
`here <https://gitlab.com/mdds/mdds/-/blob/master/example/trie_map.cpp>`__.
