
.. highlight:: cpp

Visualize R-tree structure
==========================

In this section we will illustrate a way to visualize an R-tree structure via
:cpp:func:`~mdds::rtree::export_tree` method, which can be useful when you
need to visually inspect the tree structure to see how well balanced it is (or
not).

We will be using the following set of 2-dimensional rectangles as the bounding
rectangles for input values.

.. figure:: ../_static/images/rtree_bounds_src.png
   :align: center

For input values, we'll simply use linearly increasing series of integer
values, but the values themselves are not the focus of this section, and we'll
not talk much about that.  We will also intentionally make the capacity of
directory nodes smaller so that the tree will split more frequently during
insertion even for smaller number of inputs.

Now, let's take a look at the code::

    #include <mdds/rtree.hpp>

    #include <iostream>
    #include <fstream>

    // Make the node capacity intentionally small.
    struct tiny_trait_2d
    {
        constexpr static size_t dimensions = 2;
        constexpr static size_t min_node_size = 2;
        constexpr static size_t max_node_size = 5;
        constexpr static size_t max_tree_depth = 100;

        constexpr static bool enable_forced_reinsertion = true;
        constexpr static size_t reinsertion_size = 2;
    };

    using rt_type = mdds::rtree<int, int, tiny_trait_2d>;

    int main()
    {
        // 2D rectangle with the top-left position (x, y), width and height.
        struct rect
        {
            int x;
            int y;
            int w;
            int h;
        };

        std::vector<rect> rects =
        {
            {  3731,  2433, 1356,  937 },
            {  6003,  3172, 1066,  743 },
            {  4119,  6403,  825, 1949 },
            { 10305,  2315,  776,  548 },
            { 13930,  5468, 1742,  626 },
            {  8614,  4107, 2709, 1793 },
            { 14606,  1887, 5368, 1326 },
            { 17990,  5196, 1163, 1911 },
            {  6728,  7881, 3676, 1210 },
            { 14704,  9789, 5271, 1092 },
            {  4071, 10723, 4739,  898 },
            { 11755,  9010, 1357, 2806 },
            { 13978,  4068,  776,  509 },
            { 17507,  3717,  777,  471 },
            { 20358,  6092,  824, 1093 },
            {  6390,  4535, 1066, 1715 },
            { 13978,  7182, 2516, 1365 },
            { 17942, 11580, 2854,  665 },
            {  9919, 10450,  873, 1716 },
            {  5568, 13215, 7446,  509 },
            {  7357, 15277, 3145, 3234 },
            {  3539, 12592,  631,  509 },
            {  4747, 14498,  825,  626 },
            {  4554, 16913,  969, 1443 },
            { 12771, 14693, 2323,  548 },
            { 18714,  8193, 2372,  586 },
            { 22292,  2743,  487, 1638 },
            { 20987, 17535, 1163, 1249 },
            { 19536, 18859,  632,  431 },
            { 19778, 15394, 1356,  626 },
            { 22969, 15394,  631, 2066 },
        };

        rt_type tree;

        // Insert the rectangle objects into the tree.
        int value = 0;
        for (const auto& rect : rects)
            tree.insert({{rect.x, rect.y}, {rect.x + rect.w, rect.y + rect.h}}, value++);

        // Export the tree structure as a SVG for visualization.
        std::string tree_svg = tree.export_tree(rt_type::export_tree_type::extent_as_svg);
        std::ofstream fout("bounds.svg");
        fout << tree_svg;

        return EXIT_SUCCESS;
    }

First, we need to talk about how the concrete rtree type is instantiated::

    // Make the node capacity intentionally small.
    struct tiny_trait_2d
    {
        constexpr static size_t dimensions = 2;
        constexpr static size_t min_node_size = 2;
        constexpr static size_t max_node_size = 5;
        constexpr static size_t max_tree_depth = 100;

        constexpr static bool enable_forced_reinsertion = true;
        constexpr static size_t reinsertion_size = 2;
    };

    using rt_type = mdds::rtree<int, int, tiny_trait_2d>;

