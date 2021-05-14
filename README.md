Multi-Dimensional Data Structure (mdds)
=======================================
A collection of multi-dimensional data structure and indexing 
algorithm.  

Overview of data structures included in mdds
--------------------------------------------

This library implements the following data structures:

* segment_tree
* flat_segment_tree
* point_quad_tree
* multi_type_vector
* multi_type_matrix
* sorted_string_map
* trie_map
* packed_trie_map
* rtree

### Segment Tree

Segment tree is a balanced-binary-tree based data structure efficient 
for detecting all intervals (or segments) that contain a given point.  
The segments may overlap with each other.  The end points of stored 
segments are not inclusive, that is, when an interval spans from 2 to 
6, an arbitrary point x within that interval can take a value of 2 <= 
x < 6.  

### Flat Segment Tree

Flat segment tree is a variant of segment tree that is designed to 
store a collection of non-overlapping segments.  This structure is 
efficient when you need to store values associated with 1 dimensional 
segments that never overlap with each other.  Like segment tree, 
stored segments' end points are non-inclusive.  

### Point Quad Tree

Point quad tree stores 2-dimensional points and provides an efficient 
way to query all points within specified rectangular region.  

### Multi Type Vector

Multi-type vector allows storage of unspecified number of types in a single 
logical array such that contiguous elements of identical type are stored in 
contiguous segment in memory space. 

### Multi Type Matrix

Multi-type matrix is a matrix structure that allows storage of four different 
element types: numeric, string, boolean and empty. It uses multi-type vector as 
its underlying storage. 

### Sorted String Map

Sorted string map is a simple data structure that takes a pre-sorted list of
key-value pairs that are known at compile time, and allows efficient lookup.
It does not allocate memory to duplicate its content, as it directly uses the
pre-sorted list provided by the caller.

### Trie Map

Trie map is an associative container that stores multiple key-value pairs
where keys are stored in a trie structure to optimize for prefix searches.

### Packed Trie Map

Packed trie map is nearly identical to the trie map counterpart except that
this one is immutable.  It packs all its content in a contiguous array for
optimum storage and lookup efficiency.  This implementation is based on the
paper titled [Tightly Packed Tries: How to Fit Large Models into Memory, and Make them Load Fast, Too](https://www.aclweb.org/anthology/W09-1505/)
by Ulrich Germann, Eric Joanis, and Samuel Larkin.

### R-tree

[R-tree](https://en.wikipedia.org/wiki/R-tree) is a tree-based data structure
designed to store multi-dimensional geometric data with bounding boxes and
provide optimal performance on region- or point-based queries.  The one
implemented in this library is a variant of R-tree known as
[R*-tree](https://en.wikipedia.org/wiki/R*_tree).


Documentation
=============
[Official API documentation](https://mdds.readthedocs.io/en/latest/) for general
users of the library.

Packages
========

Please see the [Releases](https://gitlab.com/mdds/mdds/-/releases) page for
source package downloads.

If you need old packages, please find them [here](OLD-DOWNLOADS.md).


Installation
============
Please refer to the [CONTRIBUTING.md](CONTRIBUTING.md) file for build and
installation instructions.


License
=======

mdds is free software. You may copy, distribute, and modify it under
the terms of the License contained in the file COPYING distributed
with this package. This license is the same as the MIT/X Consortium
license.


Who uses mdds?
==============

These are the projects that are known to use mdds.

* [LibreOffice](http://www.libreoffice.org)
* [Calligra](https://www.calligra.org/)
* [libetonyek](https://wiki.documentfoundation.org/DLP/Libraries/libetonyek)
* [ixion](https://gitlab.com/ixion/ixion)
* [orcus](https://gitlab.com/orcus/orcus)

If you use mdds and would like your project to be included in the above list,
please let us know.

