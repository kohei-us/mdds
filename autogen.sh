#!/bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

olddir=`pwd`
cd $srcdir

aclocal --version > /dev/null 2> /dev/null || {
    echo "error: aclocal not found"
    exit 1
}
automake --version > /dev/null 2> /dev/null || {
    echo "error: automake not found"
    exit 1
}

amcheck=`automake --version | grep 'automake (GNU automake) 1.5'`
if test "x$amcheck" = "xautomake (GNU automake) 1.5"; then
    echo "warning: you appear to be using automake 1.5"
    echo "         this version has a bug - GNUmakefile.am dependencies are not generated"
fi

rm -rf autom4te*.cache

aclocal $ACLOCAL_FLAGS || {
    echo "error: aclocal $ACLOCAL_FLAGS failed"
    exit 1
}
automake -a -c --foreign || {
    echo "warning: automake failed"
}
autoconf || {
    echo "error: autoconf failed"
    exit 1
}

if test x$NOCONFIGURE = x; then
    ./configure $@
fi
