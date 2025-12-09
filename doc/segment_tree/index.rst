.. SPDX-FileCopyrightText: 2025 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

.. highlight:: cpp

Segment Tree
============

`Segment tree <https://en.wikipedia.org/wiki/Segment_tree>`_ is a data structure
designed for storing one-dimensional intervals or segments, either overlapping
or non-overlapping.  It is useful for detecting all the segments that contain a
specific point.  Each segment has start and end positions where the start
position is inclusive while the end position is not.  The version of segment
tree implemented in mdds allows associating a value with each segment so that
you can use it as an associative container.

.. toctree::
   :maxdepth: 1

   quickstart.rst
   remove-segments.rst
   perf.rst
   api.rst

