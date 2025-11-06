
Use custom event handlers
=========================

It is also possible to define custom event handlers that get called when
certain events take place.  To define custom event handlers, you need to
define either a class or a struct that has the following methods:

* **void element_block_acquired(mdds::mtv::base_element_block* block)**
* **void element_block_released(mdds::mtv::base_element_block* block)**

as its public methods, specify it as type named ``event_func`` in a trait struct,
and pass it as the second template argument when instantiating your
:cpp:type:`~mdds::multi_type_vector` type.  Refer to :cpp:type:`mdds::mtv::empty_event_func`
for the detail on when each event handler method gets triggered.

The following code example demonstrates how this all works:

.. literalinclude:: ../../../example/multi_type_vector/event1.cpp
   :language: C++
   :start-after: //!code-start
   :end-before: //!code-end

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
thing to keep in mind is that since these two handlers respond to events related
to element blocks which are owned by non-empty blocks in the primary array,
and empty blocks don't store any element block instances, creations or deletions
of empty blocks don't trigger these event handlers.

The trait also allows you to configure other behaviors of :cpp:type:`~mdds::multi_type_vector`.
Refer to :cpp:type:`mdds::mtv::default_traits` for all available parameters.
