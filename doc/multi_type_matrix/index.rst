.. SPDX-FileCopyrightText: 2025 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

.. highlight:: cpp

Multi Type Matrix
=================

:cpp:class:`~mdds::multi_type_matrix` is a two-dimensional matrix that can
store elements of different types.  Internally it uses
:cpp:type:`~mdds::multi_type_vector` as its value store, and stores the
element values in column-major order.

.. toctree::
   :maxdepth: 1

   traits.rst
   custom-types.rst
   api.rst
