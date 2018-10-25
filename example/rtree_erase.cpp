
#include <mdds/rtree.hpp>

#include <string>
#include <iostream>

int main()
{
    using rt_type = mdds::rtree<int, std::string>;

    rt_type tree;

    // Insert multiple values at the same point.
    tree.insert({1, 1}, "A");
    tree.insert({1, 1}, "B");
    tree.insert({1, 1}, "C");
    tree.insert({1, 1}, "D");
    tree.insert({1, 1}, "E");

    // This should return all five values.
    auto results = tree.search({1, 1}, rt_type::search_type::match);

    for (const std::string& v : results)
        std::cout << v << std::endl;

    // Erase "C".
    for (auto it = results.begin(); it != results.end(); ++it)
    {
        if (*it == "C")
        {
            tree.erase(it);
            break; // This invalidates the iterator.  Bail out.
        }
    }

    std::cout << "'C' has been erased." << std::endl;

    // Now this should only return A, B, D and E.
    results = tree.search({1, 1}, rt_type::search_type::match);

    for (const std::string& v : results)
        std::cout << v << std::endl;

    return EXIT_SUCCESS;
}
