
Execution Policy
================

C++17 has introduced the concept of execution policies that affect how
`the standard library algorithms <https://en.cppreference.com/w/cpp/algorithm.html>`_
are executed.  As some parts of the `multi_type_vector` implementation
use the standard library algorithms, you can specify a custom execution
policy to be used in those parts in order to potentially improve the
run-time performance of those operations.

Currently execution policies are supported only in the SoA variant, and in
the following operations:

- copy construction
- :cpp:func:`~mdds::mtv::soa::multi_type_vector::clone()`
- :cpp:func:`~mdds::mtv::soa::multi_type_vector::shrink_to_fit()`
- :cpp:func:`~mdds::mtv::soa::multi_type_vector::operator==()`
- :cpp:func:`~mdds::mtv::soa::multi_type_vector::operator!=()`

When using a parallel execution policy, for instance, these operations are
performed using multiple threads so that the individual element blocks get
processed in parallel.  This can potentially improve performance if the
multi_type_vector instance contains a sufficiently large number of element
blocks, though the actual rate of improvement will depend on how the parallel
versions of the standard algorithms are implemented.

The Microsoft C++ compiler natively supports execution policies in its
standard library algorithms.  When using gcc or clang with libstdc++, you
need to have `oneTBB <https://github.com/uxlfoundation/oneTBB>`_ installed
which incurs additional linking.  On Debian-based Linux distributions, you
can simply install `libtbb-dev <https://packages.debian.org/sid/libtbb-dev>`_
and add ``-ltbb`` to your linker flag.

Let's build on the example from the :ref:`mtv-example-cloning` section and
specify the parallel execution policy.  All you need to do is to simply add
an ``exec_policy`` type alias in your traits struct that points to the
``std::execution::parallel_policy`` type:

.. literalinclude:: ../../../example/multi_type_vector/exec_policy.cpp
   :language: C++
   :start-after: //!code-start: mtv-traits
   :end-before: //!code-end: mtv-traits

Then the aforementioned operations will apply this policy.

For the cloning operation, you can additionally specify a custom execution
policy for the cloning of the individual values within each element block.
All you need to do is to add an ``exec_policy`` type alias to the `clone_value`
specialization struct, and have it point to the desired policy type:

.. literalinclude:: ../../../example/multi_type_vector/exec_policy.cpp
   :language: C++
   :start-after: //!code-start: clone-value-ts
   :end-before: //!code-end: clone-value-ts

However, it's important to benchmark your specific use case since using the
parallel execution policy may not always yield better performance due to
the added thread management overhead.
