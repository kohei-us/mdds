.. _custom-value-types:

Specifying custom types in element blocks
=========================================

There are times when you need to store a set of user-defined types in :cpp:type:`~mdds::multi_type_vector`.
That is what we are going to talk about in this section.

First, let's include the header:

.. literalinclude:: ../../../example/multi_type_vector/custom_value_types.cpp
   :language: C++
   :start-after: //!code-start: header
   :end-before: //!code-end: header

then proceed to define some constant values to use as element types. We are going
to define three custom value types, so we need three element types defined:

.. literalinclude:: ../../../example/multi_type_vector/custom_value_types.cpp
   :language: C++
   :start-after: //!code-start: element-types
   :end-before: //!code-end: element-types

Here, you need to ensure that the values used will not collide with the values
that may be used for the standard value types.  The best way to ensure that is
to assign the values that are greater than or equal to :cpp:var:`~mdds::mtv::element_type_user_start`
as the code above does.  Values less than :cpp:var:`~mdds::mtv::element_type_user_start`
are reserved for use either for the standard value types or any other internal uses
in the future.

Now, let's define the first two custom value types, and their respective block types:

.. literalinclude:: ../../../example/multi_type_vector/custom_value_types.cpp
   :language: C++
   :start-after: //!code-start: custom-values
   :end-before: //!code-end: custom-values

Here, we are using the :cpp:type:`~mdds::mtv::default_element_block` as the basis
to define their block types.  At minimum, you need to specify the element type constant
and the value type as its template arguments.  There is a third optional template
argument you can specify which will become the underlying storage type.  By
default, :cpp:class:`~mdds::mtv::delayed_delete_vector` is used when the third
argument is not given.  But you can specify other types such as :cpp:type:`std::vector`
or :cpp:type:`std::deque` instead, or any other types that have similar interfaces
to :cpp:type:`std::vector`.

Once the block types are defined, it's time to define callback functions for them.
This should be as simple as using the :c:macro:`MDDS_MTV_DEFINE_ELEMENT_CALLBACKS` with
all necessary parameters provided:

.. literalinclude:: ../../../example/multi_type_vector/custom_value_types.cpp
   :language: C++
   :start-after: //!code-start: custom-values-macro
   :end-before: //!code-end: custom-values-macro

Our third type is defined in a namespace ``ns``, and its associated block type is
also defined in the same namespace.  One thing to keep in mind is that, when the
custom type is defined in a namespace, its callback functions must also be defined
in the same namespace in order for them to be discovered per argument dependent lookup
during overload resolution.  This means that you must place the macro that defines
the callback functions in the same namespace as the namespace that encompasses the
original value type:

.. literalinclude:: ../../../example/multi_type_vector/custom_value_types.cpp
   :language: C++
   :start-after: //!code-start: custom-value-ns
   :end-before: //!code-end: custom-value-ns

.. warning::

   If the original value type is defined inside a namespace, its associated callback
   functions must also be defined in the same namespace, due to the way argument
   dependent lookup works during overload resolution.

The next step is to define a trait type that specifies these block types.  The
easiest way is to have your trait inherit from :cpp:struct:`mdds::mtv::default_traits`
and overwrite the :cpp:type:`~mdds::mtv::default_traits::block_funcs` static member
type with an instance of :cpp:type:`mdds::mtv::element_block_funcs` with one or
more block types specified as its template arguments:

.. literalinclude:: ../../../example/multi_type_vector/custom_value_types.cpp
   :language: C++
   :start-after: //!code-start: trait
   :end-before: //!code-end: trait

Now we are ready to define the final :cpp:class:`~mdds::mtv::multi_type_vector` type
with the trait we just defined:

.. literalinclude:: ../../../example/multi_type_vector/custom_value_types.cpp
   :language: C++
   :start-after: //!code-start: mtv-type
   :end-before: //!code-end: mtv-type

And that's it!  With this in place, you can write a code like the following:

.. literalinclude:: ../../../example/multi_type_vector/custom_value_types.cpp
   :language: C++
   :start-after: //!code-start: main
   :end-before: //!code-end: main
   :dedent: 4

to put some values of the custom types into your container and accessing them.  This
code should generate the following output:

.. code-block:: none

   is this custom_value1? 1
   is this custom_value2? 1
   is this ns::custom_value3? 1
