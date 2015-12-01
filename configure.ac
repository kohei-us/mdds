AC_INIT(mdds, 1.0.0, kohei.yoshida@gmail.com)
AM_INIT_AUTOMAKE([foreign dist-bzip2 subdir-objects])
AM_MAINTAINER_MODE

VERSION=AC_PACKAGE_VERSION
API_VERSION=1.0

AC_SUBST(VERSION)
AC_SUBST(API_VERSION)

PACKAGE_TARNAME=AC_PACKAGE_TARNAME
AC_SUBST(PACKAGE_TARNAME)

AC_ARG_ENABLE(debug-stdcxx,
[
  --enable-debug-stdcxx   Enable libstdc++ debug mode during make check.
]
,debug_stdcxx=yes)

OBJDIR=obj
SRCDIR=src
INCDIR=include
MISCDIR=misc
QUICKCHECKDIR=quickcheck
AC_SUBST(OBJDIR)
AC_SUBST(SRCDIR)
AC_SUBST(INCDIR)
AC_SUBST(MISCDIR)
AC_SUBST(QUICKCHECKDIR)

CPPFLAGS="$CPPFLAGS -Wall -Os -g -pedantic-errors -DMDDS_DEBUG_NODE_BASE -DMDDS_UNIT_TEST -std=c++11"

if test "$debug_stdcxx" = "yes"; then
  CPPFLAGS="$CPPFLAGS -D_GLIBCXX_DEBUG"
fi

AC_SUBST(CPPFLAGS)
AC_SUBST(CPPFLAGS_NODEBUG)

AC_PROG_CXX

AC_ARG_ENABLE([memory-tests],
  [AS_HELP_STRING([--enable-memory-tests], [Run memory tests.])],
  [enable_memory_tests=$enableval],
  [enable_memory_tests=no]
)
AS_IF([test "$enable_memory_tests" = yes],
  [AC_PATH_PROG([VALGRIND], [valgrind])]
  AS_IF([test -z "$VALGRIND"], [AC_MSG_ERROR([valgrind not found])])
)
AM_CONDITIONAL([RUN_MEMORY_TESTS], [test "$enable_memory_tests" = yes])

AC_CONFIG_FILES([
Makefile
example/Makefile
flat_segment_tree_test_mem.mem:src/test.mem.in
misc/mdds-$API_VERSION.pc:misc/mdds.pc.in
multi_type_matrix_test_mem.mem:src/test.mem.in
multi_type_vector_test_custom_mem.mem:src/test.mem.in
multi_type_vector_test_default_mem.mem:src/test.mem.in
point_quad_tree_test_mem.mem:src/test.mem.in
rectangle_set_test_mem.mem:src/test.mem.in
segment_tree_test_mem.mem:src/test.mem.in
trie_map_test_mem.mem:src/test.mem.in
VERSION
])
AC_OUTPUT
