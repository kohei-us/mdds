.. SPDX-FileCopyrightText: 2026 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

.. _mtv-example-cloning-event-handler:

Cloning with a custom event handler
===================================

Cloning a container with
:cpp:func:`~mdds::mtv::soa::multi_type_vector::clone()` normally hands the clone
a copy of the source's event handler.  That's usually fine, but sometimes you
want the clone to have a brand-new handler of its own.

One such case is when the handler updates the state of the parent object
that owns the container.  A common approach is to store a back-pointer
to that parent in the handler, so it can update the parent as events
arrive.  Copying the source's handler when the container is cloned would
not be ideal since the copy would still point back at the source's
parent, not the clone's.  Fortunately, you can solve this conundrum by
using the :cpp:func:`clone(event_func) <multi_type_vector
mdds::mtv::soa::multi_type_vector::clone(event_func) const>` overload
that lets you give the clone its own handler instead.

To illustrate, let us model a small drawing application that internally
stores multiple shapes.  Each ``shape`` stores geometry data, but we don't
show it here to keep the example minimal.  Shapes are stored in a plain
copyable element block:

.. literalinclude:: ../../../../example/multi_type_vector/clone_event_handler.cpp
   :language: C++
   :start-after: //!code-start: shape-block
   :end-before: //!code-end: shape-block

In our application, a drawing ``layer`` owns a shape store and tracks how many
element blocks its store is split into - a rough gauge of how fragmented the
layer's storage has become.  The store's event handler holds a back-pointer to
its parent layer and adjusts that count as blocks are acquired and released.
Since the layer and its handler refer to each other, we forward-declare
``layer`` before the handler:

.. literalinclude:: ../../../../example/multi_type_vector/clone_event_handler.cpp
   :language: C++
   :start-after: //!code-start: handler
   :end-before: //!code-end: handler

We plug the handler into the traits as the ``event_func`` and define the store
type:

.. literalinclude:: ../../../../example/multi_type_vector/clone_event_handler.cpp
   :language: C++
   :start-after: //!code-start: traits
   :end-before: //!code-end: traits

Now the ``layer`` itself.  It owns its shape store as a member, constructing it
with a handler that points back to the layer.  Its second constructor clones
another layer: it clones that layer's store with :cpp:func:`clone(event_func)
<multi_type_vector mdds::mtv::soa::multi_type_vector::clone(event_func) const>`,
passing a handler that points back to the new layer so the clone's block events
refer to the new layer rather than to the source.  The handler's methods, which
need the complete ``layer`` type, are defined just after it:

.. literalinclude:: ../../../../example/multi_type_vector/clone_event_handler.cpp
   :language: C++
   :start-after: //!code-start: layer
   :end-before: //!code-end: layer

Now we create a background layer and add a few shapes to it.  Creating the
element block reports an acquisition to the layer's handler, which bumps its
count:

.. literalinclude:: ../../../../example/multi_type_vector/clone_event_handler.cpp
   :language: C++
   :start-after: //!code-start: populate
   :end-before: //!code-end: populate
   :dedent: 4

This prints:

.. code-block:: none

   background block count: 1

The three shapes sit in a single contiguous block, hence a count of one.

Next we clone the whole layer.  The clone constructor clones the shape store
internally, and because we hand it a fresh handler that points back to the new
layer, the copy's block events go to the foreground layer rather than the
background:

.. literalinclude:: ../../../../example/multi_type_vector/clone_event_handler.cpp
   :language: C++
   :start-after: //!code-start: clone-layer
   :end-before: //!code-end: clone-layer
   :dedent: 4

Right after cloning, the two layers' block counts still match.  To see them
diverge, let's edit the foreground layer only.  A layer stores each shape at a
stable slot, so deleting a shape empties its slot rather than shifting the
others.  When we delete a shape from the *middle* of a run, the hole it leaves
splits that run into two blocks, and the newly split-off block is reported to
the foreground layer's handler:

.. literalinclude:: ../../../../example/multi_type_vector/clone_event_handler.cpp
   :language: C++
   :start-after: //!code-start: erase-shape
   :end-before: //!code-end: erase-shape
   :dedent: 4

We can then inspect each layer's count and where each store's handler points:

.. literalinclude:: ../../../../example/multi_type_vector/clone_event_handler.cpp
   :language: C++
   :start-after: //!code-start: observe
   :end-before: //!code-end: observe
   :dedent: 4

giving:

.. code-block:: none

   foreground block count: 2
   background block count after clone: 1
   background handler parent: background
   foreground handler parent: foreground

The two layers' block counts now differ: the foreground's fragmented into two
blocks while the background's stayed at one, confirming the two shape stores are
entirely independent after cloning.  Each store's handler also points back to
its own layer.
