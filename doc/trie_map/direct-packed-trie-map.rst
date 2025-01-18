
Using packed_trie_map directly
==============================

In the previous example, we showed a way to create an instance of :cpp:class:`~mdds::packed_trie_map`
from a populated instance of :cpp:class:`~mdds::trie_map`.  There is also a way
to instantiate and populate an instance of :cpp:class:`~mdds::packed_trie_map`
directly, and that is what we will cover in this section.

First, declare the type:

.. literalinclude:: ../../example/packed_trie_map.cpp
   :language: C++
   :start-after: //!code-start: type
   :end-before: //!code-end: type
   :dedent: 4

Once again, we are using ``std::string`` as its key, and ``int`` as its value
type.  The next step is to prepare its entries ahead of time:

.. literalinclude:: ../../example/packed_trie_map.cpp
   :language: C++
   :start-after: //!code-start: populate
   :end-before: //!code-end: populate
   :dedent: 4

We need to do this since :cpp:class:`~mdds::packed_trie_map` is immutable, and
the only time we can populate its content is at instantiation time.  Here, we
are using the :c:macro:`MDDS_ASCII` macro to expand a string literal to its
pointer value and size.  Note that you need to ensure that *the entries are sorted
by the key in ascending order.*

.. warning::

   When instantiating :cpp:class:`~mdds::packed_trie_map` directly with a static
   set of entries, the entries must be sorted by the key in ascending order.

You can then pass this list of entries to construct the instance:

.. literalinclude:: ../../example/packed_trie_map.cpp
   :language: C++
   :start-after: //!code-start: inst
   :end-before: //!code-end: inst
   :dedent: 4

Once it's instantiated, the rest of the example for performing searches will be
the same as in the previous section, which we will not repeat here.

The complete source code for the example in this section is available
`here <https://gitlab.com/mdds/mdds/-/blob/master/example/packed_trie_map.cpp>`__.
