.. highlight:: cpp


Trie Maps
=========

Example
-------

Trie Map
^^^^^^^^

Populating Trie Map
```````````````````

This section illustrates how to use :cpp:class:`~mdds::trie_map` to build a
database of city populations and perform prefix searches.  In this example,
we will use the 2013 populations of cities in North Carolina, and use the city
names as keys.

Let's define the type first::

    using trie_map_type = mdds::trie_map<mdds::trie::std_string_trait, int>;

The first template argument specifies the trait of the key.  In this example,
we are using a pre-defined trait for std::string, which is defined in
:cpp:class:`~mdds::trie::std_string_trait`.  The second template argument
specifies the value type, which in this example is simply an ``int``.

Once the type is defined, the next step is instantiation::

    trie_map_type nc_cities;

It's pretty simple as you don't need to pass any arguments to the constructor.
Now, let's populate this data structure with some population data::

    // Insert key-value pairs.
    nc_cities.insert("Charlotte",     792862);
    nc_cities.insert("Raleigh",       431746);
    nc_cities.insert("Greensboro",    279639);
    nc_cities.insert("Durham",        245475);
    nc_cities.insert("Winston-Salem", 236441);
    nc_cities.insert("Fayetteville",  204408);
    nc_cities.insert("Cary",          151088);
    nc_cities.insert("Wilmington",    112067);
    nc_cities.insert("High Point",    107741);
    nc_cities.insert("Greenville",    89130);
    nc_cities.insert("Asheville",     87236);
    nc_cities.insert("Concord",       83506);
    nc_cities.insert("Gastonia",      73209);
    nc_cities.insert("Jacksonville",  69079);
    nc_cities.insert("Chapel Hill",   59635);
    nc_cities.insert("Rocky Mount",   56954);
    nc_cities.insert("Burlington",    51510);
    nc_cities.insert("Huntersville",  50458);
    nc_cities.insert("Wilson",        49628);
    nc_cities.insert("Kannapolis",    44359);
    nc_cities.insert("Apex",          42214);
    nc_cities.insert("Hickory",       40361);
    nc_cities.insert("Goldsboro",     36306);

It's pretty straight-forward.  Each :cpp:func:`~mdds::trie_map::insert` call
expects a pair of string key and an integer value.  You can insert your data
in any order regardless of key's sort order.

Now that the data is in, let's perform prefix search to query all cities whose
name begins with "Cha"::

    cout << "Cities that start with 'Cha' and their populations:" << endl;
    auto results = nc_cities.prefix_search("Cha");
    for (const auto& kv : results)
    {
        cout << "  " << kv.first << ": " << kv.second << endl;
    }

You can perform prefix search via :cpp:func:`~mdds::trie_map::prefix_search`
method, which returns a results object that can be iterated over using a range-based
for loop.  Running this code will produce the following output:

.. code-block:: none

    Cities that start with 'Cha' and their populations:
      Chapel Hill: 59635
      Charlotte: 792862

Let's perform another prefix search, this time with a prefix of "W"::

    cout << "Cities that start with 'W' and their populations:" << endl;
    results = nc_cities.prefix_search("W");
    for (const auto& kv : results)
    {
        cout << "  " << kv.first << ": " << kv.second << endl;
    }

You'll see the following output when running this code:

.. code-block:: none

    Cities that start with 'W' and their populations:
      Wilmington: 112067
      Wilson: 49628
      Winston-Salem: 236441

Note that the results are sorted in key's ascending order.

.. note::

   Results from the prefix search are sorted in key's ascending order.


Create Packed Trie Map from Trie Map
````````````````````````````````````

There is also another variant of trie called :cpp:class:`~mdds::packed_trie_map`
which is designed to store all its data in contiguous memory region.  Unlike
:cpp:class:`~mdds::trie_map` which is mutable, :cpp:class:`~mdds::packed_trie_map`
is immutable; once populated, you can only perform queries and it is no longer
possible to add new entries into the container.

One way to create an instance of :cpp:class:`~mdds::packed_trie_map` is from
:cpp:class:`~mdds::trie_map` by calling its :cpp:func:`~mdds::trie_map::pack`
method::

    auto packed = nc_cities.pack();

The query methods of :cpp:class:`~mdds::packed_trie_map` are identical to those
of :cpp:class:`~mdds::trie_map`.  For instance, performing prefix search to find
all entries whose key begins with "C" can be done as follows::

    cout << "Cities that start with 'C' and their populations:" << endl;
    auto packed_results = packed.prefix_search("C");
    for (const auto& kv : packed_results)
    {
        cout << "  " << kv.first << ": " << kv.second << endl;
    }

Running this code will generate the following output:

.. code-block:: none

    Cities that start with 'C' and their populations:
      Cary: 151088
      Chapel Hill: 59635
      Charlotte: 792862
      Concord: 83506

You can also perform an exact-match query via :cpp:func:`~mdds::packed_trie_map::find`
method which returns an iterator associated with the key-value pair entry::

    // Individual search.
    auto it = packed.find("Wilmington");
    cout << "Population of Wilmington: " << it->second << endl;

You'll see the following output with this code:

.. code-block:: none

    Population of Wilmington: 112067

What if you performed an exact-match query with a key that doesn't exist in the
container?  You will basically get the end iterator position as its return value.
Thus, running this code::

    // You get an end position iterator when the container doesn't have the
    // specified key.
    it = packed.find("Asheboro");

    cout << "Population of Asheboro: ";

    if (it == packed.end())
        cout << "not found";
    else
        cout << it->second;

    cout << endl;

will generate the following output:

.. code-block:: none

    Population of Asheboro: not found

The complete source code for this example is available `here <https://gitlab.com/mdds/mdds/-/blob/master/example/trie_map.cpp>`__.


