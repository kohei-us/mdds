
# Windows

## Using CMake

Since mdds is a header-only library, you technically don't need to build
anything in order to use mdds in your project.  That said, the following
describes steps to build and run tests.

This documentation assumes that you are in MINGW environment (or Git Bash
environment which is equivalent).  If you are not using MINGW, please adjust
the commands as needed.

First, you need to have the [boost library](https://www.boost.org/) available
in your environment.  You won't necessarily need to build boost since mdds
depends only on the header part of boost.

While at the root of the source directory, run the following commands:

```bash
mkdir build
cd build
cmake .. -DBOOST_INCLUDEDIR=/path/to/boost/include/dir -DCMAKE_INSTALL_PREFIX=/path/to/install
```

to kick off the build configuration.  When it finishes without errors, run:

```bash
cmake --build . --target check
```

to build and execute the test programs.  To install mdds to the path you
specified via the `CMAKE_INSTALL_PREFIX` configure swtich, run:

```bash
cmake --build . --target install
```

to see all the header files being installed to the destination path.  Now mdds
is ready for use.


# Linux & Mac OS X

## Using Autotools

You need to use GNU Autotools to build the test binaries on Linux or Mac OS X.
The process follows a standard autotools workflow, which basically includes:

```bash
./autogen.sh --prefix=/path/to/install
make check
make install
```

The `make check` step is necessary only when you wish to build and execute the
test binaries to ensure integrity of the code.

Make sure you have the boost headers installed on your system, as mdds has
hard dependency on boost.

## Using CMake

You could in theory use CMake to run tests and perform installation, but it is
not well tested on non-Windows platforms.

# Build documentation

We use both `doxygen` and `sphinx` to author our documentation as well as
the `breathe` extension to bridge between the two.  We also use the
`sphinx-rtd-theme` as the theme for our documentation.  Make sure to install
all of these packages if you wish to build documentation manually.

Once you have all required packages installed, run:

```bash
./autogen.sh --enable-docs
make html-local
```

to build it, and the documentation will be available in the `doc/_build`
directory.

# Build LCOV report

We support generating a test coverage report using [GCOV](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html)
and [LCOV](https://github.com/linux-test-project/lcov).  To generate a report,
run:

```bash
./autogen.sh --enable-gcov
make check
```

to build code coverage data.  Then generate a summary report by running:

```bash
./bin/gen-test-coverage.sh out src/*.cpp
```

which should launch the report in your default web browser.  Note that this works
only when you use gcc to build the test source files.

# Run tests using clang++ and libc++ on Linux

It's a good idea to test against `libc++` since it sometimes uncovers hidden
bugs that don't surface when using the GNU C++ Library or Microsoft's
implementation of C++ standard library.  You can use `libc++` with or without
`clang++`, but it's perhaps easier to use `libc++` with `clang++`.

## Ubuntu LTS 20.04

To use `clang++` with `libc++`, first install the appropriate system packages
that provide the necessary files, then export the following variables:

```bash
export CC=clang
export CXX="clang++ -stdlib=libc++ -I/usr/lib/llvm-11/include/c++/v1"
```

Once that's done, simply proceed to run `./autogen.sh` as you normally do, then
running `make check` should build and run the tests using `clang++` and `libc++`.

For more detailed instructions, consult [this page](https://libcxx.llvm.org/UsingLibcxx.html).
