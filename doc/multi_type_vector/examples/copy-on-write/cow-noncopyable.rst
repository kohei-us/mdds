.. SPDX-FileCopyrightText: 2026 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

.. _mtv-example-cow-noncopyable:

Copy-on-write with a noncopyable block
======================================

Copy-on-write works with noncopyable element blocks, as long as those
blocks are cloneable.  This example uses a
:cpp:class:`~mdds::mtv::noncopyable_managed_element_block`: copy
construction of the container is still prohibited, but
:cpp:func:`~mdds::mtv::soa::multi_type_vector::clone()` succeeds by
sharing the blocks, and a later write detaches by cloning them.

First, let's define a value type we don't want to allow copying of.  For this
example we'll use a ``shape``: a drawing primitive defined by a sequence of
vertices.  A complex shape can accumulate a large number of vertices, so
copying one can be expensive.  To guard against duplicating that state by
accident, we delete its copy constructor:

.. literalinclude:: ../../../../example/multi_type_vector/cow_noncopyable.cpp
   :language: C++
   :start-after: //!code-start: shape-def
   :end-before: //!code-end: shape-def

Because ``shape`` instances are heap-allocated and cannot be copied, we store
them as pointers in a :cpp:class:`~mdds::mtv::noncopyable_managed_element_block`,
which takes ownership of the pointed-to objects and disposes of them when they
are removed.  We assign the block a unique element type ID and register the
callbacks the container needs to manage it:

.. literalinclude:: ../../../../example/multi_type_vector/cow_noncopyable.cpp
   :language: C++
   :start-after: //!code-start: elem-block-def
   :end-before: //!code-end: elem-block-def

To make the block cloneable we specialize ``clone_value`` for its
pointer type in the ``mdds::mtv`` namespace, exactly as we would to make
:cpp:func:`~mdds::mtv::soa::multi_type_vector::clone()` work without
copy-on-write (see :ref:`mtv-example-cloning`):

.. literalinclude:: ../../../../example/multi_type_vector/cow_noncopyable.cpp
   :language: C++
   :start-after: //!code-start: clone-value-ts
   :end-before: //!code-end: clone-value-ts

``clone_value`` duplicates one element at a time.  While not covered in
this example, for a block whose value cannot be cloned in isolation,
specialize ``clone_block`` instead to clone the whole block (see
:ref:`mtv-example-cloning-block`); the same specialization then serves
both ``clone()`` and copy-on-write detaching.

Now enable copy-on-write in the traits and define the container type:

.. literalinclude:: ../../../../example/multi_type_vector/cow_noncopyable.cpp
   :language: C++
   :start-after: //!code-start: cow-traits
   :end-before: //!code-end: cow-traits

Let's create an instance and put two shapes into it:

.. literalinclude:: ../../../../example/multi_type_vector/cow_noncopyable.cpp
   :language: C++
   :start-after: //!code-start: populate
   :end-before: //!code-end: populate
   :dedent: 4

Copy-constructing this container throws, just as it would without copy-on-write:

.. literalinclude:: ../../../../example/multi_type_vector/cow_noncopyable.cpp
   :language: C++
   :start-after: //!code-start: copy-throws
   :end-before: //!code-end: copy-throws
   :dedent: 4

Note that catching the exception here is safe as the state of the source
instance is left intact.  This code prints:

.. code-block:: none

   copy failed: attempted to copy a noncopyable element block
   source still holds a shape with 4 vertices

Cloning, on the other hand, succeeds and shares the block instead of
copying it.  The following code clones the source and compares
the memory addresses of the first blocks in the source and cloned instances:

.. literalinclude:: ../../../../example/multi_type_vector/cow_noncopyable.cpp
   :language: C++
   :start-after: //!code-start: clone-shares
   :end-before: //!code-end: clone-shares
   :dedent: 4

This code prints:

.. code-block:: none

   blocks shared after clone: true

Writing to the clone detaches it.  It uses the ``clone_value``
specialization we defined earlier to handle cloning of the stored
values, so the two containers end up with independent blocks.  The following
code first writes a pentagon shape to position 0 to overwrite it, then compares
it with the shape stored in the same position in the source:

.. literalinclude:: ../../../../example/multi_type_vector/cow_noncopyable.cpp
   :language: C++
   :start-after: //!code-start: detach-clones
   :end-before: //!code-end: detach-clones
   :dedent: 4

giving:

.. code-block:: none

   blocks shared after write: false
   source[0] vertices: 4
   clone[0] vertices: 5

See :ref:`mtv-copy-on-write` for how detaching duplicates blocks in general.
