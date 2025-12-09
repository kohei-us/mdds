#!/bin/sh

# SPDX-FileCopyrightText: 2025 Kohei Yoshida
#
# SPDX-License-Identifier: MIT

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

olddir=`pwd`
cd $srcdir

autoreconf -i || {
    echo "error: autoreconf failed"
    exit 1
}

if test x$NOCONFIGURE = x; then
    ./configure $@
fi
