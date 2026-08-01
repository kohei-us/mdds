.. SPDX-FileCopyrightText: 2026 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

.. _mtv-example-cow-detach:

Detaching before mutable access
===============================

The non-const overloads of
:cpp:func:`~mdds::mtv::soa::multi_type_vector::position`,
:cpp:func:`~mdds::mtv::soa::multi_type_vector::begin`,
:cpp:func:`~mdds::mtv::soa::multi_type_vector::end`,
:cpp:func:`~mdds::mtv::soa::multi_type_vector::rbegin` and
:cpp:func:`~mdds::mtv::soa::multi_type_vector::rend` provide mutable access to
the container's element blocks.  Calling any of them while the container is
still borrowing shared blocks throws
:cpp:class:`~mdds::mtv::shared_block_error`.  This example shows the reason
behind it, and how :cpp:func:`~mdds::mtv::soa::multi_type_vector::detach()`
resolves it.

This example also covers
:cpp:func:`~mdds::mtv::soa::multi_type_vector::release()`.  Being a mutator,
it follows the same rule as every other mutator under copy-on-write: on a
sharing container it detaches automatically rather than throwing, since its
name alone states the intent to mutate and there is no const overload to
confuse it with.  It still warrants a closer look here, because it is the one
mutator that transfers stored objects to the caller, and that makes the
automatic detach observable: the object the caller receives is from the
container's private post-detach clone, not from the shared block prior to
the call.

We reuse the noncopyable managed-block pattern from the :ref:`previous
example <mtv-example-cow-noncopyable>`: but instead of using a
``shape``, we use an ``image`` that owns a potentially large pixel
buffer, stored by pointer, with a ``clone_value`` specialization so the
block can be cloned.  The following block defines this ``image`` type,
ID and type of the block that stores it, callbacks required by the block
and the ``clone_value`` specialization:

.. literalinclude:: ../../../../example/multi_type_vector/cow_detach.cpp
   :language: C++
   :start-after: //!code-start: image-def
   :end-before: //!code-end: image-def

The next code enables copy-on-write in the traits and define the container type:

.. literalinclude:: ../../../../example/multi_type_vector/cow_detach.cpp
   :language: C++
   :start-after: //!code-start: cow-traits
   :end-before: //!code-end: cow-traits

Now we build a photo album and take a snapshot of it for undo.  A noncopyable
block cannot be copy-constructed, so we snapshot with
:cpp:func:`~mdds::mtv::soa::multi_type_vector::clone()`; under copy-on-write the
clone is cheap, borrowing the album's blocks rather than duplicating every
image:

.. literalinclude:: ../../../../example/multi_type_vector/cow_detach.cpp
   :language: C++
   :start-after: //!code-start: setup
   :end-before: //!code-end: setup
   :dedent: 4

Suppose we now want to rename the first image in place, editing it through a
mutable position object.  Because the album is still sharing its blocks with
the snapshot, the non-const ``position()`` refuses and throws:

.. literalinclude:: ../../../../example/multi_type_vector/cow_detach.cpp
   :language: C++
   :start-after: //!code-start: edit-throws
   :end-before: //!code-end: edit-throws
   :dedent: 4

This code prints:

.. code-block:: none

   mutable access failed: multi_type_vector::position: container is shared; call detach() first

The refusal is intentional.  While the album is borrowing, its images
*are* the snapshot's images - the same objects.  Editing an image
through a mutable position object would change the image the snapshot
still holds access to, silently corrupting the undo state it exists to
preserve.

Calling :cpp:func:`~mdds::mtv::soa::multi_type_vector::detach()` first
breaks the sharing: the album makes its own private copies of the images
while the snapshot keeps the originals.  With the album now the sole
owner of its blocks, the same mutable access succeeds, and the edit
stays private to the album:

.. literalinclude:: ../../../../example/multi_type_vector/cow_detach.cpp
   :language: C++
   :start-after: //!code-start: detach-then-edit
   :end-before: //!code-end: detach-then-edit
   :dedent: 4

giving:

.. code-block:: none

   album now has: beach_edited.jpg
   snapshot still has: beach.jpg

Finally we pull an image out of the album with
:cpp:func:`~mdds::mtv::soa::multi_type_vector::release` to hand it off
elsewhere.  Unlike the accessors above, it never requires an explicit detach:
even if the album were still sharing, it would detach automatically like any
other mutating method.  In our flow the
album has already detached, so the call simply succeeds.  The point to observe
is which object comes back: the extracted image is the album's private copy
created by that detach, while the snapshot still holds the originally stored
object:

.. literalinclude:: ../../../../example/multi_type_vector/cow_detach.cpp
   :language: C++
   :start-after: //!code-start: release-clone
   :end-before: //!code-end: release-clone
   :dedent: 4

giving:

.. code-block:: none

   extracted: sunset.jpg
   extracted is the snapshot's object: false

The last line confirms the extracted image is a distinct object from the one
the snapshot holds at the same position.

Although not used in this example,
:cpp:func:`~mdds::mtv::soa::multi_type_vector::release_range()` also detaches
automatically just like
:cpp:func:`~mdds::mtv::soa::multi_type_vector::release()`.

See :ref:`mtv-copy-on-write` for the mutable-access rules in general.
