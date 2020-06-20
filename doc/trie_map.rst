.. highlight:: cpp


Trie Maps
=========

Examples
--------

Populating Trie Map
^^^^^^^^^^^^^^^^^^^

This section illustrates how to use :cpp:class:`~mdds::trie_map` to build a
database of city populations and perform prefix searches.  In this example,
we will use the 2013 populations of cities in North Carolina, and use the city
names as keys.

Let's define the type first::

    using trie_map_type = mdds::trie_map<mdds::trie::std_string_trait, int>;

The first template argument specifies the trait of the key.  In this example,
we are using a pre-defined trait for std::string, which is defined in
:cpp:type:`~mdds::trie::std_string_trait`.  The second template argument
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


Creating Packed Trie Map from Trie Map
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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

The complete source code for the examples in these two sections is available
`here <https://gitlab.com/mdds/mdds/-/blob/master/example/trie_map.cpp>`__.


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

The complete source code for the example in this section is available
`here <https://gitlab.com/mdds/mdds/-/blob/master/example/packed_trie_map.cpp>`__.


Saving and loading Packed Trie Map instances
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are times when you need to save the state of a :cpp:class:`~mdds::packed_trie_map`
instance to a file, or an in-memory buffer, and load it back later.  Doing that
is now possible by using the :cpp:func:`~mdds::packed_trie_map::save_state` and
:cpp:func:`~mdds::packed_trie_map::load_state` member methods of the
:cpp:class:`~mdds::packed_trie_map` class.

First, let's define the type of use::

    using map_type = mdds::packed_trie_map<mdds::trie::std_string_trait, int>;

As with the previous examples, we will use ``std::string`` as the key type and
``int`` as the value type.  In this example, we are going to use `the world's
largest cities and their 2018 populations
<https://en.wikipedia.org/wiki/List_of_largest_cities>`__ as the data to store
in the container.

The following code defines the entries::

    std::vector<map_type::entry> entries =
    {
        { MDDS_ASCII("Ahmedabad"),        7681000  },
        { MDDS_ASCII("Alexandria"),       5086000  },
        { MDDS_ASCII("Atlanta"),          5572000  },
        { MDDS_ASCII("Baghdad"),          6812000  },
        { MDDS_ASCII("Bangalore"),        11440000 },
        { MDDS_ASCII("Bangkok"),          10156000 },
        { MDDS_ASCII("Barcelona"),        5494000  },
        { MDDS_ASCII("Beijing"),          19618000 },
        { MDDS_ASCII("Belo Horizonte"),   5972000  },
        { MDDS_ASCII("Bogota"),           10574000 },
        { MDDS_ASCII("Buenos Aires"),     14967000 },
        { MDDS_ASCII("Cairo"),            20076000 },
        { MDDS_ASCII("Chengdu"),          8813000  },
        { MDDS_ASCII("Chennai"),          10456000 },
        { MDDS_ASCII("Chicago"),          8864000  },
        { MDDS_ASCII("Chongqing"),        14838000 },
        { MDDS_ASCII("Dalian"),           5300000  },
        { MDDS_ASCII("Dallas"),           6099000  },
        { MDDS_ASCII("Dar es Salaam"),    6048000  },
        { MDDS_ASCII("Delhi"),            28514000 },
        { MDDS_ASCII("Dhaka"),            19578000 },
        { MDDS_ASCII("Dongguan"),         7360000  },
        { MDDS_ASCII("Foshan"),           7236000  },
        { MDDS_ASCII("Fukuoka"),          5551000  },
        { MDDS_ASCII("Guadalajara"),      5023000  },
        { MDDS_ASCII("Guangzhou"),        12638000 },
        { MDDS_ASCII("Hangzhou"),         7236000  },
        { MDDS_ASCII("Harbin"),           6115000  },
        { MDDS_ASCII("Ho Chi Minh City"), 8145000  },
        { MDDS_ASCII("Hong Kong"),        7429000  },
        { MDDS_ASCII("Houston"),          6115000  },
        { MDDS_ASCII("Hyderabad"),        9482000  },
        { MDDS_ASCII("Istanbul"),         14751000 },
        { MDDS_ASCII("Jakarta"),          10517000 },
        { MDDS_ASCII("Jinan"),            5052000  },
        { MDDS_ASCII("Johannesburg"),     5486000  },
        { MDDS_ASCII("Karachi"),          15400000 },
        { MDDS_ASCII("Khartoum"),         5534000  },
        { MDDS_ASCII("Kinshasa"),         13171000 },
        { MDDS_ASCII("Kolkata"),          14681000 },
        { MDDS_ASCII("Kuala Lumpur"),     7564000  },
        { MDDS_ASCII("Lagos"),            13463000 },
        { MDDS_ASCII("Lahore"),           11738000 },
        { MDDS_ASCII("Lima"),             10391000 },
        { MDDS_ASCII("London"),           9046000  },
        { MDDS_ASCII("Los Angeles"),      12458000 },
        { MDDS_ASCII("Luanda"),           7774000  },
        { MDDS_ASCII("Madrid"),           6497000  },
        { MDDS_ASCII("Manila"),           13482000 },
        { MDDS_ASCII("Mexico City"),      21581000 },
        { MDDS_ASCII("Miami"),            6036000  },
        { MDDS_ASCII("Moscow"),           12410000 },
        { MDDS_ASCII("Mumbai"),           19980000 },
        { MDDS_ASCII("Nagoya"),           9507000  },
        { MDDS_ASCII("Nanjing"),          8245000  },
        { MDDS_ASCII("New York City"),    18819000 },
        { MDDS_ASCII("Osaka"),            19281000 },
        { MDDS_ASCII("Paris"),            10901000 },
        { MDDS_ASCII("Philadelphia"),     5695000  },
        { MDDS_ASCII("Pune"),             6276000  },
        { MDDS_ASCII("Qingdao"),          5381000  },
        { MDDS_ASCII("Rio de Janeiro"),   13293000 },
        { MDDS_ASCII("Riyadh"),           6907000  },
        { MDDS_ASCII("Saint Petersburg"), 5383000  },
        { MDDS_ASCII("Santiago"),         6680000  },
        { MDDS_ASCII("Sao Paulo"),        21650000 },
        { MDDS_ASCII("Seoul"),            9963000  },
        { MDDS_ASCII("Shanghai"),         25582000 },
        { MDDS_ASCII("Shenyang"),         6921000  },
        { MDDS_ASCII("Shenzhen"),         11908000 },
        { MDDS_ASCII("Singapore"),        5792000  },
        { MDDS_ASCII("Surat"),            6564000  },
        { MDDS_ASCII("Suzhou"),           6339000  },
        { MDDS_ASCII("Tehran"),           8896000  },
        { MDDS_ASCII("Tianjin"),          13215000 },
        { MDDS_ASCII("Tokyo"),            37400068 },
        { MDDS_ASCII("Toronto"),          6082000  },
        { MDDS_ASCII("Washington, D.C."), 5207000  },
        { MDDS_ASCII("Wuhan"),            8176000  },
        { MDDS_ASCII("Xi'an"),            7444000  },
        { MDDS_ASCII("Yangon"),           5157000  },
    };

