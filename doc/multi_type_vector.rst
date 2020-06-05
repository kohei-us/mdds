.. highlight:: cpp

Multi Type Vector
=================

Quick start
-----------

The following code demonstrates a simple use case of storing values of double
and :cpp:class:`std::string` types in a single container using :cpp:class:`~mdds::multi_type_vector`.

::

    #include <mdds/multi_type_vector.hpp>
    #include <mdds/multi_type_vector_trait.hpp>
    #include <iostream>
    #include <vector>
    #include <string>

    using std::cout;
    using std::endl;

    using mtv_type = mdds::multi_type_vector<mdds::mtv::element_block_func>;

    template<typename _Blk>
    void print_block(const mtv_type::value_type& v)
    {
        // Each element block has static begin() and end() methods that return
        // begin and end iterators, respectively, from the passed element block
        // instance.
        auto it = _Blk::begin(*v.data);
        auto it_end = _Blk::end(*v.data);

        std::for_each(it, it_end,
            [](const typename _Blk::value_type& elem)
            {
                cout << " * " << elem << endl;
            }
        );
    }

    int main()
    {
        mtv_type con(20); // Initialized with 20 empty elements.

        // Set values individually.
        con.set(0, 1.1);
        con.set(1, 1.2);
        con.set(2, 1.3);

        // Set a sequence of values in one step.
        std::vector<double> vals = { 10.1, 10.2, 10.3, 10.4, 10.5 };
        con.set(3, vals.begin(), vals.end());

        // Set string values.
        con.set(10, std::string("Andy"));
        con.set(11, std::string("Bruce"));
        con.set(12, std::string("Charlie"));

        // Iterate through all blocks and print all elements.
        for (const mtv_type::value_type& v : con)
        {
            switch (v.type)
            {
                case mdds::mtv::element_type_double:
                {
                    cout << "numeric block of size " << v.size << endl;
                    print_block<mdds::mtv::double_element_block>(v);
                    break;
                }
                case mdds::mtv::element_type_string:
                {
                    cout << "string block of size " << v.size << endl;
                    print_block<mdds::mtv::string_element_block>(v);
                    break;
                }
                case mdds::mtv::element_type_empty:
                    cout << "empty block of size " << v.size << endl;
                    cout << " - no data - " << endl;
                default:
                    ;
            }
        }

        return EXIT_SUCCESS;
    }

You'll see the following console output when you compile and execute this code:

.. code-block:: none

   numeric block of size 8
    * 1.1
    * 1.2
    * 1.3
    * 10.1
    * 10.2
    * 10.3
    * 10.4
    * 10.5
   empty block of size 2
    - no data -
   string block of size 3
    * Andy
    * Bruce
    * Charlie
   empty block of size 7
    - no data -

.. figure:: _static/images/mtv_block_structure.png
   :align: right

   Ownership structure between the primary array, blocks, and element blocks.

Each container instance consists of an array of blocks each of which stores
``type``, ``position``, ``size`` and ``data`` members.  In this example code,
the ``type`` member is referenced to determine its block type and its logical
size is determine from the ``size`` member.  For the numeric and string blocks,
their ``data`` members, which should point to valid memory addresses of their
respective element blocks, are dereferenced to gain access to them in order to
print out their contents to stdout inside the ``print_block`` function.


Use custom event handlers
-------------------------

It is also possible to define custom event handlers that get called when
certain events take place.  To define custom event handlers, you need to
define either a class or a struct that has the following methods:

* **void element_block_acquired(mdds::mtv::base_element_block* block)**
* **void element_block_released(mdds::mtv::base_element_block* block)**

as its public methods, then pass it as the second template argument when
instantiating your :cpp:class:`~mdds::multi_type_vector` type.  Refer to
:cpp:type:`mdds::multi_type_vector::event_func` for the details on when each
event handler method gets triggered.

