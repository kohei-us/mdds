.. SPDX-FileCopyrightText: 2026 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

.. _mtv-example-cloning-stateful:

Stateful Cloning
================

The :ref:`previous section <mtv-example-cloning>` showed how a
``clone_value`` specialization makes a non-copyable block cloneable by
cloning one element at a time.  This section builds on that and
illustrates how the cloner itself can carry state across the elements of
a block.

When :cpp:func:`~mdds::mtv::soa::multi_type_vector::clone()` clones a
block, it creates exactly one instance of the ``clone_value``
specialization for that block, and applies it to the elements strictly in
the order they are stored in the block.  If an exception is thrown mid-way
through a block, the partially built copy of the block gets destroyed,
which in turn destroys the values cloned up to that point.  Thanks to the
first two guarantees, the ``operator()`` of a specialization may be
non-const, which allows the cloner to carry state across the elements of a
block while cloning.

.. warning::

   These guarantees, including the exception and memory leak safety, only
   apply when the ``clone_value`` specialization does not declare an
   ``exec_policy`` type.  Refer to the :ref:`mtv-example-exec-policy`
   section for how a specialization with an ``exec_policy`` behaves.

To see how a stateful cloner can be useful, consider a scenario where each
element references a string stored in a pool shared by all elements in the
same block.  When cloning such elements, the pool itself should be cloned
only once per block, with all the cloned elements referencing the new
pool.  This requires the cloner to remember the cloned pool between its
invocations, which is exactly what a stateful cloner enables.

First, let's define the shared pool and the value type that references it:

.. literalinclude:: ../../../../example/multi_type_vector/clone_stateful.cpp
   :language: C++
   :start-after: //!code-start: pooled-string
   :end-before: //!code-end: pooled-string

Since ``pooled_string`` is non-copyable, we'll store its instances as
pointers in a block of type
:cpp:class:`~mdds::mtv::noncopyable_managed_element_block`:

.. literalinclude:: ../../../../example/multi_type_vector/clone_stateful.cpp
   :language: C++
   :start-after: //!code-start: elem-block-def
   :end-before: //!code-end: elem-block-def

Once the element block type is defined, plug it into the traits type and
define the actual ``multi_type_vector`` type:

.. literalinclude:: ../../../../example/multi_type_vector/clone_stateful.cpp
   :language: C++
   :start-after: //!code-start: mtv-def
   :end-before: //!code-end: mtv-def

Next, we'll define the ``clone_value`` specialization, this time with a
non-const ``operator()`` and a member variable that holds the cloned pool:

.. literalinclude:: ../../../../example/multi_type_vector/clone_stateful.cpp
   :language: C++
   :start-after: //!code-start: clone-value-ts
   :end-before: //!code-end: clone-value-ts

The first invocation clones the pool and stores it in the ``cloned_pool``
member, and all subsequent invocations simply re-use it.  Since one cloner
instance clones the elements of exactly one block, the pool gets cloned
exactly once per block.

Since we have all necessary pieces defined, let's instantiate our
``multi_type_vector`` instance and populate it with elements that all
share one pool:

.. literalinclude:: ../../../../example/multi_type_vector/clone_stateful.cpp
   :language: C++
   :start-after: //!code-start: populate-store
   :end-before: //!code-end: populate-store
   :dedent: 4

Let's clone this instance by calling the
:cpp:func:`~mdds::mtv::soa::multi_type_vector::clone()` method:

.. literalinclude:: ../../../../example/multi_type_vector/clone_stateful.cpp
   :language: C++
   :start-after: //!code-start: clone-store
   :end-before: //!code-end: clone-store
   :dedent: 4

Finally, let's inspect both containers to confirm that each element was
cloned and that the original and the clone reference separate pools:

.. literalinclude:: ../../../../example/multi_type_vector/clone_stateful.cpp
   :language: C++
   :start-after: //!code-start: inspect-store
   :end-before: //!code-end: inspect-store
   :dedent: 4

You will see output similar to the following when executing this code:

.. code-block:: none

   cloning the container...
   cloning a pool of 4 strings
   done cloning
   element 0: "alpha" (pool is separate)
   element 1: "gamma" (pool is separate)
   element 2: "beta" (pool is separate)
   element 3: "delta" (pool is separate)

The output indicates that the shared pool was cloned only once for the
whole block, and that the cloned container references its own pool fully
independent of the original.

When even a stateful cloner does not fit your scenario and you need full
control over how an entire block gets reconstructed, you can take over
the cloning of the whole block as described in
:ref:`mtv-example-cloning-block`.
