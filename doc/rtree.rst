
.. highlight:: cpp

R-tree
======

Overview
--------

`R-tree <https://en.wikipedia.org/wiki/R-tree>`_ is a tree-based data
structure designed for optimal query performance on multi-dimensional spatial
objects with rectangular bounding shapes.  The R-tree implementation included
in this library is a variant of R-tree known as `R*-tree
<https://en.wikipedia.org/wiki/R*_tree>`_ which differs from the original
R-tree in that it may re-insert an object if it causes the original target
directory to overflow in order to keep the overall tree more balanced.


Code Example
------------

TODO


API Reference
-------------

.. doxygenclass:: mdds::rtree
   :members:

.. doxygenstruct:: mdds::detail::rtree::default_rtree_trait
   :members:

.. doxygenstruct:: mdds::detail::rtree::integrity_check_properties
   :members:

.. doxygenenum:: mdds::detail::rtree::export_tree_type
   :project: mdds

.. doxygenenum:: mdds::detail::rtree::search_type
   :project: mdds
