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

    using trie_map_type = mdds::trie_map<mdds::trie::std_string_traits, int>;

The first template argument specifies the trait of the key.  In this example,
we are using a pre-defined trait for std::string, which is defined in
:cpp:type:`~mdds::trie::std_string_traits`.  The second template argument
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

    using trie_map_type = mdds::packed_trie_map<mdds::trie::std_string_traits, int>;

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

    using map_type = mdds::packed_trie_map<mdds::trie::std_string_traits, int>;

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

    std::ofstream outfile("cities.bin", std::ios::binary);
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

    std::ifstream infile("cities.bin", std::ios::binary);
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


Saving Packed Trie Map with custom value type
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In the previos example, you didn't have to explicitly specify the serializer type
to the :cpp:func:`~mdds::packed_trie_map::save_state` and
:cpp:func:`~mdds::packed_trie_map::load_state` methods, even though these two
methods require the serializer type as their template arguments.  That's because
the library provides default serializer types for

* numeric value types i.e. integers, float and double,
* ``std::string``, and
* the standard sequence types, such as ``std::vector``, whose elements are of
  numeric value types,

and the previous example used ``int`` as the value type.

In this section, we are going to illustrate how you can write your own custom
serializer to allow serialization of your own custom value type.  In this example,
we are going to use `the list of presidents of the United States
<https://en.wikipedia.org/wiki/List_of_presidents_of_the_United_States>`__,
with the names of the presidents as the keys, and their years of inauguration
and political affiliations as the values.

We will use the following structure to store the values::

    enum affiliated_party_t : uint8_t
    {
        unaffiliated = 0,
        federalist,
        democratic_republican,
        democratic,
        whig,
        republican,
        national_union,
        republican_national_union,
    };

    struct us_president
    {
        uint16_t year;
        affiliated_party_t party;
    };

Each entry stores the year as a 16-bit integer and the affiliated party as an enum
value of 8-bit width.

Next, let's define the container type::

    using map_type = mdds::packed_trie_map<mdds::trie::std_string_traits, us_president>;

As with the previous example, the first step is to define the entries that are
sorted by the keys, which in this case are the president's names::

    std::vector<map_type::entry> entries =
    {
        { MDDS_ASCII("Abraham Lincoln"),        { 1861, republican_national_union } },
        { MDDS_ASCII("Andrew Jackson"),         { 1829, democratic                } },
        { MDDS_ASCII("Andrew Johnson"),         { 1865, national_union            } },
        { MDDS_ASCII("Barack Obama"),           { 2009, democratic                } },
        { MDDS_ASCII("Benjamin Harrison"),      { 1889, republican                } },
        { MDDS_ASCII("Bill Clinton"),           { 1993, democratic                } },
        { MDDS_ASCII("Calvin Coolidge"),        { 1923, republican                } },
        { MDDS_ASCII("Chester A. Arthur"),      { 1881, republican                } },
        { MDDS_ASCII("Donald Trump"),           { 2017, republican                } },
        { MDDS_ASCII("Dwight D. Eisenhower"),   { 1953, republican                } },
        { MDDS_ASCII("Franklin D. Roosevelt"),  { 1933, democratic                } },
        { MDDS_ASCII("Franklin Pierce"),        { 1853, democratic                } },
        { MDDS_ASCII("George H. W. Bush"),      { 1989, republican                } },
        { MDDS_ASCII("George W. Bush"),         { 2001, republican                } },
        { MDDS_ASCII("George Washington"),      { 1789, unaffiliated              } },
        { MDDS_ASCII("Gerald Ford"),            { 1974, republican                } },
        { MDDS_ASCII("Grover Cleveland 1"),     { 1885, democratic                } },
        { MDDS_ASCII("Grover Cleveland 2"),     { 1893, democratic                } },
        { MDDS_ASCII("Harry S. Truman"),        { 1945, democratic                } },
        { MDDS_ASCII("Herbert Hoover"),         { 1929, republican                } },
        { MDDS_ASCII("James A. Garfield"),      { 1881, republican                } },
        { MDDS_ASCII("James Buchanan"),         { 1857, democratic                } },
        { MDDS_ASCII("James K. Polk"),          { 1845, democratic                } },
        { MDDS_ASCII("James Madison"),          { 1809, democratic_republican     } },
        { MDDS_ASCII("James Monroe"),           { 1817, democratic_republican     } },
        { MDDS_ASCII("Jimmy Carter"),           { 1977, democratic                } },
        { MDDS_ASCII("John Adams"),             { 1797, federalist                } },
        { MDDS_ASCII("John F. Kennedy"),        { 1961, democratic                } },
        { MDDS_ASCII("John Quincy Adams"),      { 1825, democratic_republican     } },
        { MDDS_ASCII("John Tyler"),             { 1841, whig                      } },
        { MDDS_ASCII("Lyndon B. Johnson"),      { 1963, democratic                } },
        { MDDS_ASCII("Martin Van Buren"),       { 1837, democratic                } },
        { MDDS_ASCII("Millard Fillmore"),       { 1850, whig                      } },
        { MDDS_ASCII("Richard Nixon"),          { 1969, republican                } },
        { MDDS_ASCII("Ronald Reagan"),          { 1981, republican                } },
        { MDDS_ASCII("Rutherford B. Hayes"),    { 1877, republican                } },
        { MDDS_ASCII("Theodore Roosevelt"),     { 1901, republican                } },
        { MDDS_ASCII("Thomas Jefferson"),       { 1801, democratic_republican     } },
        { MDDS_ASCII("Ulysses S. Grant"),       { 1869, republican                } },
        { MDDS_ASCII("Warren G. Harding"),      { 1921, republican                } },
        { MDDS_ASCII("William Henry Harrison"), { 1841, whig                      } },
        { MDDS_ASCII("William Howard Taft"),    { 1909, republican                } },
        { MDDS_ASCII("William McKinley"),       { 1897, republican                } },
        { MDDS_ASCII("Woodrow Wilson"),         { 1913, democratic                } },
        { MDDS_ASCII("Zachary Taylor"),         { 1849, whig                      } },
    };