Using Packed Trie Map directly
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In the previous example, we showed a way to create an instance of :cpp:class:`~mdds::packed_trie_map`
from a populated instance of :cpp:class:`~mdds::trie_map`.  There is also a way
to instantiate and populate an instance of :cpp:class:`~mdds::packed_trie_map`
directly, and that is what we will cover in this section.

First, declare the type::

    using trie_map_type = mdds::packed_trie_map<mdds::trie::std_string_trait, int>;

Once again, we are using the pre-defined trait for std::string as its key, and int
as its value type.  The next step is to prepare its entries ahead of time::

    trie_map_type::entry entries[] =
    {
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

We need to do this since :cpp:class:`~mdds::packed_trie_map` is immutable, and
the only time we can populate its content is at instantiation time.  Here, we
are using the :c:macro:`MDDS_ASCII` macro to expand a string literal to its
pointer value and size.  Note that you need to ensure that the entries are sorted
by the key in ascending order.

.. warning::

   When instantiating :cpp:class:`~mdds::packed_trie_map` directly with a static
   set of entries, the entries must be sorted by the key in ascending order.

You can then pass this list of entries to construct the instance::

    trie_map_type nc_cities(entries, MDDS_N_ELEMENTS(entries));

The :c:macro:`MDDS_N_ELEMENTS` macro will infer the size of a fixed-size array
from its static definition.  Once it's instantiated, the rest of the example
for performing searches will be the same as in the previous section, which we
will not repeat here.

The complete source code for this example is available `here <https://gitlab.com/mdds/mdds/-/blob/master/example/packed_trie_map.cpp>`__.


API Reference
-------------

Trie Map
^^^^^^^^

.. doxygenclass:: mdds::trie_map
   :members:


Packed Trie Map
^^^^^^^^^^^^^^^

.. doxygenclass:: mdds::packed_trie_map
   :members:

String Trait
^^^^^^^^^^^^

.. doxygenstruct:: mdds::trie::std_string_trait
   :members:
