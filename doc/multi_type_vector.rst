.. highlight:: cpp

Multi Type Vector
=================

.. doxygenstruct:: mdds::detail::mtv_event_func
   :members:

.. doxygenclass:: mdds::multi_type_vector
   :members:


Example
-------

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
   #include <vector>
   #include <string>

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