The first and second template arguments specify the key and value types to be
both ``int``.  This time around, however, we are passing a third template
argument which is a struct containing several static constant values.  These
constant values define certain characteristics of your R-tree, and there are
some restrictions you need to be aware of in case you need to use your own
custom trait for your R-tree.  Refer to
:cpp:class:`~mdds::detail::rtree::default_rtree_traits`, which is the default
trait used when you don't specify your own, for the descriptions of the
individual constants that your trait struct is expected to have as well as
restrictions that you must be aware of.

Also be aware that these constants must all be constant expressions with
``constexpr`` specifiers, as some of them are used within ``static_assert``
declarations, and even those that are currently not used within
``static_assert`` may be used in ``static_assert`` in the future.

As far as our current example goes, the only part of the custom trait we need
to highlight is that we are setting the directory node size to 2-to-5 instead
of the default size of 40-to-100, to trigger more node splits and make the
tree artificially deeper.

Let's move on to the next part of the code::

    // 2D rectangle with the top-left position (x, y), width and height.
    struct rect
    {
        int x;
        int y;
        int w;
        int h;
    };

    std::vector<rect> rects =
    {
        {  3731,  2433, 1356,  937 },
        {  6003,  3172, 1066,  743 },
        {  4119,  6403,  825, 1949 },
        { 10305,  2315,  776,  548 },
        { 13930,  5468, 1742,  626 },
        {  8614,  4107, 2709, 1793 },
        { 14606,  1887, 5368, 1326 },
        { 17990,  5196, 1163, 1911 },
        {  6728,  7881, 3676, 1210 },
        { 14704,  9789, 5271, 1092 },
        {  4071, 10723, 4739,  898 },
        { 11755,  9010, 1357, 2806 },
        { 13978,  4068,  776,  509 },
        { 17507,  3717,  777,  471 },
        { 20358,  6092,  824, 1093 },
        {  6390,  4535, 1066, 1715 },
        { 13978,  7182, 2516, 1365 },
        { 17942, 11580, 2854,  665 },
        {  9919, 10450,  873, 1716 },
        {  5568, 13215, 7446,  509 },
        {  7357, 15277, 3145, 3234 },
        {  3539, 12592,  631,  509 },
        {  4747, 14498,  825,  626 },
        {  4554, 16913,  969, 1443 },
        { 12771, 14693, 2323,  548 },
        { 18714,  8193, 2372,  586 },
        { 22292,  2743,  487, 1638 },
        { 20987, 17535, 1163, 1249 },
        { 19536, 18859,  632,  431 },
        { 19778, 15394, 1356,  626 },
        { 22969, 15394,  631, 2066 },
    };

This ``rects`` variable holds an array of 2-dimensional rectangle data that
represent the positions and sizes of rectangles shown earlier in this section.
This will be used as bounding rectangles for the input values in the next part
of the code::

    rt_type tree;

    // Insert the rectangle objects into the tree.
    int value = 0;
    for (const auto& rect : rects)
        tree.insert({{rect.x, rect.y}, {rect.x + rect.w, rect.y + rect.h}}, value++);

Here, the tree is instantiated, and the rectangles are inserted with their
associated values one at a time.  Once the tree is populated, the code that
follows will export the structure of the tree as an SVG string, which will
then be saved to a file on disk::

    // Export the tree structure as a SVG for visualization.
    std::string tree_svg = tree.export_tree(rt_type::export_tree_type::extent_as_svg);
    std::ofstream fout("bounds.svg");
    fout << tree_svg;

When you open the exported SVG file named **bounds.svg** in a SVG viewer,
you'll see something similar to this:

.. figure:: ../_static/images/rtree_bounds_tree.png
   :align: center

which depicts not only the bounding rectangles of the inserted values
(the red rectangles), but also the bounding rectangles of the directory
nodes as well (the light green rectangles).
