.. SPDX-FileCopyrightText: 2025 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

.. highlight:: cpp

Quick Start
===========

Let's go over how to use :cpp:class:`~mdds::sorted_string_map` with a code
example.  In this example, we will define an enum type that includes all 50 US
states, and associate them with their names as string keys.

Let's include some headers first:

.. literalinclude:: ../../example/sorted_string_map.cpp
   :language: C++
   :start-after: //!code-start: header
   :end-before: //!code-end: header
   :dedent: 0

We include ``<mdds/sorted_string_map.hpp>`` to make the
:cpp:class:`~mdds::sorted_string_map` class available, and ``<iostream>`` so
that we can print some debug statements.

Next, let's define the enum type for the US states:

.. literalinclude:: ../../example/sorted_string_map.cpp
   :language: C++
   :start-after: //!code-start: enum
   :end-before: //!code-end: enum
   :dedent: 0

This enum type enumerates values for all 50 states plus ``unknown`` as its first
member.  This value is inplicitly assigned the value of 0 by virtue of it being
the first member, and we will be using it as the null value for the container.
A null value is the value returned by the
:cpp:func:`~mdds::sorted_string_map::find()` method when it fails to find an
associated value for the key passed by the caller.  It is required during
construction.

The next step is to define a concrete type:

.. literalinclude:: ../../example/sorted_string_map.cpp
   :language: C++
   :start-after: //!code-start: type
   :end-before: //!code-end: type
   :dedent: 0

We are defining a type alias named ``us_state_map_type`` that uses
:cpp:class:`~mdds::sorted_string_map` with ``us_state_t`` as its value type.

Once the concrete type is defined, we can then define the key-value entries as
a static array of type :cpp:type:`mdds::sorted_string_map::entry_type`:

.. literalinclude:: ../../example/sorted_string_map.cpp
   :language: C++
   :start-after: //!code-start: entries
   :end-before: //!code-end: entries
   :dedent: 0

Each entry contains two members of types :cpp:class:`std::string_view` and
:cpp:type:`mdds::sorted_string_map::value_type`, respectively.  Here, you must
ensure that these entries are sorted alphabetically by the keys in ascending
order.

Now we can construct an instance of this container.  To create an instance, you
must pass the pointer to the key-value entries array, the size of the array, and
the null value:

.. literalinclude:: ../../example/sorted_string_map.cpp
   :language: C++
   :start-after: //!code-start: init
   :end-before: //!code-end: init
   :dedent: 4

We are marking it as a const instance since it's immutable; its state won't
change once it is constructed.

Now we can perform some lookups by calling the
:cpp:func:`~mdds::sorted_string_map::find()` method:

.. literalinclude:: ../../example/sorted_string_map.cpp
   :language: C++
   :start-after: //!code-start: find
   :end-before: //!code-end: find
   :dedent: 4

Running this code generates the following output:

.. code-block:: none

   virginia? true
   north_carolina? true

When calling :cpp:func:`~mdds::sorted_string_map::find()` with a string key not
included in the pre-defined entries, it will return the null value:

.. literalinclude:: ../../example/sorted_string_map.cpp
   :language: C++
   :start-after: //!code-start: find-null
   :end-before: //!code-end: find-null
   :dedent: 4

Running this code generates the following output:

.. code-block:: none

   unknown? true

You can also perform reverse lookup of finding a key from a value by calling the
:cpp:func:`~mdds::sorted_string_map::find_key()` method:

.. literalinclude:: ../../example/sorted_string_map.cpp
   :language: C++
   :start-after: //!code-start: find-key
   :end-before: //!code-end: find-key
   :dedent: 4

Running this code generates the following output:

.. code-block:: none

   key for rhode_island: Rhode Island

Note however, that :cpp:func:`~mdds::sorted_string_map::find_key()` by default
performs linear search through the pre-defined entries to find the key from the
value, so its runtime performance is not as efficient as that of
:cpp:func:`~mdds::sorted_string_map::find()`, which performs binary search.

The algorithm used in :cpp:func:`~mdds::sorted_string_map::find_key()` depends
on the second template argument passed to the
:cpp:class:`~mdds::sorted_string_map` class, which by default is assigned the
type :cpp:class:`mdds::ssmap::linear_key_finder`.  In case linear search is not
desirable, one can specify :cpp:class:`mdds::ssmap::hash_key_finder` as the
second template argument to switch to a hash-table based lookup algorithm
instead.
