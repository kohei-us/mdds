Multi-Dimensional Data Structure (mdds)

A collection of multi-dimensional data structure and indexing 
algorithm.  


Overview
========

This library implements the following data structure:

    * flat segment tree 
    * segment tree
    * rectangle set
    * point quad tree
    * mixed type matrix

Segment Tree

Segment tree is a balanced-binary-tree based data structure efficient 
for detecting all intervals (or segments) that contain a given point.  
The segments may overlap with each other.  The end points of stored 
segments are not inclusive, that is, when an interval spans from 2 to 
6, an arbitrary point x within that interval can take a value of 2 <= 
x < 6.  

Flat Segment Tree

Flat segment tree is a variant of segment tree that is designed to 
store a collection of non-overlapping segments.  This structure is 
efficient when you need to store values associated with 1 dimensional 
segments that never overlap with each other.  Like segment tree, 
stored segments' end points are non-inclusive.  

Rectangle Set

Rectangle set stores 2-dimensional rectangles and provides an 
efficient way to query all rectangles that contain a given point in 
2-dimensional space.  It internally uses nested segment tree.  Each 
rectangle is defined by two 2-dimensional points: the top-left and 
bottom-right points, and the bottom-right point is non-inclusive.  For 
instance, if a rectangle ranges from (x=2, y=2) to (x=10, y=20), then 
a 2-dimension point A (x,y) is said to be inside that rectangle only 
when 2 <= x < 10 and 2 <= y < 20.

Point Quad Tree

Point quad tree stores 2-dimensional points and provides an efficient 
way to query all points within specified rectangular region.  

Mixed Type Matrix

Mixed type matrix (MTM) allows storage of elements of various types: 
boolean, numeric, string, and empty types.  It also allows storage of 
additional value associated with each element.  MTM allows two storage 
back-ends: filled storage and sparse storage.  Filled storage 
allocates memory for all elements at all times, whereas sparse storage 
allocates memory only for elements having non-default values.  


How-To
======

Please take a look at simple example files under the 'example' 
directory on how to use these data structures.


API Incompatibility Note
========================

0.8.1 to 0.9.0

multi_type_vector

* The number of template parameters in custom_block_func1,
  custom_block_func2 and custom_block_func3 have been reduced by half,
  by deducing the numerical block type ID from the block type
  definition directly.  If you use the older variant, simply remove
  the template arguments that are numerical block IDs.

0.7.1 to 0.8.0

flat_segment_tree

* The search_tree() method in 0.8.0 returns std::pair<const_iterator,
  bool> instead of just returning bool as of 0.7.1.  If you use this
  method and relies on the return value of the old version, use the
  second parameter of the new return value which is equivalent of the
  previous return value.

0.4.0 to 0.5.0

flat_segment_tree

* The search() method now returns ::std::pair<const_iterator, bool>.  
  This method previously returned only bool.  Use the second parameter of 
  the new return value which is equivalent of the previous return value.


License
=======

mdds is free software. You may copy, distribute, and modify it under
the terms of the License contained in the file COPYING distributed
with this package. This license is the same as the MIT/X Consortium
license.


Miscellaneous
=============

Version detection

When installing this library, file named VERSION that contains nothing 
but the version number string gets installed in the document directory 
(docdir).  This may be used to detect the version number of this 
library via script.  
