.. highlight:: cpp


Trie Maps
=========

Example
-------

The following code example illustrates how to populate a :cpp:class:`~mdds::trie_map`
instance and perform prefix searches.  The later part of the example also
shows how you can create a packed version of the content for faster lookups
and reduced memory footprint.

::

    #include <mdds/global.hpp>  // for MDDS_ASCII
    #include <mdds/trie_map.hpp>
    #include <iostream>

    using namespace std;

    typedef mdds::trie_map<mdds::trie::std_string_trait, int> trie_map_type;

    int main()
    {
        // Cities in North Carolina and their populations in 2013.
        trie_map_type nc_cities;

        // Insert key-value pairs.
        nc_cities.insert(MDDS_ASCII("Charlotte"),     792862);
        nc_cities.insert(MDDS_ASCII("Raleigh"),       431746);
        nc_cities.insert(MDDS_ASCII("Greensboro"),    279639);
        nc_cities.insert(MDDS_ASCII("Durham"),        245475);
        nc_cities.insert(MDDS_ASCII("Winston-Salem"), 236441);
        nc_cities.insert(MDDS_ASCII("Fayetteville"),  204408);
        nc_cities.insert(MDDS_ASCII("Cary"),          151088);
        nc_cities.insert(MDDS_ASCII("Wilmington"),    112067);
        nc_cities.insert(MDDS_ASCII("High Point"),    107741);
        nc_cities.insert(MDDS_ASCII("Greenville"),    89130);
        nc_cities.insert(MDDS_ASCII("Asheville"),     87236);
        nc_cities.insert(MDDS_ASCII("Concord"),       83506);
        nc_cities.insert(MDDS_ASCII("Gastonia"),      73209);
        nc_cities.insert(MDDS_ASCII("Jacksonville"),  69079);
        nc_cities.insert(MDDS_ASCII("Chapel Hill"),   59635);
        nc_cities.insert(MDDS_ASCII("Rocky Mount"),   56954);
        nc_cities.insert(MDDS_ASCII("Burlington"),    51510);
        nc_cities.insert(MDDS_ASCII("Huntersville"),  50458);
        nc_cities.insert(MDDS_ASCII("Wilson"),        49628);
        nc_cities.insert(MDDS_ASCII("Kannapolis"),    44359);
        nc_cities.insert(MDDS_ASCII("Apex"),          42214);
        nc_cities.insert(MDDS_ASCII("Hickory"),       40361);
        nc_cities.insert(MDDS_ASCII("Goldsboro"),     36306);

        cout << "Cities that start with 'Cha' and their populations:" << endl;
        auto results = nc_cities.prefix_search(MDDS_ASCII("Cha"));
        for (const trie_map_type::key_value_type& kv : results)
        {
            cout << "  " << kv.first << ": " << kv.second << endl;
        }

        cout << "Cities that start with 'W' and their populations:" << endl;
        results = nc_cities.prefix_search(MDDS_ASCII("W"));
        for (const trie_map_type::key_value_type& kv : results)
        {
            cout << "  " << kv.first << ": " << kv.second << endl;
        }

        // Create a compressed version of the container.  It works nearly identically.
        auto packed = nc_cities.pack();

        cout << "Cities that start with 'C' and their populations:" << endl;
        auto packed_results = packed.prefix_search(MDDS_ASCII("C"));
        for (const trie_map_type::key_value_type& kv : packed_results)
        {
            cout << "  " << kv.first << ": " << kv.second << endl;
        }

        // Individual search.
        auto it = packed.find(MDDS_ASCII("Wilmington"));
        cout << "Population of Wilmington: " << it->second << endl;

        // You get an end position iterator when the container doesn't have the
        // specified key.
        it = packed.find(MDDS_ASCII("Asheboro"));

        cout << "Population of Asheboro: ";

        if (it == packed.end())
            cout << "not found";
        else
            cout << it->second;

        cout << endl;

        return EXIT_SUCCESS;
    }

One thing to note in the above example is the use of :c:macro:`MDDS_ASCII` macro,
which expands a literal string definition into a literal string and its length
as two parameters.  This macro comes in handy when you need to define a
literal and immediately pass it to a function that expects a pointer to a
string and its length.

You'll get the following output when compiling the above code and executing it:

