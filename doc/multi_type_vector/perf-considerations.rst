Performance Considerations
==========================

Select SoA or AoS storage types
-------------------------------

If you instantiate a multi_type_vector instance via
:cpp:type:`mdds::multi_type_vector`, which is an alias type for
:cpp:class:`mdds::mtv::soa::multi_type_vector`, you will be using the
structure-of-arrays (SoA) variant of its implementation which is new in 2.0.
Prior to 2.0, multi_type_vector used the array-of-structures (AoS) layout which
is still available post 2.0 via :cpp:class:`mdds::mtv::aos::multi_type_vector`
in case you need it.

Note, however, that the SoA variant generally yields better overall performance
since it can make more efficient use of CPU caches.  It is therefore highly
recommended that you stick with the SoA variant unless you have a specific
reason not to.

Also note that both variants are API compatibile with each other.


Use of position hints to avoid the cost of block position lookup
----------------------------------------------------------------

Consider the following example code:

.. literalinclude:: ../../example/multi_type_vector/pos_hint.cpp
   :language: C++
   :start-after: //!code-start: no-pos-hint
   :end-before: //!code-end: no-pos-hint
   :dedent: 4

which, when executed, may take quite sometime to complete especially when you
are using an older version of mdds.  This particular example exposes one
weakness that multi_type_vector has; because it needs to first look up the
position of the block to operate with, and that lookup *always* starts from the
first block, the time it takes to find the correct block increases as the number
of blocks goes up.  This example demonstrates the worst case scenario of such
lookup complexity since it always inserts the next value at the last block
position.

Fortunately, there is a simple solution to this which the following code
demonstrates:

.. literalinclude:: ../../example/multi_type_vector/pos_hint.cpp
   :language: C++
   :start-after: //!code-start: pos-hint
   :end-before: //!code-end: pos-hint
   :dedent: 4

Compiling and executing this code should take only a fraction of a second.

The only difference between the second example and the first one is that the
second one uses an interator as a position hint to keep track of the position of
the last modified block.  Each
:cpp:func:`~mdds::mtv::soa::multi_type_vector::set` method call returns an
iterator which can then be passed to the next
:cpp:func:`~mdds::mtv::soa::multi_type_vector::set` call as the position hint.
Because an iterator object internally stores the location of the block the value
was inserted to, this lets the method to start the block position lookup process
from the last modified block, which in this example is always one block behind
the one the new value needs to go.  Using the big-O notation, the use of the
position hint essentially turns the complexity of O(n^2) in the first example
into O(1) in the second one if you are using an older version of mdds where the
block position lookup had a linear complexity.

This strategy should work with any methods in :cpp:type:`~mdds::multi_type_vector`
that take a position hint as the first argument.

Note that, if you are using a more recent version of mdds (1.6.0 or newer), the
cost of block position lookup is significantly lessoned thanks to the switch to
binary search in performing the lookup.

.. note::

   If you are using mdds 1.6.0 or newer, the cost of block position lookup is
   much less significant even without the use of position hints. But the benefit
   of using position hints may still be there.  It's always a good idea to profile
   your specific use case and decide whether the use of position hints is worth
   it.

One important thing to note is that, as a user, you must ensure that the position
hint you pass stays valid between the calls.  A position hint becomes invalid when
the content of the container changes.  A good strategy to maintain a valid position
hint is to always receive the iterator returned from the mutator method you called
to which you passed the previous position hint, which is what the code above does.
Passing an invalid position hint to a method that takes one may result in invalid
memory access or otherwise in some sort of undefined behavior.

.. warning::

   You must ensure that the position hint you pass stays valid. Passing an invalid
   position hint to a method that takes one may result in invalid memory access
   or otherwise in some sort of undefined behavior.


Block shifting performance and loop-unrolling factor
----------------------------------------------------

The introduction of binary search in the block position lookup implementation
in version 1.6 has significantly improved its lookup performance, but has
also resulted in slight performance hit when shifting blocks during value
insertion.  This is because when shifting the logical positions of the blocks
below the insertion point, their head positions need to be re-calculated to
account for their new positions.

The good news is that the switch to the structure-of-arrays (SoA) storage
layout in 2.0 alone may bring subtle but measurable improvement in the
block position adjustment performance due to the logical block positions now
being stored in a separate array thereby improving its cache efficiency.  In
reality, however, this was somewhat dependent on the CPU types since some CPU's
didn't show any noticeable improvements or even showed worse performance, while
other CPU types showed consistent improvements with SoA over AoS.

Another factor that may play a role is `loop unrolling <https://en.wikipedia.org/wiki/Loop_unrolling>`_
factor which can be configured via the :cpp:var:`~mdds::mtv::default_traits::loop_unrolling`
variable in your custom trait type if you use version 2.0 or newer.  This variable
is an enum class of type :cpp:type:`mdds::mtv::lu_factor_t` which enumerates
several pre-defined loop-unrolling factors as well as some SIMD features.

The hardest part is to figure out which loop unrolling factor is the best option
in your runtime environment, since it is highly dependent on the environment.
Luckily mdds comes with a tool called `runtime-env <https://gitlab.com/mdds/mdds/-/tree/master/tools/runtime-env>`_
which, when run, will perform some benchmarks and give you the best loop-unrolling
factor in your runtime environment.  Be sure to build this tool with the same
compiler and compiler flags as your target program in order for this tool to give
you a representative answer.
