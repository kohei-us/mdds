#!/bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

olddir=`pwd`
cd $srcdir

automake -a -c --foreign || {
    echo "warning: automake failed"
}

autoreconf || {
    echo "error: autoreconf failed"
    exit 1
}

if test x$NOCONFIGURE = x; then
    ./configure $@
fi
