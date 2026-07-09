.. SPDX-FileCopyrightText: 2026 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

.. highlight:: cpp

.. _mtv-copy-on-write:

Copy-on-write
=============

Overview
--------

When many copies of a container are made but only a few of them are ever
modified, eagerly duplicating every element block on each copy is wasted work.
Copy-on-write (COW) avoids that cost: a copied or cloned container shares the
source's element blocks instead of duplicating them, and defers the duplication
until the first time one of the sharers is modified.  At that point, and only
then, the container being modified makes its own private copy of the storage.

COW is an opt-in, compile-time feature.  When it is disabled - which is the
default - the container behaves exactly as before and carries no runtime or
memory overhead for it.

.. note::

   Copy-on-write is currently available only for the structure-of-arrays (SoA)
   variant :cpp:class:`mdds::mtv::soa::multi_type_vector`, which is the variant
   aliased by :cpp:type:`mdds::multi_type_vector`.

Enabling copy-on-write
----------------------

Copy-on-write is controlled by the
:cpp:var:`~mdds::mtv::default_traits::enable_cow` trait value, which defaults to
``false``.  To turn it on, derive your own trait type from
:cpp:struct:`mdds::mtv::default_traits` and set the value to ``true``::

    struct my_traits : mdds::mtv::default_traits
    {
        static constexpr bool enable_cow = true;
    };

    using mtv_type = mdds::mtv::soa::multi_type_vector<my_traits>;

Because the flag is a compile-time constant, the sharing mechanism is compiled
in only when it is enabled; a container whose ``enable_cow`` trait is ``false``
has none of the block-sharing code compiled in.

Sharing and detaching
---------------------

With COW enabled, the copy constructor, the copy-assignment operator, and the
:cpp:func:`~mdds::mtv::soa::multi_type_vector::clone()` method all share the
source's element blocks rather than duplicating them.  Sharing is symmetric: the
source becomes a borrower too, so after the copy neither container is the sole
owner of the blocks - both refer to the same shared storage and both are
considered borrowers.

The deferred duplication happens automatically on the first mutation.  When a
borrowing container is modified, it *detaches*: it makes a private, deep copy of
the whole store, switches to being the sole owner of the blocks, and then
applies the mutation.  The other borrowers are unaffected and keep referring to
the original shared storage.  Detaching happens once; subsequent mutations
operate in place on the now solely-owned blocks.

How detaching duplicates blocks
-------------------------------

Detaching duplicates each element block with ``clone_block``, not ``copy_block``.
For a copyable block, ``clone_block`` uses the block's copy constructor, so the
result is identical to what an eager copy would have produced.  For a
non-copyable or managed block it uses the block's
:cpp:class:`~mdds::mtv::clone_value` or :cpp:class:`~mdds::mtv::clone_block`
specialization - the very same specialization that makes
:cpp:func:`~mdds::mtv::soa::multi_type_vector::clone()` work.

The practical consequence is that a container of non-copyable blocks can use
copy-on-write as long as those blocks provide a ``clone_value`` or
``clone_block`` specialization.  These are exactly the specializations covered
by the :ref:`cloning <mtv-example-cloning>` and
:ref:`block-level cloning <mtv-example-cloning-block>` examples; the
:ref:`mtv-example-cow-noncopyable` example puts one to work under
copy-on-write.

There is one restriction.  A value type stored in a copyable block must not
also specialize ``clone_value`` or ``clone_block``; a ``static_assert`` rejects
it at compile time.  A copyable block already has a copy constructor, and adding
such a specialization would give it a second, independent way to be duplicated,
so a plain copy and a clone of the same block could produce different results.
These specializations are therefore reserved for non-copyable blocks, which have
no copy constructor to fall back on.

.. warning::

   Do not specialize ``clone_value`` or ``clone_block`` for a value type stored
   in a copyable block; a ``static_assert`` rejects it at compile time.

Forcing sole ownership with detach()
------------------------------------

Mutating methods detach automatically, so under normal use you never need to
detach by hand.  When you do need to force sole ownership up front, call
:cpp:func:`~mdds::mtv::soa::multi_type_vector::detach()`.  It performs the
deferred duplication immediately when the container is borrowing, and is a no-op
when the container already owns its blocks or when COW is disabled.

One such use case is when you need to release elements from the ownership of
the container.  Both
:cpp:func:`~mdds::mtv::soa::multi_type_vector::release` and
:cpp:func:`~mdds::mtv::soa::multi_type_vector::release_range` hand ownership of
stored elements back to the caller.  If the container were still borrowing,
however, those released elements - for a managed block, pointers to objects it
owns - would still point into storage shared with other containers.  Any
modification made to those "released" elements would therefore affect the
shared storage and all its borrowers - not the expected behavior.  This is why
the container throws :cpp:class:`~mdds::mtv::shared_block_error` when attempting
to release elements while it is still borrowing.

.. warning::

   :cpp:func:`~mdds::mtv::soa::multi_type_vector::release` and
   :cpp:func:`~mdds::mtv::soa::multi_type_vector::release_range` require the
   container to be the sole owner of its blocks.  Called while the container is
   still borrowing, they throw :cpp:class:`~mdds::mtv::shared_block_error`.
   Call :cpp:func:`~mdds::mtv::soa::multi_type_vector::detach()` first.

Iterators under copy-on-write
-----------------------------

When copy-on-write is on, a container hands out read-only iterators only.  The
non-const ``begin``, ``end``, ``rbegin``, ``rend``, and ``position`` accessors
are disabled for the whole type - not just while it happens to be sharing - so
``v.begin()`` always gives you a ``const_iterator``.  That is deliberate: a
mutable iterator would let you edit shared blocks directly and skip the private
copy that is meant to happen on the first write.  Modify a container through its
own methods instead, which detach automatically as needed.

Copying a const source is not read-only
---------------------------------------

Copying or cloning a container writes to an internal member of the source, even
when the source is const, because the block sharing happens lazily.  So, unlike
most operations on a const object, copying one is not safe to do from several
threads at once on the same source.

For example, handing the same source to two threads and letting each make its
own copy is a data race:

.. code-block:: cpp

    const mtv_type src = make_source();

    // BAD: both threads copy the same source at once, and each copy writes to
    // src's internal sharing state.
    std::thread t1([&] { mtv_type copy(src); /* ... */ });
    std::thread t2([&] { mtv_type copy(src); /* ... */ });

Make the copies up front, before the handoff, so nothing writes to ``src``
concurrently:

.. code-block:: cpp

    const mtv_type src = make_source();

    mtv_type a(src); // both copies made on one thread
    mtv_type b(src);

    std::thread t1([&] { /* use a */ });
    std::thread t2([&] { /* use b */ });

.. warning::

   It is not safe to copy or clone the same source from multiple threads
   concurrently.
