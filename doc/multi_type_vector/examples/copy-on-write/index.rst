.. SPDX-FileCopyrightText: 2026 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

.. highlight:: cpp

Copy-on-write
=============

These examples illustrate how copy-on-write lets copies and clones of a
:cpp:type:`~mdds::multi_type_vector` share their element blocks until the first
write.  See :ref:`mtv-copy-on-write` for the concept and its caveats.

.. toctree::
   :maxdepth: 1

   cow-basics.rst
