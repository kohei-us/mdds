#!/bin/sh
set -u
cmd=`echo "$1" | sed -e s/_mem\\.mem\$/$EXEEXT/`
shift
$VALGRIND --tool=memcheck --leak-check=full --error-exitcode=42 $VALGRINDFLAGS $cmd "$@" func
