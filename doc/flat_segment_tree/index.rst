.. SPDX-FileCopyrightText: 2025 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

.. highlight:: cpp

Flat Segment Tree
=================

Flat segment tree is a derivative of `segment tree
<https://en.wikipedia.org/wiki/Segment_tree>`_, and is designed to store
non-overlapping 1-dimensional range values such that *the values of the
neighboring ranges are guaranteed to be different.*  An insertion of a range
value into this structure will always overwrite one or more existing ranges
that overlap with the new range.  If an insertion of a new range would cause
any adjacent ranges to have the equal value, those ranges will be merged into
one range.

An instance of this structure is initialized with fixed lower and upper
bounaries, which will not change throughout the life time of the instance.

The flat segment tree structure consists of two parts: the leaf-node part
which also forms a doubly-linked list, and the non-leaf-node part which forms
a balanced-binary tree and is used only when performing tree-based queries.
The range values are stored in the leaf-nodes, while the non-leaf nodes are
used only for queries.

.. toctree::
   :maxdepth: 1

   quickstart.rst
   iterate-segments.rst
   api.rst

