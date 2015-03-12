# Multi-Dimensional Data Structure (mdds) #
A collection of multi-dimensional data structures and indexing algorithms.

**The download page has moved to [here](http://code.google.com/p/multidimalgorithm/wiki/Downloads). Source packages of 0.9.1 and newer can be downloaded from this page.**

## Overview of data structures included in mdds ##

### Segment tree ###

[Segment tree](http://en.wikipedia.org/wiki/Segment_tree) is a balanced-binary-tree based data structure efficient for detecting all intervals (or segments) that contain a given point.  The segments may overlap with each other.  The end points of stored segments are not inclusive, that is, when an interval spans from 2 to 6, an arbitrary point x within that interval can take a value of 2 <= x < 6, but not the exact value of 6 itself.

### Flat segment tree ###

Flat segment tree is a variant of segment tree that is designed to store a collection of non-overlapping segments.  This structure is efficient when you need to store values associated with 1 dimensional segments that never overlap with each other.  Like segment tree, stored segments' end points are non-inclusive.

### Rectangle set ###

Rectangle set stores 2-dimensional rectangles and provides an efficient way to query all rectangles that contain a given point in 2-dimensional space.  It internally uses nested segment tree.  Each rectangle is defined by two 2-dimensional points: the top-left and bottom-right points, and the bottom-right point is non-inclusive.  For instance, if a rectangle ranges from (x=2, y=2) to (x=10, y=20), then a 2-dimension point A (x,y) is said to be inside that rectangle only when 2 <= x < 10 and 2 <= y < 20.

### Point quad tree ###

Point quad tree stores 2-dimensional points and provides an efficient
way to query all points within specified rectangular region.

### Mixed type matrix (deprecated as of 0.6.0) ###

Mixed type matrix data structure stores elements of four different types: numeric, string, boolean and empty.  It can also store additional flags associated with each element.  It supports two different storage types (filled and sparse) and two different default element types (numeric zero and empty).

### Multi-type vector ###

Multi-type vector allows storage of unspecified number of types in a single logical array such that contiguous elements of identical type are stored in contiguous segment in memory space.

### Multi-type matrix ###

Multi-type matrix is a matrix structure that allows storage of four different element types: numeric, string, boolean and empty.  It uses multi-type vector as its underlying storage.

## Credits ##

Core developers

  * Kohei Yoshida
  * David Tardon
  * Caolán McNamara

Contributors

  * Markus Mohrhard
  * Philipp Thomas
  * Petr Mladek
  * Stephan Bergmann