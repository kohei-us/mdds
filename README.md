Multi-Dimensional Data Structure (mdds)
=======================================
A collection of multi-dimensional data structure and indexing 
algorithm.  

Overview of data structures included in mdds
--------------------------------------------

This library implements the following data structures:

* segment_tree
* flat_segment_tree
* rectangle_set (deprecated)
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

### Rectangle Set (deprecated)

Rectangle set stores 2-dimensional rectangles and provides an 
efficient way to query all rectangles that contain a given point in 
2-dimensional space.  It internally uses nested segment tree.  Each 
rectangle is defined by two 2-dimensional points: the top-left and 
bottom-right points, and the bottom-right point is non-inclusive.  For 
instance, if a rectangle ranges from (x=2, y=2) to (x=10, y=20), then 
a 2-dimension point A (x,y) is said to be inside that rectangle only 
when 2 <= x < 10 and 2 <= y < 20.

*Note that this data structure has been deprecated, and will be removed in
future releases.*

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
* [Official API documentation](https://mdds.readthedocs.io/en/latest/) for
  general users of the library.
* [Doxygen code documentation](http://kohei.us/files/mdds/doxygen/) for
  maintainers and potential contributors to the library.


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


Download
========

| Version | API Version | Release Date | Download | Check Sum | File Size (bytes) |
|---------|-------------|--------------|----------|-----------|-------------------|
| 1.6.0 | 1.5 | 2020-02-06 | [mdds-1.6.0.tar.bz2](http://kohei.us/files/mdds/src/mdds-1.6.0.tar.bz2) | sha256sum: f1585c9cbd12f83a6d43d395ac1ab6a9d9d5d77f062c7b5f704e24ed72dae07d | 350406 |
| 1.5.0 | 1.5 | 2019-08-15 | [mdds-1.5.0.tar.bz2](http://kohei.us/files/mdds/src/mdds-1.5.0.tar.bz2) | sha256sum: 144d6debd7be32726f332eac14ef9f17e2d3cf89cb3250eb31a7127e0789680d | 346888 |
| 1.4.3 | 1.4 | 2018-10-30 | [mdds-1.4.3.tar.bz2](http://kohei.us/files/mdds/src/mdds-1.4.3.tar.bz2) | sha256sum: 25ce3d5af9f6609e1de05bb22b2316e57b74a72a5b686fbb2da199da72349c81 | 334515 |
| 1.4.2 | 1.4 | 2018-09-14 | [mdds-1.4.2.tar.bz2](http://kohei.us/files/mdds/src/mdds-1.4.2.tar.bz2) | sha256sum: 82f38750248c007956c38ffefcc549932c8b257b76c72fb79a06eabc50107369 | 318164 |
| 1.4.1 | 1.4 | 2018-08-24 | [mdds-1.4.1.tar.bz2](http://kohei.us/files/mdds/src/mdds-1.4.1.tar.bz2) | sha256sum: 9ac690c37f5f06dc88551405d5daf9d9ad25edf65aae6325b59e905c2ba444c3 | 317460 |
| 1.4.0 | 1.4 | 2018-08-21 | [mdds-1.4.0.tar.bz2](http://kohei.us/files/mdds/src/mdds-1.4.0.tar.bz2) | sha256sum: 496b11564e2d95615090bf31a3524718260c5e8246e9d552216c4c56f5a24529 | 316451 |
| 1.3.1 | 1.2 | 2017-11-10 | [mdds-1.3.1.tar.bz2](http://kohei.us/files/mdds/src/mdds-1.3.1.tar.bz2) | sha256sum: dcb8cd2425567a5a5ec164afea475bce57784bca3e352ad4cbdd3d1a7e08e5a1 | 287612 |
| 1.3.0 | 1.2 | 2017-10-30 | [mdds-1.3.0.tar.bz2](http://kohei.us/files/mdds/src/mdds-1.3.0.tar.bz2) | sha256sum: 00aa92a28af9f1168a8e5c38e46f311abb65ef5b113ef56078ff104b94211460 | 287367 |
| 1.2.3 | 1.2 | 2017-05-24 | [mdds-1.2.3.tar.bz2](http://kohei.us/files/mdds/src/mdds-1.2.3.tar.bz2) | sha256sum: 402fec18256f95b89517d54d85f00bce1faa6e517cb3d7c98a720fddd063354f | 287079 |
| 1.2.2 | 1.2 | 2016-09-09 | [mdds-1.2.2.tar.bz2](http://kohei.us/files/mdds/src/mdds-1.2.2.tar.bz2) | sha256sum: 141e730b39110434b02cd844c5ad3442103f7c35f7e9a4d6a9f8af813594cc9d | 286185 |
| 1.2.1 | 1.2 | 2016-06-23 | [mdds-1.2.1.tar.bz2](http://kohei.us/files/mdds/src/mdds-1.2.1.tar.bz2) | sha256sum: 1e2f49dfc7b9d444bad07064837099741f4c2d061807173392ad2357116dfc7b | 285426 |
| 1.2.0 | 1.2 | 2016-05-11 | [mdds-1.2.0.tar.bz2](http://kohei.us/files/mdds/src/mdds-1.2.0.tar.bz2) | sha256sum: f44fd0635de94c7d490f9a65f74b5e55860d7bdd507951428294f9690fda45b6 | 284962 |
| 1.1.0 | 1.0 | 2016-02-10 | [mdds-1.1.0.tar.bz2](http://kohei.us/files/mdds/src/mdds-1.1.0.tar.bz2) | sha256sum: 4253ab93fe8bb579321a50e247f1f800191ab99fe2d8c6c181741b8bd3fb161f | 258691 |
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