The following code example demonstrates how this all works::

    #include <mdds/multi_type_vector.hpp>
    #include <mdds/multi_type_vector_trait.hpp>
    #include <iostream>

    using namespace std;

    class event_hdl
    {
    public:
        void element_block_acquired(mdds::mtv::base_element_block* block)
        {
            cout << "  * element block acquired" << endl;
        }

        void element_block_released(mdds::mtv::base_element_block* block)
        {
            cout << "  * element block released" << endl;
        }
    };

    using mtv_type = mdds::multi_type_vector<mdds::mtv::element_block_func, event_hdl>;

    int main()
    {
        mtv_type db;  // starts with an empty container.

        cout << "inserting string 'foo'..." << endl;
        db.push_back(string("foo"));  // creates a new string element block.

        cout << "inserting string 'bah'..." << endl;
        db.push_back(string("bah"));  // appends to an existing string block.

        cout << "inserting int 100..." << endl;
        db.push_back(int(100)); // creates a new int element block.

        cout << "emptying the container..." << endl;
        db.clear(); // releases both the string and int element blocks.

        cout << "exiting program..." << endl;

        return EXIT_SUCCESS;
    }

You'll see the following console output when you compile and execute this code:

.. code-block:: none

   inserting string 'foo'...
     * element block acquired
   inserting string 'bah'...
   inserting int 100...
     * element block acquired
   emptying the container...
     * element block released
     * element block released
   exiting program...

In this example, the **element_block_acquired** handler gets triggered each
time the container creates (thus acquires) a new element block to store a value.
It does *not* get called when a new value is appended to a pre-existing element
block.  Similarly, the **element_block_releasd** handler gets triggered each
time an existing element block storing non-empty values gets deleted.  One
thing to keep in mind is that since these two handlers pertain to element
blocks which are owned by non-empty blocks, and empty blocks don't own element
block instances, creations or deletions of empty blocks don't trigger these
event handlers.


Get raw pointer to element block array
--------------------------------------

Sometimes you need to expose a pointer to an element block array
especially when you need to pass such an array pointer to C API that
requires one.  You can do this by calling the ``data`` method of the
element_block template class .  This works since the element block
internally just wraps :cpp:class:`std::vector` (or
:cpp:class:`std::deque` in case the ``MDDS_MULTI_TYPE_VECTOR_USE_DEQUE``
preprocessing macro is defined), and its ``data`` method simply exposes
vector's own ``data`` method which returns the memory location of its
internal array storage.

The following code demonstrates this by exposing raw array pointers to the
internal arrays of numeric and string element blocks, and printing their
element values directly from these array pointers.

::

    #include <mdds/multi_type_vector.hpp>
    #include <mdds/multi_type_vector_trait.hpp>
    #include <iostream>

    using namespace std;
    using mdds::mtv::double_element_block;
    using mdds::mtv::string_element_block;

    using mtv_type = mdds::multi_type_vector<mdds::mtv::element_block_func>;

    int main()
    {
        mtv_type db;  // starts with an empty container.

        db.push_back(1.1);
        db.push_back(1.2);
        db.push_back(1.3);
        db.push_back(1.4);
        db.push_back(1.5);

        db.push_back(string("A"));
        db.push_back(string("B"));
        db.push_back(string("C"));
        db.push_back(string("D"));
        db.push_back(string("E"));

        // At this point, you have 2 blocks in the container.
        cout << "block size: " << db.block_size() << endl;
        cout << "--" << endl;

        // Get an iterator that points to the first block in the primary array.
        mtv_type::const_iterator it = db.begin();

        // Get a pointer to the raw array of the numeric element block using the
        // 'data' method.
        const double* p = double_element_block::data(*it->data);

        // Print the elements from this raw array pointer.
        for (const double* p_end = p + it->size; p != p_end; ++p)
            cout << *p << endl;

        cout << "--" << endl;

        ++it; // move to the next block, which is a string block.

        // Get a pointer to the raw array of the string element block.
        const string* pz = string_element_block::data(*it->data);

        // Print out the string elements.
        for (const string* pz_end = pz + it->size; pz != pz_end; ++pz)
            cout << *pz << endl;

        return EXIT_SUCCESS;
    }

