.. SPDX-FileCopyrightText: 2025 Kohei Yoshida
..
.. SPDX-License-Identifier: MIT

.. highlight:: cpp


Trie Maps
=========

The :cpp:class:`~mdds::trie_map` class is an associative container that stores
multiple key-value pairs similar to ``std::map``, but the key-value pairs are
stored in a `trie structure <https://en.wikipedia.org/wiki/Trie>`_ to optimize
for prefix searches.

Another data structure similar to :cpp:class:`~mdds::trie_map` is
:cpp:class:`~mdds::packed_trie_map`, which provides search API nearly identical
to that of :cpp:class:`~mdds::trie_map` but is immutable once created.  Instead,
:cpp:class:`~mdds::packed_trie_map` stores all of its content in a contiguous
memory region to improve memory locality, and also supports serialization of its
state to an external buffer.

.. toctree::
   :maxdepth: 1

   populate-trie-map.rst
   pack-trie-map.rst
   direct-packed-trie-map.rst
   save-load-packed-trie-map.rst
   save-load-packed-trie-map-custom-value.rst
   api.rst

