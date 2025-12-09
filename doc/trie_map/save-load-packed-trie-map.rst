.. SPDX-FileCopyrightText: 2025 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

Saving and loading packed_trie_map
==================================

There are times when you need to save the state of a :cpp:class:`~mdds::packed_trie_map`
instance to a file, or an in-memory buffer, and load it back later.  Doing that
is now possible by using the :cpp:func:`~mdds::packed_trie_map::save_state` and
:cpp:func:`~mdds::packed_trie_map::load_state` member methods of the
:cpp:class:`~mdds::packed_trie_map` class.

First, let's define the type of use:

.. literalinclude:: ../../example/packed_trie_state_int.cpp
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

.. literalinclude:: ../../example/packed_trie_state_int.cpp
   :language: C++
   :start-after: //!code-start: entries
   :end-before: //!code-end: entries
   :dedent: 4

It's a bit long as it contains entries for 81 cities.  We are then going to
create an instance of the :cpp:class:`~mdds::packed_trie_map` class directly:

.. literalinclude:: ../../example/packed_trie_state_int.cpp
   :language: C++
   :start-after: //!code-start: inst
   :end-before: //!code-end: inst
   :dedent: 4

Let's print the size of the container to make sure the container has been
successfully populated:

.. literalinclude:: ../../example/packed_trie_state_int.cpp
   :language: C++
   :start-after: //!code-start: print-n-cities
   :end-before: //!code-end: print-n-cities
   :dedent: 4

You will see the following output:

.. code-block:: none

    Number of cities: 81

if the container has been successfully populated.  Now, let's run a prefix
search on names beginning with an 'S':

.. literalinclude:: ../../example/packed_trie_state_int.cpp
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

.. literalinclude:: ../../example/packed_trie_state_int.cpp
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

.. literalinclude:: ../../example/packed_trie_state_int.cpp
   :language: C++
   :start-after: //!code-start: load-state-cities
   :end-before: //!code-end: load-state-cities
   :dedent: 4

Here, we used the :cpp:func:`~mdds::packed_trie_map::load_state` method to
restore the state from the file we have previously created.  Let's make sure
that this new instance has content equivalent to that of the original:

.. literalinclude:: ../../example/packed_trie_state_int.cpp
   :language: C++
   :start-after: //!code-start: check-equal
   :end-before: //!code-end: check-equal
   :dedent: 4

If you see the following output:

.. code-block:: none

    Equal to the original? true

then this new instance has equivalent contant as the original one.  Let's also
make sure that it contains the same number of entries as the original:

.. literalinclude:: ../../example/packed_trie_state_int.cpp
   :language: C++
   :start-after: //!code-start: print-n-cities-2
   :end-before: //!code-end: print-n-cities-2
   :dedent: 4

Hopefully you will see the following output:

.. code-block:: none

    Number of cities: 81

Lastly, let's run on this new instance the same prefix search we did on the
original instance, to make sure we still get the same results:

.. literalinclude:: ../../example/packed_trie_state_int.cpp
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