.. code-block:: none

    Cities that start with 'Cha' and their populations:
      Chapel Hill: 59635
      Charlotte: 792862
    Cities that start with 'W' and their populations:
      Wilmington: 112067
      Wilson: 49628
      Winston-Salem: 236441
    Cities that start with 'C' and their populations:
      Cary: 151088
      Chapel Hill: 59635
      Charlotte: 792862
      Concord: 83506
    Population of Wilmington: 112067
    Population of Asheboro: not found

Here is a version that uses :cpp:class:`~mdds::packed_trie_map`::

    #include <mdds/global.hpp>  // for MDDS_ASCII and MDDS_N_ELEMENTS
    #include <mdds/trie_map.hpp>
    #include <iostream>

    using namespace std;

    typedef mdds::packed_trie_map<mdds::trie::std_string_trait, int> trie_map_type;

    int main()
    {
        // Entries must be known prior to creating the instance, and they must be
        // sorted by the key in ascending order.
        trie_map_type::entry entries[] = {
            { MDDS_ASCII("Apex"),           42214 },
            { MDDS_ASCII("Asheville"),      87236 },
            { MDDS_ASCII("Burlington"),     51510 },
            { MDDS_ASCII("Cary"),          151088 },
            { MDDS_ASCII("Chapel Hill"),    59635 },
            { MDDS_ASCII("Charlotte"),     792862 },
            { MDDS_ASCII("Concord"),        83506 },
            { MDDS_ASCII("Durham"),        245475 },
            { MDDS_ASCII("Fayetteville"),  204408 },
            { MDDS_ASCII("Gastonia"),       73209 },
            { MDDS_ASCII("Goldsboro"),      36306 },
            { MDDS_ASCII("Greensboro"),    279639 },
            { MDDS_ASCII("Greenville"),     89130 },
            { MDDS_ASCII("Hickory"),        40361 },
            { MDDS_ASCII("High Point"),    107741 },
            { MDDS_ASCII("Huntersville"),   50458 },
            { MDDS_ASCII("Jacksonville"),   69079 },
            { MDDS_ASCII("Kannapolis"),     44359 },
            { MDDS_ASCII("Raleigh"),       431746 },
            { MDDS_ASCII("Rocky Mount"),    56954 },
            { MDDS_ASCII("Wilmington"),    112067 },
            { MDDS_ASCII("Wilson"),         49628 },
            { MDDS_ASCII("Winston-Salem"), 236441 },
        };

        // Cities in North Carolina and their populations in 2013.
        trie_map_type nc_cities(entries, MDDS_N_ELEMENTS(entries));

        cout << "Cities that start with 'Cha' and their populations:" << endl;
        auto results = nc_cities.prefix_search(MDDS_ASCII("Cha"));
        for (const trie_map_type::key_value_type& kv : results)
        {
            cout << "  " << kv.first << ": " << kv.second << endl;
        }

        cout << "Cities that start with 'W' and their populations:" << endl;
        results = nc_cities.prefix_search(MDDS_ASCII("W"));
        for (const trie_map_type::key_value_type& kv : results)
        {
            cout << "  " << kv.first << ": " << kv.second << endl;
        }

        cout << "Cities that start with 'C' and their populations:" << endl;
        results = nc_cities.prefix_search(MDDS_ASCII("C"));
        for (const trie_map_type::key_value_type& kv : results)
        {
            cout << "  " << kv.first << ": " << kv.second << endl;
        }

        // Individual search.
        auto it = nc_cities.find(MDDS_ASCII("Wilmington"));
        cout << "Population of Wilmington: " << it->second << endl;

        // You get an end position iterator when the container doesn't have the
        // specified key.
        it = nc_cities.find(MDDS_ASCII("Asheboro"));

        cout << "Population of Asheboro: ";

        if (it == nc_cities.end())
            cout << "not found";
        else
            cout << it->second;

        cout << endl;

        return EXIT_SUCCESS;
    }

This code generates exactly the same output as the first example that uses
:cpp:class:`~mdds::trie_map`.  The only difference is that you need to provide
the list of entries *pre-sorted* prior to instantiating the map object.

This example uses another useful macro :c:macro:`MDDS_N_ELEMENTS` which
computes the length of an array by dividing the size of the whole array by the
size of its first element.  This macro is useful when the array definition is
given in the same compilation unit and therefore its size is known at the call
site where the macro is used.


API Reference
-------------

Trie Map
````````

.. doxygenclass:: mdds::trie_map
   :members:


Packed Trie Map
```````````````

.. doxygenclass:: mdds::packed_trie_map
   :members:

String Trait
````````````

.. doxygenstruct:: mdds::trie::std_string_trait
   :members:
