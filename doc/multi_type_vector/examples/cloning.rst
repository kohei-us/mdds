.. _mtv-example-cloning:

Cloning
=======
In this section we are going to illustrate the concept of cloning a
``multi_type_vector`` instance.

Cloning is probably only relevant when your ``multi_type_vector`` type
includes at least one element block whose type is either
:cpp:class:`~mdds::mtv::noncopyable_element_block` or
:cpp:class:`~mdds::mtv::noncopyable_managed_element_block`.  These
blocks don't allow copy construction; it would throw an
:cpp:class:`~mdds::mtv::element_block_error` if attempted.  When you
call the :cpp:func:`~mdds::mtv::soa::multi_type_vector::clone()` method,
however, you can create a copy of the original instance as if it was
copy-constructed provided that you implement a necessary template
specialization for cloning an element value stored in such a block.

In the example below, we are going to set up the code to allow cloning
of elements stored in an block whose type is
:cpp:class:`~mdds::mtv::noncopyable_managed_element_block`.

First, let's define a value type that we don't want to allow copying of:

.. literalinclude:: ../../../example/multi_type_vector/clone.cpp
   :language: C++
   :start-after: //!code-start: stream-store
   :end-before: //!code-end: stream-store

This fictional class stores a stream of bytes whose size can be
potentially very large that we don't want to allow copying to avoid
potential performance bottleneck.  So we'll store them as pointers in a
block of type :cpp:class:`~mdds::mtv::noncopyable_managed_element_block`:

.. literalinclude:: ../../../example/multi_type_vector/clone.cpp
   :language: C++
   :start-after: //!code-start: elem-block-def
   :end-before: //!code-end: elem-block-def

Once the element block type is defined, plug it into the traits type and
define the actual ``multi_type_vector`` type:

.. literalinclude:: ../../../example/multi_type_vector/clone.cpp
   :language: C++
   :start-after: //!code-start: mtv-def
   :end-before: //!code-end: mtv-def

We'll also define a fictional function that fetches a stream of potentially
large data from an external location:

.. literalinclude:: ../../../example/multi_type_vector/clone.cpp
   :language: C++
   :start-after: //!code-start: fetch-buffer
   :end-before: //!code-end: fetch-buffer

The content of this function is not very important, but the relevant piece is
that it returns a sequence of bytes when called.

Next, we'll define a template specialization for cloning a ``stream_store``
instance.  Since we are storing its instances as pointers, we need to
specialize for the ``stream_store*`` type:

.. literalinclude:: ../../../example/multi_type_vector/clone.cpp
   :language: C++
   :start-after: //!code-start: clone-value-ts
   :end-before: //!code-end: clone-value-ts

The specialization must be a struct, must be in the ``mdds::mtv``
namespace, and must have a public method whose signature is ``T
operator()(const T) const`` where the ``T`` is the type that it is
specialized for.

Since we have all necessary pieces defined, let's instantiate our
``multi_type_vector`` instance and populate it:

.. literalinclude:: ../../../example/multi_type_vector/clone.cpp
   :language: C++
   :start-after: //!code-start: populate-store
   :end-before: //!code-end: populate-store
   :dedent: 4

We are storing heap-allocated instances directly in the container, which
will manage their life cycles.  Let's clone this instance by calling the
:cpp:func:`~mdds::mtv::soa::multi_type_vector::clone()` method:

.. literalinclude:: ../../../example/multi_type_vector/clone.cpp
   :language: C++
   :start-after: //!code-start: clone-store
   :end-before: //!code-end: clone-store
   :dedent: 4

This will internally call the ``clone_value`` specialization we defined
earlier to perform cloning.  You will see output similar to the following
when executing this code:

.. code-block:: none

   storing a buffer of size 736421
   storing a buffer of size 273794
   storing a buffer of size 444271
   storing a buffer of size 1044677
   storing a buffer of size 492239
   cloning the container...
   storing a buffer of size 736421
   storing a buffer of size 273794
   storing a buffer of size 444271
   storing a buffer of size 1044677
   storing a buffer of size 492239
   done cloning
   disposing of the buffer of size 736421
   disposing of the buffer of size 273794
   disposing of the buffer of size 444271
   disposing of the buffer of size 1044677
   disposing of the buffer of size 492239
   disposing of the buffer of size 736421
   disposing of the buffer of size 273794
   disposing of the buffer of size 444271
   disposing of the buffer of size 1044677
   disposing of the buffer of size 492239

The output indicates that cloning did clone all stored ``stream_store``
instances, and all of the stored instances were properly disposed of
when the two ``multi_type_vector`` container instances storing them were
destroyed.
