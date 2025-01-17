
Performance considerations
==========================

Given a set of segments, the cost of building a tree increases with the number
of unique start and end positions of all of the segments combined, as that
determines how deep the tree has to be.  The tree building process involves
building the tree structure itself, followed by the "insertions" of individual
segments into the tree.  The insertion of a segment involves descending the tree
from the root node and marking the appropriate nodes.  This gets repeated for
the number of segments to be inserted.  This process is moderately expensive.

The search process, on the other hand, typically consists of two steps: 1)
retrieving the results set, and 2) iterating through the results.  Step 1 is
proportional to the depth of the tree, and should be a quick process even with a
sufficiently large number of segments.  Step 2, on the other hand, is
proportional to the number of the results one needs to iterate through,
regardless of the size of the results set.

Removing a segment can be considerably expensive especially when a large number
of segments need to be removed from the tree.  Excessive use of either
:cpp:func:`~mdds::segment_tree::erase()` or
:cpp:func:`~mdds::segment_tree::erase_if()` is therefore not recommended.  Use
them sparingly.

Given these performance characteristics of :cpp:class:`~mdds::segment_tree`, an
ideal use case for this data structure would be one where:

* you already have all or most of the segments known ahead of time,
* you can only build the tree once, and mostly need to run queries afterward, and
* you need to remove segments only in some rare occasions, or none at all.

Note that occasionally updating the data set and re-building the tree is
reasonable, as long as your use case can tolerate the cost of building the tree.
