runtime-env
===========

This tools performs some benchmarks to determine the optimal loop-unrolling factor
combined with SIMD features (if available) for `mdds::multi_type_vector`.

Build
-----

You can build this tool in one of two ways, as illustrated below.

### Autotools

First, run `./autogen.sh` at the project root directory while passing your desired
compiler flags via `CXXFLAGS` environment variable, like below:

```
CXXFLAGS="-O3 -march=native -DNDEBUG" ./autogen.sh
```
In this example we are using `gcc`, and by specifying `-march=native` it uses
the architecture type in the current environment as its target.  Then change
directory into `tools/runtime-env`, and run `make runtime-env` to build the program:

```
cd tools/runtime-env
make runtime-env
```

### CMake

To specify custom compiler flags in cmake, run with `cmake -E env CXXFLAGS="..."`
followed by the usual cmake configuration command.  Here is an example of specifying
`-O3 -march=native -DNDEBUG` as the compiler flags to cmake:

```
mkdir build
cd build
cmake -E env CXXFLAGS="-O3 -march=native -DNDEBUG" cmake ..
```

Once the build configuration step is finished, build the tool by

```
cmake --build . --target runtime-env
```
or simply

```
make runtime-env
```

if you use GNU Make as your backend.  The program will be built at `./tools/runtime-env/runtime-env`.

Run it
------

To run the tool, simply execute it with no arguments.  It should show you a progress
bar indicating that it's performing benchmarks.  Once finished, it should display an
ASCII chart similar to the following:

```
 Category      | Average duration (seconds)
---------------+-------------------------------------------------------->
(SoA, 16)      | ooooooooooooo 0.04123
(SoA, 32)      | oooooooooooooo 0.04409
(SoA, 08)      | oooooooooooooooo 0.05151
(AoS, 08)      | ++++++++++++++++++ 0.05712
(AoS, 16)      | ++++++++++++++++++ 0.05799
(AoS, 04)      | ++++++++++++++++++ 0.0584
(SoA, sse2+16) | ooooooooooooooooooo 0.06204
(AoS, 32)      | ++++++++++++++++++++ 0.06285
(SoA, sse2+08) | oooooooooooooooooooo 0.06412
(SoA, sse2+04) | ooooooooooooooooooooo 0.0673
(SoA, 00)      | ooooooooooooooooooooo 0.06833
(SoA, 04)      | ooooooooooooooooooooooo 0.07281
(SoA, sse2+00) | ooooooooooooooooooooooooooooo 0.09283
(AoS, 00)      | +++++++++++++++++++++++++++++++++++++++++++++++++++++++ 0.1712

Storage of SoA with the LU factor of 16 appears to be the best choice
in this environment.
```

What the chart shows are the average durations for some artificially created
block position adjustment tasks, and fewer the number the better.  The results
are sorted in ascending order, so the item at the very top is considered to be
the most optimum choice in the current run-time environment.

The program also generates the following files:

* graph-output.txt
* raw-data.csv

`graph-output.txt` simply contains the same ascii graph that was also output
to the terminal.  `raw-data.csv` contains the raw benchmarks results which can
be useful for further analyses.
