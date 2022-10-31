.. highlight:: cpp

API Reference
=============

Core
----

mdds::multi_type_vector
^^^^^^^^^^^^^^^^^^^^^^^

.. doxygentypedef:: mdds::multi_type_vector

mdds::mtv::soa::multi_type_vector
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenclass:: mdds::mtv::soa::multi_type_vector
   :members:

mdds::mtv::aos::multi_type_vector
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenclass:: mdds::mtv::aos::multi_type_vector
   :members:

mdds::mtv::empty_event_func
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenstruct:: mdds::mtv::empty_event_func
   :members:

mdds::mtv::default_traits
^^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenstruct:: mdds::mtv::default_traits
   :members:

Element Stores
--------------

.. doxygenclass:: mdds::mtv::delayed_delete_vector
   :members:

Element Blocks
--------------

.. doxygenclass:: mdds::mtv::base_element_block
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

.. doxygenstruct:: mdds::mtv::element_block_funcs
   :members:


Types
-----

mdds::mtv::element_t
^^^^^^^^^^^^^^^^^^^^

.. doxygentypedef:: mdds::mtv::element_t

.. doxygenvariable:: mdds::mtv::element_type_empty
.. doxygenvariable:: mdds::mtv::element_type_reserved_start
.. doxygenvariable:: mdds::mtv::element_type_reserved_end
.. doxygenvariable:: mdds::mtv::element_type_user_start

mdds::mtv::lu_factor_t
^^^^^^^^^^^^^^^^^^^^^^

.. doxygenenum:: mdds::mtv::lu_factor_t

mdds::mtv::trace_method_t
^^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenenum:: mdds::mtv::trace_method_t

mdds::mtv::trace_method_properties_t
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenstruct:: mdds::mtv::trace_method_properties_t


Standard Element Blocks
-----------------------

The following types are automatically defined by default when including one of
the following headers:

* ``mdds/multi_type_vector.hpp``
* ``mdds/multi_type_vector/aos/main.hpp``
* ``mdds/multi_type_vector/soa/main.hpp``

To disable automatic definitions of these standard element block types, you must
define the :c:macro:`MDDS_MTV_USE_STANDARD_ELEMENT_BLOCKS` macro and set its value
to 0.  Refer to the :ref:`custom-value-types-custom-store` section for more details
on when you may want to disable these block types.

Constants
^^^^^^^^^

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

Block Types and Traits
^^^^^^^^^^^^^^^^^^^^^^

.. doxygentypedef:: mdds::mtv::boolean_element_block
.. doxygentypedef:: mdds::mtv::int8_element_block
.. doxygentypedef:: mdds::mtv::uint8_element_block
.. doxygentypedef:: mdds::mtv::int16_element_block
.. doxygentypedef:: mdds::mtv::uint16_element_block
.. doxygentypedef:: mdds::mtv::int32_element_block
.. doxygentypedef:: mdds::mtv::uint32_element_block
.. doxygentypedef:: mdds::mtv::int64_element_block
.. doxygentypedef:: mdds::mtv::uint64_element_block
.. doxygentypedef:: mdds::mtv::float_element_block
.. doxygentypedef:: mdds::mtv::double_element_block
.. doxygentypedef:: mdds::mtv::string_element_block

.. doxygenstruct:: mdds::mtv::standard_element_blocks_traits
   :members:


Exceptions
----------

.. doxygenclass:: mdds::mtv::element_block_error


Macros
------

.. doxygendefine:: MDDS_MTV_DEFINE_ELEMENT_CALLBACKS
.. doxygendefine:: MDDS_MTV_DEFINE_ELEMENT_CALLBACKS_PTR


Collection
----------

.. doxygenclass:: mdds::mtv::collection
   :members:
