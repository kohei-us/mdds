
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
`breathe` and `cloud_sptheme` sphinx extensions.  Make sure to install all of
these packages if you wish to build documentation manually.

Once you have all required packages installed, run:

```bash
./autogen.sh --enable-docs
make html-local
```

to build it, and the documentation will be available in the `doc/_build`
directory.
