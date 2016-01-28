Multi-Dimensional Data Structure (mdds)
=======================================
A collection of multi-dimensional data structure and indexing 
algorithm.  

Overview of data structures included in mdds
--------------------------------------------

This library implements the following data structure:

* flat segment tree 
* segment tree
* rectangle set
* point quad tree
* multi-type vector
* multi-type matrix

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

### Rectangle Set

Rectangle set stores 2-dimensional rectangles and provides an 
efficient way to query all rectangles that contain a given point in 
2-dimensional space.  It internally uses nested segment tree.  Each 
rectangle is defined by two 2-dimensional points: the top-left and 
bottom-right points, and the bottom-right point is non-inclusive.  For 
instance, if a rectangle ranges from (x=2, y=2) to (x=10, y=20), then 
a 2-dimension point A (x,y) is said to be inside that rectangle only 
when 2 <= x < 10 and 2 <= y < 20.

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

API Documentation
=================
* [Official API documentation](http://kohei.us/files/mdds/doc/).
* [Doxygen code documentation](http://kohei.us/files/mdds/doxygen/).

How-To
======

Please take a look at simple example files under the 'example' 
directory on how to use these data structures.

API Incompatibility Notes
=========================

1.0
---

* Starting with version 1.0, mdds now requires support for C++11.  Stick with
  0.12 or earlier versions if you use a compiler that doesn't support C++11.

* data_type has been renamed to value_type for segment_tree, rectangle_set,
  and point_quad_tree.


0.9
---

### multi_type_vector

* The number of template parameters in custom_block_func1,
  custom_block_func2 and custom_block_func3 have been reduced by half,
  by deducing the numerical block type ID from the block type
  definition directly.  If you use the older variant, simply remove
  the template arguments that are numerical block IDs.

0.8
---

### flat_segment_tree

* The search_tree() method in 0.8.0 returns std::pair<const_iterator,
  bool> instead of just returning bool as of 0.7.1.  If you use this
  method and relies on the return value of the old version, use the
  second parameter of the new return value which is equivalent of the
  previous return value.

0.5
---

### flat_segment_tree

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


Download
========

| Version | API Version | Release Date | Download | Check Sum | File Size (bytes) |
|---------|-------------|--------------|----------|-----------|-------------------|
| 1.0.0 | 1.0 | 2015-10-06 | [mdds_1.0.0.tar.bz2](http://kohei.us/files/mdds/src/mdds_1.0.0.tar.bz2) | sha256sum: ef8abc1236b54c7ca16ae1ee38abfb9cdbc5d1e6a2427c65b92b8c1003e3bf56 | 166619 |
| 0.12.1 || 2015-06-11 | [mdds_0.12.1.tar.bz2](http://kohei.us/files/mdds/src/mdds_0.12.1.tar.bz2) | md5sum: ef2560ed5416652a7fe195305b14cebe<br/>sha1sum: e7469349f8d0c65545896fe553918f3ea93bd84d | - |
| 0.12.0 || 2015-02-05 | [mdds_0.12.0.tar.bz2](http://kohei.us/files/mdds/src/mdds_0.12.0.tar.bz2) | md5sum: 17edb780d4054e4205cd956910672b83<br/>sha1sum: 043590edde76a1df3e96070c46cbc7ae5f88f081 | - |
| 0.11.2 || 2014-12-18 | [mdds_0.11.2.tar.bz2](http://kohei.us/files/mdds/src/mdds_0.11.2.tar.bz2) | md5sum: cb4207cb913c7a5a8bfa5b91234618ee<br/>sha1sum: 17d2d06a1df818de61bba25a9322541e80f6eed7 | - |
| 0.11.1 || 2014-10-02 | [mdds_0.11.1.tar.bz2](http://kohei.us/files/mdds/src/mdds_0.11.1.tar.bz2) | md5sum: 896272c1a9e396b871cb4dffbd694503<br/>sha1sum: 0c1ace97ad310e5293c538f395176d9a506cdeda | - |
| 0.11.0 || 2014-09-18 | [mdds_0.11.0.tar.bz2](http://kohei.us/files/mdds/src/mdds_0.11.0.tar.bz2) | md5sum: a67a46ec9d00d283a7cd8dbdd2906b59<br/>sha1sum: cefd57cf7cd0408737b3d76ed0771694f26bda58 | - |
| 0.10.3 || 2014-04-23 | [mdds_0.10.3.tar.bz2](http://kohei.us/files/mdds/src/mdds_0.10.3.tar.bz2) | md5sum: aa5ca9d1ed1082890835afab26400a39<br/>sha1sum: 0c4fa77918b8cc8ad32460c8d8a679e065976dbe | - |
| 0.10.2 || 2014-02-12 | [mdds_0.10.2.tar.bz2](http://kohei.us/files/mdds/src/mdds_0.10.2.tar.bz2) | md5sum: 47203e7cade74e5c385aa812f21e7932<br/>sha1sum: 26027170f7cdf7a4dcc39ea01376d394dcd21ffc | - |
| 0.10.1 || 2014-01-08 | [mdds_0.10.1.tar.bz2](http://kohei.us/files/mdds/src/mdds_0.10.1.tar.bz2) | md5sum: 01a380acfec23bf617117ce98e318f3d<br/>sha1sum: 199e609afa5ae08d164754f7a0a54b01f88692d0 | - |
| 0.10.0 || 2014-01-03 | [mdds_0.10.0.tar.bz2](http://kohei.us/files/mdds/src/mdds_0.10.0.tar.bz2) | md5sum: 26272a8e8c984d21ba800b4edcd3ada8<br/>sha1sum: 2234e98f9e36041d0a41f037f628f2178f707307 | - |
| 0.9.1 || 2013-10-21 | [mdds_0.9.1.tar.bz2](http://kohei.us/files/mdds/src/mdds_0.9.1.tar.bz2) | md5sum: 8c853024fbcff39113d9285250dafc66<br/>sha1sum: d80f6b74827d5e36ecbb8975b0f8f42896162d95 | - |

Older releases are archived [here](https://code.google.com/p/multidimalgorithm/downloads/list).


Installation
============

Once you have downloaded the package, run the following commands to install
the mdds headers to your system:

```bash
tar xvf mdds_<version>.tar.bz2
cd mdds_<version>
./autogen.sh
make check  # optional for executing tests.
make install
```

It will install the headers under `/usr/local` by default.  Use the `--prefix`
option of the autogen.sh script to specify custom install location if you wish
to install it to a non-default location.

