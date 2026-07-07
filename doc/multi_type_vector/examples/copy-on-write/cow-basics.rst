.. SPDX-FileCopyrightText: 2026 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

.. _mtv-example-cow-basics:

Copy-on-write basics
====================

This example walks through the basic share-then-detach lifecycle of a
copy-on-write ``multi_type_vector``: a copy initially shares the source's
element blocks, and only makes its own private copy when it is first written
to.

Copy-on-write is a compile-time opt-in.  Derive a traits type from an existing
one and set :cpp:var:`~mdds::mtv::default_traits::enable_cow` to ``true``:

.. literalinclude:: ../../../../example/multi_type_vector/cow.cpp
   :language: C++
   :start-after: //!code-start: cow-traits
   :end-before: //!code-end: cow-traits

Here we extend :cpp:struct:`~mdds::mtv::standard_element_blocks_traits` so the
container can store the standard element types out of the box.

Let's create an instance and populate it with a few values:

.. literalinclude:: ../../../../example/multi_type_vector/cow.cpp
   :language: C++
   :start-after: //!code-start: populate
   :end-before: //!code-end: populate
   :dedent: 4

Now we make a copy.  Under copy-on-write the copy does not duplicate the
element blocks; it *borrows* them from the source.  We can observe this by
comparing the block pointers exposed by the iterator's ``data`` member, which
point to the same block for both containers:

.. literalinclude:: ../../../../example/multi_type_vector/cow.cpp
   :language: C++
   :start-after: //!code-start: copy-shares
   :end-before: //!code-end: copy-shares
   :dedent: 4

which prints:

.. code-block:: none

   blocks shared right after copy: true

The first write to the copy triggers a *detach*: the copy makes its own private
copy of the store before modifying it, so the two containers no longer share
the block:

.. literalinclude:: ../../../../example/multi_type_vector/cow.cpp
   :language: C++
   :start-after: //!code-start: detach-on-write
   :end-before: //!code-end: detach-on-write
   :dedent: 4

which now prints:

.. code-block:: none

   blocks shared after the write: false

Because the copy detached before writing, the source is left untouched:

.. literalinclude:: ../../../../example/multi_type_vector/cow.cpp
   :language: C++
   :start-after: //!code-start: original-intact
   :end-before: //!code-end: original-intact
   :dedent: 4

giving:

.. code-block:: none

   original[0] = 1.1
   copied[0] = 9.9

The detach happens automatically on the first mutation, so under normal use you
never need to trigger it manually.  See :ref:`mtv-copy-on-write` for the full
semantics and caveats.
