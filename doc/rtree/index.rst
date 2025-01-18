
.. highlight:: cpp

R-tree
======

`R-tree <https://en.wikipedia.org/wiki/R-tree>`_ is a tree-based data
structure designed for optimal query performance on multi-dimensional spatial
objects with rectangular bounding shapes.  The R-tree implementation included
in this library is a variant of R-tree known as `R*-tree
<https://en.wikipedia.org/wiki/R*_tree>`_ which differs from the original
R-tree in that it may re-insert an object if the insertion of that object
would cause the original target directory to overflow.  Such re-insertions
lead to more balanced tree which in turn lead to better query performance, at
the expense of slightly more overhead at insertion time.

Our implementation of R-tree theoretically supports any number of dimensions
although certain functionalities, especially those related to visualization,
are only supported for 2-dimensional instances.

R-tree consists of three types of nodes.  Value nodes store the values
inserted externally and always sit at the bottom of the tree.  Leaf directory
nodes sit directly above the value nodes, and store only value nodes as their
child nodes.  The rest are all non-leaf directory nodes which can either store
leaf or non-leaf directory nodes.

.. toctree::
   :maxdepth: 1

   quickstart.rst
   remove-value.rst
   visualize.rst
   bulkload.rst
   api.rst