Compiling and execute this code produces the following output:

.. code-block:: none

   block size: 2
   --
   1.1
   1.2
   1.3
   1.4
   1.5
   --
   A
   B
   C
   D
   E


Traverse multiple multi_type_vector instances "sideways"
--------------------------------------------------------

In this section we will demonstrate a way to traverse multiple instances of
:cpp:class:`~mdds::multi_type_vector` "sideways" using the
:cpp:class:`mdds::mtv::collection` class.  What this class does is to wrap
multiple instances of :cpp:class:`~mdds::multi_type_vector` and generate
iterators that let you iterate the individual element values collectively in
the direction orthogonal to the direction of the individual vector instances.

The best way to explain this feature is to use a spreadsheet analogy.  Let's
say we are implementing a data store to store a 2-dimensional tabular data
where each cell in the data set is associated with row and column indices.
Each cell may store a value of string type, integer type, numeric type, etc.
And let's say that the data looks like the following spreadsheet data:

.. figure:: _static/images/mtv_collection_sheet.png
   :align: center

It consists of five columns, with each column storing 21 rows of data.  The
first row is a header row, followed by 20 rows of values.  In this example, We
will be using one :cpp:class:`~mdds::multi_type_vector` instance for each
column thus creating five instances in total, and store them in a
``std::vector`` container.

The declaration of the data store will look like this::

    using mtv_type = mdds::multi_type_vector<mdds::mtv::element_block_func>;
    using collection_type = mdds::mtv::collection<mtv_type>;

    std::vector<mtv_type> columns(5);

The first two lines specify the concrete :cpp:class:`~mdds::multi_type_vector`
type used for each individual column and the collection type that wraps the
columns.  The third line instantiates the ``std::vector`` instance to store
the columns, and we are setting its size to five to accommodate for five
columns.  We will make use of the collection_type later in this example after
the columns have been populated.

Now, we need to populate the columns with values.  First, we are setting the
header row::

    // Populate the header row.
    auto headers = { "ID", "Make", "Model", "Year", "Color" };
    size_t i = 0;
    std::for_each(headers.begin(), headers.end(), [&](const char* v) { columns[i++].push_back<std::string>(v); });

We are then filling each column individually from column 1 through column 5.
First up is column 1::

    // Fill column 1.
    auto c1_values = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
    std::for_each(c1_values.begin(), c1_values.end(), [&columns](int v) { columns[0].push_back(v); });

Hopefully this code is straight-forward.  It initializes an array of values
and push them to the column one at a time via
:cpp:func:`~mdds::multi_type_vector::push_back`.  Next up is column 2::

    // Fill column 2.
    auto c2_values =
    {
        "Nissan", "Mercedes-Benz", "Nissan", "Suzuki", "Saab", "Subaru", "GMC", "Mercedes-Benz", "Toyota", "Nissan",
        "Mazda", "Dodge", "Ford", "Bentley", "GMC", "Audi", "GMC", "Mercury", "Pontiac", "BMW",
    };

    std::for_each(c2_values.begin(), c2_values.end(), [&columns](const char* v) { columns[1].push_back<std::string>(v); });

This is similar to the code for column 1, except that because we are using an
array of string literals which implicitly becomes an initializer list of type
``const char*``, we need to explicitly specify the type for the
:cpp:func:`~mdds::multi_type_vector::push_back` call to be ``std::string``.

The code for column 3 is very similar to this::

    // Fill column 3.
    auto c3_values =
    {
        "Frontier", "W201", "Frontier", "Equator", "9-5", "Tribeca", "Yukon XL 2500", "E-Class", "Camry Hybrid", "Frontier",
        "MX-5", "Ram Van 1500", "Edge", "Azure", "Sonoma Club Coupe", "S4", "3500 Club Coupe", "Villager", "Sunbird", "3 Series",
    };

    std::for_each(c3_values.begin(), c3_values.end(), [&columns](const char* v) { columns[2].push_back<std::string>(v); });

