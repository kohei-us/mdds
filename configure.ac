AC_INIT(mdds, 0.10.1, kohei.yoshida@gmail.com)

VERSION=AC_PACKAGE_VERSION
AC_SUBST(VERSION)

PACKAGE_TARNAME=AC_PACKAGE_TARNAME
AC_SUBST(PACKAGE_TARNAME)

AC_ARG_WITH(hash-container,
[
  --with-hash-container   specify the type of hash containers to use in test
                          programs.  Supported types are: boost, c++0x, or
                          stlport.  c++0x is used if unspecified.

                          With c++0x, we assume that unordered_map and
                          unordered_set are available under the std namespace,
                          whereas with boost they are assumed to be defined in
                          the boost namespace.  When stlport is specified, we
                          assume that hash_map and hash_set are available
                          under the std namespace.

                          Example:  --with-hash-container=boost],
,with_hash_container=c++0x)

AC_ARG_ENABLE(debug-stdcxx,
[
  --enable-debug-stdcxx   Enable libstdc++ debug mode during make check.
]
,debug_stdcxx=yes)

AC_MSG_CHECKING([hash container type])
AC_MSG_RESULT([$with_hash_container])

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

CPPFLAGS_NODEBUG="-Wall -Os -g -pedantic-errors"
if test $with_hash_container = stlport; then
  CPPFLAGS_NODEBUG="$CPPFLAGS_NODEBUG -DMDDS_HASH_CONTAINER_STLPORT"
elif test $with_hash_container = boost; then
  CPPFLAGS_NODEBUG="$CPPFLAGS_NODEBUG -DMDDS_HASH_CONTAINER_BOOST"
fi

CPPFLAGS="$CPPFLAGS_NODEBUG -DMDDS_DEBUG_NODE_BASE -DMDDS_UNIT_TEST -std=c++0x"

if test "$debug_stdcxx" = "yes"; then
  CPPFLAGS="$CPPFLAGS -D_GLIBCXX_DEBUG"
fi

AC_SUBST(CPPFLAGS)
AC_SUBST(CPPFLAGS_NODEBUG)

AC_OUTPUT(Makefile)
AC_OUTPUT(example/Makefile)
AC_OUTPUT(misc/mdds.pc)
AC_OUTPUT(VERSION)
