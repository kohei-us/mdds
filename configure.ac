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

AC_CONFIG_FILES([
  Makefile
  example/Makefile
  misc/mdds-$API_VERSION.pc:misc/mdds.pc.in
  VERSION
])
AC_OUTPUT