Populating column 4 needs slight pre-processing.  We are inserting a string
value of "unknown" in lieu of an integer value of -1.  Therefore the following
code will do::

    // Fill column 4.  Replace -1 with "unknown".
    std::initializer_list<int32_t> c4_values =
    {
        1998, 1986, 2009, -1, -1, 2008, 2009, 2008, 2010, 2001,
        2008, 2000, -1, 2009, 1998, 2013, 1994, 2000, 1990, 1993,
    };

    for (int32_t v : c4_values)
    {
        if (v < 0)
            // Insert a string value "unknown".
            columns[3].push_back<std::string>("unknown");
        else
            columns[3].push_back(v);
    }

Finally, the last column to fill, which uses the same logic as for columns 2
and 3::

    // Fill column 5
    auto c5_values =
    {
        "Turquoise", "Fuscia", "Teal", "Fuscia", "Green", "Khaki", "Pink", "Goldenrod", "Turquoise", "Yellow",
        "Orange", "Goldenrod", "Fuscia", "Goldenrod", "Mauv", "Crimson", "Turquoise", "Teal", "Indigo", "LKhaki",
    };

    std::for_each(c5_values.begin(), c5_values.end(), [&columns](const char* v) { columns[4].push_back<std::string>(v); });

At this point, the content we've put into the ``columns`` variable roughly
reflects the tabular data shown at the beginning of this section.  Now we can
use the collection type we've declared earlier to wrap the columns::

    // Wrap the columns with the 'collection'...
    collection_type rows(columns.begin(), columns.end());

We are naming this variable ``rows`` since what we are doing with this wrapper
is to traverse the content of the tabular data in row-wise direction.  For
this reason, calling it ``rows`` is quite fitting.

The :cpp:class:`~mdds::mtv::collection` class offers some flexibility as to
how the instances that you are trying to traverse orthogonally are stored.
That being said, you must meet the following prerequisites when passing the
collection of vector instances to the constructor of the
:cpp:class:`~mdds::mtv::collection` class:

1. All :cpp:class:`~mdds::multi_type_vector` instances that comprise the
   collection must be of the same logical length i.e. their
   :cpp:func:`~mdds::multi_type_vector::size` methods must all return the same
   value.
2. The instances in the collection must be stored in the source container
   either as

   * concrete instances (as in this example),
   * as pointers, or
   * as heap instances wrapped within smart pointer class such as
     ``std::shared_ptr`` or ``std::unique_ptr``.

Although we are storing the vector instances in a ``std::vector`` container in
this example, you have the flexibility to pick a different type of container
to store the individual vector instances as long as it provides STL-compatible
standard iterator functionality.

Additionally, when using the :cpp:class:`~mdds::mtv::collection` class, you
must ensure that the content of the vector instances that it references will
not change for the duration of its use.

Finally, here is the code that does the traversing::

    // Traverse the tabular data in row-wise direction.
    for (const auto& cell : rows)
    {
        if (cell.index > 0)
            // Insert a column separator before each cell except for the ones in the first column.
            std::cout << " | ";

        switch (cell.type)
        {
            // In this example, we use two element types only.
            case mdds::mtv::element_type_int32:
                std::cout << cell.get<mdds::mtv::int32_element_block>();
                break;
            case mdds::mtv::element_type_string:
                std::cout << cell.get<mdds::mtv::string_element_block>();
                break;
            default:
                std::cout << "???"; // The default case should not hit in this example.
        }

        if (cell.index == 4)
            // We are in the last column. Insert a line break.
            std::cout << std::endl;
    }

It's a simple for-loop, and in each iteration you get a single cell node that
contains metadata about that cell including its value.  The node contains the
following members:

* ``type`` - an integer value representing the type of the value.
* ``index`` -  a 0-based index of the :cpp:class:`~mdds::multi_type_vector`
  instance within the collection.  You can think of this as column index in
  this example.
* ``position`` - a 0-based logical element position within each
  :cpp:class:`~mdds::multi_type_vector` instance.  You can think of this as
  row index in this example.

In the current example we are only making use of the ``type`` and ``index``
members, but the ``position`` member will be there if you need it.

The node also provides a convenient ``get()`` method to fetch the value of the
cell.  This method is a template method, and you need to explicitly specify
the element block type in order to access the value.

When executing this code, you will see the following outout:

.. code-block:: none

    ID | Make | Model | Year | Color
    1 | Nissan | Frontier | 1998 | Turquoise
    2 | Mercedes-Benz | W201 | 1986 | Fuscia
    3 | Nissan | Frontier | 2009 | Teal
    4 | Suzuki | Equator | unknown | Fuscia
    5 | Saab | 9-5 | unknown | Green
    6 | Subaru | Tribeca | 2008 | Khaki
    7 | GMC | Yukon XL 2500 | 2009 | Pink
    8 | Mercedes-Benz | E-Class | 2008 | Goldenrod
    9 | Toyota | Camry Hybrid | 2010 | Turquoise
    10 | Nissan | Frontier | 2001 | Yellow
    11 | Mazda | MX-5 | 2008 | Orange
    12 | Dodge | Ram Van 1500 | 2000 | Goldenrod
    13 | Ford | Edge | unknown | Fuscia
    14 | Bentley | Azure | 2009 | Goldenrod
    15 | GMC | Sonoma Club Coupe | 1998 | Mauv
    16 | Audi | S4 | 2013 | Crimson
    17 | GMC | 3500 Club Coupe | 1994 | Turquoise
    18 | Mercury | Villager | 2000 | Teal
    19 | Pontiac | Sunbird | 1990 | Indigo
    20 | BMW | 3 Series | 1993 | LKhaki

which clearly shows that the code has traversed the content of the tabular
data horizontally across columns as intended.

Now, one feature that may come in handy is the ability to limit the iteration
range within the collection.  You can do that by calling either
:cpp:func:`~mdds::mtv::collection::set_collection_range` to limit the column
range or :cpp:func:`~mdds::mtv::collection::set_element_range` to limit the
row range, or perhaps both.

Let's see how this works in the current example.  Here, we are going to limit
the iteration range to only columns 2 and 3, and rows 2 through 11.  The following
code will set this limit::

    rows.set_collection_range(1, 2); // only columns 2 and 3.
    rows.set_element_range(1, 10);   // only rows 2 through 11.

Then iterate through the collection once again::

    for (const auto& cell : rows)
    {
        if (cell.index > 1)
            // Insert a column separator before each cell except for the ones in the first column.
            std::cout << " | ";

        switch (cell.type)
        {
            // In this example, we use two element types only.
            case mdds::mtv::element_type_int32:
                std::cout << cell.get<mdds::mtv::int32_element_block>();
                break;
            case mdds::mtv::element_type_string:
                std::cout << cell.get<mdds::mtv::string_element_block>();
                break;
            default:
                std::cout << "???"; // The default case should not hit in this example.
        }

        if (cell.index == 2)
            // We are in the last column. Insert a line break.
            std::cout << std::endl;
    }

This code is nearly identical to the previous one except for the index values
used to control when to insert column separators and line breaks at the top
and bottom of each iteration.  When executing this code, you'll see the
following output:

.. code-block:: none

    Nissan | Frontier
    Mercedes-Benz | W201
    Nissan | Frontier
    Suzuki | Equator
    Saab | 9-5
    Subaru | Tribeca
    GMC | Yukon XL 2500
    Mercedes-Benz | E-Class
    Toyota | Camry Hybrid
    Nissan | Frontier

which clearly shows that your iteration range did indeed shrink as expected.


Performance Considerations
--------------------------

Use of position hint to avoid expensive block position lookup
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Consider the following example code::

    using mtv_type = mdds::multi_type_vector<mdds::mtv::element_block_func>;

    size_t size = 50000;

    // Initialize the container with one empty block of size 50000.
    mtv_type db(size);

    // Set non-empty value at every other logical position from top down.
    for (size_t i = 0; i < size; ++i)
    {
        if (i % 2)
            db.set<double>(i, 1.0);
    }