Note that we need to add numeric suffixes to the entries for Grover Cleveland,
who became president twice in two separate periods, in order to make the keys
for his entries unique.

Now, proceed to create an instance of :cpp:class:`~mdds::packed_trie_map`::

    map_type us_presidents(entries.data(), entries.size());

and inspect its size to make sure it is instantiated properly::

    cout << "Number of entries: " << us_presidents.size() << endl;

You should see the following output:

.. code-block:: none

    Number of entries: 45

Before we proceed to save the state of this instance, let's define the custom
serializer type first::

    struct us_president_serializer
    {
        union bin_buffer
        {
            char buffer[2];
            uint16_t i16;
            affiliated_party_t party;
        };

        static constexpr bool variable_size = false;
        static constexpr size_t value_size = 3;

        static void write(std::ostream& os, const us_president& v)
        {
            bin_buffer buf;

            // Write the year value first.
            buf.i16 = v.year;
            os.write(buf.buffer, 2);

            // Write the affiliated party value.
            buf.party = v.party;
            os.write(buf.buffer, 1);
        }

        static void read(std::istream& is, size_t n, us_president& v)
        {
            // For a fixed-size value type, this should equal the defined value size.
            assert(n == 3);

            bin_buffer buf;

            // Read the year value.
            is.read(buf.buffer, 2);
            v.year = buf.i16;

            // Read the affiliated party value.
            is.read(buf.buffer, 1);
            v.party = buf.party;
        }
    };

A custom value type can be either variable-size or fixed-size.  For a variable-size
value type, each value segment is preceded by the byte length of that segment.
For a fixed-size value type, the byte length of all of the value segments
is written only once up-front, followed by one or more value segments of equal
byte length.

Since the value type in this example is fixed-size, we set the value of the
``variable_size`` static constant to false, and define the size of the value to 3 (bytes)
as the ``value_size`` static constant.  Keep in mind that you need to define
the ``value_size`` constant *only* for fixed-size value types; if your value
type is variable-size, you can leave it out.

Additionally, you need to define two static methods - one for writing to the
output stream, and one for reading from the input stream.  The write method
must have the following signature::

    static void write(std::ostream& os, const T& v);

where the ``T`` is the value type.  In the body of this method you write to the
output stream the bytes that represent the value.  The length of the bytes you
write must match the size specified by the ``value_size`` constant.

The read method must have the following signature::

    static void read(std::istream& is, size_t n, T& v);

where the ``T`` is the value type, and the ``n`` specifies the length of the
bytes you need to read for the value.  For a fixed-size value type, the value
of ``n`` should equal the ``value_size`` constant.  Your job is to read the
bytes off of the input stream for the length specified by the ``n``, and
populate the value instance passed to the method as the third argument.

Now that we have defined the custom serializer type, let's proceed to save the
state to a file::

    std::ofstream outfile("us-presidents.bin", std::ios::binary);
    us_presidents.save_state<us_president_serializer>(outfile);

This time around, we are specifying the serializer type explicitly as the template
argument to the :cpp:func:`~mdds::packed_trie_map::save_state` method.  Otherwise
it is no different than what we did in the previous example.

Let's create another instance of :cpp:class:`~mdds::packed_trie_map` and restore
the state back from the file we just created::

    map_type us_presidents_loaded;

    std::ifstream infile("us-presidents.bin", std::ios::binary);
    us_presidents_loaded.load_state<us_president_serializer>(infile);

Once again, aside from explicitly specifying the serializer type as the template
argument to the :cpp:func:`~mdds::packed_trie_map::load_state` method, it is
identical to the way we did in the previous example.

Let's compare the new instance with the old one to see if the two are equal::

    cout << "Equal to the original? " << std::boolalpha << (us_presidents == us_presidents_loaded) << endl;

The output says:

.. code-block:: none

    Equal to the original? true

They are.  While we are at it, let's run a simple prefix search to find out
all the US presidents whose first name is 'John'::

    cout << "Presidents whose first name is 'John':" << endl;
    auto results = us_presidents_loaded.prefix_search("John");
    for (const auto& entry : results)
        cout << "  * " << entry.first << " (" << entry.second.year << "; " << entry.second.party << ")" << endl;

Here is the output:

.. code-block:: none

    Presidents whose first name is 'John':
      * John Adams (1797; Federalist)
      * John F. Kennedy (1961; Democratic)
      * John Quincy Adams (1825; Democratic Republican)
      * John Tyler (1841; Whig)

This looks like the correct results!

You can find the complete source code for this example `here
<https://gitlab.com/mdds/mdds/-/blob/master/example/packed_trie_state_custom.cpp>`__.


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

.. doxygenstruct:: mdds::trie::std_container_traits
   :members:

.. doxygentypedef:: mdds::trie::std_string_traits


Value Serializers
^^^^^^^^^^^^^^^^^

.. doxygenstruct:: mdds::trie::value_serializer
   :members:

.. doxygenstruct:: mdds::trie::numeric_value_serializer
   :members:

.. doxygenstruct:: mdds::trie::variable_value_serializer
   :members:

.. doxygenstruct:: mdds::trie::numeric_sequence_value_serializer
   :members:
