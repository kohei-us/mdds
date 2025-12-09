.. SPDX-FileCopyrightText: 2025 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

Sorted String Map
=================

Sorted string map is an immutable associative container that provides an
efficient way to map string keys to values of a user-specified type.  Both keys
and values must be known at compile time and the key-value entries must be
sorted by the keys in ascending order.  The lookup algorithm takes advantage of
the pre-sorted entries and performs binary search to find the value from a key.

Besides the minimal amount of memory required to store the size and memory
address of the caller-provided key-value entries and a few extra data, it does
not allocate any additional memory; it simply re-uses the caller-provided
key-value entries in all of its operations.

.. toctree::
   :maxdepth: 1

   quickstart.rst
   api.rst
