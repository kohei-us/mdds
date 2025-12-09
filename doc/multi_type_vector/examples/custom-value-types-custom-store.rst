.. SPDX-FileCopyrightText: 2025 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

.. _custom-value-types-custom-store:

Specifying different storage type
=================================

By default, :cpp:class:`mdds::mtv::default_element_block` uses :cpp:class:`mdds::mtv::delayed_delete_vector`
as its underlying storage type to store its elements starting with version 2.1.  Prior to
2.1, :cpp:class:`std::vector` was used as the only storage type of choice.  If you use 2.1 or newer
versions of the library, you can specify your own storage type as the third template argument to
:cpp:class:`mdds::mtv::default_element_block`.

Let's tweak the previous example to specify :cpp:class:`std::vector` and :cpp:class:`std::deque` as the
storage types for ``custom_value1_block`` and ``custom_value2_block``, respectively:

.. literalinclude:: ../../../example/multi_type_vector/custom_value_types_custom_store.cpp
   :language: C++
   :start-after: //!code-start: blocks-1-2
   :end-before: //!code-end: blocks-1-2

For ``custom_value3_block``, we will leave it as the default storage type, namely,
:cpp:class:`mdds::mtv::delayed_delete_vector`:

.. literalinclude:: ../../../example/multi_type_vector/custom_value_types_custom_store.cpp
   :language: C++
   :start-after: //!code-start: block-3
   :end-before: //!code-end: block-3

You can specify different storage types for different block types as you can see above. But unless you have a
good reason to do so, you may want to stick with the same storage type for all of your blocks in order to have
consistent performance characteristics.

With this now in place, let's run the following code:

.. literalinclude:: ../../../example/multi_type_vector/custom_value_types_custom_store.cpp
   :language: C++
   :start-after: //!code-start: main
   :end-before: //!code-end: main
   :dedent: 4

which should generate the following output:

.. code-block:: none

   custom_value1 stored in std::vector? 1
   custom_value2 stored in std::deque? 1
   ns::custom_value3 stored in delayed_delete_vector? 1

One thing to note is that, in order for a class to be usable as the storage type for
:cpp:class:`~mdds::mtv::default_element_block`, it must be a template class
with two parameters: the first one being the value type while the second one
is the allocator type just like how :cpp:class:`std::vector` or :cpp:class:`std::deque`
are defined.
