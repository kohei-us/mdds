
Get raw pointer to element block array
======================================

Sometimes you need to expose a pointer to an element block array especially
when you need to pass such an array pointer to C API that requires one.  You
can do this by calling the ``data`` method of the element_block template
class.  This works since the element block internally just wraps
:cpp:class:`std::vector` or one that acts like it, such as
:cpp:class:`std::deque` or :cpp:class:`~mdds::mtv::delayed_delete_vector`, and
its ``data`` method simply exposes the internal storage types's own ``data``
method which returns the memory location of its internal buffer.

The following code demonstrates this by exposing raw array pointers to the
internal arrays of numeric and string element blocks, and printing their
element values directly from these array pointers.

.. literalinclude:: ../../../example/multi_type_vector/element_block1.cpp
   :language: C++
   :start-after: //!code-start
   :end-before: //!code-end

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
