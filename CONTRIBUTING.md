
# Windows

## Building boost

Note that, since mdds only makes use of the header-only part of boost, the
following build process is technically not required.  But if you want to build
it anyway, follow these steps.

First, open your MINGW64 shell.  You can simply use the MINGW64 shell that
comes shipped with the Windows version of Git.  Once you are in it, change
directory to the root of the boost library directory, and run the following
command:

```bash
bootstrap.bat
mkdir -p stage/x64
./b2 --stagedir=./stage/x64 address-model=64 --build-type=complete -j 16
```

where you may change the part `-j 16` which controls how many concurrent
processes to use for the build.

## Using CMake to build the test binaries.

Since mdds is a header-only library, you technically don't need to build
anything in order to use mdds in your project.  That said, the following
describes steps to build test binaries using CMake.

While at the root of the source directory, run the following commands:

```bash
cmake -H. -Bbuild -DBOOST_BUILD_DIR=/path/to/boost/include/dir
cmake --build build
```

This will create a `build` directory along with a whole bunch of build-related
files.  You do need to specify the boost header directory to use since mdds
has hard dependency on boost.  The final executables are found in `build/Debug`.
