.. SPDX-FileCopyrightText: 2025 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

Debugging
=========

Tracing of public methods
-------------------------

When using :cpp:class:`~mdds::mtv::soa::multi_type_vector` to handle a series
of data reads and writes in an non-trivial code base, sometimes you may find
yourself needing to track which methods are getting called when following a
certain code path during a debugging session.  In such a situation, you can enable
an optional trace method which gets called whenever a public method of :cpp:class:`~mdds::mtv::soa::multi_type_vector`
is called.

First, you need to define a preprocessor macro named
``MDDS_MULTI_TYPE_VECTOR_TRACE`` before including the header for
:cpp:class:`~mdds::mtv::soa::multi_type_vector`:

.. literalinclude:: ../../example/multi_type_vector/debug_trace.cpp
   :language: C++
   :start-after: //!code-start: header
   :end-before: //!code-end: header

to enable additional call-tracinng code.  In this example the value of
the macro is set to 1, but it doesn't matter what the value of the macro
is as long as it is defined.  You can also define one as a compiler
option as well.

Once defined, the next step is to add a ``trace`` method as a static function to
the trait type you pass as a template argument of multi_type_vector:

.. literalinclude:: ../../example/multi_type_vector/debug_trace.cpp
   :language: C++
   :start-after: //!code-start: types
   :end-before: //!code-end: types

Here, we are simply inheriting our trait type from the
:cpp:class:`~mdds::mtv::default_traits` type and simply adding a static ``trace``
function to it, and passing this trait type to the mtv_type definition below.
This trace function must take one argument of type
:cpp:class:`mdds::mtv::trace_method_properties_t` which includes various
properties of the traced call.  In this example, we are simply printing the
properties named
:cpp:member:`~mdds::mtv::trace_method_properties_t::function_name` and
:cpp:member:`~mdds::mtv::trace_method_properties_t::function_args` each time a
traced method is called.  Both of these properties are printable string types.

Note that this ``trace`` function is entirely optional; the code will compile
fine even when it's not defined.  Also, it must be declared as static for it to
be called.

Let's instantiate an object of ``mtv_type``, call some of its methods and see
what happens.  When executing the following code:

.. literalinclude:: ../../example/multi_type_vector/debug_trace.cpp
   :language: C++
   :start-after: //!code-start: main
   :end-before: //!code-end: main
   :dedent: 4

You will see the following output:

.. code-block:: text

    function:
      name: multi_type_vector
      args: init_size=10
    function:
      name: set
      args: pos=0; value=? (type=5)
    function:
      name: set
      args: pos=2; value=? (type=1)
    function:
      name: set
      args: pos=4; value=? (type=3)
    function:
      name: ~multi_type_vector
      args:

The :cpp:member:`~mdds::mtv::trace_method_properties_t::function_name`
property is hopefully self-explanatory.  The
:cpp:member:`~mdds::mtv::trace_method_properties_t::function_args` property is
a single string value containing the information about the function's
arguments and optionally their values if their values are known to be
printable.  If the value of an argument cannot be printed, ``?`` is placed
instead.  For some argument types, an additional information is displayed e.g.
``(type=5)`` in the above output which indicates that the type of the value
being passed to the function is :cpp:var:`~mdds::mtv::element_type_int32`.

If you want to limit your tracing to a specific function type or types, you
can make use of the :cpp:member:`~mdds::mtv::trace_method_properties_t::type`
property which specifies the type of the traced method.  Likewise, if you want
to only trace methods of a certain instance, use
:cpp:member:`~mdds::mtv::trace_method_properties_t::instance` to filter the
incoming trace calls based on the memory addresses of the instances whose
methods are being traced.

Note that this feature is available for version 2.0.2 and newer, and currently
only available for the SoA variant of :cpp:class:`~mdds::mtv::soa::multi_type_vector`.

.. note::

   This feature is only available for version 2.0.2 and newer, and only for the
   SoA variant.
