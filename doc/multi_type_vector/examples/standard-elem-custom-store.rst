
Different storage types in standard element blocks
==================================================

Now, what if you need to specify different storage types in the blocks already defined for the
standard value types, given that, as explained in the :ref:`standard-element-blocks` section,
those standard element blocks are automagically defined?

The answer is that it is possible to do such a thing, but it will require that you follow a certain
set of steps, as outlined below:

First, manually define the element type constants, block types, and their respecitve callback functions
for the standard value types you need to use as if they were user-defined types. When doing so, specify
the non-default storage types you need to use for these blocks.

Include the header for the multi_type_vector definition with the special macro value named
:c:macro:`MDDS_MTV_USE_STANDARD_ELEMENT_BLOCKS` defined and its value is set to 0.  This bypasses the
automatic inclusion of the block types for the standard value types when this header is included.

Lastly, define a custom trait type and overwrite the ``block_funcs`` member type to specify the block types
defined in the first step.  This is essentially the same step you would take when you define custom
block types for user-defined value types.

Let's do this step-by-step.  First, include the necessary headers:

.. literalinclude:: ../../../example/multi_type_vector/standard_custom_store.cpp
   :language: C++
   :start-after: //!code-start: header
   :end-before: //!code-end: header

The ``types.hpp`` header is required for the :cpp:type:`~mdds::mtv::element_t` and
:cpp:class:`~mdds::mtv::default_element_block`, and the ``macro.hpp`` header is required
for the :c:macro:`MDDS_MTV_DEFINE_ELEMENT_CALLBACKS` macro.  The ``<deque>`` header is so
that we can use :cpp:class:`std::deque` as storage types in our block types.

Next, let's define the element and block types as well as their callback functions:

.. literalinclude:: ../../../example/multi_type_vector/standard_custom_store.cpp
   :language: C++
   :start-after: //!code-start: block-defs
   :end-before: //!code-end: block-defs

This is very similar to how it is done in the :ref:`custom-value-types-custom-store` section.
The only difference is that, this part needs to happen *before* the header for the
multi_type_vector type gets included, in order for the multi_type_vector implementation code
to reference the callback functions now that the callback functions for the standard value
types will no longer be included.

Let's proceed to include the multi_type_vector header:

.. literalinclude:: ../../../example/multi_type_vector/standard_custom_store.cpp
   :language: C++
   :start-after: //!code-start: mtv-header
   :end-before: //!code-end: mtv-header

Here, we define the :c:macro:`MDDS_MTV_USE_STANDARD_ELEMENT_BLOCKS` macro and set its value to
0, to skip the inclusion of the standard element blocks.  It is also worth noting that we
are including the ``mdds/multi_type_vector/soa/main.hpp`` header directly instead of
``mdds/multi_type_vector.hpp``, which indirectly includes the first header.

Lastly, let's define the trait type to specify the block types to use, and instantiate the final
multi_type_vector type:

.. literalinclude:: ../../../example/multi_type_vector/standard_custom_store.cpp
   :language: C++
   :start-after: //!code-start: mtv-def
   :end-before: //!code-end: mtv-def

Now that the concrete multi_type_vector is defined, we can use it to store some values of the
specified types:

.. literalinclude:: ../../../example/multi_type_vector/standard_custom_store.cpp
   :language: C++
   :start-after: //!code-start: main
   :end-before: //!code-end: main
   :dedent: 4

If you inspect the storage types of the element blocks like the following:

.. literalinclude:: ../../../example/multi_type_vector/standard_custom_store.cpp
   :language: C++
   :start-after: //!code-start: main-block-type
   :end-before: //!code-end: main-block-type
   :dedent: 4

you should see the following output:

.. code-block:: none

   my_double_block: is std::deque its store type? 1
   my_int32_block: is std::deque its store type? 1

which indicates that they are indeed :cpp:class:`std::deque`.