which, when executed, takes quite sometime to complete.  This particular example
exposes one weakness that multi_type_vector has; because it needs to first
look up the position of the block to operate with, and that lookup *always*
starts from the first block, the time it takes to find the correct block
increases as the number of blocks goes up.  This example demonstrates the
worst case scenario of such lookup complexity since it always inserts the next
value at the last block position.

Fortunately, there is a simple solution to this which the following code
demonstrates::

    using mtv_type = mdds::multi_type_vector<mdds::mtv::element_block_func>;

    size_t size = 50000;

    // Initialize the container with one empty block of size 50000.
    mtv_type db(size);
    mtv_type::iterator pos = db.begin();

    // Set non-empty value at every other logical position from top down.
    for (size_t i = 0; i < size; ++i)
    {
        if (i % 2)
            // Pass the position hint as the first argument, and receive a new
            // one returned from the method for the next call.
            pos = db.set<double>(pos, i, 1.0);
    }

Compiling and executing this code should take only a fraction of a second.

The only difference between the second example and the first one is that the
second one uses an interator as a position hint to keep track of the position
of the last modified block.  Each :cpp:func:`~mdds::multi_type_vector::set`
method call returns an iterator which can then be passed to the next
:cpp:func:`~mdds::multi_type_vector::set` call as the position hint.
Because an iterator object internally stores the location of the block the
value was inserted to, this lets the method to start the block position lookup
process from the last modified block, which in this example is always one
block behind the one the new value needs to go.  Using the big-O notation, the
use of the position hint essentially turns the complexity of O(n^2) in the
first example into O(1) in the second one.

This strategy should work with any methods in :cpp:class:`~mdds::multi_type_vector`
that take a position hint as the first argument.


API Reference
-------------

.. doxygenstruct:: mdds::detail::mtv::event_func
   :members:

.. doxygenclass:: mdds::multi_type_vector
   :members:

.. doxygenclass:: mdds::mtv::collection
   :members:

Element Blocks
^^^^^^^^^^^^^^

.. doxygenstruct:: mdds::mtv::base_element_block
   :members:

.. doxygenclass:: mdds::mtv::element_block
   :members:

.. doxygenstruct:: mdds::mtv::default_element_block
   :members:

.. doxygenclass:: mdds::mtv::copyable_element_block
   :members:

.. doxygenclass:: mdds::mtv::noncopyable_element_block
   :members:

.. doxygenstruct:: mdds::mtv::managed_element_block
   :members:

.. doxygenstruct:: mdds::mtv::noncopyable_managed_element_block
   :members:

.. doxygenstruct:: mdds::mtv::element_block_func
   :members:

Element Types
^^^^^^^^^^^^^

.. doxygentypedef:: mdds::mtv::element_t

.. doxygenvariable:: mdds::mtv::element_type_empty
.. doxygenvariable:: mdds::mtv::element_type_boolean
.. doxygenvariable:: mdds::mtv::element_type_int8
.. doxygenvariable:: mdds::mtv::element_type_uint8
.. doxygenvariable:: mdds::mtv::element_type_int16
.. doxygenvariable:: mdds::mtv::element_type_uint16
.. doxygenvariable:: mdds::mtv::element_type_int32
.. doxygenvariable:: mdds::mtv::element_type_uint32
.. doxygenvariable:: mdds::mtv::element_type_int64
.. doxygenvariable:: mdds::mtv::element_type_uint64
.. doxygenvariable:: mdds::mtv::element_type_float
.. doxygenvariable:: mdds::mtv::element_type_double
.. doxygenvariable:: mdds::mtv::element_type_string
.. doxygenvariable:: mdds::mtv::element_type_user_start

Exceptions
^^^^^^^^^^

.. doxygenclass:: mdds::mtv::element_block_error
