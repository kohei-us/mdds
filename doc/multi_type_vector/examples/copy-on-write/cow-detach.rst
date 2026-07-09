.. SPDX-FileCopyrightText: 2026 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

.. _mtv-example-cow-detach:

Detaching before releasing elements
===================================

:cpp:func:`~mdds::mtv::soa::multi_type_vector::release` and
:cpp:func:`~mdds::mtv::soa::multi_type_vector::release_range` hand ownership of
stored elements back to the caller.  They require the container to be the sole
owner of its blocks, so calling either one while the container is still
borrowing throws :cpp:class:`~mdds::mtv::shared_block_error`.  This example
shows why, and how :cpp:func:`~mdds::mtv::soa::multi_type_vector::detach()`
resolves it.

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

Suppose we now want to pull the first two images out of the album to hand them
off elsewhere.  Because the album is still sharing its blocks with the
snapshot, ``release_range`` refuses and throws:

.. literalinclude:: ../../../../example/multi_type_vector/cow_detach.cpp
   :language: C++
   :start-after: //!code-start: release-throws
   :end-before: //!code-end: release-throws
   :dedent: 4

This code prints:

.. code-block:: none

   release failed: multi_type_vector::release_range: requires sole ownership; call detach() first

The refusal is deliberate.  While the album is borrowing, its images
*are* the snapshot's images - the same objects.  Handing them to the
caller via
:cpp:func:`~mdds::mtv::soa::multi_type_vector::release_range()` would
give the caller the appearance of exclusive ownership of them when in
fact the snapshot is still borrowing them.  If the caller then frees the
images, it would leave the snapshot pointing at deleted objects, and if
the caller edits them it would change the images the snapshot still
holds access to.

Calling :cpp:func:`~mdds::mtv::soa::multi_type_vector::detach()` first
breaks the sharing: the album makes its own private copies of the images
while the snapshot keeps the originals.  With the album now the sole
owner of its blocks,
:cpp:func:`~mdds::mtv::soa::multi_type_vector::release_range()`
succeeds.  To show the released images really are the album's own
copies, we rename one of them and then read the snapshot's image at the
same position:

.. literalinclude:: ../../../../example/multi_type_vector/cow_detach.cpp
   :language: C++
   :start-after: //!code-start: detach-then-release
   :end-before: //!code-end: detach-then-release
   :dedent: 4

giving:

.. code-block:: none

   extracted and renamed: beach_edited.jpg and sunset.jpg
   snapshot still has: beach.jpg

The last line confirms the two are independent: renaming the released image did
not touch the snapshot's copy, which still carries the original name.

See :ref:`mtv-copy-on-write` for the release precondition in general.
