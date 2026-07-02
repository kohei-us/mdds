.. SPDX-FileCopyrightText: 2026 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

.. _mtv-example-cloning-block:

Block-level Cloning
===================

The :ref:`previous section <mtv-example-cloning>` showed how to make a
non-copyable block cloneable by specializing
:cpp:class:`~mdds::mtv::clone_value` for its value type.  That approach
clones one element at a time, which is all you need when each element can
be duplicated on its own.

Sometimes, however, an element cannot be cloned in isolation because it
shares state with the other elements in the same block.  In that case you
can specialize :cpp:class:`~mdds::mtv::clone_block` instead, which hands
you the whole block and lets you decide how to reconstruct it.  In fact,
the per-element ``clone_value`` path is just a convenience layer built on
top of ``clone_block``; specializing ``clone_block`` directly gives you
full control over the cloning of an entire block.

In the example below, every element references a string in a pool that is
shared across the entire block.  We want to clone that shared pool exactly
once per block rather than once per element, which cannot be done through
the ``clone_value`` specialization path.

First, let's define the shared pool and the value type that references it:

.. literalinclude:: ../../../../example/multi_type_vector/clone_block.cpp
   :language: C++
   :start-after: //!code-start: pooled-string
   :end-before: //!code-end: pooled-string

Since ``pooled_string`` is non-copyable, we'll store its instances as
pointers in a block of type
:cpp:class:`~mdds::mtv::noncopyable_managed_element_block`:

.. literalinclude:: ../../../../example/multi_type_vector/clone_block.cpp
   :language: C++
   :start-after: //!code-start: elem-block-def
   :end-before: //!code-end: elem-block-def

Once the element block type is defined, plug it into the traits type and
define the actual ``multi_type_vector`` type:

.. literalinclude:: ../../../../example/multi_type_vector/clone_block.cpp
   :language: C++
   :start-after: //!code-start: mtv-def
   :end-before: //!code-end: mtv-def

Next, we'll define a template specialization for cloning a whole
``pooled_string_block_type`` instance:

.. literalinclude:: ../../../../example/multi_type_vector/clone_block.cpp
   :language: C++
   :start-after: //!code-start: clone-block-ts
   :end-before: //!code-end: clone-block-ts

The specialization must be a struct, must be in the ``mdds::mtv``
namespace, and must have a public method whose signature is ``BlockT*
operator()(const BlockT&) const`` where ``BlockT`` is the block type that
it is specialized for.  Because the whole block is passed in, we can clone
the shared pool a single time and have every cloned element point to the
new pool.

Since we have all necessary pieces defined, let's instantiate our
``multi_type_vector`` instance and populate it with elements that all share
one pool:

.. literalinclude:: ../../../../example/multi_type_vector/clone_block.cpp
   :language: C++
   :start-after: //!code-start: populate-store
   :end-before: //!code-end: populate-store
   :dedent: 4

Let's clone this instance by calling the
:cpp:func:`~mdds::mtv::soa::multi_type_vector::clone()` method:

.. literalinclude:: ../../../../example/multi_type_vector/clone_block.cpp
   :language: C++
   :start-after: //!code-start: clone-store
   :end-before: //!code-end: clone-store
   :dedent: 4

This will internally call the ``clone_block`` specialization we defined
earlier.  Finally, let's inspect both containers to confirm that each
element was cloned and that the original and the clone reference separate
pools:

.. literalinclude:: ../../../../example/multi_type_vector/clone_block.cpp
   :language: C++
   :start-after: //!code-start: inspect-store
   :end-before: //!code-end: inspect-store
   :dedent: 4

You will see output similar to the following when executing this code:

.. code-block:: none

   cloning the container...
   cloning a pool of 4 strings shared by 4 elements
   done cloning
   element 0: "alpha" (pool is separate)
   element 1: "gamma" (pool is separate)
   element 2: "beta" (pool is separate)
   element 3: "delta" (pool is separate)

The output indicates that the shared pool was cloned only once for the
whole block, and that the cloned container references its own pool that is
fully independent of the original.
