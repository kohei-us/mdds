runtime-env
===========

Build
-----

You can build this tool in one of two ways, as illustrated below.

### Autotools

First, run `./autogen.sh` at the project root directory while passing your desired
compiler flags via `CXXFLAGS` environment variable, like below:

```
CXXFLAGS="-O3 -march=native" ./autogen.sh
```
In this example we are using `gcc`, and by specifying `-march=native` it uses
the architecture type in the current environment as its target.  Then change
directory into `tools/runtime-env`, and run `make runtime-env` to build the program:

```
cd tools/runtime-env
make runtime-env
```

