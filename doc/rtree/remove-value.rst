
.. highlight:: cpp

Removing a value from tree
==========================

Removing an existing value from the tree first requires you to perform the
search to obtian search results, then from the search results get the iterator
and advance it to the position of the value you wish to remove.  Once you have
your iterator set to the right position, pass it to the
:cpp:func:`~mdds::rtree::erase` method to remove that value.

Note that you can only remove one value at a time, and the iterator becomes
invalid each time you call the :cpp:func:`~mdds::rtree::erase` method to
remove a value.

Here is a contrived example to demonstrate how erasing a value works::

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

In this code, we are intentionally putting 5 values to the same 2-dimensional
point (1, 1), then removing one of them based on matching criteria (of being
equal to "C").

Compiling and running this code will generate the following output:

.. code-block:: none

    A
    B
    C
    D
    E
    'C' has been erased.
    A
    B
    D
    E

which clearly shows that the 'C' has been successfully erased.
