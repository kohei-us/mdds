AC_INIT(mdds, 3.0.0, kohei.yoshida@gmail.com)
AM_INIT_AUTOMAKE([foreign dist-bzip2 dist-xz subdir-objects])
AM_MAINTAINER_MODE

VERSION=AC_PACKAGE_VERSION
API_VERSION=3.0

AC_SUBST(VERSION)
AC_SUBST(API_VERSION)
AC_CONFIG_MACRO_DIRS([m4])

PACKAGE_TARNAME=AC_PACKAGE_TARNAME
AC_SUBST(PACKAGE_TARNAME)

OBJDIR=obj
INCDIR=include
MISCDIR=misc
AC_SUBST(OBJDIR)
AC_SUBST(INCDIR)
AC_SUBST(MISCDIR)

AX_CXX_COMPILE_STDCXX_17([noext], [mandatory])

CPPFLAGS="$CPPFLAGS -Wall -Wshadow -g -pedantic-errors"
CPPFLAGS="$CPPFLAGS -I/usr/include -I/usr/local/include"

AC_CHECK_SIZEOF([void *])
CXXFLAGS="$CXXFLAGS -DSIZEOF_VOID_P=$ac_cv_sizeof_void_p"

AC_ARG_ENABLE(debug-stdcxx,
[
  --enable-debug-stdcxx   Enable libstdc++ debug mode during make check.
]
,debug_stdcxx=yes)

AS_IF([test x"$debug_stdcxx" = "xyes"], [
  CXXFLAGS="$CXXFLAGS -D_GLIBCXX_DEBUG"
])

AC_ARG_ENABLE(release-tests,
[
  --enable-release-tests   Enable release builds for unit tests.
]
,release_tests=yes)

AS_IF([test x"$release_tests" = "xyes"], [
  CXXFLAGS="$CXXFLAGS -DNDEBUG -O2"
])

AC_ARG_ENABLE(gcov,
    AS_HELP_STRING([--enable-gcov], [Enable generation of gcov information.]),
    [enable_gcov="$enableval"],[enable_gcov=no]
)

AC_ARG_ENABLE(loop-unrolling,
    AS_HELP_STRING([--disable-loop-unrolling], [Disable use of loop unrolling.]),
    [enable_loop_unrolling="$enableval"],[enable_loop_unrolling=yes]
)

AS_IF([test x"$enable_loop_unrolling" = "xno"], [
  CXXFLAGS="$CXXFLAGS -DMDDS_LOOP_UNROLLING=0"
])

AS_IF([test x"$enable_gcov" = "xyes"], [
  CXXFLAGS="$CXXFLAGS --coverage -O0"
])

AC_ARG_ENABLE(openmp,
    AS_HELP_STRING([--enable-openmp], [Enable use of OpenMP.]),
    [enable_openmp="$enableval"],[enable_openmp=no]
)

AS_IF([test x"$enable_openmp" = "xyes"], [
  CXXFLAGS="$CXXFLAGS -fopenmp -DMDDS_USE_OPENMP=1"
  LDFLAGS="$LDFLAGS -fopenmp"
])

AC_ARG_ENABLE(sanitizer-coverage,
    AS_HELP_STRING([--enable-sanitizer-coverage], [Enable generation of sanitizer coverage information.]),
    [enable_sanitizer_coverage="$enableval"],[enable_sanitizer_coverage=no]
)

AS_IF([test x"$enable_sanitizer_coverage" = "xyes"], [
  CXXFLAGS="$CXXFLAGS -fprofile-instr-generate -fcoverage-mapping -O0"
])

AC_SUBST(CPPFLAGS)
AC_SUBST(CPPFLAGS_NODEBUG)

AC_PROG_CXX
AC_PATH_PROG([GDB], [gdb])
AC_PATH_PROG([EXPECT], [expect])
AC_PATH_PROG([RUNTEST_BIN], [runtest])

