
#include <mdds/rtree.hpp>

#include <string>
#include <iostream>

int main()
{
    // key values are of type double, and we are storing std::string as a
    // value for each spatial object.  By default, tree becomes 2-dimensional
    // object store unless otherwise specified.
    using rt_type = mdds::rtree<double, std::string>;

    rt_type tree;

    tree.insert({{0.0, 0.0}, {15.0, 20.0}}, "first rectangle data");

    rt_type::extent_type bounds({-2.0, -1.0}, {1.0, 2.0});
    std::cout << "inserting value for " << bounds.to_string() << std::endl;
    tree.insert(bounds, "second rectangle data");

    bounds.start.d[0] = -1.0; // Change the first dimension value of the start rectangle point.
    bounds.end.d[1] += 1.0; // Increment the second dimension value of the end rectangle point.
    std::cout << "inserting value for " << bounds.to_string() << std::endl;
    tree.insert(bounds, "third rectangle data");

    tree.insert({5.0, 6.0}, "first point data");

    {
        // Search for all objects that overlap with a (4, 4) - (7, 7) rectangle.
        auto results = tree.search({{4.0, 4.0}, {7.0, 7.0}}, rt_type::search_type::overlap);

        for (const std::string& v : results)
            std::cout << "value: " << v << std::endl;
    }

    {
        // Search for all objects whose bounding rectangles are exactly (4, 4) - (7, 7).
        auto results = tree.search({{4.0, 4.0}, {7.0, 7.0}}, rt_type::search_type::match);
        std::cout << "number of results: " << std::distance(results.begin(), results.end()) << std::endl;
    }

    {
        // Search for all objects whose bounding rectangles are exactly (0, 0) - (15, 20).
        auto results = tree.search({{0.0, 0.0}, {15.0, 20.0}}, rt_type::search_type::match);
        std::cout << "number of results: " << std::distance(results.begin(), results.end()) << std::endl;

        std::cout << "value: " << *results.begin() << std::endl;

        std::cout << "--" << std::endl;

        auto it = results.begin();
        std::cout << "value: " << *it << std::endl;
        std::cout << "extent: " << it.extent().to_string() << std::endl;
        std::cout << "depth: " << it.depth() << std::endl;
    }

    return EXIT_SUCCESS;
}
