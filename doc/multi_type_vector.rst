.. highlight:: cpp

Multi Type Vector
=================

Code Example
------------

Simple use case
^^^^^^^^^^^^^^^

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

   typedef mdds::multi_type_vector<mdds::mtv::element_block_func> mtv_type;

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
       std::for_each(con.begin(), con.end(),
           [](const mtv_type::value_type& v)
           {
               switch (v.type)
               {
                   case mdds::mtv::element_type_numeric:
                   {
                       cout << "numeric block of size " << v.size << endl;
                       print_block<mdds::mtv::numeric_element_block>(v);
                   }
                   break;
                   case mdds::mtv::element_type_string:
                   {
                       cout << "string block of size " << v.size << endl;
                       print_block<mdds::mtv::string_element_block>(v);
                   }
                   break;
                   case mdds::mtv::element_type_empty:
                       cout << "empty block of size " << v.size << endl;
                       cout << " - no data - " << endl;
                   default:
                       ;
               }
           }
       );

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

   Figure depicting the ownership structure between the primary array, blocks,
   and element blocks.

Each container instance consists of an array of blocks each of which stores
``type``, ``position``, ``size`` and ``data`` members.  In this example code,
the ``type`` member is referenced to determine its block type and its logical
size is determine from the ``size`` member.  For the numeric and string blocks,
their ``data`` members, which should point to valid memory addresses of their
respective element blocks, are dereferenced to gain access to them in order to
print out their contents to stdout inside the ``print_block`` function.

Use custom event handlers
^^^^^^^^^^^^^^^^^^^^^^^^^

It is also possible to define custom event handlers that get called when
certain events take place.  To define custom event handlers, you need to
define either a class or a struct that has the following methods:

* **void element_block_acquired(mdds::mtv::base_element_block* block)**
* **void element_block_released(mdds::mtv::base_element_block* block)**

as its public methods, then pass it as the second template argument when
instantiating your :cpp:class:`~mdds::multi_type_vector` type.  Refer to
:cpp:member:`mdds::multi_type_vector::event_func` for the details on when each
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

   typedef mdds::multi_type_vector<mdds::mtv::element_block_func, event_hdl> mtv_type;

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
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Sometimes you need to expose a pointer to an element block array especially
when you need to pass such an array pointer to C API that requires one.  You
can do this by calling the ``at`` method of the element_block template class
and taking the memory address of the reference returned by the method.  This
works since the element block internally just wraps :cpp:class:`std::vector`
(or :cpp:class:`std::deque` in case the ``MDDS_MULTI_TYPE_VECTOR_USE_DEQUE``
preprocessing macro is defined), and its ``at`` method simply exposes vector's
own ``at`` method which returns a reference to an element within it.

The following code demonstrates this by exposing raw array pointers to the
internal arrays of numeric and string element blocks, and printing their
element values directly from these array pointers.

::

   #include <mdds/multi_type_vector.hpp>
   #include <mdds/multi_type_vector_trait.hpp>
   #include <iostream>

   using namespace std;
   using mdds::mtv::numeric_element_block;
   using mdds::mtv::string_element_block;

   typedef mdds::multi_type_vector<mdds::mtv::element_block_func> mtv_type;

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
       // 'at' method and taking the address of the returned reference.
       const double* p = &numeric_element_block::at(*it->data, 0);

       // Print the elements from this raw array pointer.
       for (const double* p_end = p + it->size; p != p_end; ++p)
           cout << *p << endl;

       cout << "--" << endl;

       ++it; // move to the next block, which is a string block.

       // Get a pointer to the raw array of the string element block.
       const string* pz = &string_element_block::at(*it->data, 0);

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

Performance Considerations
--------------------------

Use of position hint to avoid expensive block position lookup
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Consider the following example code::

   typedef mdds::multi_type_vector<mdds::mtv::element_block_func> mtv_type;

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

   typedef mdds::multi_type_vector<mdds::mtv::element_block_func> mtv_type;

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
of the last modified block.  Each :cpp:member:`~mdds::multi_type_vector::set`
method call returns an iterator which can then be passed to the next
:cpp:member:`~mdds::multi_type_vector::set` call as the position hint.
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