AC_ARG_ENABLE([docs],
  [AS_HELP_STRING([--enable-docs], [Generate docs during build.])],
  [enable_docs=$enableval],
  [enable_docs=no]
)
AS_IF([test "$enable_docs" = yes],
  [
    AC_PATH_PROG([DOXYGEN], [doxygen])
    AS_IF([test -z "$DOXYGEN"], [AC_MSG_ERROR([doxygen not found])])
    AC_PATH_PROG([SPHINX], [sphinx-build])
    AS_IF([test -z "$SPHINX"], [AC_MSG_ERROR([sphinx-build not found])])
  ]
)
AM_CONDITIONAL([BUILD_DOCS], [test "$enable_docs" = yes])

AC_ARG_ENABLE([werror],
	[AS_HELP_STRING([--enable-werror], [Treat all warnings as errors, useful for development])],
	[enable_werror="$enableval"],
	[enable_werror=no]
)
AS_IF([test x"$enable_werror" = "xyes"], [
	CXXFLAGS="$CXXFLAGS -Werror"
])

CXXFLAGS_UNITTESTS="-DMDDS_DEBUG_NODE_BASE -DMDDS_UNIT_TEST"
AC_SUBST(CXXFLAGS_UNITTESTS)

AX_VALGRIND_CHECK()

AC_CONFIG_FILES([
Makefile
include/Makefile
include/mdds/Makefile
include/mdds/multi_type_vector/Makefile
include/mdds/multi_type_vector/aos/Makefile
include/mdds/multi_type_vector/soa/Makefile
example/Makefile
example/multi_type_vector/Makefile
test/Makefile
test/gdb/Makefile
test/gdb/src/Makefile
test/flat_segment_tree/Makefile
test/include/Makefile
test/multi_type_matrix/Makefile
test/multi_type_vector/Makefile
test/multi_type_vector/collection/Makefile
test/multi_type_vector/collection/aos/Makefile
test/multi_type_vector/collection/soa/Makefile
test/multi_type_vector/collection/tc/Makefile
test/multi_type_vector/custom/Makefile
test/multi_type_vector/custom/aos/Makefile
test/multi_type_vector/custom/soa/Makefile
test/multi_type_vector/custom/tc/Makefile
test/multi_type_vector/custom-trait/Makefile
test/multi_type_vector/custom-trait/aos/Makefile
test/multi_type_vector/custom-trait/soa/Makefile
test/multi_type_vector/debug-util/Makefile
test/multi_type_vector/debug-util/soa/Makefile
test/multi_type_vector/default/Makefile
test/multi_type_vector/default/aos/Makefile
test/multi_type_vector/default/soa/Makefile
test/multi_type_vector/default/tc/Makefile
test/multi_type_vector/element-blocks/Makefile
test/multi_type_vector/event/Makefile
test/multi_type_vector/event/aos/Makefile
test/multi_type_vector/event/soa/Makefile
test/multi_type_vector/event/tc/Makefile
test/multi_type_vector/perf/Makefile
test/multi_type_vector/push-emplace-back/Makefile
test/multi_type_vector/push-emplace-back/aos/Makefile
test/multi_type_vector/push-emplace-back/soa/Makefile
test/multi_type_vector/push-emplace-back/tc/Makefile
test/point_quad_tree/Makefile
test/rtree/Makefile
test/segment_tree/Makefile
test/sorted_string_map/Makefile
test/trie_map/Makefile
tools/Makefile
tools/runtime-env/Makefile
misc/mdds-$API_VERSION.pc:misc/mdds.pc.in
])
AC_OUTPUT

AC_MSG_NOTICE([
==============================================================================
Build configuration:
        gcov                  $enable_gcov
        openmp                $enable_openmp
        sanitizer-coverage    $enable_sanitizer_coverage
        loop-unrolling        $enable_loop_unrolling

        CXXFLAGS              $CXXFLAGS
==============================================================================
])