It's a bit long as it contains entries for 81 cities.  We are then going to
create an instance of the :cpp:class:`~mdds::packed_trie_map` class directly::

    map_type cities(entries.data(), entries.size());

Let's print the size of the container to make sure the container has been
successfully populated::

    cout << "Number of cities: " << cities.size() << endl;

You will see the following output:

.. code-block:: none

    Number of cities: 81

if the container has been successfully populated.  Now, let's run a prefix
search on names beginning with an 'S'::

    cout << "Cities that begin with 'S':" << endl;
    auto results = cities.prefix_search("S");
    for (const auto& city : results)
        cout << "  * " << city.first << ": " << city.second << endl;

to make sure you get the following ten cities and their populations as the
output:

.. code-block:: none

    Cities that begin with 'S':
      * Saint Petersburg: 5383000
      * Santiago: 6680000
      * Sao Paulo: 21650000
      * Seoul: 9963000
      * Shanghai: 25582000
      * Shenyang: 6921000
      * Shenzhen: 11908000
      * Singapore: 5792000
      * Surat: 6564000
      * Suzhou: 6339000

So far so good.  Next, we will use the :cpp:func:`~mdds::packed_trie_map::save_state`
method to dump the internal state of this container to a file named **cities.bin**::

    std::ofstream outfile("cities.bin", ios::binary);
    cities.save_state(outfile);

This will create a file named **cities.bin** which contains a binary blob
representing the content of this container in the current working directory.
Run the ``ls -l cities.bin`` command to make sure the file has been created:

.. code-block:: none

    -rw-r--r-- 1 kohei kohei 17713 Jun 20 12:49 cities.bin

Now that the state of the container has been fully serialized to a file, let's
work on restoring its content in another, brand-new instance of
:cpp:class:`~mdds::packed_trie_map`.

::

    map_type cities_loaded;

    std::ifstream infile("cities.bin", ios::binary);
    cities_loaded.load_state(infile);

Here, we used the :cpp:func:`~mdds::packed_trie_map::load_state` method to
restore the state from the file we have previously created.  Let's make sure
that this new instance has content equivalent to that of the original::

    cout << "Equal to the original? " << std::boolalpha << (cities == cities_loaded) << endl;

If you see the following output:

.. code-block:: none

    Equal to the original? true

then this new instance has equivalent contant as the original one.  Let's also
make sure that it contains the same number of entries as the original::

    cout << "Number of cities: " << cities_loaded.size() << endl;

Hopefully you will see the following output:

.. code-block:: none

    Number of cities: 81

Lastly, let's run on this new instance the same prefix search we did on the
original instance, to make sure we still get the same results::

    cout << "Cities that begin with 'S':" << endl;
    auto results = cities_loaded.prefix_search("S");
    for (const auto& city : results)
        cout << "  * " << city.first << ": " << city.second << endl;

You should see the following output:

.. code-block:: none

    Cities that begin with 'S':
      * Saint Petersburg: 5383000
      * Santiago: 6680000
      * Sao Paulo: 21650000
      * Seoul: 9963000
      * Shanghai: 25582000
      * Shenyang: 6921000
      * Shenzhen: 11908000
      * Singapore: 5792000
      * Surat: 6564000
      * Suzhou: 6339000

which is the same output we saw in the first prefix search.

The complete source code for this example is found
`here <https://gitlab.com/mdds/mdds/-/blob/master/example/packed_trie_state_int.cpp>`__.


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

Traits
^^^^^^

.. doxygenstruct:: mdds::trie::std_container_trait
   :members:

.. doxygentypedef:: mdds::trie::std_string_trait
